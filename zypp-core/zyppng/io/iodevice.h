/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/

#ifndef ZYPPNG_IO_IODEVICE_DEFINED
#define ZYPPNG_IO_IODEVICE_DEFINED

#include <zypp-core/base/Flags.h>
#include <zypp-core/zyppng/base/Base>
#include <zypp-core/zyppng/base/Signals>
#include <zypp-core/zyppng/core/ByteArray>

namespace zyppng {

  class IODevicePrivate;
  class IODevice : public Base
  {
    ZYPP_DECLARE_PRIVATE(IODevice);
  public:

    enum OpenModeFlag {
      Closed = 0x0,
      ReadOnly = 0x1,
      WriteOnly = 0x2,
      ReadWrite = ReadOnly | WriteOnly
    };
    ZYPP_DECLARE_FLAGS( OpenMode, OpenModeFlag );

    using Ptr = std::shared_ptr<IODevice>;
    using WeakPtr = std::weak_ptr<IODevice>;

    IODevice();
    virtual void close ();

    void setReadChannel ( uint channel );
    uint currentReadChannel () const;
    int  readChannelCount () const;

    bool canRead () const;
    bool canWrite () const;
    bool isOpen () const;

    ByteArray readAll ();
    ByteArray read ( size_t maxSize );
    size_t read ( char *buf, size_t maxSize );
    virtual ByteArray readLine ( const size_t maxSize = 0 );
    virtual size_t bytesAvailable () const;
    bool canReadLine () const;

    ByteArray readAll ( uint channel );
    ByteArray read ( uint channel, size_t maxSize );
    size_t read ( uint channel, char *buf, size_t maxSize );
    virtual ByteArray channelReadLine ( uint channel, const size_t maxSize = 0 );
    virtual size_t bytesAvailable ( uint channel ) const;
    bool canReadLine ( uint channel ) const;

    off_t write ( const ByteArray &data );
    off_t write ( const char *data, size_t len );

    /*!
     * Signal is emitted when there is data available to read on the current default read channel
     */
    SignalProxy<void ()> sigReadyRead ();

    /*!
     * Signal is emitted when there is data available on the given channel
     */
    SignalProxy<void (uint)> sigChannelReadyRead ();

    /*!
     * Signal is emitted every time bytes have been written to the underlying device.
     * This can be used to track how much data was actually sent.
     */
    SignalProxy< void (std::size_t)> sigBytesWritten ();

    /*!
     * Signal is emitted every time all bytes have been written to the underlying device.
     */
    SignalProxy< void ()> sigAllBytesWritten ();

  protected:
    IODevice( IODevicePrivate &d );
    virtual bool open ( const OpenMode mode );
    virtual size_t rawBytesAvailable ( uint channel ) const = 0;
    virtual off_t writeData ( const char *data, off_t count ) = 0;
    virtual off_t readData  ( uint channel, char *buffer, off_t bufsize ) = 0;
    virtual void  readChannelChanged ( uint channel ) = 0;
    void setReadChannelCount ( uint channels );
  };
  ZYPP_DECLARE_OPERATORS_FOR_FLAGS( IODevice::OpenMode );

}

#endif
