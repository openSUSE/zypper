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

#ifndef ZYPPNG_IO_SOCKET_DEFINED
#define ZYPPNG_IO_SOCKET_DEFINED

#include <zypp/zyppng/io/IODevice>
#include <zypp/zyppng/io/SockAddr>
#include <zypp/zyppng/base/Signals>

namespace zyppng {

  class SocketPrivate;
  class Socket : public IODevice
  {
    ZYPP_DECLARE_PRIVATE(Socket);
  public:

    enum SocketError {
      NoError,
      UnknownSocketError,
      InsufficientPermissions,
      InvalidSocketOptions,
      InsufficientRessources,
      UnsupportedSocketOptions,
      FailedSocketOperation,
      SocketAlreadyBound,
      SocketNotListening,
      AddressInUse,
      AddressNotAvailable,
      AddressIssue,
      OperationNotSupported,
      ConnectionTimeout,
      ConnectionDelayed,
      ConnectionRefused,
      ConnectionClosedByRemote,
      NetworkUnreachable,
      InternalError
    };

    enum SocketState {
      InitialState,
      ConnectingState,
      ConnectedState,
      ListeningState,
      ClosingState,
      ClosedState
    };

    using Ptr = std::shared_ptr<Socket>;

    static Ptr create ( int domain, int type, int protocol );
    virtual ~Socket();

    // IODevice interface
    void close() override;
    size_t bytesAvailable() const override;

    SocketState state () const;

    bool bind ( std::shared_ptr<SockAddr> addr );
    bool listen ( int backlog = 50 );
    Ptr accept ();

    /*!
     * This will set the socket into blocking mode. Can be used if there is no
     * EventLoop running. (Make sure a EventDispatcher instance exists for the thread)
     */
    bool setBlocking ( const bool set = true );


    /*!
     * Starts to disconnect from the host, wait for the actual
     * \ref sigDisconnected signal to arrive before destroying the socket
     * otherwise the connection is forcefully aborted
     */
    void disconnect ();

    /*!
     * Forcefully aborts the connection, no matter if there is still data to be written
     * in the write buffer
     */
    void abort ();

    bool connect ( std::shared_ptr<SockAddr> addr );

    bool waitForConnected ( int timeout = -1 );
    bool waitForAllBytesWritten ( int timeout = -1 );

    int nativeSocket () const;
    SocketError lastError () const;

    SignalProxy<void ()> sigIncomingConnection ();

    SignalProxy<void ()> sigConnected ();
    SignalProxy<void ()> sigDisconnected ();
    SignalProxy<void ()> sigReadyRead ();
    SignalProxy<void ( std::size_t )> sigBytesWritten ();

    SignalProxy<void (Socket::SocketError)> sigError ();

  protected:
    Socket ( int domain, int type, int protocol );
    static Ptr fromSocket ( int fd, SocketState state );


    // IODevice interface
  protected:
    off_t writeData(const char *data, off_t count) override;
    off_t readData(char *buffer, off_t bufsize) override;
  };
}
#endif
