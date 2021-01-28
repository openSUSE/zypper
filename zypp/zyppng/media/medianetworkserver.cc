/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#include "private/medianetworkserver_p.h"
#include <zypp/zyppng/media/network/downloader.h>
#include <zypp/zyppng/media/network/downloadspec.h>
#include <zypp/zyppng/media/network/private/mirrorcontrol_p.h>
#include <zypp-core/zyppng/base/private/threaddata_p.h>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <zypp-core/zyppng/base/EventLoop>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-core/zyppng/base/SocketNotifier>

#include <zypp/media/TransferSettings.h>
#include <zypp/PathInfo.h>

#include <zypp-proto/envelope.pb.h>
#include <zypp-proto/messages.pb.h>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::MediaNetworkServer"
#include <zypp/zyppng/media/network/private/mediadebug_p.h>

#include <algorithm>

namespace zyppng {

  using HeaderSizeType = uint32_t;

  MediaNetworkServer::MediaNetworkServer( )
    : _downloadManager( std::make_shared<Downloader>( MirrorControl::create() ) )
  {
  }

  void MediaNetworkServer::listen( const std::string &sockPath )
  {
    if ( _serverSocket )
      return;

    _serverSocket = zyppng::Socket::create( AF_UNIX, SOCK_STREAM, 0 );

    // bind to a abstract unix domain socket address, which means we do not need to care about cleaning it up
    _serverSocket->bind( std::make_shared<zyppng::UnixSockAddr>( sockPath, true ) );
    _serverSocket->Base::connect( &zyppng::Socket::sigIncomingConnection, *this, &MediaNetworkServer::onIncomingConnection );
    _serverSocket->listen();

    MIL << "MediaNetworkServer started to listen for connections " << std::endl;
  }

  std::shared_ptr<Downloader> MediaNetworkServer::downloader()
  {
    return _downloadManager;
  }

  void MediaNetworkServer::onIncomingConnection()
  {
    auto sock = _serverSocket->accept();
    if ( !sock )
      return;

    MIL << "MediaNetworkServer received new connection " << sock.get() << std::endl;

    auto client = std::make_shared<MediaNetworkConn>( *this, std::move(sock) );
    client->connectFunc( &MediaNetworkConn::sigDisconnected, [ this, wp = std::weak_ptr(client) ](){
      auto p = wp.lock();
      if (!p) {
        MIL_MEDIA << "MediaNetworkServer client disconnected but the pointer was already deleted" << std::endl;
        return;
      }

      MIL << "MediaNetworkServer client disconnected " << p.get() << std::endl;

      _clients.erase(
        std::remove_if( _clients.begin(), _clients.end(), [ &p ]( auto listElem ){
          return p.get() == listElem.get();
        }
      ));
    }, *this);

    _clients.push_back( client );
  }

  struct MediaNetworkConn::Request {
    Request ( ) { }
    ~Request() {
      clearConnections();
    }

    void clearConnections () {
      std::for_each( trackingConns.begin(), trackingConns.end(), []( auto &conn){ conn.disconnect(); } );
      trackingConns.clear();
    }

    zypp::proto::Request request;
    std::shared_ptr<zyppng::Download> dl;
    std::vector<sigc::connection> trackingConns;
  };


  MediaNetworkConn::MediaNetworkConn( MediaNetworkServer &server, std::shared_ptr<Socket> &&socket ) : _server(server), _connection ( std::move( socket ) )
  {
    MIL_MEDIA << "Initializing Connection object " << std::endl;
    _connection->Base::connect( &Socket::sigReadyRead, *this, &MediaNetworkConn::onReadyRead );
    _connection->Base::connect( &Socket::sigDisconnected, *this, &MediaNetworkConn::onDisconnected );
    _connection->Base::connect( &Socket::sigError, *this, &MediaNetworkConn::onError );

    //make sure we read possibly available data
    onReadyRead();
  }

  MediaNetworkConn::~MediaNetworkConn()
  {
    MIL_MEDIA << "Closing connection " << std::endl;
    while( _requests.size() ) {
      auto &req = _requests.front();
      // make sure we do not receive signals while shutting down
      // this would break due to weak_from_this
      req->clearConnections();
      req->dl->cancel();

      // still make sure we actually send all signals about finished downloads.
      trackedDlFinished( *req );
    }
    MIL_MEDIA << "Closing connection done!" << std::endl;
  }

  SignalProxy<void ()> MediaNetworkConn::sigDisconnected()
  {
    return _disconnected;
  }

  void MediaNetworkConn::onDisconnected()
  {
    MIL_MEDIA << "Socket was closed, requesting cleanup." << std::endl;
    _disconnected.emit();
  }

  void MediaNetworkConn::onReadyRead()
  {
    const auto &sendStatus = [this]( RequestId id, auto code, const auto &reason) {
      zypp::proto::Status status;
      status.set_requestid( id );
      status.set_code( code );
      status.set_reason( reason );
      sendMessage( status );
    };

    const auto &makeNewRequest = [this] ( zypp::proto::Request &&req ){
      auto newRequest = std::make_shared<Request>( );
      newRequest->request = std::move( req );
      newRequest->dl = _server.downloader()->downloadFile( newRequest->request.spec() );
      newRequest->dl->start();

      if ( newRequest->request.prioritize() )
        newRequest->dl->prioritize();

      trackRequest( *newRequest );

      _requests.push_back( std::move(newRequest) );
      return _requests.back();
    };

    // read until buffers are empty
    while ( _connection->bytesAvailable() ) {

      // MIL << "Server has bytes" << std::endl;

      if ( !_pendingMessageSize ) {
        // if we cannot read the message size wait some more
        if ( _connection->bytesAvailable() < sizeof( RequestId ) )
          return;

        HeaderSizeType msgSize;
        _connection->read( reinterpret_cast<char *>( &msgSize ), sizeof( decltype (msgSize) ) );
        _pendingMessageSize = msgSize;
      }

      // wait for the full message size to be available
      if ( _connection->bytesAvailable() < static_cast<size_t>( *_pendingMessageSize ) )
        return;

      ByteArray message = _connection->read( *_pendingMessageSize );
      _pendingMessageSize.reset();

      zypp::proto::Envelope m;
      if (! m.ParseFromArray( message.data(), message.size() ) ) {
        //send error and close, we can not recover from this. Bytes might be mixed up on the socket
        sendStatus( -1, zypp::proto::Status::InvalidMessage, "This message is misformed." );
        _connection->close();
        return;
      }

      //DBG << "Server recv: " << m.messagetypename() << " " << m.value().size() << std::endl;

      const auto &mName = m.messagetypename();
      if (  mName == "zypp.proto.Request" )  {

        zypp::proto::Request prefetchReq;
        if (! prefetchReq.ParseFromString( m.value() ) ) {
          sendStatus( -1, zypp::proto::Status::MalformedRequest, "Unable to deserialize the request." );
          continue;
        }
        const auto &r = makeNewRequest( std::move(prefetchReq) );
        sendStatus( r->request.requestid(), zypp::proto::Status::Ok, "OK" );

      } else if ( mName == "zypp.proto.Prefetch" )  {

        zypp::proto::Prefetch prefetchReq;
        if (! prefetchReq.ParseFromString( m.value() ) ) {
          sendStatus( -1, zypp::proto::Status::MalformedRequest, "Unable to deserialize the request." );
          continue;
        }

        for ( auto &file : *prefetchReq.mutable_requests() ) {
          makeNewRequest( std::move(file) );
        }
        sendStatus( prefetchReq.requestid(), zypp::proto::Status::Ok, "OK" );

      } else if ( mName == "zypp.proto.CancelDownload" )  {
        zypp::proto::CancelDownload cancel;
        if (! cancel.ParseFromString( m.value() ) ) {
          sendStatus( -1, zypp::proto::Status::MalformedRequest, "Unable to deserialize the request." );
          continue;
        }

        for ( auto &req : _requests ) {
          if ( req->request.requestid() == cancel.requestid() ) {
            req->dl->cancel();
            break;
          }
        }
        sendStatus( cancel.requestid(), zypp::proto::Status::Ok, "OK" );

      } else if ( mName == "zypp.proto.NewAuthDataAvailable" )  {
        zypp::proto::NewAuthDataAvailable data;
        if (! data.ParseFromString( m.value() ) ) {
          sendStatus( -1, zypp::proto::Status::MalformedRequest, "Unable to deserialize the request." );
          continue;
        }

        MIL_MEDIA << "Got new Auth data, restarting failed requests" << std::endl;

        // we got new auth data in the CredentialManager, lets restart all our requests that are currently
        // in failed auth state.
        for ( auto &req : _requests ) {
          if ( req->dl->state() == Download::Finished && req->dl->hasError()
               && ( req->dl->lastRequestError().type() == NetworkRequestError::AuthFailed || req->dl->lastRequestError().type() == NetworkRequestError::Unauthorized ) ) {
            MIL_MEDIA << "Found request waiting for Auth, restarting " << req->dl->spec().url() << std::endl;
            req->dl->spec().settings().protoData() = data.settings();
            req->dl->start();
          }
        }
        sendStatus( data.requestid(), zypp::proto::Status::Ok, "OK" );

      } else if ( mName == "zypp.proto.SubscribeProgress" )  {
        zypp::proto::SubscribeProgress sub;
        if (! sub.ParseFromString( m.value() ) ) {
          sendStatus( -1, zypp::proto::Status::MalformedRequest, "Unable to deserialize the request." );
          continue;
        }

        bool found = false;
        for ( auto &req : _requests ) {
          if ( req->request.requestid() == sub.requestid() ) {
            req->request.set_streamprogress( true );
            if ( sub.prioritize() )
              req->dl->prioritize();
            sendStatus( sub.requestid(), zypp::proto::Status::Ok, "OK" );
            found = true;
            break;
          }
        }

        if ( !found )
          sendStatus( sub.requestid(), zypp::proto::Status::UnknownId, "Could not find the request ID in running requests" );

      } else if ( mName == "zypp.proto.UnSubscribeProgress" )  {
        zypp::proto::UnSubscribeProgress unSub;
        if (! unSub.ParseFromString( m.value() ) ) {
          sendStatus( -1, zypp::proto::Status::MalformedRequest, "Unable to deserialize the request." );
          return;
        }

        bool found = false;
        for ( auto &req : _requests ) {
          if ( req->request.requestid() == unSub.requestid() ) {
            req->request.set_streamprogress( false );
            found = true;
            sendStatus( unSub.requestid(), zypp::proto::Status::Ok, "OK" );
            break;
          }
        }
        if ( !found )
          sendStatus( unSub.requestid(), zypp::proto::Status::UnknownId, "Could not find the request ID in running requests" );

      } else {
        sendStatus( -1, zypp::proto::Status::UnknownRequest, "This request type is not known." );
      }
    }
  }

  void MediaNetworkConn::onError( Socket::SocketError err )
  {
    MIL << "Socket received error: " << err << std::endl;
    _connection->disconnect();
  }

  void MediaNetworkConn::trackRequest( Request &r )
  {
    r.trackingConns = {
      r.dl->connectFunc( &Download::sigStarted, [ this, &r ]( auto & ) {
        this->signalRequestStarted( r );
      }, *this),

      r.dl->connectFunc( &Download::sigAlive, [ this, &r ]( auto &, off_t now ) {

          if ( !r.request.streamprogress() )
            return;

          zypp::proto::DownloadProgress prog;
          prog.set_requestid( r.request.requestid() );
          prog.set_url( r.request.spec().url() );
          prog.set_now( now );
          this->sendMessage( prog );
        }, *this ),

      r.dl->connectFunc( &Download::sigProgress, [ this, &r ]( auto &, off_t total, off_t now ) {

        if ( !r.request.streamprogress() ) {
          return;
        }

        zypp::proto::DownloadProgress prog;
        prog.set_requestid( r.request.requestid() );
        prog.set_url( r.request.spec().url() );
        prog.set_now( now );
        prog.set_total( total );
        this->sendMessage( prog );
      }, *this ),
      r.dl->connectFunc( &Download::sigFinished, [ this, &r ]( auto & ) {
        this->trackedDlFinished( r );
      }, *this )
    };
  }

  void MediaNetworkConn::signalRequestStarted(Request &r)
  {
    zypp::proto::DownloadStart start;
    start.set_requestid( r.request.requestid() );
    start.set_url( r.request.spec().url() );
    this->sendMessage( start );
  }

  void MediaNetworkConn::trackedDlFinished( MediaNetworkConn::Request &r )
  {
    MIL << "Download finished by MediaNetworkServer: " << r.request.spec().url() << std::endl;

    zypp::proto::DownloadFin fin;
    fin.set_requestid( r.request.requestid() );
    if ( r.dl->lastAuthTimestamp() > 0 )
      fin.set_last_auth_timestamp( r.dl->lastAuthTimestamp() );
    r.clearConnections();

    std::string err;
    if ( !r.dl->hasError() ) {
      fin.clear_error();
    } else {
      const auto &lerr = r.dl->lastRequestError();
      fin.mutable_error()->set_error( lerr.type() );
      fin.mutable_error()->set_errordesc( err );
      fin.mutable_error()->set_nativeerror( lerr.nativeErrorString() );

      // this is rather ugly, we need to convert the extra infos to string
      // we will only handle the ones the client actually cares about
      fin.mutable_error()->mutable_extra_info()->insert( { "requestUrl", lerr.extraInfoValue("requestUrl", r.dl->spec().url()).asString() } );
      if ( lerr.type() == NetworkRequestError::Unauthorized )
        fin.mutable_error()->mutable_extra_info()->insert( { "authHint", lerr.extraInfoValue("authHint", std::string()) } );
    }

    this->sendMessage( fin );

    MIL_MEDIA << "Download was finished, releasing all ressources." << std::endl;
    _requests.erase (
      std::remove_if( _requests.begin(), _requests.end(), [ &r ]( const auto &reqInList ){
        return reqInList.get() == &r;
      }
    ));
    return;
    // ATTENTION, r will be dangling from here on out
  }

  template< typename T >
  void MediaNetworkConn::
  sendMessage( T &m )
  {
    zypp::proto::Envelope env;
    env.set_messagetypename( m.GetTypeName() );
    m.SerializeToString( env.mutable_value() );

    //DBG << "Sending message\n" << env.DebugString() << std::endl;

    std::string data = env.SerializeAsString();
    const HeaderSizeType size = data.size();

    _connection->write( reinterpret_cast<const char *>( &size ), sizeof(decltype(size)) );
    _connection->write( data.data(), size );
  }

  MediaNetworkThread::MediaNetworkThread()
  {
    MIL_MEDIA << "Initializing MediaNetworkThread" << std::endl;
    //start up thread
    _t = std::thread( [this](){ threadMain(); });
  }

  void MediaNetworkThread::threadMain()
  {
    // force the kernel to pick another thread to handle those signals
    zyppng::blockSignalsForCurrentThread( { SIGTERM, SIGINT, SIGPIPE, } );

    zyppng::ThreadData::current().setName("Zypp-MediaNetwork");

    auto dispatch = zyppng::EventLoop::create();

    auto server = std::make_shared<MediaNetworkServer>();
    server->listen( sockPath() );

    // we are using a pipe to wake up from the event loop, the SocketNotifier will throw a signal every
    // time there is data available
    auto sNotify = _shutdownSignal.makeNotifier( false );
    sNotify->connectFunc( &zyppng::SocketNotifier::sigActivated, [&dispatch]( const zyppng::SocketNotifier &, int ) {
      dispatch->quit();
    });
    sNotify->setEnabled( true );

    MIL_MEDIA << "Starting event loop " << std::endl;

    dispatch->run();

    MIL_MEDIA << "Thread exit " << std::endl;
  }

  MediaNetworkThread::~MediaNetworkThread()
  {
    _shutdownSignal.notify();
    _t.join();
  }

  MediaNetworkThread &MediaNetworkThread::instance()
  {
    static MediaNetworkThread t;
    return t;
  }

}
