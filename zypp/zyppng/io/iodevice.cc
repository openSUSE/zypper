#include "private/iodevice_p.h"

namespace zyppng {

  IODevice::IODevice() : Base( *( new IODevicePrivate) )
  { }

  IODevice::IODevice(IODevicePrivate &d) : Base(d)
  { }

  bool IODevice::open( const OpenMode mode )
  {
    Z_D();

    d->_readBuf.clear();
    d->_mode = mode;

    return true;
  }

  void IODevice::close()
  {
    d_func()->_mode = IODevice::Closed;
    d_func()->_readBuf.clear();
  }

  bool IODevice::isOpen() const
  {
    return d_func()->_mode == IODevice::Closed;
  }

  bool IODevice::canReadLine() const
  {
    Z_D();
    return d->_readBuf.canReadLine();
  }

  size_t IODevice::bytesAvailable() const
  {
    Z_D();
    return d->_readBuf.size();
  }

  ByteArray IODevice::readAll()
  {
    return read( bytesAvailable() );
  }

  ByteArray IODevice::read( size_t maxSize )
  {
    ByteArray res( maxSize, '\0' );
    const auto r = read( res.data(), maxSize );
    res.resize( r );
    return res;
  }

  size_t IODevice::read( char *buf, size_t maxSize )
  {
    Z_D();
    size_t readSoFar = d->_readBuf.read( buf, maxSize );

    // try to read more from the device
    if ( readSoFar < maxSize ) {
      size_t readFromDev = readData( buf+readSoFar, maxSize - readSoFar );
      if ( readFromDev > 0 )
        return readSoFar + readFromDev;
    }
    return readSoFar;
  }

  ByteArray IODevice::readLine(const size_t maxSize)
  {
    return d_func()->_readBuf.readLine( maxSize );
  }

  off_t zyppng::IODevice::write(const zyppng::ByteArray &data)
  {
    return write( data.data(), data.size() );
  }

  off_t IODevice::write(const char *data, size_t len)
  {
    return writeData( data, len );
  }

}

