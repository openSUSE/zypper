#include "private/socket_p.h"
#include <errno.h>
#include <string.h>
#include <zypp/base/Logger.h>
#include <zypp/AutoDispose.h>
#include <zypp/zyppng/base/EventDispatcher>
#include <zypp/zyppng/base/private/linuxhelpers_p.h>
#include <sys/ioctl.h> //For FIONREAD
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

namespace zyppng {

  SocketPrivate::ClosingState::ClosingState(IOBuffer &&writeBuffer)
    : _writeBuffer( std::move(writeBuffer) )
  { }

  bool SocketPrivate::initSocket()
  {
    if ( _socket >= 0 )
      return true;

    // Since Linux 2.6.27 we can pass additional flags with the type argument to avoid fcntl
    // if creating sockets fails we might need to change that
    _socket = ::socket( _domain, _type | SOCK_NONBLOCK | SOCK_CLOEXEC, _protocol );
    if ( _socket >= 0 )
      return true;

    switch ( errno ) {
      case EACCES:
        setError( Socket::InsufficientPermissions, strerr_cxx() );
        break;
      case EINVAL:
        setError( Socket::InvalidSocketOptions, strerr_cxx() );
        break;
      case EMFILE:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
        setError( Socket::InsufficientRessources, strerr_cxx() );
        break;
      case EAFNOSUPPORT:
      case EPROTONOSUPPORT:
        setError ( Socket::UnsupportedSocketOptions, strerr_cxx() );
        break;
      default:
        setError( Socket::UnknownSocketError, strerr_cxx() );
        break;
    }

    return false;
  }

  void SocketPrivate::setError(Socket::SocketError error , std::string &&err )
  {
    _error = error;
    _errorDesc = std::move(err);
    _sigError.emit( error );
  }

  bool SocketPrivate::transition( Socket::SocketState newState )
  {
    const auto oldState = state();

    if ( oldState == newState )
      return true;

    switch ( newState ) {
      case Socket::InitialState:
        setError( Socket::InternalError, "Invalid state transition" );
        return false;
      case Socket::ConnectingState:
        if ( oldState != Socket::InitialState && oldState != Socket::ClosedState ) {
          setError( Socket::InternalError, "Invalid state transition" );
          return false;
        }
        _state.emplace<SocketPrivate::ConnectingState>( );
        return connectToHost();
        break;
      case Socket::ConnectedState: {
        if ( oldState != Socket::InitialState && oldState != Socket::ConnectingState ) {
          setError( Socket::InternalError, "Invalid state transition" );
          return false;
        }
        auto &s = _state.emplace<SocketPrivate::ConnectedState>();
        s._socketNotifier = SocketNotifier::create( _socket, SocketNotifier::Read | SocketNotifier::Error, true );
        s._socketNotifier->sigActivated().connect( [this]( const auto &, auto ev){ onSocketActivated( ev );} );
        _connected.emit();
        break;
      }
      case Socket::ListeningState: {
        if ( state() != Socket::InitialState ) {
          setError( Socket::InternalError, "Invalid state transition" );
          return false;
        }
        auto &s = _state.emplace<SocketPrivate::ListeningState>();
        s._socketNotifier = SocketNotifier::create( _socket, SocketNotifier::Read, true );
        s._socketNotifier->sigActivated().connect( [this]( const auto & , auto ev){ onSocketActivated( ev );} );
        break;
      }
      case Socket::ClosingState: {
        if ( state() != Socket::ConnectedState ) {
          setError( Socket::InternalError, "Invalid state transition" );
          return false;
        }
        auto &s = _state.emplace<SocketPrivate::ClosingState>( std::move( std::get<ConnectedState>(_state)._writeBuffer) );
        s._socketNotifier = SocketNotifier::create( _socket, SocketNotifier::Write, true );
        s._socketNotifier->sigActivated().connect( [this]( const auto & , auto ev){ onSocketActivated( ev );} );
        break;
      }
      case Socket::ClosedState: {
        _state.emplace<SocketPrivate::ClosedState>();
        if ( _socket >= 0 )
          ::close( _socket );
        _socket = -1;
        _targetAddr.reset();
        z_func()->IODevice::close();
        _disconnected.emit();
        break;
      }
    }
    return true;
  }

  bool SocketPrivate::connectToHost()
  {
    auto &state = std::get<ConnectingState>( _state );

    const int res = eintrSafeCall( ::connect, _socket, _targetAddr->nativeSockAddr(), _targetAddr->size() );

    auto doDelayedConnect = [ this, &state ](){
      if ( !state._connectNotifier ) {
        state._connectNotifier = SocketNotifier::create( _socket, SocketNotifier::Write, true );
        state._connectNotifier->sigActivated().connect( [this]( const auto &, auto ev){ this->onSocketActivated( ev );} );
      }

      if ( !state._connectTimeout ) {
        state._connectTimeout = Timer::create();
        state._connectTimeout->sigExpired().connect( [this, &state ]( const auto &) {
          setError( Socket::ConnectionTimeout, "The connection timed out." );
          state._connectNotifier.reset();
          state._connectTimeout.reset();
        });
      }
      state._connectTimeout->setSingleShot( true );
      state._connectTimeout->start( 30000 );
      setError( Socket::ConnectionDelayed, "" );
      return false;
    };

    if ( res < 0 ) {
      switch ( errno ) {
        case EAGAIN: {
          if ( _targetAddr->nativeSockAddr()->sa_family == AF_UNIX ) {
            // the Servers backlog is full , we need to wait
            return doDelayedConnect();
          } else {
            setError( Socket::FailedSocketOperation, strerr_cxx() );
            z_func()->close();
            return false;
          }
          break;
        }
        case EINPROGRESS:
          return doDelayedConnect();
          break;

        default:
          if ( handleConnectError( errno ) == false ) {
            z_func()->close();
            return false;
          }
      }
    }

    // connected yay
    if ( !transition( Socket::ConnectedState ) ) {
      z_func()->close();
      return false;
    }
    return true;
  }

  int zyppng::SocketPrivate::rawBytesAvailable() const
  {
    if ( state() != Socket::ConnectedState )
      return 0;

    int value;
    if ( ioctl( _socket, FIONREAD, &value) >= 0 )
      return value;

    return 0;
  }

  bool SocketPrivate::readRawBytesToBuffer()
  {
    auto bytesToRead = rawBytesAvailable();
    if ( bytesToRead == 0 ) {
      // make sure to check if bytes are available even if the ioctl call returns something different
      bytesToRead = 4096;
    }

    char *buf = _readBuf.reserve( bytesToRead );
    const auto bytesRead = z_func()->readData( buf, bytesToRead );

    if ( bytesToRead > bytesRead )
      _readBuf.chop( bytesToRead-bytesRead );

    if ( bytesRead > 0 ) {
      _readyRead.emit();
      return true;
    }
    //handle remote close
    else if ( bytesRead == 0 ) {
      setError( Socket::ConnectionClosedByRemote, "The remote host closed the connection" );
      return false;
    }

    if ( errno == EAGAIN || errno == EWOULDBLOCK )
      return true;

    setError( Socket::InternalError, strerr_cxx() );
    return false;
  }

  bool SocketPrivate::writePendingData()
  {
    return std::visit( [this]( auto &s ){
      using T = std::decay_t<decltype (s)>;
      if constexpr ( std::is_same_v<T, ConnectedState> || std::is_same_v<T, ClosingState> ) {

        const auto nwrite = s._writeBuffer.frontSize();
        if ( !nwrite ) {
          // disable Write notifications so we do not wake up without the need to
          s._socketNotifier->setMode( SocketNotifier::Read | SocketNotifier::Error );
          return true;
        }

        const auto nBuf = s._writeBuffer.front();
        const auto written = eintrSafeCall( ::send, _socket, nBuf, nwrite, MSG_NOSIGNAL );
        if ( written == -1 ) {
          switch ( errno ) {
            case EACCES:
              setError( Socket::InsufficientPermissions, strerr_cxx() );
              return false;
            case EAGAIN:
#if EAGAIN != EWOULDBLOCK
            case EWOULDBLOCK:
#endif
              return true;
            case EPIPE:
            case ECONNRESET:
              setError( Socket::ConnectionClosedByRemote, strerr_cxx() );
              return false;
            default:
              setError( Socket::InternalError, strerr_cxx() );
              return false;
          }
          return false;
        }
        s._writeBuffer.discard( written );
        _sigBytesWritten.emit( written );
      }
      return true;
    }, _state );
  }

  /*!
   * Maps \a error into a \ref Socket::SocketError and sets it accordingly in
   * the socket. Returns false if the error is fatal and connection needs to stop.
   */
  bool zyppng::SocketPrivate::handleConnectError( int error )
  {
    switch ( error ) {
      case EACCES:
      case EPERM:
        setError( Socket::InsufficientPermissions, strerr_cxx() );
        return false;
      case EADDRINUSE:
        setError( Socket::AddressInUse, strerr_cxx() );
        return false;
      case EADDRNOTAVAIL:
        setError( Socket::AddressNotAvailable, strerr_cxx() );
        return false;
      case EAFNOSUPPORT:
        setError( Socket::AddressIssue, strerr_cxx() );
        return false;
      case ETIMEDOUT:
        setError( Socket::ConnectionTimeout, strerr_cxx() );
        return false;
      case EALREADY:
        setError( Socket::ConnectionDelayed, "" );
        return false;
      case ECONNREFUSED:
        setError( Socket::ConnectionRefused, strerr_cxx() );
        return false;
      case EBADF:
      case EFAULT:
      case ENOTSOCK:
        setError( Socket::InternalError, strerr_cxx() ); // this can only happen if we screw up
        return false;
      case ENETUNREACH:
        setError( Socket::NetworkUnreachable, strerr_cxx() );
        return false;
      case EPROTOTYPE:
        setError ( Socket::InvalidSocketOptions, strerr_cxx() );
        return false;
      case EISCONN:
        break;
    }
    return true;
  }


  void SocketPrivate::onSocketActivated( int ev )
  {
    std::visit( [ this, &ev ] ( const auto &currState ) {
      using T = std::decay_t<decltype(currState)>;
      if constexpr ( std::is_same<ConnectingState, T>() ) {
        if ( (ev & SocketNotifier::Write) == SocketNotifier::Write ) {
          if ( this->_targetAddr->nativeSockAddr()->sa_family == AF_UNIX ) {
            // for AF_UNIX sockets we just call connect again
            this->connectToHost();
            return;
          } else {

            // for others we check with getsockopt as mentioned in connect(2) if the conn was successful
            int err = 0;
            socklen_t errSize = sizeof ( err );
            ::getsockopt( _socket, SOL_SOCKET, SO_ERROR, &err, &errSize );

            if ( err == 0 || err == EISCONN ) {
              transition( Socket::ConnectedState );
            } else {
              if ( err == EINPROGRESS || err == EAGAIN || err == EALREADY )
                return;
              handleConnectError( err );
              z_func()->abort();
            }
          }
        }

      } else if constexpr ( std::is_same<ConnectedState, T>() ) {
        if ( (ev & SocketNotifier::Write) == SocketNotifier::Write ) {
          if ( !writePendingData() ) {
            z_func()->abort();
            return;
          }
        }
        if ( (ev & SocketNotifier::Read) == SocketNotifier::Read ) {
          if ( !readRawBytesToBuffer() ) {
            z_func()->abort();
            return;
          }
        }
        if ( (ev & SocketNotifier::Error) == SocketNotifier::Error ) {
          return;
        }

      } else if constexpr ( std::is_same<ClosingState, T>() ) {

        if ( (ev & SocketNotifier::Write) == SocketNotifier::Write ) {
          if ( !writePendingData() ) {
            z_func()->abort();
            return;

          }

          if ( currState._writeBuffer.size() == 0 ) {
            transition( Socket::ClosedState );
          }
        }

      } else if constexpr ( std::is_same<ListeningState, T>() ) {

        //signal that we have pending connections
        _incomingConnection.emit();

      } else {
        DBG << "Unexpected state on socket activation" << std::endl;
      }
    },_state);
  }

  Socket::SocketState SocketPrivate::state() const
  {
    return std::visit([]( const auto &s ) constexpr { return s.type(); }, _state );
  }

  Socket::Socket( int domain, int type, int protocol )
    : IODevice( *( new SocketPrivate( domain, type, protocol )))
  { }

  Socket::~Socket()
  {
    this->abort();
  }

  Socket::Ptr Socket::create( int domain, int type, int protocol )
  {
    return Ptr( new Socket( domain, type, protocol ) );
  }

  bool Socket::bind( std::shared_ptr<SockAddr> addr )
  {
    Z_D();
    if ( !addr || !d->initSocket() )
      return false;

    int res = ::bind( d->_socket, addr->nativeSockAddr(), addr->size() );
    if ( res >= 0) return true;

    switch ( errno ) {
      case EACCES:
        d->setError( Socket::InsufficientPermissions, strerr_cxx() );
        break;
      case EADDRINUSE:
        d->setError( Socket::AddressInUse, strerr_cxx() );
        break;
      case EBADF:
      case ENOTSOCK:
      case EFAULT:
        d->setError( Socket::InternalError, strerr_cxx() ); // this can only happen if we screw up
        break;
      case EINVAL:
        d->setError( Socket::SocketAlreadyBound, strerr_cxx() );
        break;
      case EADDRNOTAVAIL:
        d->setError( Socket::AddressNotAvailable, strerr_cxx() );
        break;
      case ELOOP:
      case ENAMETOOLONG:
      case ENOENT:
      case ENOTDIR:
      case EROFS:
        d->setError( Socket::AddressIssue, strerr_cxx() );
        break;
      case ENOMEM:
        d->setError( Socket::InsufficientRessources, strerr_cxx() );
        break;
      default:
        d->setError( Socket::UnknownSocketError, strerr_cxx() );
        break;
    }

    abort();
    return false;
  }

  bool Socket::listen(int backlog)
  {
    Z_D();
    if ( !d->initSocket() )
      return false;

    int res = ::listen( d->_socket, backlog );
    if ( res >= 0 ) {
      d->transition( Socket::ListeningState );
      return true;
    }

    switch ( errno ) {

      case EADDRINUSE:
        d->setError( Socket::AddressInUse, strerr_cxx() );
        break;
      case EBADF:
      case ENOTSOCK:
        d->setError( Socket::InternalError, strerr_cxx() ); // this can only happen if we screw up
        break;
      case EOPNOTSUPP:
        d->setError( Socket::OperationNotSupported, strerr_cxx() );
        break;

    }
    return false;
  }

  Socket::Ptr Socket::accept()
  {
    Z_D();
    if ( d->_socket == -1 )
      return nullptr;

    //accept new pending connections
    const auto res = eintrSafeCall( ::accept4, d->_socket, (struct sockaddr*)nullptr, (socklen_t *)nullptr, SOCK_CLOEXEC );
    if ( res < 0 ) {
      switch ( errno ) {
#if EAGAIN != EWOULDBLOCK
        case EWOULDBLOCK:
#endif
        case EAGAIN:
        case ECONNABORTED:
          return nullptr;
          break;
        default:
          d->setError( Socket::InternalError, strerr_cxx() );
          break;
      }
    }

    return Socket::fromSocket( res, Socket::ConnectedState );
  }

  Socket::Ptr Socket::fromSocket( int fd, Socket::SocketState state )
  {

    int domain;
    socklen_t optlen = sizeof(domain);
    int res = getsockopt( fd, SOL_SOCKET, SO_DOMAIN, &domain, &optlen );
    if ( res < 0 ) {
      DBG << "Error querying socket domain: " << strerr_cxx() << std::endl;
      ::close(fd);
      return nullptr;
    }

    int protocol;
    optlen = sizeof(domain);
    res = getsockopt( fd, SOL_SOCKET, SO_PROTOCOL, &protocol, &optlen );
    if ( res < 0 ) {
      DBG << "Error querying socket protocol: " << strerr_cxx() << std::endl;
      ::close(fd);
      return nullptr;
    }

    int type;
    optlen = sizeof(domain);
    res = getsockopt( fd, SOL_SOCKET, SO_TYPE, &type, &optlen );
    if ( res < 0 ) {
      DBG << "Error querying socket type: " << strerr_cxx() << std::endl;
      ::close(fd);
      return nullptr;
    }

    // from here on the Socket instance owns the fd, no need to manually close it
    // in case of error
    auto sptr = Socket::create( domain, type, protocol );
    sptr->d_func()->_socket = fd;

    // make sure the socket is non blocking
    if ( !sptr->setBlocking( false ) ) {
      DBG << "Failed to unblock socket." << std::endl;
      return nullptr;
    }

    if( sptr->d_func()->transition( state ) )
      return sptr;

    return nullptr;
  }

  bool Socket::setBlocking( const bool set )
  {
    Z_D();

    if ( !d->initSocket() )
      return false;

    const int oldFlags = fcntl( d->_socket, F_GETFL, 0 );
    if (oldFlags == -1) return false;

    const int flags = set ? ( oldFlags & ~(O_NONBLOCK) ) : ( oldFlags | O_NONBLOCK );

    // no need to do a syscall if we do not change anything
    if ( flags == oldFlags )
      return true;

    if ( fcntl( d->_socket, F_SETFL, flags ) != 0) {
      return false;
    }
    return true;
  }

  bool Socket::connect( std::shared_ptr<SockAddr> addr )
  {
    Z_D();

    if ( !addr || d->state() != Socket::InitialState )
      return false;

    if ( !d->initSocket() )
      return false;

    d->_targetAddr = addr;
    if ( !d->transition( Socket::ConnectingState ) ) {
      abort();
      return false;
    }

    return d->state() == Socket::ConnectedState;
  }

  int Socket::nativeSocket() const
  {
    Z_D();
    return d->_socket;
  }

  Socket::SocketError Socket::lastError() const
  {
    Z_D();
    return d->_error;
  }

  void Socket::abort()
  {
    Z_D();
    d->transition( ClosedState );
  }

  void Socket::close()
  {
    disconnect();
  }

  void Socket::disconnect()
  {
    Z_D();
    std::visit([&d]( const auto &s ){
      using Type = std::decay_t<decltype (s)>;
      if constexpr ( std::is_same_v<Type, SocketPrivate::ConnectedState > ) {
        // we still have pending data, we need to wait for it to be written
        if ( s._writeBuffer.size() ) {
          d->transition( Socket::ClosingState );
          return;
        }
      }
      d->transition( Socket::ClosedState );
    }, d->_state );

  }

  off_t Socket::writeData( const char *data, off_t count )
  {
    Z_D();
    if ( d->state() != SocketState::ConnectedState )
      return 0;

    auto &s = std::get<SocketPrivate::ConnectedState>( d->_state );

    // if the write buffer has already data we need to append to it to keep the correct order
    if ( s._writeBuffer.size() ) {
      s._writeBuffer.append( data, count );
      //lets try to write some of it
      d->writePendingData();
      return count;
    }

    const auto written = eintrSafeCall( ::send, d->_socket, data, count, MSG_NOSIGNAL );
    if ( written >= 0 && written < count ) {
      // append the rest of the data to the buffer, so we can return always the full count
      s._writeBuffer.append( data + written, count - written );
      s._socketNotifier->setMode( SocketNotifier::Read | SocketNotifier::Write | SocketNotifier::Error );
      d->_sigBytesWritten.emit( written );
    }
    return count;
  }

  bool Socket::waitForConnected( int timeout )
  {
    Z_D();
    if ( d->state() == Socket::ConnectedState )
      return true;
    // we can only wait if we are in connecting state
    while ( d->state() == Socket::ConnectingState ) {
      int rEvents = 0;
      if ( EventDispatcher::waitForFdEvent( d->_socket, AbstractEventSource::Write, rEvents, timeout ) ) {
        d->onSocketActivated( rEvents );
      }
    }
    return d->state() == Socket::ConnectedState;
  }

  bool Socket::waitForAllBytesWritten( int timeout )
  {
    Z_D();

    bool canContinue = true;
    bool bufferEmpty = false;

    const auto bytesWrittenCB = [ & ]( const auto ){
      std::visit([&]( const auto &s ){
        using T = std::decay_t<decltype (s)>;
        if constexpr ( std::is_same_v<T, SocketPrivate::ConnectedState> || std::is_same_v<T, SocketPrivate::ClosingState> ) {
          bufferEmpty = (s._writeBuffer.size() == 0);
          if ( bufferEmpty ) canContinue = false;
        }
      }, d->_state );
    };

    zypp::AutoDispose<sigc::connection> conn( d->_sigBytesWritten.connect( bytesWrittenCB ), []( auto conn ) { conn.disconnect(); } );

    while ( canContinue && !bufferEmpty ) {

      if ( d->state() != Socket::ConnectedState &&  d->state() != Socket::ClosingState)
        return false;

      std::visit([&]( const auto &s ){
        using T = std::decay_t<decltype (s)>;
        if constexpr ( std::is_same_v<T, SocketPrivate::ConnectedState> || std::is_same_v<T, SocketPrivate::ClosingState> ) {
          if ( s._writeBuffer.size() == 0 ) {
            canContinue = false;
            bufferEmpty = true;
          } else {
            int rEvents = 0;
            if ( EventDispatcher::waitForFdEvent( d->_socket, AbstractEventSource::Write | AbstractEventSource::Read, rEvents, timeout ) ) {
              //this will trigger the bytesWritten signal, we check there if the buffer is empty
              d->onSocketActivated( rEvents );
            }
          }
        }
      }, d->_state );
    }
    return bufferEmpty;
  }

  off_t Socket::readData( char *buffer, off_t bufsize )
  {

    Z_D();
    if ( d->state() != SocketState::ConnectedState )
      return -1;

    return ::read( d->_socket, buffer, bufsize );
  }

  size_t Socket::bytesAvailable() const
  {
    Z_D();
    return IODevice::bytesAvailable() + d->rawBytesAvailable();
  }

  Socket::SocketState Socket::state() const
  {
    return d_func()->state();
  }

  SignalProxy<void ()> Socket::sigReadyRead()
  {
    return d_func()->_readyRead;
  }

  SignalProxy<void ()> Socket::sigIncomingConnection()
  {
    return d_func()->_incomingConnection;
  }

  SignalProxy<void ()> Socket::sigConnected()
  {
    return d_func()->_connected;
  }

  SignalProxy<void ()> Socket::sigDisconnected()
  {
    return d_func()->_disconnected;
  }

  SignalProxy<void (Socket::SocketError)> Socket::sigError()
  {
    return d_func()->_sigError;
  }

  SignalProxy<void (std::size_t)> Socket::sigBytesWritten()
  {
    return d_func()->_sigBytesWritten;
  }

}
