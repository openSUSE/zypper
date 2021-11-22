#ifndef ASYNCDATASOURCE_H
#define ASYNCDATASOURCE_H

#include <zypp-core/zyppng/base/Base>
#include <zypp-core/zyppng/io/IODevice>

namespace zyppng {

  class AsyncDataSourcePrivate;

  class AsyncDataSource : public IODevice
  {
    ZYPP_DECLARE_PRIVATE(AsyncDataSource);
  public:

    enum ChannelCloseReason {
      RemoteClose,    // the other side of the fd was closed
      AccessError,    // we got an EACCESS when polling the fd
      InternalError,  // we got a unexpected errno when polling the fd
      UserRequest     // channel were closed because close() was called
    };

    using Ptr     = std::shared_ptr<AsyncDataSource>;
    using WeakPtr = std::weak_ptr<AsyncDataSource>;

    static Ptr create ();
    bool openFds ( std::vector<int> readFds, int writeFd = -1 );
    void close () override;

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
    bool waitForReadyRead(uint channel, int timeout);

    /*!
     * Blocks the current event loop to wait until all bytes currently in the buffer have been written to
     * the write fd.
     *
     * \note do not use until there is a very good reason, like event processing should not continue until readyRead was sent
     */
    void flush ();

    /*!
     * Signal is emitted always when the write channel is closed.
     * All data that was not written yet will be discarded and canWrite() will return
     * false.
     */
    SignalProxy<void( AsyncDataSource::ChannelCloseReason )> sigWriteFdClosed();

    /*!
     * Signal is emitted always when the write channel is closed, for example
     * when the write side of a pipe is closed. All data still residing in the read buffer
     * can still be read.
     */
    SignalProxy<void( uint, AsyncDataSource::ChannelCloseReason )> sigReadFdClosed();

    /*!
     * Returns true as long as the default read channel was not closed ( e.g. sigReadFdClosed was emitted )
     */
    bool readFdOpen () const;

    /*!
     * Returns true as long as the given read channel was not closed ( e.g. sigReadFdClosed was emitted )
     */
    bool readFdOpen ( uint channel ) const;

  protected:
    AsyncDataSource (  );
    AsyncDataSource( AsyncDataSourcePrivate &d );
    off_t writeData(const char *data, off_t count) override;

  private:
    using IODevice::open;

    off_t readData( uint channel, char *buffer, off_t bufsize ) override;
    size_t rawBytesAvailable( uint channel ) const override;
    void readChannelChanged ( uint channel ) override;
  };
}


#endif // ASYNCDATASOURCE_H
