#include "private/iodevice_p.h"

namespace zyppng {

  IODevicePrivate::IODevicePrivate(IODevice &p) : BasePrivate(p)
  { }

  ZYPP_IMPL_PRIVATE(IODevice)

  IODevice::IODevice() : Base( *( new IODevicePrivate(*this)) )
  { }

  IODevice::IODevice(IODevicePrivate &d) : Base(d)
  { }

  bool IODevice::open( const OpenMode mode )
  {
    Z_D();

    d->_mode = mode;
    d->_readChannels.clear();
    if ( canRead() ) {
      d->_readChannels.push_back( IOBuffer( d->_readBufChunkSize ) );
      setReadChannel( 0 );
    }

    return true;
  }

  void IODevice::close()
  {
    Z_D();
    d->_mode = IODevice::Closed;
    d->_readChannels.clear();
  }

  void IODevice::setReadChannelCount ( uint channels ) {
    Z_D();
    if ( canRead() ) {
      d->_readChannels.resize( channels );
    }
  }

  void IODevice::setReadChannel ( uint channel )
  {
    Z_D();
    if ( !canRead() )
      return;
    if ( channel >= d->_readChannels.size() ) {
      ERR << constants::outOfRangeErrMsg << std::endl;
      throw std::out_of_range( constants::outOfRangeErrMsg.data() );
    }
    d->_currentReadChannel = channel;
    readChannelChanged( channel );
  }

  uint IODevice::currentReadChannel () const
  {
    Z_D();
    if ( !canRead() )
      return 0;
    return d->_currentReadChannel;
  }

  int IODevice::readChannelCount () const
  {
    Z_D();
    if ( !canRead() )
      return 0;
    return d->_readChannels.size();
  }

  bool IODevice::canRead() const
  {
    return ( d_func()->_mode & IODevice::ReadOnly );
  }

  bool IODevice::canWrite() const
  {
    return ( d_func()->_mode & IODevice::WriteOnly );
  }

  bool IODevice::isOpen() const
  {
    return d_func()->_mode != IODevice::Closed;
  }

  bool IODevice::canReadLine() const
  {
    Z_D();
    if ( !canRead() )
      return false;

    return canReadLine( d->_currentReadChannel );
  }

  int64_t IODevice::bytesAvailable() const
  {
    Z_D();
    return bytesAvailable( d->_currentReadChannel );
  }

  ByteArray IODevice::readAll()
  {
    Z_D();
    return readAll( d->_currentReadChannel );
  }

  ByteArray IODevice::read( int64_t maxSize )
  {
    if ( !canRead() || maxSize <= 0 )
      return {};
    return read( d_func()->_currentReadChannel, maxSize );
  }

  int64_t IODevice::read(char *buf, int64_t maxSize )
  {
    Z_D();
    if ( !canRead() )
      return -1;
    return read( d->_currentReadChannel, buf, maxSize );
  }

  ByteArray IODevice::readLine( const int64_t maxSize )
  {
    if ( !canRead() )
      return {};

    return channelReadLine( d_func()->_currentReadChannel, maxSize );
  }

  ByteArray IODevice::readAll( uint channel )
  {
    return read( channel, bytesAvailable( channel ) );
  }

  ByteArray IODevice::read( uint channel, int64_t maxSize )
  {
    if ( !canRead() || maxSize <= 0 )
      return {};

    ByteArray res( maxSize, '\0' );
    const auto r = read( channel, res.data(), maxSize );
    res.resize( r );
    return res;
  }

  int64_t IODevice::read( uint channel, char *buf, int64_t maxSize )
  {
    Z_D();
    if ( !canRead() || maxSize < 0 )
      return -1;

    if ( channel >= d->_readChannels.size() ) {
      ERR << constants::outOfRangeErrMsg << std::endl;
      throw std::out_of_range( constants::outOfRangeErrMsg.data() );
    }

    int64_t readSoFar = d->_readChannels[ channel ].read( buf, maxSize );

    // try to read more from the device
    if ( readSoFar < maxSize ) {
      int64_t readFromDev = readData( channel, buf+readSoFar, maxSize - readSoFar );
      if ( readFromDev > 0 )
        return readSoFar + readFromDev;
    }
    return readSoFar;
  }

  ByteArray IODevice::channelReadLine( uint channel, int64_t maxSize )
  {
    Z_D();
    if ( !canRead() || maxSize < 0 )
      return {};

    if ( channel >= d->_readChannels.size() ) {
      ERR << constants::outOfRangeErrMsg << std::endl;
      throw std::out_of_range( constants::outOfRangeErrMsg.data() );
    }

    ByteArray result;
    // largest possible ByteArray in int64_t boundaries
    const auto maxBArrSize = int64_t( std::min( ByteArray::maxSize(), std::size_t(std::numeric_limits<int64_t>::max()) ) );
    if ( maxSize > maxBArrSize ) {
      ERR << "Calling channelReadLine with maxSize > int64_t(ByteArray::maxSize) " << std::endl;
      maxSize = maxBArrSize - 1;
    }

    // how much did we read?
    int64_t readSoFar = 0;

    // if we have no size or the size is really big we read incrementally, use the buffer chunk size
    // to read full chunks from the buffer if possible
    if ( maxSize == 0 || maxSize >= (maxBArrSize - 1) ) {

      // largest possible ByteArray
      maxSize = maxBArrSize;

      // we need to read in chunks until we get a \n
      int64_t lastReadSize = 0;
      result.resize (1); // leave room for \0
      do {
        result.resize( std::min( std::size_t(maxSize), std::size_t(result.size() + d->_readBufChunkSize )) );
        lastReadSize = channelReadLine( channel, result.data() + readSoFar, result.size() - readSoFar );
        if ( lastReadSize > 0)
          readSoFar += lastReadSize;

      // check for equal _readBufSize,
      // our readData request is always 1 byte bigger than the _readBufChunkSize because of the initial byte we allocated in the result buffer,
      // so the \0 that is appended by readLine does not make a difference.
      } while( lastReadSize == d->_readBufChunkSize
                && result[readSoFar-1] != '\n' );

    } else {
      result.resize( maxSize );
      readSoFar = channelReadLine( channel, result.data(), result.size() );
    }

    if ( readSoFar > 0 ) {
      // we do not need to keep the \0 in the ByteArray
      result.resize( readSoFar );
    } else {
      result.clear ();
    }

    // make sure we do not waste memory
    result.shrink_to_fit();

    return result;
  }

  int64_t IODevice::channelReadLine( uint channel, char *buf, const int64_t maxSize )
  {
    Z_D();

    if ( !canRead() || maxSize < 0 )
      return -1;

    if ( channel >= d->_readChannels.size() ) {
      ERR << constants::outOfRangeErrMsg << std::endl;
      throw std::out_of_range( constants::outOfRangeErrMsg.data() );
    }

    if ( maxSize < 2 ) {
      ERR << "channelReadLine needs at least a buffsize of 2" << std::endl;
      return -1;
    }

    int64_t toRead    = maxSize - 1; // append \0 at the end
    int64_t readSoFar = 0;
    if ( d->_readChannels[channel].size () > 0 )
      readSoFar = d->_readChannels[channel].readLine( buf, toRead + 1 /*IOBuffer appends \0*/ );

    if ( readSoFar == toRead || ( readSoFar > 0 && buf[readSoFar-1] == '\n' ) ) {
      buf[readSoFar] = '\0';
      return readSoFar;
    }

    bool hasError = false;
    // if we reach here, the buffer was either empty, or does not contain a \n, in both cases we need to
    // read from the device directly until we hit a ending condition
    while ( readSoFar < toRead ) {
      const auto r = readData( channel, buf+readSoFar, 1 );
      if ( r == 0 ) {
        // no data available to be read -> EOF, or data stream empty
        break;
      }
      else if ( r < 0 ) {
        hasError = true;
        break;
      }
      readSoFar+=r;

      if ( buf[readSoFar-1] == '\n' )
        break;
    }

    if ( readSoFar == 0 )
      return hasError ? -1 : 0;

    buf[readSoFar] = '\0';
    return readSoFar;
  }

  int64_t IODevice::bytesAvailable ( uint channel ) const
  {
    Z_D();
    if ( !canRead() )
      return 0;
    return d->_readChannels[channel].size() + rawBytesAvailable( channel );
  }

  bool IODevice::canReadLine ( uint channel ) const
  {
    Z_D();
    if ( !canRead() || channel >= d->_readChannels.size() )
      return false;
    return d->_readChannels[channel].canReadLine();
  }

  int64_t zyppng::IODevice::write( const zyppng::ByteArray &data)
  {
    if ( !canWrite() )
      return 0;
    return write( data.data(), data.size() );
  }

  int64_t IODevice::write( const char *data, int64_t len)
  {
    if ( !canWrite() || len <= 0 )
      return 0;
    return writeData( data, len );
  }

  bool IODevice::waitForReadyRead( int timeout)
  {
    Z_D();
    if ( !canRead() )
      return false;

    return waitForReadyRead( d->_currentReadChannel, timeout );
  }

  SignalProxy<void ()> IODevice::sigReadyRead()
  {
    return d_func()->_readyRead;
  }

  SignalProxy<void (uint)> IODevice::sigChannelReadyRead ()
  {
    return d_func()->_channelReadyRead;
  }

  SignalProxy<void (int64_t)> IODevice::sigBytesWritten()
  {
    return d_func()->_sigBytesWritten;
  }

  SignalProxy< void ()> IODevice::sigAllBytesWritten ()
  {
    return d_func()->_sigAllBytesWritten;
  }
}

