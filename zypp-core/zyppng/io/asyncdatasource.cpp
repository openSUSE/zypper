#include "asyncdatasource.h"

#include <zypp-core/base/IOTools.h>
#include <zypp-core/zyppng/base/AutoDisconnect>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-core/zyppng/io/private/iodevice_p.h>
#include <zypp-core/zyppng/io/private/iobuffer_p.h>
#include <zypp-core/zyppng/base/SocketNotifier>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>

namespace zyppng {

  class AsyncDataSourcePrivate : public IODevicePrivate {
    ZYPP_DECLARE_PUBLIC(AsyncDataSource);
  public:
    AsyncDataSourcePrivate ( AsyncDataSource &pub ) : IODevicePrivate(pub) {}
    SocketNotifier::Ptr _readNotifier;
    SocketNotifier::Ptr _writeNotifier;
    IOBuffer _writeBuffer;
    int _readFd  = -1;
    int _writeFd = -1;

    void notifierActivated (const SocketNotifier &notify, int evTypes );
    void readyRead  ( );
    void readyWrite ( );

    void closeWriteChannel ( AsyncDataSource::ChannelCloseReason reason );
    void closeReadChannel  ( AsyncDataSource::ChannelCloseReason reason );

    Signal<void( AsyncDataSource::ChannelCloseReason )> _sigWriteFdClosed;
    Signal<void( AsyncDataSource::ChannelCloseReason )> _sigReadFdClosed;
    Signal< void (std::size_t)> _sigBytesWritten;
  };

  void AsyncDataSourcePrivate::notifierActivated( const SocketNotifier &notify, int evTypes )
  {
    if ( _readNotifier.get() == &notify ) {
      readyRead();
    } else if ( _writeNotifier.get() == &notify ) {
      if ( evTypes & SocketNotifier::Error ) {
        DBG << "Closing due to error when polling" << std::endl;
        closeWriteChannel(  AsyncDataSource::RemoteClose );
        return;
      }
      readyWrite();
    }
  }

  void AsyncDataSourcePrivate::readyRead()
  {
    auto bytesToRead = z_func()->rawBytesAvailable();
    if ( bytesToRead == 0 ) {
      // make sure to check if bytes are available even if the ioctl call returns something different
      bytesToRead = 4096;
    }

    char *buf = _readBuf.reserve( bytesToRead );
    const auto bytesRead = z_func()->readData( buf, bytesToRead );

    if ( bytesRead < 0 ) {
      _readBuf.chop( bytesToRead );
      return;
    }

    if ( bytesToRead > (size_t)bytesRead )
      _readBuf.chop( bytesToRead-bytesRead );

    if ( bytesRead > 0 ) {
      _readyRead.emit();
      return;
    }
    //handle remote close
    else if ( bytesRead == 0 && errno != EAGAIN && errno != EWOULDBLOCK  ) {
      closeReadChannel(  AsyncDataSource::RemoteClose );
    }

    if ( errno == EAGAIN || errno == EWOULDBLOCK )
      return;

    //setError( Socket::InternalError, strerr_cxx() );
    closeReadChannel(  AsyncDataSource::InternalError );
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

  void AsyncDataSourcePrivate::closeReadChannel( AsyncDataSource::ChannelCloseReason reason )
  {
    // we do not clear the read buffer so code has the opportunity to read whats left in there
    bool sig = _readFd >= 0;
    _readNotifier.reset();
    _readFd = -1;
    if ( sig )
      _sigReadFdClosed.emit( reason );
  }

  AsyncDataSource::AsyncDataSource() : IODevice( *( new AsyncDataSourcePrivate(*this) ) )
  { }

  AsyncDataSource::Ptr AsyncDataSource::create()
  {
    return std::shared_ptr<AsyncDataSource>( new AsyncDataSource );
  }

  bool AsyncDataSource::open( int readFd, int writeFd )
  {
    Z_D();
    close();
    IODevice::OpenMode mode;
    if ( readFd >= 0 ) {
      mode |= IODevice::ReadOnly;
      d->_readFd = readFd;
      zypp::io::setFDBlocking( readFd, false );
      d->_readNotifier = SocketNotifier::create( readFd, SocketNotifier::Read | AbstractEventSource::Error, true );
      d->_readNotifier->connect( &SocketNotifier::sigActivated, *d, &AsyncDataSourcePrivate::notifierActivated );
    }
    if ( writeFd >= 0 ) {
      mode |= IODevice::WriteOnly;
      d->_writeFd = writeFd;
      zypp::io::setFDBlocking( writeFd, false );
      d->_writeNotifier = SocketNotifier::create( writeFd, SocketNotifier::Write | AbstractEventSource::Error, false );
      d->_writeNotifier->connect( &SocketNotifier::sigActivated, *d, &AsyncDataSourcePrivate::notifierActivated );
    }
    return IODevice::open( mode );
  }

  off_t zyppng::AsyncDataSource::writeData( const char *data, off_t count )
  {
    Z_D();
    if ( count > 0 ) {
      // we always use the write buffer, to make sure the fd is actually writeable
      d->_writeBuffer.append( data, count );
      d->_writeNotifier->setEnabled( true );
    }
    return count;
  }

  off_t zyppng::AsyncDataSource::readData( char *buffer, off_t bufsize )
  {
    Z_D();
    const auto read = eintrSafeCall( ::read, d->_readFd, buffer, bufsize );

    // special case for remote close
    if ( read == 0 ) {
      d->closeReadChannel( RemoteClose );
      return -1;
    } else if ( read < 0 ) {
      switch ( errno ) {
#if EAGAIN != EWOULDBLOCK
        case EWOULDBLOCK:
#endif
        case EAGAIN: {
          return 0;
        }
        default: {
          d->closeReadChannel( InternalError );
          return -1;
        }
      }
    }
    return read;
  }

  size_t AsyncDataSource::rawBytesAvailable() const
  {
    if ( isOpen() && canRead() )
      return zyppng::bytesAvailableOnFD( d_func()->_readFd );
    return 0;
  }

  void zyppng::AsyncDataSource::close()
  {
    Z_D();
    d->_readNotifier.reset();
    d->_writeNotifier.reset();
    d->_writeBuffer.clear();

    if ( d->_readFd >= 0)
      d->_sigReadFdClosed.emit( UserRequest );
    if ( d->_writeFd >= 0 )
      d->_sigWriteFdClosed.emit( UserRequest );

    IODevice::close();
  }

  bool AsyncDataSource::waitForReadyRead(int timeout)
  {
    Z_D();
    if ( !canRead() )
      return false;

    bool gotRR = false;
    auto rrConn = AutoDisconnect( d->_readyRead.connect([&](){
      gotRR = true;
    }) );

    // we can only wait if we are open for reading
    while ( canRead() && !gotRR ) {
      int rEvents = 0;
      if ( EventDispatcher::waitForFdEvent( d->_readFd,  AbstractEventSource::Read | AbstractEventSource::Error , rEvents, timeout ) ) {
        //simulate signal from read notifier
        d->notifierActivated( *d->_readNotifier, rEvents );
      } else {
        //timeout
        return false;
      }
    }
    return gotRR;
  }

  SignalProxy<void (AsyncDataSource::ChannelCloseReason)> AsyncDataSource::sigWriteFdClosed()
  {
    return d_func()->_sigWriteFdClosed;
  }

  SignalProxy<void (AsyncDataSource::ChannelCloseReason)> AsyncDataSource::sigReadFdClosed()
  {
    return d_func()->_sigReadFdClosed;
  }

  SignalProxy<void (std::size_t)> AsyncDataSource::sigBytesWritten()
  {
    return d_func()->_sigBytesWritten;
  }
}
