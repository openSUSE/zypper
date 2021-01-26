/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#include "medianetwork.h"
#include "private/medianetworkserver_p.h"

#include <zypp/media/MediaException.h>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <zypp-core/zyppng/base/EventLoop>
#include <zypp-core/zyppng/base/AutoDisconnect>
#include <zypp-core/zyppng/io/SockAddr>
#include <zypp/zyppng/media/network/downloadspec.h>

#include <zypp/zyppng/media/network/networkrequestdispatcher.h>
#include <zypp/zyppng/media/network/networkrequesterror.h>
#include <zypp/zyppng/media/network/AuthData>
#include <zypp/zyppng/media/network/private/mediadebug_p.h>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-core/zyppng/base/AbstractEventSource>

#include <zypp/media/CurlHelper.h>
#include <zypp/media/MediaUserAuth.h>
#include <zypp/media/MediaException.h>
#include <zypp/media/CredentialManager.h>
#include <zypp/ZConfig.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/base/String.h>
#include <zypp/base/Gettext.h>
#include <zypp/ZYppFactory.h>

#include <zypp/ZYppCallbacks.h>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::MediaNetwork"

namespace zyppng {

  using HeaderSizeType = uint32_t;

  struct MediaNetwork::ProgressData
  {
    ProgressData( const zyppng::Url & _url = zyppng::Url(),
      zypp::callback::SendReport<zypp::media::DownloadProgressReport> *_report = nullptr )
      : url( _url )
      , report( _report )
    {}

    zypp::Url	url;
    zypp::callback::SendReport<zypp::media::DownloadProgressReport> *report;

    time_t _timeStart	= 0;	///< Start total stats
    time_t _timeLast	= 0;	///< Start last period(~1sec)

    double _dnlTotal	= 0.0;	///< Bytes to download or 0 if unknown
    double _dnlLast	= 0.0;	///< Bytes downloaded at period start
    double _dnlNow	= 0.0;	///< Bytes downloaded now

    int    _dnlPercent= 0;	///< Percent completed or 0 if _dnlTotal is unknown

    double _drateTotal= 0.0;	///< Download rate so far
    double _drateLast	= 0.0;	///< Download rate in last period

    void updateStats( double dltotal = 0.0, double dlnow = 0.0 )
    {
      time_t now = time(0);

      // If called without args (0.0), recompute based on the last values seen
      if ( dltotal && dltotal != _dnlTotal )
        _dnlTotal = dltotal;

      if ( dlnow && dlnow != _dnlNow )
      {
        _dnlNow = dlnow;
      }
      else if ( !_dnlNow && !_dnlTotal )
      {
        // Start time counting as soon as first data arrives.
        // Skip the connection / redirection time at begin.
        return;
      }

      // init or reset if time jumps back
      if ( !_timeStart || _timeStart > now )
        _timeStart = _timeLast = now;

      // percentage:
      if ( _dnlTotal )
        _dnlPercent = int(_dnlNow * 100 / _dnlTotal);

      // download rates:
      _drateTotal = _dnlNow / std::max( int(now - _timeStart), 1 );

      if ( _timeLast < now )
      {
        _drateLast = (_dnlNow - _dnlLast) / int(now - _timeLast);
        // start new period
        _timeLast  = now;
        _dnlLast   = _dnlNow;
      }
      else if ( _timeStart == _timeLast )
        _drateLast = _drateTotal;
    }

    void reportProgress( bool &cancel ) const
    {
      if ( report && !(*report)->progress( _dnlPercent, url, _drateTotal, _drateLast ) )
        cancel = true;	// user requested abort
    }
  };


  struct MediaNetwork::Request {

    Request ( DownloadSpec &&spec, const zypp::Pathname &path, RequestId id ) : _targetFile(path) {
      _targetFile.autoCleanup( true );
      _proto.set_requestid( id );

      // download to temporary path instead of the real one
      spec.setTargetPath( _targetFile.path().asString() );
      *_proto.mutable_spec() = std::move( spec.protoData() );
    }

    void ref   () {
      _requestCount++;
    }

    int unref () {
      _requestCount--;
      return _requestCount;
    }

    void startReporting ( ProgressData &tracker ) {
      _report = &tracker;
    }

    void stopReporting  () {
      _report = nullptr;
    }

    void setProgress ( double dltotal, double dlnow, bool &cancel ) {
      if ( _report ) {
        _report->updateStats( dltotal, dlnow );
        _report->reportProgress( cancel );
      }
    }

    void reset () {
      _result.reset();
    }

    const std::optional<zypp::proto::DownloadFin> &result () const {
      return _result;
    }

    void setResult ( zypp::proto::DownloadFin &&res ) {
      _result = std::move(res);
    }

    zypp::proto::Request &proto () {
      return _proto;
    }

    const zypp::proto::Request &proto () const {
      return _proto;
    }

  private:
    zypp::proto::Request _proto;
    std::optional<zypp::proto::DownloadFin> _result;
    zypp::filesystem::TmpFile _targetFile;
    ProgressData *_report = nullptr;
    int _requestCount = 1;
  };


  /*!
 * This object helps us to simulate sync behaviour over async code. We can block waiting
 * for a specific message or response while still handling intermediate message that represent
 * status updates from concurrently running requests.
 *
 * The DispatchContext needs to be released after finishing the operation that was requested from
 * the sync entry points are done. It will tear down the event loop and release the socket fd so we
 * can reuse it at a later point again.
 */
  struct MediaNetwork::DispatchContext {

    DispatchContext( MediaNetwork &p ) : parent(&p), ev( zyppng::EventLoop::create() ){
      if ( parent->_socket ) {
        MIL_MEDIA << "Reusing open connection" << std::endl;
        sock = zyppng::Socket::fromSocket( *parent->_socket, zyppng::Socket::ConnectedState );
      } else {
        MIL_MEDIA << "Connecting to backend" << std::endl;
        sock = zyppng::Socket::create(  AF_UNIX, SOCK_STREAM, 0 );
        parent->_readBuffer.clear();

        while ( true ) {
          sock->connect( std::make_shared<zyppng::UnixSockAddr>( MediaNetworkThread::instance().sockPath(), true ) );
          if ( !sock->waitForConnected() ) {
            if ( sock->lastError() == Socket::ConnectionRefused )
              continue;
            sock.reset();
          } else {
            parent->_socket = sock->nativeSocket();
            MIL_MEDIA << "Connected" << std::endl;
          }
          break;
        }

      }
    }
    ~DispatchContext() {
      // if the socket was disconnected we can not keep the fd around anymore
      if ( !sock || sock->state() == Socket::ClosedState ) {
        parent->_socket.reset();

        // if the socket is destroyed the read buffer won't make sense anymore
        parent->_readBuffer.clear();
      } else {
        // make sure no data is lost before we release the socket descriptor
        if ( sock->bytesAvailable() )
          parent->_readBuffer.append( sock->readAll() );
        sock->waitForAllBytesWritten();
        sock->releaseSocket();
      }
    }

    /*!
   * Send a messagee to the server side, it will be enclosed in a Envelope
   * and immediately sent out.
   */
    template <typename T>
    bool sendEnvelope ( const T &m ) {
      zypp::proto::Envelope env;

      env.set_messagetypename( m.GetTypeName() );
      m.SerializeToString( env.mutable_value() );

      //DBG << "Preparing to send messagE: " << env.messagetypename() << " " << env.value().size() << std::endl;

      const auto &str = env.SerializeAsString();
      HeaderSizeType msgSize = str.length();
      sock->write( (char *)(&msgSize), sizeof( HeaderSizeType ) );
      sock->write( str.data(), str.size() );
      return true;
    }

    /*!
   * Executes the EventLoop until a zypp.proto.Status message was received. The
   * Status is returned to the caller. The \a handleOtherMsg callback works just like
   * in \ref waitFor.
   */
    template <typename T>
    zypp::proto::Status waitForStatus ( const RequestId reqId, T &&handleOtherMsg ) {
      zypp::proto::Status res;
      dispatchUntil( [ &reqId, &res, otherMsgCB = std::forward<T>(handleOtherMsg), this ]( const zypp::proto::Envelope &env ){
        if ( env.messagetypename() == "zypp.proto.Status" ) {
          zypp::proto::Status s;
          s.ParseFromString( env.value() );

          // we received a out of band response, there is only ONE active request to the
          // server allowed at any time
          if ( s.requestid() != reqId ) {
            ERR_MEDIA << "Out of band status for a request that was not active" << std::endl;
            ZYPP_THROW( zypp::media::MediaException( "Out of band status for a request that was not active" ) );
          }
          res = std::move(s);
          return true;
        }
        // not the message we wanted, process it otherwise
        otherMsgCB( *this, env );
        return false;
      });
      return res;
    }

    /*!
   * Executes the EventLoop until a Message of type T was received, the
   * received message is returned to the caller.
   * The \a handleOtherMsg callback is invoked for all messages that are received
   * but are of a different type than the one requested.
   */
    template <typename T, typename Callback>
    T waitFor ( const RequestId reqId, Callback &&handleOtherMsg ) {
      T res;
      dispatchUntil( [ &reqId, &res, otherMsgCB = std::forward<Callback>(handleOtherMsg), this ]( const zypp::proto::Envelope &env ){
        if ( env.messagetypename() == T::default_instance().GetTypeName() ) {
          T s;
          s.ParseFromString( env.value() );
          if ( s.requestid() == reqId ) {
            res = std::move(s);
            return true;
          }
        }
        // not the message we wanted, process it otherwise
        otherMsgCB( *this, env );
        return false;
      });
      return res;
    }

    /*!
   * This function executes the EventLoop and keep it running until the predicate
   * returns true. All messages that are received are passed into the predicate,
   * allowing us to implement eventhandling outside of actual async code.
   */
    template <typename Predicate>
    void dispatchUntil( Predicate &&predicate ) const
    {
      if ( !ev || !sock )
        return;

      std::optional<int32_t> pendingMessageSize;
      std::exception_ptr excp;

      bool stopRequested = false;

      auto &readBuf = parent->_readBuffer;

      const auto &readMessages = [ &, pred = std::forward<Predicate>(predicate), this ]( ){

        // read all data from socket into internal buffer
        readBuf.append( sock->readAll() );

        try {
          while ( readBuf.size() ) {
            if ( !pendingMessageSize ) {
              // if we cannot read the message size wait some more
              if ( readBuf.size() < sizeof( HeaderSizeType ) )
                return;

              HeaderSizeType msgSize;
              readBuf.read( reinterpret_cast<char *>( &msgSize ), sizeof( decltype (msgSize) ) );
              pendingMessageSize = msgSize;
            }

            // wait for the full message size to be available
            if ( readBuf.size() < static_cast<size_t>( *pendingMessageSize ) )
              return;

            ByteArray message ( *pendingMessageSize, '\0' );
            readBuf.read( message.data(), *pendingMessageSize );
            pendingMessageSize.reset();

            zypp::proto::Envelope m;
            if (! m.ParseFromArray( message.data(), message.size() ) ) {
              //abort, we can not recover from this. Bytes might be mixed up on the socket
              ZYPP_THROW( zypp::media::MediaException("MediaNetwork backend connection broken.") );
            }

            //MIL << "Dispatching message " << m.DebugString() << std::endl;

            if ( pred(m) ) {
              stopRequested = true;
              ev->quit();
              return;
            }
          }
        }  catch ( ... ) {
          // we cannot let the exception travel through the event loop, instead we remember it for later
          stopRequested = true;
          excp = std::current_exception();
          sock->abort();
          ev->quit();
        }
      };

      auto rrConn = AutoDisconnect( sock->connectFunc( &zyppng::Socket::sigReadyRead, readMessages ) );

      auto disConn = AutoDisconnect ( sock->connectFunc( &zyppng::Socket::sigDisconnected, [&excp, this](){
        try {
          ZYPP_THROW( zypp::media::MediaException("MediaNetwork backend disconnected.") );
        } catch ( ... ) {
          // we cannot let the exception travel through the event loop, instead we remember it for later
          excp = std::current_exception();
          ev->quit();
        }
      }) );

      readMessages(); // read all pending messages
      if ( !stopRequested )
        ev->run();

      if ( excp )
        std::rethrow_exception( excp );

      // if we reach this part, we can not have pending message data
      if ( pendingMessageSize ) ERR << "This is a bug, message was partially read and event loop closed" << std::endl;
      assert( !pendingMessageSize );
    }

    MediaNetwork *parent;
    zyppng::EventLoop::Ptr ev;
    zyppng::Socket::Ptr sock;
  };

  MediaNetwork::MediaNetwork(
    const Url & url_r,
    const zypp::Pathname & attach_point_hint_r )
    : MediaHandler( url_r, attach_point_hint_r,
        "/", // urlpath at attachpoint
        true ) // does_download
  {
    _workingDir.autoCleanup( true );

    MIL << "MediaNetwork::MediaNetwork(" << url_r << ", " << attach_point_hint_r << ")" << std::endl;

    if( !attachPoint().empty()){
      zypp::PathInfo ainfo(attachPoint());
      zypp::Pathname apath(attachPoint() + "XXXXXX");
      char    *atemp = ::strdup( apath.asString().c_str());
      char    *atest = NULL;
      if( !ainfo.isDir() || !ainfo.userMayRWX() ||
           atemp == NULL || (atest=::mkdtemp(atemp)) == NULL) {
        WAR << "attach point " << ainfo.path()
            << " is not useable for " << url_r.getScheme() << std::endl;
        setAttachPoint("", true);
      }
      else if( atest != NULL)
        ::rmdir(atest);

      if( atemp != NULL)
        ::free(atemp);
    }
  }

  MediaNetwork::~MediaNetwork() {
    DBG << "Releasing MediaNetwork with tmpdir: " << _workingDir.path().asString() << std::endl;
    try { release(); } catch(...) {}

    if ( _socket ) {
      DBG_MEDIA << "Explicitely calling socket cleanup, this should've happened during release()" << std::endl;
      zyppng::eintrSafeCall( ::close, *_socket );
      _socket.reset();
    }
  }

  TransferSettings &MediaNetwork::settings()
  {
    return _settings;
  }

  std::unique_ptr<MediaNetwork::DispatchContext> MediaNetwork::ensureConnected() const
  {
    auto ctx = std::make_unique<DispatchContext>( *const_cast<MediaNetwork *>(this) );
    if ( !ctx->sock )
      ZYPP_THROW( zypp::media::MediaException("Failed to connect to backend") );
    return ctx;
  }

  void MediaNetwork::disconnectFrom()
  {
    if ( _socket ) {
      zyppng::eintrSafeCall( ::close, *_socket );
      _socket.reset();
    }

    _readBuffer.clear();
    _requests.clear();
    _nextRequestId = 0;

    MediaHandler::disconnectFrom();
  }

  void MediaNetwork::attachTo(bool next)
  {
    if ( next )
      ZYPP_THROW( zypp::media::MediaNotSupportedException(_url) );

    if ( !_url.isValid() )
      ZYPP_THROW( zypp::media::MediaBadUrlException(_url) );

    if( !NetworkRequestDispatcher::supportsProtocol( _url ) ) {
      std::string msg("Unsupported protocol '");
      msg += _url.getScheme();
      msg += "'";
      ZYPP_THROW( zypp::media::MediaBadUrlException(_url, msg) );
    }

    if( !isUseableAttachPoint( attachPoint() ) )
    {
      setAttachPoint( createAttachPoint(), true );
    }

    disconnectFrom();

    // FIXME: need a derived class to propelly compare url's
    zypp::media::MediaSourceRef media( new zypp::media::MediaSource(_url.getScheme(), _url.asString()) );
    setMediaSource(media);
  }

  void MediaNetwork::releaseFrom(const std::string &)
  {
    disconnect();
  }

  Url MediaNetwork::getFileUrl( const zypp::Pathname & filename_r ) const
  {
    // Simply extend the URLs pathname. An 'absolute' URL path
    // is achieved by encoding the leading '/' in an URL path:
    //   URL: ftp://user@server		-> ~user
    //   URL: ftp://user@server/		-> ~user
    //   URL: ftp://user@server//		-> ~user
    //   URL: ftp://user@server/%2F	-> /
    //                         ^- this '/' is just a separator
    Url newurl( _url );
    newurl.setPathName( ( zypp::Pathname("./"+_url.getPathName()) / filename_r ).asString().substr(1) );
    return newurl;
  }

  void MediaNetwork::handleRequestResult(const Request &req, const zypp::filesystem::Pathname &filename ) const
  {
    if ( !req.result() ) {
      ZYPP_THROW( zypp::media::MediaCurlException( req.proto().spec().url(), "Request did not return a result" , "" ) );
    }

    if ( !req.result()->has_error() )
      return;

    const auto &err = req.result()->error();
    if ( err.error() != NetworkRequestError::NoError ) {
      Url reqUrl = err.extra_info().at("requestUrl");
      switch ( err.error() )
      {
        case NetworkRequestError::Unauthorized: {
          std::string hint = err.extra_info().at("authHint");
          ZYPP_THROW( zypp::media::MediaUnauthorizedException(
            reqUrl, err.errordesc(), err.nativeerror(), hint
            ));
          break;
        }
        case NetworkRequestError::TemporaryProblem:
          ZYPP_THROW( zypp::media::MediaTemporaryProblemException(reqUrl) );
          break;
        case NetworkRequestError::Timeout:
          ZYPP_THROW( zypp::media::MediaTimeoutException(reqUrl) );
          break;
        case NetworkRequestError::Forbidden:
          ZYPP_THROW( zypp::media::MediaForbiddenException(reqUrl, err.errordesc()));
          break;
        case NetworkRequestError::NotFound:
          ZYPP_THROW( zypp::media::MediaFileNotFoundException( reqUrl, filename ) );
          break;
        default:
          break;
      }
      ZYPP_THROW( zypp::media::MediaCurlException( reqUrl, err.errordesc(), err.nativeerror() ) );
    }
    ZYPP_THROW( zypp::media::MediaCurlException( req.proto().spec().url(), err.errordesc() , "" ) );
  }

  bool MediaNetwork::retry( DispatchContext &ctx, Request &req ) const
  {
    const auto &res = req.result();
    if ( !res || !res->has_error() )
      return false;

    const auto &resErr = res->error();
    if ( resErr.error() != NetworkRequestError::AuthFailed && resErr.error() != NetworkRequestError::Unauthorized )
      return false;

    const std::string &availAuth =  resErr.extra_info().count("authHint") ? resErr.extra_info().at("authHint") : "";

    zypp::callback::SendReport<zypp::media::AuthenticationReport> auth_report;

    zypp::media::CredentialManager cm( zypp::media::CredManagerOptions( zypp::ZConfig::instance().repoManagerRoot()) );
    auto authDataPtr = cm.getCred( _url );

    NetworkAuthData_Ptr curlcred( new NetworkAuthData() );
    curlcred->setUrl( _url );

    // preset the username if present in current url
    if ( !_url.getUsername().empty() )
      curlcred->setUsername(_url.getUsername());
    // if CM has found some credentials, preset the username from there
    else if ( authDataPtr )
      curlcred->setUsername( authDataPtr->username() );

    // set available authentication types
    // might be needed in prompt
    curlcred->setAuthType( availAuth );

    std::string prompt_msg = zypp::str::Format( _("Authentication required for '%s'") ) % _url.asString();

    // the auth data that was used in the request is older than whats in the Credential manager,
    // so instead of asking the user for creds we first try to use the new ones
    if ( res->last_auth_timestamp() > 0 && res->last_auth_timestamp() < authDataPtr->lastDatabaseUpdate() ) {
      _settings.setUsername( authDataPtr->username() );
      _settings.setPassword( authDataPtr->password() );
      retryRequest( ctx, req );
      return true;
    } else {

      // ask user
      if ( auth_report->prompt( _url, prompt_msg, *curlcred ) ) {
        DBG << "callback answer: retry" << std::endl
            << "CurlAuthData: " << *curlcred << std::endl;

        if ( curlcred->valid() ) {

          _settings.setUsername( curlcred->username() );
          _settings.setPassword( curlcred->password() );

          // set available authentication types from the exception
          if ( curlcred->authType() == CURLAUTH_NONE )
            curlcred->setAuthType( availAuth );

          // set auth type (seems this must be set _after_ setting the userpwd)
          if ( curlcred->authType() != CURLAUTH_NONE) {
            // FIXME: only overwrite if not empty?
            _settings.setAuthType( curlcred->authTypeAsString() );
          }

          cm.addCred( *curlcred );
          cm.save();

          // we need to restart all already failed requests so we do not double ask
          // for credentials
          for ( auto &r : _requests ) {
            if ( &r == &req )
              continue;
            if ( !r.result() || !r.result()->has_error() )
              continue;
            const auto rErr = r.result()->error();
            if ( rErr.error() != NetworkRequestError::AuthFailed && rErr.error() != NetworkRequestError::Unauthorized ) {
              retryRequest( ctx, r );
            }
          }

          retryRequest( ctx, req );
          return true;
        }
      }
    }
    return false;
  }

  void MediaNetwork::handleStreamMessage( DispatchContext &ctx, const zypp::proto::Envelope &e ) const
  {
    RequestId dlId = -1;
    bool cancel = false;

    const auto &mName = e.messagetypename();
    if ( mName == "zypp.proto.DownloadStart" ) {
      zypp::proto::DownloadStart st;
      st.ParseFromString( e.value() );

      auto r = findRequest( st.requestid() );
      if ( !r )
        return;

      MIL << "Received the download started event for file "<< zypp::Url( r->proto().spec().url() ) <<" from server." << std::endl;
      dlId = st.requestid();
      r->setProgress( 0.0, 0.0, cancel );

    } else if ( mName == "zypp.proto.DownloadProgress" ) {
      zypp::proto::DownloadProgress prog;
      prog.ParseFromString( e.value() );

      auto r = findRequest( prog.requestid() );
      if ( !r )
        return;

      //MIL << "Received progress for download: " << zypp::Url( r->url() ) << std::endl;
      dlId = prog.requestid();
      r->setProgress( prog.total(), prog.now(), cancel );

    } else if ( mName == "zypp.proto.DownloadFin" ) {
      zypp::proto::DownloadFin fin;
      fin.ParseFromString( e.value() );

      auto r = findRequest( fin.requestid() );
      if ( !r )
        return;

      MIL << "Received FIN for download: " << zypp::Url( r->proto().spec().url() ) << std::endl;
      r->setResult( std::move(fin) );
    } else {
      // error , unknown message
      WAR << "Received unexpected message... ignoring" << std::endl;
    }

    if ( dlId >= 0 && cancel ) {
      zypp::proto::CancelDownload cl;
      cl.set_requestid( dlId );
      ctx.sendEnvelope( cl );
      ctx.sock->waitForAllBytesWritten();
    }
  }

  MediaNetwork::Request MediaNetwork::makeRequest ( const zypp::OnMediaLocation &loc ) const
  {
    DBG << loc.filename().asString() << std::endl;

    if(!_url.isValid())
      ZYPP_THROW(zypp::media::MediaBadUrlException(_url));

    if(_url.getHost().empty())
      ZYPP_THROW(zypp::media::MediaBadUrlEmptyHostException(_url));

    Url url( getFileUrl(loc.filename()) );

    DownloadSpec spec( url, loc.filename(), loc.downloadSize() );
    spec.setTransferSettings( _settings )
      .setHeaderSize( loc.headerSize() )
      .setHeaderChecksum( loc.headerChecksum() )
      .setDeltaFile( loc.deltafile() );

    // we are downloading the file into a temporary location first
    Request fileReq( std::move(spec), _workingDir, ++_nextRequestId );
    fileReq.proto().set_streamprogress( false );
    return fileReq;
  }

  void MediaNetwork::trackRequest ( DispatchContext &ctx, Request &req ) const
  {
    zypp::proto::DownloadFin result;

    bool retry = true;
    while ( retry ) {

      retry = false; // try to stop after this iteration

      if ( !req.result() ) {
        result = ctx.waitFor<zypp::proto::DownloadFin>( req.proto().requestid(), std::bind( &MediaNetwork::handleStreamMessage, this, std::placeholders::_1, std::placeholders::_2 ) );
        req.setResult( std::move(result) );
      }

      retry = this->retry( ctx, req );
    }
  }



  void MediaNetwork::retryRequest ( DispatchContext &ctx, MediaNetwork::Request &req ) const
  {
    req.reset();
    // update request with new settings
    *req.proto().mutable_spec()->mutable_settings() = _settings.protoData();
    ctx.sendEnvelope( req.proto() );
    const auto &s = ctx.waitForStatus( req.proto().requestid(), std::bind( &MediaNetwork::handleStreamMessage, this, std::placeholders::_1, std::placeholders::_2 ) );
    if ( s.code() != zypp::proto::Status::Ok ) {
      ZYPP_THROW( zypp::media::MediaException( s.reason() ) );
    }
  }

  MediaNetwork::Request *MediaNetwork::findRequest( const Url url ) const
  {
    const auto &i = std::find_if( _requests.begin(), _requests.end(), [ &url ]( const auto &r ) {
      return ( url == zyppng::Url( r.proto().spec().url() ) );
    });
    if ( i == _requests.end() )
      return nullptr;
    return &(*i);
  }

  MediaNetwork::Request *MediaNetwork::findRequest(const RequestId id ) const
  {
    const auto &i = std::find_if( _requests.begin(), _requests.end(), [ &id ]( const auto &r ) {
      return ( id == r.proto().requestid() );
    });
    if ( i == _requests.end() )
      return nullptr;
    return &(*i);
  }

  bool MediaNetwork::getDoesFileExist( const zypp::filesystem::Pathname &filename ) const
  {

    auto ctx = ensureConnected();

    // here we always send a new request without even considering probably already existing ones
    auto req = makeRequest( filename );
    req.proto().mutable_spec()->set_checkexistanceonly( true );
    req.proto().set_prioritize( true );


    ctx->sendEnvelope( req.proto() );
    const auto &s = ctx->waitForStatus( req.proto().requestid(), std::bind( &MediaNetwork::handleStreamMessage, this, std::placeholders::_1, std::placeholders::_2 ) );
    if ( s.code() != zypp::proto::Status::Ok ) {
      ZYPP_THROW( zypp::media::MediaException( s.reason() ) );
    }

    try
    {
      trackRequest( *ctx, req );

      //this will throw if the file does not exist
      handleRequestResult( req, filename );
    } catch ( const zypp::media::MediaFileNotFoundException &e ) {
      return false;
    } catch ( const zypp::media::MediaException &e ) {
      // some error, we are not sure about file existence, rethrw
      ZYPP_RETHROW(e);
    }

    return true;
  }


  void MediaNetwork::getFile( const zypp::OnMediaLocation &file ) const
  {
    const auto &filename = file.filename();
    auto ctx = ensureConnected();

    const auto url = getFileUrl( filename );
    auto downloadReq = findRequest( url );

    zypp::callback::SendReport<zypp::media::DownloadProgressReport> report;
    ProgressData data( url, &report );

    report->start( url, filename );

    if ( !downloadReq ) {
      auto newReq = makeRequest( file );

      newReq.proto().set_streamprogress( true );
      newReq.proto().set_prioritize( true );

      ctx->sendEnvelope( newReq.proto() );

      const auto &s = ctx->waitForStatus( newReq.proto().requestid(), std::bind( &MediaNetwork::handleStreamMessage, this, std::placeholders::_1, std::placeholders::_2 ) );
      if ( s.code() != zypp::proto::Status::Ok && s.code() ) {
        ZYPP_THROW( zypp::media::MediaException( s.reason() ) );
      }

      _requests.push_back( newReq );
      downloadReq = &_requests.back();

    } else {
      // if the download is not yet finished we request to track it
      if ( !downloadReq->result() ) {

        zypp::proto::SubscribeProgress sub;
        sub.set_requestid( downloadReq->proto().requestid() );
        sub.set_prioritize( true );
        ctx->sendEnvelope( sub );

        const auto &s = ctx->waitForStatus( sub.requestid(), std::bind( &MediaNetwork::handleStreamMessage, this, std::placeholders::_1, std::placeholders::_2 ) );

        // status could be unknown ID if the request was already in finished state and the message was stuck in our socket!
        if ( s.code() != zypp::proto::Status::Ok && s.code() != zypp::proto::Status::UnknownId ) {
          ZYPP_THROW( zypp::media::MediaException( s.reason() ) );
        }
      }
    }

    downloadReq->startReporting( data );
    zypp::OnScopeExit cleanup( [&](){ downloadReq->stopReporting(); });

    try {
      trackRequest( *ctx, *downloadReq );

      //this will throw if the file does not exist
      handleRequestResult( *downloadReq, filename );
    } catch ( zypp::media::MediaUnauthorizedException & ex_r ) {
      report->finish(url, zypp::media::DownloadProgressReport::ACCESS_DENIED, ex_r.asUserHistory());
      ZYPP_RETHROW(ex_r);
    }
    // unexpected exception
    catch ( zypp::media::MediaException & excpt_r )
    {
      zypp::media::DownloadProgressReport::Error reason = zypp::media::DownloadProgressReport::ERROR;
      if( typeid(excpt_r) == typeid( zypp::media::MediaFileNotFoundException )  ||
           typeid(excpt_r) == typeid( zypp::media::MediaNotAFileException ) )
      {
        reason = zypp::media::DownloadProgressReport::NOT_FOUND;
      }
      report->finish(url, reason, excpt_r.asUserHistory());
      ZYPP_RETHROW(excpt_r);
    }

    // seems we were successful
    const auto &targetPath = localPath(filename).absolutename();

    const auto errCode = zypp::filesystem::assert_dir( targetPath.dirname() );
    if( errCode ) {
      std::string err = zypp::str::Str() << "assert_dir " << targetPath.dirname() << " failed";
      DBG << err << std::endl;
      ZYPP_THROW( zypp::media::MediaCurlException( url, err, "" ) );
    }

    if( zypp::filesystem::hardlinkCopy( downloadReq->proto().spec().targetpath(), targetPath ) != 0 ) {
      std::string err = zypp::str::Str() << "Failed to hardlinkCopy the requested file <<" << downloadReq->proto().spec().targetpath() << " to the targetPath " << targetPath;
      DBG << err << std::endl;
      ZYPP_THROW( zypp::media::MediaCurlException( url, err, "" ) );
    }

    if (::chmod( targetPath.c_str(), zypp::filesystem::applyUmaskTo( 0644 ))) {
      ERR << "Failed to chmod file " << targetPath << std::endl;
    }

    // check if the file was required the same number of times it was requested
    if ( downloadReq->unref() <= 0 ) {
      _requests.remove_if( [ & ]( const auto &r ){
        return ( r.proto().requestid() == downloadReq->proto().requestid() );
      });
    }

    report->finish(url, zypp::media::DownloadProgressReport::NO_ERROR, "");
  }

  void MediaNetwork::getDir(const zypp::filesystem::Pathname &dirname, bool recurse_r) const
  {
    //we could make this download concurrently, but its not used anywhere in the code, so why bother
    zypp::filesystem::DirContent content;
    getDirInfo( content, dirname, /*dots*/false );

    for ( zypp::filesystem::DirContent::const_iterator it = content.begin(); it != content.end(); ++it ) {
      zypp::Pathname filename = dirname + it->name;
      int res = 0;

      switch ( it->type ) {
        case zypp::filesystem::FT_NOT_AVAIL: // old directory.yast contains no typeinfo at all
        case zypp::filesystem::FT_FILE:
          getFile( zypp::OnMediaLocation( filename ) );
          break;
        case zypp::filesystem::FT_DIR: // newer directory.yast contain at least directory info
          if ( recurse_r ) {
            getDir( filename, recurse_r );
          } else {
            res = assert_dir( localPath( filename ) );
            if ( res ) {
              WAR << "Ignore error (" << res <<  ") on creating local directory '" << localPath( filename ) << "'" << std::endl;
            }
          }
          break;
        default:
          // don't provide devices, sockets, etc.
          break;
      }
    }
  }

  void MediaNetwork::precacheFiles(const std::vector<zypp::OnMediaLocation> &files)
  {
    zypp::proto::Prefetch req;
    req.set_requestid( ++_nextRequestId );

    std::vector<Request> sentRequests;

    for ( const auto &file : files ) {
      const auto url = getFileUrl( file.filename() );
      auto existingReq = findRequest( url );
      if ( existingReq )
        existingReq->ref();
      else {
        Request fileReq = makeRequest( file );
        fileReq.proto().set_streamprogress( false );
        *req.mutable_requests()->Add() = fileReq.proto();
        sentRequests.push_back( fileReq );
      }
    }

    if ( req.requests().size() ) {

      auto ctx = ensureConnected();
      ctx->sendEnvelope( req );

      const auto &status = ctx->waitForStatus( req.requestid(), std::bind( &MediaNetwork::handleStreamMessage, this, std::placeholders::_1, std::placeholders::_2 ) );
      if ( status.code() == zypp::proto::Status::Ok ) {
        MIL_MEDIA << "Request was acknowledged by server, downloads should start soon" << std::endl;
        _requests.insert( _requests.end(), std::move_iterator(sentRequests.begin()), std::move_iterator(sentRequests.end()) );
        return;
      }

      MIL << "Request failed " << status.reason() << std::endl;
      ZYPP_THROW( zypp::media::MediaException( status.reason() ) );
    }
  }

  void MediaNetwork::getDirInfo(std::list<std::string> &retlist, const zypp::filesystem::Pathname &dirname, bool dots) const
  {
    getDirectoryYast( retlist, dirname, dots );
  }

  void MediaNetwork::getDirInfo(zypp::filesystem::DirContent &retlist, const zypp::filesystem::Pathname &dirname, bool dots) const
  {
    getDirectoryYast( retlist, dirname, dots );
  }

  bool MediaNetwork::checkAttachPoint(const zypp::Pathname &apoint) const
  {
    return MediaHandler::checkAttachPoint( apoint, true, true);
  }


}
