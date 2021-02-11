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

#ifndef ZYPPNG_IO_SOCKET_P_DEFINED
#define ZYPPNG_IO_SOCKET_P_DEFINED

#include <zypp-core/zyppng/io/socket.h>
#include <zypp-core/zyppng/io/private/iobuffer_p.h>
#include <zypp-core/zyppng/base/socketnotifier.h>
#include <zypp-core/zyppng/base/Timer>
#include "iodevice_p.h"

#include <variant>

namespace zyppng {

  class SocketPrivate : public IODevicePrivate
  {
    ZYPP_DECLARE_PUBLIC(Socket);
  public:

    SocketPrivate( int domain, int type, int protocol, Socket &p ) : IODevicePrivate(p),
      _domain(domain),
      _type( type ),
      _protocol( protocol )
    { }

    bool initSocket () ;
    void setError ( Socket::SocketError error, std::string &&err );
    bool handleConnectError ( int error );

    bool transition ( Socket::SocketState newState );
    Socket::SocketState state () const;

    bool connectToHost ();
    void onSocketActivated ( int ev );
    void onSocketActivatedSlot ( const SocketNotifier &, int ev ) {
      return onSocketActivated(ev);
    }
    int  rawBytesAvailable () const;
    bool readRawBytesToBuffer ();
    bool writePendingData ();

    static int bytesAvailableOnSocket ( int socket );


    int _domain;
    int _type;
    int _protocol;
    std::shared_ptr<SockAddr> _targetAddr;

    int _socket = -1;
    bool _borrowedSocket = false;

    //error handling
    Socket::SocketError _error = Socket::NoError;
    std::string _errorDesc;

    //signals
    Signal< void(Socket::SocketError)> _sigError;
    Signal< void (std::size_t)> _sigBytesWritten;
    Signal< void()> _readyRead;
    Signal< void()> _incomingConnection;
    Signal< void()> _connected;
    Signal< void()> _disconnected;


    struct InitialState {
      static constexpr Socket::SocketState type() { return Socket::InitialState; }
    };

    struct ConnectingState : public sigc::trackable {
      NON_COPYABLE(ConnectingState);
      ConnectingState() = default;

      static constexpr Socket::SocketState type() { return Socket::ConnectingState; }

      SocketNotifier::Ptr _connectNotifier;
      Timer::Ptr _connectTimeout;
    };

    struct ConnectedState : public sigc::trackable {
      NON_COPYABLE(ConnectedState);
      ConnectedState() = default;
      static constexpr Socket::SocketState type() { return Socket::ConnectedState; }
      SocketNotifier::Ptr _socketNotifier;
      IOBuffer _writeBuffer;
    };

    struct ListeningState : public sigc::trackable {
      NON_COPYABLE(ListeningState);
      ListeningState() = default;
      static constexpr Socket::SocketState type() { return Socket::ListeningState; }
      SocketNotifier::Ptr _socketNotifier;
    };

    struct ClosingState {
      static constexpr Socket::SocketState type() { return Socket::ClosingState; }
      ClosingState( IOBuffer &&writeBuffer );

      SocketNotifier::Ptr _socketNotifier;
      IOBuffer _writeBuffer;
    };

    struct ClosedState {
      static constexpr Socket::SocketState type() { return Socket::ClosedState; }
    };
    std::variant< InitialState, ConnectingState, ConnectedState, ListeningState, ClosingState, ClosedState > _state = InitialState();
  };

}

#endif
