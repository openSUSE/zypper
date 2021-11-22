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
      d->_readChannels.push_back( IOBuffer() );
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

  size_t IODevice::bytesAvailable() const
  {
    Z_D();
    return bytesAvailable( d->_currentReadChannel );
  }

  ByteArray IODevice::readAll()
  {
    Z_D();
    return readAll( d->_currentReadChannel );
  }

  ByteArray IODevice::read( size_t maxSize )
  {
    if ( !canRead() || maxSize == 0 )
      return {};
    return read( d_func()->_currentReadChannel, maxSize );
  }

  size_t IODevice::read( char *buf, size_t maxSize )
  {
    Z_D();
    if ( !canRead() )
      return 0;
    return read( d->_currentReadChannel, buf, maxSize );
  }

  ByteArray IODevice::readLine(const size_t maxSize)
  {
    if ( !canRead() )
      return {};

    return channelReadLine( d_func()->_currentReadChannel, maxSize );
  }

  ByteArray IODevice::readAll( uint channel )
  {
    return read( channel, bytesAvailable( channel ) );
  }

  ByteArray IODevice::read( uint channel, size_t maxSize )
  {
    if ( !canRead() || maxSize == 0 )
      return {};

    ByteArray res( maxSize, '\0' );
    const auto r = read( channel, res.data(), maxSize );
    res.resize( r );
    return res;
  }

  size_t IODevice::read( uint channel, char *buf, size_t maxSize )
  {
    Z_D();
    if ( !canRead() )
      return 0;

    if ( channel >= d->_readChannels.size() ) {
      ERR << constants::outOfRangeErrMsg << std::endl;
      throw std::out_of_range( constants::outOfRangeErrMsg.data() );
    }

    size_t readSoFar = d->_readChannels[ channel ].read( buf, maxSize );

    // try to read more from the device
    if ( readSoFar < maxSize ) {
      size_t readFromDev = readData( channel, buf+readSoFar, maxSize - readSoFar );
      if ( readFromDev > 0 )
        return readSoFar + readFromDev;
    }
    return readSoFar;
  }

  ByteArray IODevice::channelReadLine( uint channel, const size_t maxSize )
  {
    Z_D();
    if ( !canRead() )
      return {};

    if ( channel >= d->_readChannels.size() ) {
      ERR << constants::outOfRangeErrMsg << std::endl;
      throw std::out_of_range( constants::outOfRangeErrMsg.data() );
    }
    return d->_readChannels[channel].readLine( maxSize );
  }

  size_t IODevice::bytesAvailable ( uint channel ) const 
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

  off_t zyppng::IODevice::write(const zyppng::ByteArray &data)
  {
    if ( !canWrite() )
      return 0;
    return write( data.data(), data.size() );
  }

  off_t IODevice::write(const char *data, size_t len)
  {
    if ( !canWrite() )
      return 0;
    return writeData( data, len );
  }

  SignalProxy<void ()> IODevice::sigReadyRead()
  {
    return d_func()->_readyRead;
  }

  SignalProxy<void (uint)> IODevice::sigChannelReadyRead () 
  {
    return d_func()->_channelReadyRead;
  }

  SignalProxy< void (std::size_t)> IODevice::sigBytesWritten () 
  {
    return d_func()->_sigBytesWritten;  
  }

  SignalProxy< void ()> IODevice::sigAllBytesWritten () 
  {
    return d_func()->_sigAllBytesWritten;
  }
}

