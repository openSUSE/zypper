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

  /*!
   * The IODevice class represents a async sequential IO device, like a Socket or Pipe,
   * to receive and or send data.
   */
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
    ByteArray read ( int64_t maxSize );
    int64_t read ( char *buf, int64_t maxSize );
    virtual ByteArray readLine (const int64_t maxSize = 0 );
    virtual int64_t bytesAvailable () const;
    bool canReadLine () const;

    ByteArray readAll ( uint channel );
    ByteArray read ( uint channel, int64_t maxSize );
    int64_t read ( uint channel, char *buf, int64_t maxSize );

    /*!
     * Convenience function that reads a line from the device into a ByteArray. Since
     * this function has no way to signal if a error happened, a empty ByteArray is
     * returned if there was no data or if a error occured.
     */
    ByteArray channelReadLine ( uint channel, int64_t maxSize = 0 );

    /*!
     * Reads data from the device until one of the following conditions are met:
     * - A \n is encountered
     * - maxSize nr of bytes have been read
     * - a error occurs on the device
     *
     * If bytes have been read from the device this always returns the number of bytes that
     * have been read, otherwise if no data was read 0 is returned or if a error occurs -1 is returned.
     */
    virtual int64_t channelReadLine ( uint channel, char *buf, const int64_t maxSize );
    virtual int64_t bytesAvailable( uint channel ) const;

    /*!
     * Returns true if a line can be read from the currently buffered data in the given channel
     */
    bool canReadLine ( uint channel ) const;

    int64_t write ( const ByteArray &data );
    int64_t write ( const char *data, int64_t len );

    /*!
     * Blocks the current event loop to wait until there are bytes available to read from the device.
     * This always operates on the read channel that is selected as the default when the function is first called,
     * even if the default channel would be changed during the wait.
     *
     * \sa zyppng::IODevice::currentReadChannel
     * \note do not use until there is a very good reason, like event processing should not continue until readyRead was sent
     */
    bool waitForReadyRead(int timeout);

    /*!
     * Blocks the current event loop to wait until there are bytes available to read from the given read channel.
     *
     * \note do not use until there is a very good reason, like event processing should not continue until readyRead was sent
     */
    virtual bool waitForReadyRead(uint channel, int timeout) = 0;


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
    SignalProxy< void (int64_t)> sigBytesWritten ();

    /*!
     * Signal is emitted every time all bytes have been written to the underlying device.
     */
    SignalProxy< void ()> sigAllBytesWritten ();

  protected:
    IODevice( IODevicePrivate &d );
    virtual bool open ( const OpenMode mode );
    virtual int64_t rawBytesAvailable ( uint channel ) const = 0;
    virtual int64_t writeData ( const char *data, int64_t count ) = 0;
    virtual int64_t readData  ( uint channel, char *buffer, int64_t bufsize ) = 0;
    virtual void  readChannelChanged ( uint channel ) = 0;
    void setReadChannelCount ( uint channels );
  };
  ZYPP_DECLARE_OPERATORS_FOR_FLAGS( IODevice::OpenMode );

}

#endif
