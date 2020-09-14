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

  /*!
   * Combines Sockets with the zypp event loop. Generally every socket type that is supported
   * by the socket(2) API should work, however currently only Unix Domain sockets are tested.
   *
   * The useage pattern of this class is similar to the socket(2) API, on the server endpoint
   * one listening socket is created and bound to a adress to accept incoming connections.
   * For every appected connections a connected socket instance is returned which can be used for communication with the peer.
   *
   */
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

    /*!
     * Creates a new socket with the given \a doman , \a type and \a protocol.
     * See socket(2) for available arguments.
     *
     * \note currently only AF_UNIX, SOCK_STREAM sockets are tested
     */
    static Ptr create ( int domain, int type, int protocol );
    virtual ~Socket();

    /*!
     * Closed the socket and disconnects from the peer.
     * The \a disconnected signal will be emitted.
     * This is similar to calling \ref disconnect.
     */
    void close() override;

    /*!
     * Returns the current number of bytes that can be read from the socket.
     */
    size_t bytesAvailable() const override;

    /*!
     * Returns the current number of bytes that are not yet written to the socket.
     */
    size_t bytesPending() const;

    /*!
     * Returns the current state the socket is in,
     * check \ref SocketState for possible values.
     */
    SocketState state () const;

    /*!
     * Bind the socket to a local address, this is usually required to listen for incoming connections.
     */
    bool bind ( std::shared_ptr<SockAddr> addr );

    /*!
     * Puts the socket in listen mode, the state is set accordingly.
     */
    bool listen ( int backlog = 50 );

    /*!
     * Accepts a pending incoming connection and returns it as a
     * intialized connected socket.
     * Returns nullptr if there is no pending connection.
     */
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

    /*!
     * Starts the connection process to the given adress in \a addr.
     * Returns true on success, if false is returned the current state should be
     * checked if the connection was simply delayed. In that case the state is \ref ConnectingState
     * and error is set to \ref ConnectionDelayed and the socket still tries to connect.
     */
    bool connect ( std::shared_ptr<SockAddr> addr );

    /*!
     * Blocks the current event loop to wait for the connected event on the socket.
     * \note do not use until there is no other way
     */
    bool waitForConnected ( int timeout = -1 );

    /*!
     * Blocks the current event loop to wait until all bytes from the buffer have been written
     * to the device.
     *
     * \note do not use until there is no other way
     */
    bool waitForAllBytesWritten ( int timeout = -1 );

    /*!
     * Blocks the current event loop to wait until there are bytes available to read from the device
     *
     * \note do not use until there is no other way
     */
    bool waitForReadyRead ( int timeout = -1 );

    /*!
     * Returns the native socket handle.
     */
    int nativeSocket () const;

    /*!
     * This will release the current socket and instantly transition into
     * ClosedState. All unwritten data in the buffers will be discarded,
     * so consider to use \ref waitForAllBytesWritten before you call this function.
     *
     * \returns the socket or -1 if there was no socket connected
     */
    int releaseSocket ();

    /*!
     * Return the last error that was encountered by the socket.
     */
    SocketError lastError () const;

    /*!
     * Signals when there are new incoming connection pending.
     * \note Make sure to accept all connection otherwise the event loop keeps triggering that signal
     */
    SignalProxy<void ()> sigIncomingConnection ();

    /*!
     * Signal is emitted as soon as the socket enters the connected state. It is not possible to
     * read and write data.
     */
    SignalProxy<void ()> sigConnected ();

    /*!
     * Signal is emitted always when the socket was closed.
     * \note when the socket should be deleted in a slot connected to this signal make sure
     *       to delay the delete using the EventDispatcher.
     */
    SignalProxy<void ()> sigDisconnected ();

    /*!
     * Signal is emitted when there is data available to read
     */
    SignalProxy<void ()> sigReadyRead ();

    /*!
     * Signal is emitted every time bytes have been written to the underlying socket.
     * This can be used to track how much data was actually sent.
     */
    SignalProxy<void ( std::size_t )> sigBytesWritten ();

    /*!
     * Signal is emitted whenever a error happend in the socket. Make sure to check
     * the actual error code to determine if the error is fatal.
     */
    SignalProxy<void (Socket::SocketError)> sigError ();


    /*!
     * Makes a socket instance out of a existing socket file descriptor,
     * the instance takes ownership of the fd
     */
    static Ptr fromSocket ( int fd, SocketState state );

  protected:
    Socket ( int domain, int type, int protocol );


    // IODevice interface
  protected:
    off_t writeData(const char *data, off_t count) override;
    off_t readData(char *buffer, off_t bufsize) override;
  };
}
#endif
