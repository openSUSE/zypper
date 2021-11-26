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
    bool open ( int readFd = -1, int writeFd = -1 );
    void close () override;

    /*!
     * Blocks the current event loop to wait until there are bytes available to read from the device
     *
     * \note do not use until there is a very good reason, like event processing should not continue until readyRead was sent
     */
    bool waitForReadyRead(int timeout);

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
    SignalProxy<void( AsyncDataSource::ChannelCloseReason )> sigReadFdClosed();

    /*!
     * Signal is emitted every time bytes have been written to the underlying fd.
     * This can be used to track how much data was actually sent.
     */
    SignalProxy< void (std::size_t)> sigBytesWritten ();

  protected:
    AsyncDataSource (  );
    off_t writeData(const char *data, off_t count) override;
    off_t readData(char *buffer, off_t bufsize) override;
    size_t rawBytesAvailable() const override;

  private:
    using IODevice::open;
  };
}


#endif // ASYNCDATASOURCE_H
