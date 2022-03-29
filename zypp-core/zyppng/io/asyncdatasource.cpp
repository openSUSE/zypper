#include "private/asyncdatasource_p.h"

#include <zypp-core/base/IOTools.h>
#include <zypp-core/zyppng/base/AutoDisconnect>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>

namespace zyppng {

  void AsyncDataSourcePrivate::notifierActivated( const SocketNotifier &notify, int evTypes )
  {
    if ( _writeNotifier.get() == &notify ) {
      if ( evTypes & SocketNotifier::Error ) {
        DBG << "Closing due to error when polling" << std::endl;
        closeWriteChannel(  AsyncDataSource::RemoteClose );
        return;
      }
      readyWrite();
    } else {

        auto dev = std::find_if( _readFds.begin(), _readFds.end(),
        [ &notify ]( const auto &dev ){ return ( dev._readNotifier.get() == &notify ); } );

        if ( dev == _readFds.end() ) {
          return;
        }

        readyRead( std::distance( _readFds.begin(), dev ) );
    }
  }

  void AsyncDataSourcePrivate::readyRead( uint channel )
  {
    auto bytesToRead = z_func()->rawBytesAvailable( channel );
    if ( bytesToRead == 0 ) {
      // make sure to check if bytes are available even if the ioctl call returns something different
      bytesToRead = 4096;
    }

    auto &_readBuf = _readChannels[channel];
    char *buf = _readBuf.reserve( bytesToRead );
    const auto bytesRead = z_func()->readData( channel, buf, bytesToRead );

    if ( bytesRead <= 0 ) {
      _readBuf.chop( bytesToRead );

      switch( bytesRead ) {
        // remote close , close the read channel
        case 0: {
          closeReadChannel( channel, AsyncDataSource::RemoteClose );
          break;
        }
        // no data is available , just try again later
        case -2: break;
        // anything else
        default:
        case -1: {
          closeReadChannel( channel, AsyncDataSource::InternalError );
          break;
        }
      }
      return;
    }

    if ( bytesToRead > bytesRead )
      _readBuf.chop( bytesToRead-bytesRead );

    if ( channel == _currentReadChannel )
      _readyRead.emit();

    _channelReadyRead.emit( channel );
    return;
  }

  void AsyncDataSourcePrivate::readyWrite()
  {
    const auto nwrite = _writeBuffer.frontSize();
    if ( !nwrite ) {
      // disable Write notifications so we do not wake up without the need to
      _writeNotifier->setEnabled( false );
      return;
    }

    const auto nBuf = _writeBuffer.front();
    const auto written = eintrSafeCall( ::write, _writeFd, nBuf, nwrite );
    if ( written == -1 ) {
      switch ( errno ) {
        case EACCES:
          closeWriteChannel( AsyncDataSource::AccessError );
          return;
        case EAGAIN:
#if EAGAIN != EWOULDBLOCK
        case EWOULDBLOCK:
#endif
          return;
        case EPIPE:
        case ECONNRESET:
          closeWriteChannel(  AsyncDataSource::RemoteClose );
          return;
        default:
          closeWriteChannel(  AsyncDataSource::InternalError );
          return;
      }
      return;
    }
    _writeBuffer.discard( written );
    _sigBytesWritten.emit( written );

    if ( _writeBuffer.size() == 0 )
      _sigAllBytesWritten.emit();
  }

  void AsyncDataSourcePrivate::closeWriteChannel( AsyncDataSource::ChannelCloseReason reason )
  {
    bool sig = _writeFd >= 0;
    _writeNotifier.reset();
    _writeFd = -1;
    _writeBuffer.clear();
    _mode.unsetFlag( AsyncDataSource::WriteOnly );
    if ( sig )
      _sigWriteFdClosed.emit( reason );
  }

  void AsyncDataSourcePrivate::closeReadChannel( uint channel, AsyncDataSource::ChannelCloseReason reason )
  {
    auto &readFd = _readFds[channel];
    // we do not clear the read buffer so code has the opportunity to read whats left in there
    bool sig = readFd._readFd >= 0;
    readFd._readNotifier.reset();
    readFd._readFd = -1;
    if ( sig )
      _sigReadFdClosed.emit( channel, reason );
  }

  ZYPP_IMPL_PRIVATE(AsyncDataSource)

  AsyncDataSource::AsyncDataSource() : IODevice( *( new AsyncDataSourcePrivate(*this) ) )
  { }

  AsyncDataSource::AsyncDataSource( AsyncDataSourcePrivate &d )
    : IODevice(d)
  {}

  AsyncDataSource::Ptr AsyncDataSource::create()
  {
    return std::shared_ptr<AsyncDataSource>( new AsyncDataSource );
  }


  bool AsyncDataSource::openFds ( std::vector<int> readFds, int writeFd )
  {
    Z_D();

    if ( d->_mode != IODevice::Closed )
      return false;

    IODevice::OpenMode mode;

    bool error = false;
    for ( const auto readFd : readFds ) {
      if ( readFd >= 0 ) {
        mode |= IODevice::ReadOnly;
        d->_readFds.push_back( {
          readFd,
          SocketNotifier::create( readFd, SocketNotifier::Read | AbstractEventSource::Error, true )
        });
        if ( zypp::io::setFDBlocking( readFd, false ) == zypp::io::BlockingMode::FailedToSetMode ) {
          ERR << "Failed to set read FD to non blocking" << std::endl;
          error = true;
          break;
        }
        d->_readFds.back()._readNotifier->connect( &SocketNotifier::sigActivated, *d, &AsyncDataSourcePrivate::notifierActivated );
      }
    }

    if ( writeFd >= 0 && !error ) {
      mode |= IODevice::WriteOnly;
      if ( zypp::io::setFDBlocking( writeFd, false ) == zypp::io::BlockingMode::FailedToSetMode ) {
        ERR << "Failed to set write FD to non blocking" << std::endl;
        error = true;
      } else {
        d->_writeFd = writeFd;
        d->_writeNotifier = SocketNotifier::create( writeFd, SocketNotifier::Write | AbstractEventSource::Error, false );
        d->_writeNotifier->connect( &SocketNotifier::sigActivated, *d, &AsyncDataSourcePrivate::notifierActivated );
      }
    }

    if( error || !IODevice::open( mode ) ) {
      d->_mode = IODevice::Closed;
      d->_readFds.clear();
      d->_writeNotifier.reset();
      d->_writeFd = -1;
      return false;
    }

    // make sure we have enough read buffers
    setReadChannelCount( d->_readFds.size() );
    return true;
  }

  int64_t zyppng::AsyncDataSource::writeData( const char *data, int64_t count )
  {
    Z_D();
    if ( count > 0 ) {
      // we always use the write buffer, to make sure the fd is actually writeable
      d->_writeBuffer.append( data, count );
      d->_writeNotifier->setEnabled( true );
    }
    return count;
  }

  int64_t zyppng::AsyncDataSource::readData( uint channel, char *buffer, int64_t bufsize )
  {
    Z_D();
    if ( channel >= d->_readFds.size() ) {
      ERR << constants::outOfRangeErrMsg << std::endl;
      throw std::logic_error( constants::outOfRangeErrMsg.data() );
    }
    const auto read = eintrSafeCall( ::read, d->_readFds[channel]._readFd, buffer, bufsize );
    if ( read < 0 ) {
      switch ( errno ) {
      #if EAGAIN != EWOULDBLOCK
              case EWOULDBLOCK:
      #endif
        case EAGAIN: {
          return -2;
        }
        default:
          break;
      }
    }
    return read;
  }

  int64_t AsyncDataSource::rawBytesAvailable( uint channel ) const
  {
    Z_D();

    if ( channel >= d->_readFds.size() ) {
      ERR << constants::outOfRangeErrMsg << std::endl;
      throw std::logic_error( constants::outOfRangeErrMsg.data() );
    }

    if ( isOpen() && canRead() )
      return zyppng::bytesAvailableOnFD( d->_readFds[channel]._readFd );
    return 0;
  }

  void AsyncDataSource::readChannelChanged ( uint channel )
  {
    Z_D();
    if ( channel >= d->_readFds.size() ) {
      ERR << constants::outOfRangeErrMsg << std::endl;
      throw std::logic_error( constants::outOfRangeErrMsg.data() );
    }
  }

  void zyppng::AsyncDataSource::close()
  {
    Z_D();
    for( uint i = 0; i < d->_readFds.size(); ++i ) {
      auto &readChan = d->_readFds[i];
      readChan._readNotifier.reset();
      if ( readChan._readFd >= 0)
        d->_sigReadFdClosed.emit( i, UserRequest );
    }
    d->_readFds.clear();

    d->_writeNotifier.reset();
    d->_writeBuffer.clear();
    if ( d->_writeFd >= 0 ) {
      d->_writeFd = -1;
      d->_sigWriteFdClosed.emit( UserRequest );
    }

    IODevice::close();
  }

  void AsyncDataSource::closeWriteChannel()
  {
    Z_D();

    // if we are open writeOnly, simply call close();
    if ( !canRead() ) {
      close();
      return;
    }

    d->_mode = ReadOnly;
    d->_writeNotifier.reset();
    d->_writeBuffer.clear();

    if ( d->_writeFd >= 0 ) {
      d->_writeFd = -1;
      d->_sigWriteFdClosed.emit( UserRequest );
    }
  }

  bool AsyncDataSource::waitForReadyRead( uint channel, int timeout )
  {
    Z_D();
    if ( !canRead() )
      return false;

    if ( channel >= d->_readFds.size() ) {
      ERR << constants::outOfRangeErrMsg << std::endl;
      throw std::logic_error( constants::outOfRangeErrMsg.data() );
    }

    bool gotRR = false;
    auto rrConn = AutoDisconnect( d->_channelReadyRead.connect([&]( uint activated ){
      gotRR = ( channel == activated );
    }) );

    // we can only wait if we are open for reading and still have a valid fd
    auto &channelRef = d->_readFds[ channel ];
    while ( readFdOpen(channel) && canRead() && !gotRR ) {
      int rEvents = 0;
      if ( EventDispatcher::waitForFdEvent( channelRef._readFd,  AbstractEventSource::Read | AbstractEventSource::Error , rEvents, timeout ) ) {
        //simulate signal from read notifier
        d->notifierActivated( *channelRef._readNotifier, rEvents );
      } else {
        //timeout
        return false;
      }
    }
    return gotRR;
  }

  void AsyncDataSource::flush ()
  {
    Z_D();
    if ( !canWrite() )
      return;

    int timeout = -1;
    while ( canWrite() && d->_writeBuffer.frontSize() ) {
      int rEvents = 0;
      if ( EventDispatcher::waitForFdEvent( d->_writeFd,  AbstractEventSource::Write | AbstractEventSource::Error , rEvents, timeout ) ) {
        //simulate signal from write notifier
        d->readyWrite();
      } else {
        //timeout
        return;
      }
    }
  }

  SignalProxy<void (AsyncDataSource::ChannelCloseReason)> AsyncDataSource::sigWriteFdClosed()
  {
    return d_func()->_sigWriteFdClosed;
  }

  SignalProxy<void( uint, AsyncDataSource::ChannelCloseReason )> AsyncDataSource::sigReadFdClosed()
  {
    return d_func()->_sigReadFdClosed;
  }

  bool AsyncDataSource::readFdOpen() const
  {
    Z_D();
    if ( !d->_readChannels.size() )
      return false;
    return readFdOpen( d_func()->_currentReadChannel );
  }

  bool AsyncDataSource::readFdOpen(uint channel) const
  {
    Z_D();
    if ( channel >= d->_readFds.size() ) {
      ERR << constants::outOfRangeErrMsg << std::endl;
      throw std::logic_error( constants::outOfRangeErrMsg.data() );
    }
    auto &channelRef = d->_readFds[ channel ];
    return ( channelRef._readNotifier && channelRef._readFd >= 0 );
  }

}
