/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "private/providedbg_p.h"
#include "private/provideitem_p.h"
#include "private/provide_p.h"
#include "private/providemessage_p.h"
#include "private/provideres_p.h"
#include "provide-configvars.h"
#include <zypp-media/MediaException>
#include <zypp-core/base/UserRequestException>
#include "mediaverifier.h"
#include <zypp-core/fs/PathInfo.h>

using namespace std::literals;

namespace zyppng {

  static constexpr std::string_view DEFAULT_MEDIA_VERIFIER("SuseMediaV1");

  expected<ProvideRequestRef> ProvideRequest::create(ProvideItem &owner, const std::vector<zypp::Url> &urls, const std::string &id, ProvideMediaSpec &spec )
  {
    if ( urls.empty() )
      return expected<ProvideRequestRef>::error( ZYPP_EXCPT_PTR ( zypp::media::MediaException("List of URLs can not be empty") ) );

    auto m = ProvideMessage::createAttach( ProvideQueue::InvalidId, urls.front(), id, spec.label() );
    if ( !spec.mediaFile().empty() ) {
      m.setValue( AttachMsgFields::VerifyType, std::string(DEFAULT_MEDIA_VERIFIER.data()) );
      m.setValue( AttachMsgFields::VerifyData, spec.mediaFile().asString() );
      m.setValue( AttachMsgFields::MediaNr, int32_t(spec.medianr()) );
    }

    const auto &cHeaders = spec.customHeaders();
    for ( auto i = cHeaders.beginList (); i != cHeaders.endList(); i++) {
      for ( const auto &val : i->second )
        m.addValue( i->first, val );
    }

    return expected<ProvideRequestRef>::success( ProvideRequestRef( new ProvideRequest(&owner, urls, std::move(m))) );
  }

  expected<ProvideRequestRef> ProvideRequest::create( ProvideItem &owner, const std::vector<zypp::Url> &urls, ProvideFileSpec &spec )
  {
    if ( urls.empty() )
      return expected<ProvideRequestRef>::error( ZYPP_EXCPT_PTR ( zypp::media::MediaException("List of URLs can not be empty") ) );

    auto m = ProvideMessage::createProvide ( ProvideQueue::InvalidId, urls.front() );
    const auto &destFile = spec.destFilenameHint();
    const auto &deltaFile = spec.deltafile();
    const int64_t fSize = spec.downloadSize();;

    if ( !destFile.empty() )
      m.setValue( ProvideMsgFields::Filename, destFile.asString() );
    if ( !deltaFile.empty() )
      m.setValue( ProvideMsgFields::DeltaFile, deltaFile.asString() );
    if ( fSize )
      m.setValue( ProvideMsgFields::ExpectedFilesize, fSize );
    m.setValue( ProvideMsgFields::CheckExistOnly, spec.checkExistsOnly() );

    const auto &cHeaders = spec.customHeaders();
    for ( auto i = cHeaders.beginList (); i != cHeaders.endList(); i++) {
      for ( const auto &val : i->second )
        m.addValue( i->first, val );
    }

    return expected<ProvideRequestRef>::success( ProvideRequestRef( new ProvideRequest(&owner, urls, std::move(m)) ) );
  }

  expected<ProvideRequestRef> ProvideRequest::createDetach( const zypp::Url &url )
  {
    auto m = ProvideMessage::createDetach ( ProvideQueue::InvalidId , url );
    return expected<ProvideRequestRef>::success( ProvideRequestRef( new ProvideRequest( nullptr, { url }, std::move(m) ) ) );
  }

  ZYPP_IMPL_PRIVATE(ProvideItem);

  ProvideItem::ProvideItem( ProvidePrivate &parent )
    : Base( *new ProvideItemPrivate( parent, *this ) )
  { }

  ProvideItem::~ProvideItem()
  { }

  ProvidePrivate &ProvideItem::provider()
  {
    return d_func()->_parent;
  }

  bool ProvideItem::safeRedirectTo( ProvideRequestRef startedReq, const zypp::Url &url )
  {
    if ( !canRedirectTo( startedReq, url ) )
      return false;

    redirectTo( startedReq, url );
    return true;
  }

  void ProvideItem::redirectTo( ProvideRequestRef startedReq, const zypp::Url &url )
  {
    //@TODO strip irrelevant stuff from URL
    startedReq->_pastRedirects.push_back ( url );
  }

  bool ProvideItem::canRedirectTo( ProvideRequestRef startedReq, const zypp::Url &url )
  {
    // make sure there is no redirect loop
    if ( !startedReq->_pastRedirects.size() )
      return true;

    if ( std::find( startedReq->_pastRedirects.begin(), startedReq->_pastRedirects.end(), url ) != startedReq->_pastRedirects.end() )
      return false;

    return true;
  }

  const std::optional<ProvideItem::ItemStats> &ProvideItem::currentStats() const
  {
    return d_func()->_currStats;
  }

  const std::optional<ProvideItem::ItemStats> &ProvideItem::previousStats() const
  {
    return d_func()->_prevStats;
  }

  std::chrono::steady_clock::time_point ProvideItem::startTime() const
  {
      return d_func()->_itemStarted;
  }

  std::chrono::steady_clock::time_point ProvideItem::finishedTime() const {
      return d_func()->_itemFinished;
  }

  void ProvideItem::pulse ()
  {
    Z_D();
    if ( d->_currStats )
      d->_prevStats = d->_currStats;

    d->_currStats = makeStats();

    // once the item is finished the pulse time is always the finish time
    if ( d->_itemState == Finished )
      d->_currStats->_pulseTime = d->_itemFinished;
  }

  zypp::ByteCount ProvideItem::bytesExpected () const
  {
    return 0;
  }

  ProvideItem::ItemStats ProvideItem::makeStats ()
  {
    return ItemStats {
      ._pulseTime = std::chrono::steady_clock::now(),
      ._runningRequests = _runningReq ? (uint)1 : (uint)0
    };
  }

  void ProvideItem::informalMessage ( ProvideQueue &, ProvideRequestRef req, const ProvideMessage &msg  )
  {
    if ( req != _runningReq ) {
      WAR << "Received event for unknown request, ignoring" << std::endl;
      return;
    }

    if ( msg.code() == ProvideMessage::Code::ProvideStarted ) {
      MIL << "Request: "<< req->url() << " was started" << std::endl;
    }

  }

  void ProvideItem::cacheMiss( ProvideRequestRef req )
  {
    if ( req != _runningReq ) {
      WAR << "Received event for unknown request, ignoring" << std::endl;
      return;
    }

    MIL << "Request: "<< req->url() << " CACHE MISS, request will be restarted by queue." << std::endl;
  }

  void ProvideItem::finishReq(ProvideQueue &, ProvideRequestRef finishedReq, const ProvideMessage &msg )
  {
    if ( finishedReq != _runningReq ) {
      WAR << "Received event for unknown request, ignoring" << std::endl;
      return;
    }

    auto log = provider().log();

    // explicitely handled codes
    const auto code = msg.code();
    if ( code == ProvideMessage::Code::Redirect ) {

      // remove the old request
      _runningReq.reset();

      try {

        MIL << "Request finished with redirect." << std::endl;

        zypp::Url newUrl( msg.value( RedirectMsgFields::NewUrl ).asString() );
        if ( !safeRedirectTo( finishedReq, newUrl ) ) {
          cancelWithError( ZYPP_EXCPT_PTR ( zypp::media::MediaException("Redirect Loop")) );
          return;
        }

        MIL << "Request redirected to: " << newUrl << std::endl;

        if ( log ) log->requestRedirect( *this, msg.requestId(), newUrl );

        finishedReq->setUrl( newUrl );

        if ( !enqueueRequest( finishedReq ) ) {
          cancelWithError( ZYPP_EXCPT_PTR(zypp::media::MediaException("Failed to queue request")) );
        }
      }  catch ( ... ) {
        cancelWithError( std::current_exception() );
        return;
      }
      return;

    } else if ( code == ProvideMessage::Code::Metalink ) {

      // remove the old request
      _runningReq.reset();

      MIL << "Request finished with mirrorlist from server." << std::endl;

      //@TODO do we need to merge this with the mirrorlist we got from the user?
      //      or does a mirrorlist from d.o.o invalidate that?

      std::vector<zypp::Url> urls;
      const auto &mirrors = msg.values( MetalinkRedirectMsgFields::NewUrl );
      for( auto i = mirrors.cbegin(); i != mirrors.cend(); i++ ) {
        try {
          zypp::Url newUrl( i->asString() );
          if ( !canRedirectTo( finishedReq, newUrl ) )
            continue;
          urls.push_back ( newUrl );
        }  catch ( ... ) {
          if ( i->isString() )
            WAR << "Received invalid URL from worker: " << i->asString() << " ignoring!" << std::endl;
          else
            WAR << "Received invalid value for newUrl from worker ignoring!" << std::endl;
        }
      }

      if ( urls.size () == 0 ) {
        cancelWithError( ZYPP_EXCPT_PTR ( zypp::media::MediaException("No mirrors left to redirect to.")) );
        return;
      }

      MIL << "Found usable nr of mirrors: " << urls.size () << std::endl;
      finishedReq->setUrls( urls );

      // disable metalink
      finishedReq->provideMessage().setValue( ProvideMsgFields::MetalinkEnabled, false );

      if ( log ) log->requestDone( *this, msg.requestId() );

      if ( !enqueueRequest( finishedReq ) ) {
        cancelWithError( ZYPP_EXCPT_PTR(zypp::media::MediaException("Failed to queue request")) );
      }

      MIL << "End of mirrorlist handling"<< std::endl;
      return;

    } else if ( code >= ProvideMessage::Code::FirstClientErrCode && code <= ProvideMessage::Code::LastSrvErrCode ) {

      // remove the old request
      _runningReq.reset();

      std::exception_ptr errPtr;
      try {
        const auto reqUrl = finishedReq->activeUrl().value();
        const auto reason  = msg.value( ErrMsgFields::Reason ).asString();
        switch ( code ) {
          case ProvideMessage::Code::BadRequest:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaException (zypp::str::Str() << "Bad request for URL: " << reqUrl << " " << reason ) );
            break;
          case ProvideMessage::Code::PeerCertificateInvalid:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaException(zypp::str::Str() << "PeerCertificateInvalid Error for URL: " << reqUrl << " " << reason) );
            break;
          case ProvideMessage::Code::ConnectionFailed:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaException(zypp::str::Str() << "ConnectionFailed Error for URL: " << reqUrl << " " << reason ) );
            break;
          case ProvideMessage::Code::ExpectedSizeExceeded: {

            std::optional<int64_t> filesize;
            finishedReq->provideMessage ().forEachVal( [&]( const std::string &key, const auto &val ){
              if ( key == ProvideMsgFields::ExpectedFilesize && val.valid() )
                filesize = val.asInt64();
              return true;
            });

            if ( !filesize ) {
              errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaException( zypp::str::Str() << "ExceededExpectedSize Error for URL: " << reqUrl << " " << reason ) );
            } else {
              errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaFileSizeExceededException(reqUrl, *filesize ) );
            }
            break;
          }
          case ProvideMessage::Code::Cancelled:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaException( zypp::str::Str() << "Request was cancelled: " << reqUrl << " " << reason ) );
            break;
          case ProvideMessage::Code::InvalidChecksum:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaException( zypp::str::Str() << "InvalidChecksum Error for URL: " << reqUrl << " " << reason ) );
            break;
          case ProvideMessage::Code::Timeout:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaTimeoutException(reqUrl) );
            break;
          case ProvideMessage::Code::NotFound:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaFileNotFoundException(reqUrl, "") );
            break;
          case ProvideMessage::Code::Forbidden:
          case ProvideMessage::Code::Unauthorized: {

            const auto &hintVal = msg.value( "authHint"sv );
            std::string hint;
            if ( hintVal.valid() && hintVal.isString() ) {
              hint = hintVal.asString();
            }

            //@TODO retry here with timestamp from cred store check
            // we let the request fail after it checked the store

            errPtr = ZYPP_EXCPT_PTR ( zypp::media::MediaUnauthorizedException(
              reqUrl, reason, "", hint
              ));
            break;

          }
          case ProvideMessage::Code::MountFailed:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaException( zypp::str::Str() << "MountFailed Error for URL: " << reqUrl << " " << reason ) );
            break;
          case ProvideMessage::Code::Jammed:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaJammedException() );
            break;
          case ProvideMessage::Code::MediaChangeSkip:
            errPtr = ZYPP_EXCPT_PTR( zypp::SkipRequestException ( zypp::str::Str() << "User-requested skipping for URL: " << reqUrl << " " << reason ) );
            break;
          case ProvideMessage::Code::MediaChangeAbort:
            errPtr = ZYPP_EXCPT_PTR( zypp::AbortRequestException( zypp::str::Str() <<"Aborting requested by user for URL: " << reqUrl << " " << reason ) );
            break;
          case ProvideMessage::Code::InternalError:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaException( zypp::str::Str() << "WorkerSpecific Error for URL: " << reqUrl << " " << reason ) );
            break;
          case ProvideMessage::Code::NotAFile:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaNotAFileException(reqUrl, "") );
            break;
          case ProvideMessage::Code::MediumNotDesired:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaNotDesiredException(reqUrl) );
            break;
          default:
            errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaException( zypp::str::Str() << "Unknown Error for URL: " << reqUrl << " " << reason ) );
            break;
        }
      } catch (...) {
        errPtr = ZYPP_EXCPT_PTR( zypp::media::MediaException( zypp::str::Str() << "Invalid error message received for URL: " << *finishedReq->activeUrl() << " code: " << code ) );
      }

      if ( log ) log->requestFailed( *this, msg.requestId(), errPtr );
      // finish the request
      cancelWithError( errPtr );
      return;
    }

    // if we reach here we don't know how to handle the message
    _runningReq.reset();
    cancelWithError( ZYPP_EXCPT_PTR (zypp::media::MediaException("Unhandled message received for ProvideFileItem")) );
  }

  void ProvideItem::finishReq(ProvideQueue *, ProvideRequestRef finishedReq , const std::exception_ptr excpt)
  {
    if ( finishedReq != _runningReq ) {
      WAR << "Received event for unknown request, ignoring" << std::endl;
      return;
    }

    if ( _runningReq ) {
      auto log = provider().log();
      if ( log ) log->requestFailed( *this, finishedReq->provideMessage().requestId(), excpt );
    }

    _runningReq.reset();
    cancelWithError(excpt);
  }

  expected<zypp::media::AuthData> ProvideItem::authenticationRequired ( ProvideQueue &queue, ProvideRequestRef req, const zypp::Url &effectiveUrl, int64_t lastTimestamp, const std::map<std::string, std::string> &extraFields )
  {

    if ( req != _runningReq ) {
      WAR << "Received authenticationRequired for unknown request, rejecting" << std::endl;
      return expected<zypp::media::AuthData>::error( ZYPP_EXCPT_PTR( zypp::media::MediaException("Unknown request in authenticationRequired, this is a bug.") ) );
    }

    try {
      zypp::media::CredentialManager mgr ( provider().credManagerOptions() );

      MIL << "Looking for existing auth data for " << effectiveUrl << "more recent then " << lastTimestamp << std::endl;

      auto credPtr = mgr.getCred( effectiveUrl );
      if ( credPtr && credPtr->lastDatabaseUpdate() > lastTimestamp ) {
        MIL << "Found existing auth data for " << effectiveUrl << "ts: " <<  credPtr->lastDatabaseUpdate() << std::endl;
        return expected<zypp::media::AuthData>::success( *credPtr );
      }

      if ( credPtr ) MIL << "Found existing auth data for " << effectiveUrl << "but too old ts: " <<  credPtr->lastDatabaseUpdate() << std::endl;

      std::string username;
      if ( auto i = extraFields.find( std::string(AuthDataRequestMsgFields::LastUser) ); i != extraFields.end() ) {
        username = i->second;
      }


      MIL << "NO Auth data found, asking user. Last tried username was: " << username << std::endl;

      auto userAuth = provider()._sigAuthRequired.emit( effectiveUrl, username, extraFields );
      if ( !userAuth || !userAuth->valid() ) {
        MIL << "User rejected to give auth" << std::endl;
        return expected<zypp::media::AuthData>::error( ZYPP_EXCPT_PTR( zypp::media::MediaException("No auth given by user." ) ) );
      }

      mgr.addCred( *userAuth );
      mgr.save();

      // rather ugly, but update the timestamp to the last mtime of the cred database our URL belongs to
      // otherwise we'd need to reload the cred database
      userAuth->setLastDatabaseUpdate( mgr.timestampForCredDatabase( effectiveUrl ) );

      return expected<zypp::media::AuthData>::success(*userAuth);
    } catch ( const zypp::Exception &e ) {
      ZYPP_CAUGHT(e);
      return expected<zypp::media::AuthData>::error( std::current_exception() );
    }
  }

  bool ProvideItem::enqueueRequest( ProvideRequestRef request )
  {
    // base item just supports one running request at a time
    if ( _runningReq )
      return ( _runningReq == request );

    _runningReq = request;
    return d_func()->_parent.queueRequest( request );
  }

  void ProvideItem::updateState( const State newState )
  {
    Z_D();
    if ( d->_itemState != newState ) {

      bool started = ( d->_itemState == Uninitialized && ( newState != Finished ));
      auto log = provider().log();

      const auto oldState = d->_itemState;
      d->_itemState = newState;
      d->_sigStateChanged( *this, oldState, d->_itemState );

      if ( started ) {
        d->_itemStarted = std::chrono::steady_clock::now();
        pulse();
        if ( log ) log->itemStart( *this );
      }

      if ( newState == Finished ) {
        d->_itemFinished = std::chrono::steady_clock::now();
        pulse();
        if ( log) log->itemDone( *this );
        d->_parent.dequeueItem(this);
      }
      // CAREFUL, 'this' might be invalid from here on
    }
  }

  void ProvideItem::released()
  {
    if ( state() == Finished || state() == Finalizing )
      return;

    MIL << "Item Cleanup due to released Promise in state:" << state() << std::endl;
    cancelWithError( ZYPP_EXCPT_PTR(zypp::media::MediaException("Cancelled by user.")) );
  }

  ProvideItem::State ProvideItem::state () const
  {
    return d_func()->_itemState;
  }

  void ProvideRequest::setCurrentQueue( ProvideQueueRef ref )
  {
    _myQueue = ref;
  }

  ProvideQueueRef ProvideRequest::currentQueue()
  {
    return _myQueue.lock();
  }

  const std::optional<zypp::Url> ProvideRequest::activeUrl() const
  {
    ProvideMessage::FieldVal url;
    switch ( this->_message.code () ) {
      case ProvideMessage::Code::Attach:
        url = _message.value( AttachMsgFields::Url );
        break;
      case ProvideMessage::Code::Detach:
        url = _message.value( DetachMsgFields::Url );
        break;
      case ProvideMessage::Code::Provide:
        url = _message.value( ProvideMsgFields::Url );
        break;
      default:
        // should never happen because we guard the constructor
        throw std::logic_error("Invalid message type in ProvideRequest");
    }
    if ( !url.valid() ) {
      return {};
    }

    try {
      auto u = zypp::Url( url.asString() );
      return u;
    }  catch ( const zypp::Exception &e ) {
      ZYPP_CAUGHT(e);
    }

    return {};
  }

  void ProvideRequest::setActiveUrl(const zypp::Url &urlToUse) {

    switch ( this->_message.code () ) {
      case ProvideMessage::Code::Attach:
        this->_message.setValue( AttachMsgFields::Url, urlToUse.asCompleteString() );
        break;
      case ProvideMessage::Code::Detach:
        this->_message.setValue( DetachMsgFields::Url, urlToUse.asCompleteString() );
        break;
      case ProvideMessage::Code::Provide:
        this->_message.setValue( ProvideMsgFields::Url, urlToUse.asCompleteString() );
        break;
      default:
        // should never happen because we guard the constructor
        throw std::logic_error("Invalid message type in ProvideRequest");
    }
  }

  ProvideFileItem::ProvideFileItem(const std::vector<zypp::Url> &urls, const ProvideFileSpec &request, ProvidePrivate &parent)
    : ProvideItem( parent )
    , _mirrorList  ( urls )
    , _initialSpec ( request )
  { }

  ProvideFileItemRef ProvideFileItem::create(const std::vector<zypp::Url> &urls, const ProvideFileSpec &request, ProvidePrivate &parent )
  {
    return ProvideFileItemRef( new ProvideFileItem( urls, request, parent ) );
  }

  void ProvideFileItem::initialize()
  {
    if ( state() != Uninitialized || _runningReq ) {
      WAR << "Double init of ProvideFileItem!" << std::endl;
      return;
    }

    auto req =  ProvideRequest::create( *this, _mirrorList, _initialSpec );
    if ( !req ){
      cancelWithError( req.error() );
      return ;
    }

    if ( enqueueRequest( *req ) ) {
      _expectedBytes = _initialSpec.downloadSize();
      updateState( Pending );
    } else {
      cancelWithError( ZYPP_EXCPT_PTR(zypp::media::MediaException("Failed to queue request")) );
      return ;
    }
  }

  ProvidePromiseRef<ProvideRes> ProvideFileItem::promise()
  {
    if ( !_promiseCreated ) {
      _promiseCreated = true;
      auto promiseRef = std::make_shared<ProvidePromise<ProvideRes>>( shared_this<ProvideItem>() );
      _promise = promiseRef;
      return promiseRef;
    }
    return _promise.lock();
  }

  void ProvideFileItem::setMediaRef ( Provide::MediaHandle &&hdl )
  {
    _handleRef = std::move(hdl);
  }

  Provide::MediaHandle &ProvideFileItem::mediaRef ()
  {
    return _handleRef;
  }

  void ProvideFileItem::informalMessage ( ProvideQueue &, ProvideRequestRef req, const ProvideMessage &msg  )
  {
    if ( req != _runningReq ) {
      WAR << "Received event for unknown request, ignoring" << std::endl;
      return;
    }

    if ( msg.code() == ProvideMessage::Code::ProvideStarted ) {
      MIL << "Provide File Request: "<< req->url() << " was started" << std::endl;
      auto log = provider().log();

      auto locPath = msg.value( ProvideStartedMsgFields::LocalFilename, std::string() ).asString();
      if ( !locPath.empty() )
        _targetFile = zypp::Pathname(locPath);

      locPath = msg.value( ProvideStartedMsgFields::StagingFilename, std::string() ).asString();
      if ( !locPath.empty() )
        _stagingFile = zypp::Pathname(locPath);

      if ( log ) {
        auto effUrl = req->activeUrl().value_or( zypp::Url() );
        try {
          effUrl = zypp::Url( msg.value( ProvideStartedMsgFields::Url).asString() );
        } catch( const zypp::Exception &e ) {
          ZYPP_CAUGHT(e);
        }

        AnyMap m;
        m["spec"] = _initialSpec;
        if ( log ) log->requestStart( *this, msg.requestId(), effUrl, m );
        updateState( Downloading );
      }
    }
  }

  void zyppng::ProvideFileItem::ProvideFileItem::finishReq( zyppng::ProvideQueue &queue, ProvideRequestRef finishedReq, const ProvideMessage &msg )
  {
    if ( finishedReq != _runningReq ) {
      WAR << "Received event for unknown request, ignoring" << std::endl;
      return;
    }

    if ( msg.code () == ProvideMessage::Code::ProvideFinished ) {

      auto log = provider().log();
      if ( log ) {
        AnyMap m;
        m["spec"] = _initialSpec;
        if ( log ) log->requestDone( *this, msg.requestId(), m );
      }

      MIL << "Request was successfully finished!" << std::endl;
      // request is def done
      _runningReq.reset();

      try {

        const auto locFilename = msg.value( ProvideFinishedMsgFields::LocalFilename ).asString();
        const auto cacheHit    = msg.value( ProvideFinishedMsgFields::CacheHit ).asBool();
        const auto &wConf      = queue.workerConfig();

        const bool doesDownload     = wConf.worker_type() == ProvideQueue::Config::Downloading;
        const bool fileNeedsCleanup = doesDownload || ( wConf.worker_type() == ProvideQueue::Config::CPUBound && wConf.cfg_flags() & ProvideQueue::Config::FileArtifacts );

        std::optional<zypp::ManagedFile> resFile;

        if ( doesDownload ) {

          resFile = provider().addToFileCache ( locFilename );
          if ( !resFile ) {
            if ( cacheHit ) {
              MIL << "CACHE MISS, file " << locFilename << " was already removed, queueing again" << std::endl;
              cacheMiss ( finishedReq );
              finishedReq->clearForRestart();
              enqueueRequest( finishedReq );
              return;
            } else {
              // if we reach here it seems that a new file, that was not in cache before, vanished between
              // providing it and receiving the finished message.
              // unlikely this can happen but better be safe than sorry
              cancelWithError( ZYPP_EXCPT_PTR( zypp::media::MediaException("File vanished between downloading and adding it to cache.")) );
              return;
            }
          }

        } else {
          resFile = zypp::ManagedFile( zypp::filesystem::Pathname(locFilename) );
          if ( fileNeedsCleanup )
            resFile->setDispose( zypp::filesystem::unlink );
          else
            resFile->resetDispose();
        }

        _targetFile = locFilename;

        // keep the media handle around as long as the file is used by the code
        auto resObj = std::make_shared<ProvideResourceData>();
        resObj->_mediaHandle     = this->_handleRef;
        resObj->_myFile          = *resFile;
        resObj->_resourceUrl     = *(finishedReq->activeUrl());
        resObj->_responseHeaders = msg.headers();

        auto p = promise();
        if ( p ) {
          try {
            p->setReady( expected<ProvideRes>::success( ProvideRes( resObj )) );
          } catch( const zypp::Exception &e ) {
            ZYPP_CAUGHT(e);
          }
        }

        updateState( Finished );

      } catch ( const zypp::Exception &e ) {
        ZYPP_CAUGHT(e);
        cancelWithError( std::current_exception() );
      } catch ( ...) {
        cancelWithError( std::current_exception() );
      }

    } else {
      ProvideItem::finishReq ( queue, finishedReq, msg );
    }
  }


  void zyppng::ProvideFileItem::cancelWithError( std::exception_ptr error )
  {
    if ( _runningReq ) {
      auto weakThis = weak_from_this ();
      provider().dequeueRequest ( _runningReq, error );
      if ( weakThis.expired () )
        return;
    }

    // if we reach this place for some reason finishReq was not called, lets clean up manually
    _runningReq.reset();
    auto p = promise();
    if ( p ) {
      try {
        p->setReady( expected<ProvideRes>::error( error ) );
      } catch( const zypp::Exception &e ) {
        ZYPP_CAUGHT(e);
      }
    }
    updateState( Finished );
  }

  expected<zypp::media::AuthData> ProvideFileItem::authenticationRequired ( ProvideQueue &queue, ProvideRequestRef req, const zypp::Url &effectiveUrl, int64_t lastTimestamp, const std::map<std::string, std::string> &extraFields )
  {
    zypp::Url urlToUse = effectiveUrl;
    if ( _handleRef.isValid() ) {
      // if we have a attached medium this overrules the URL we are going to ask the user about... this is how the old media backend did handle this
      // i guess there were never password protected repositories that have different credentials on the redirection targets
      auto &attachedMedia = provider().attachedMediaInfos();
      auto i = std::find_if( attachedMedia.begin(), attachedMedia.end(), [&]( const auto &m ) { return m._name == _handleRef.handle(); } );
      if ( i == attachedMedia.end() )
        return expected<zypp::media::AuthData>::error( ZYPP_EXCPT_PTR( zypp::media::MediaException("Attachment handle vanished during request.") ) );

      urlToUse = i->_attachedUrl;
    }
    return ProvideItem::authenticationRequired( queue, req, urlToUse, lastTimestamp, extraFields );
  }

  ProvideFileItem::ItemStats ProvideFileItem::makeStats ()
  {
    zypp::ByteCount providedByNow;

    bool checkStaging = false;
    if ( !_targetFile.empty() ) {
      zypp::PathInfo inf( _targetFile );
      if ( inf.isExist() && inf.isFile() )
        providedByNow = zypp::ByteCount( inf.size() );
      else
        checkStaging = true;
    }

    if ( checkStaging && !_stagingFile.empty() ) {
      zypp::PathInfo inf( _stagingFile );
      if ( inf.isExist() && inf.isFile() )
        providedByNow = zypp::ByteCount( inf.size() );
    }

    auto baseStats = ProvideItem::makeStats();
    baseStats._bytesExpected = bytesExpected();
    baseStats._bytesProvided = providedByNow;
    return baseStats;
  }

  zypp::ByteCount ProvideFileItem::bytesExpected () const
  {
    return (_initialSpec.checkExistsOnly() ? zypp::ByteCount(0) : _expectedBytes);
  }

  AttachMediaItem::AttachMediaItem( const std::vector<zypp::Url> &urls, const ProvideMediaSpec &request, ProvidePrivate &parent )
    : ProvideItem  ( parent )
    , _mirrorList  ( urls )
    , _initialSpec ( request )
  { }

  AttachMediaItem::~AttachMediaItem()
  {
    MIL << "Killing the AttachMediaItem" << std::endl;
  }

  ProvidePromiseRef<Provide::MediaHandle> AttachMediaItem::promise()
  {
    if ( !_promiseCreated ) {
      _promiseCreated = true;
      auto promiseRef = std::make_shared<ProvidePromise<Provide::MediaHandle>>( shared_this<ProvideItem>() );
      _promise = promiseRef;
      return promiseRef;
    }
    return _promise.lock();
  }

  void AttachMediaItem::initialize()
  {
    if ( state() != Uninitialized ) {
      WAR << "Double init of AttachMediaItem!" << std::endl;
      return;
    }
    updateState(Processing);

    if ( _mirrorList.empty() ) {
      cancelWithError( ZYPP_EXCPT_PTR( zypp::media::MediaException("No usable mirrors in mirrorlist.")) );
      return;
    }

    // shortcut to the provider instance
    auto &prov= provider();

    // sanitize the mirrors to contain only URLs that have same worker types
    std::vector<zypp::Url> usableMirrs;
    std::optional<ProvideQueue::Config> scheme;

    for ( auto mirrIt = _mirrorList.begin() ; mirrIt != _mirrorList.end(); mirrIt++ ) {
      const auto &s = prov.schemeConfig( prov.effectiveScheme( mirrIt->getScheme() ) );
      if ( !s ) {
        WAR << "URL: " << *mirrIt << " is not supported, ignoring!" << std::endl;
        continue;
      }
      if ( !scheme ) {
        scheme = *s;
        usableMirrs.push_back ( *mirrIt );
      } else {
        if ( scheme->worker_type () == s->worker_type () ) {
          usableMirrs.push_back( *mirrIt );
        } else {
          WAR << "URL: " << *mirrIt << " has different worker type than the primary URL: "<< usableMirrs.front() <<", ignoring!" << std::endl;
        }
      }
    }

    // save the sanitized mirrors
    _mirrorList = usableMirrs;

    if ( !scheme || _mirrorList.empty() ) {
      auto prom = promise();
      if ( prom ) {
        try {
          prom->setReady( expected<Provide::MediaHandle>::error( ZYPP_EXCPT_PTR ( zypp::media::MediaException("No valid mirrors available") )) );
        } catch( const zypp::Exception &e ) {
          ZYPP_CAUGHT(e);
        }
      }
      updateState( Finished );
      return;
    }

    // first check if there is a already attached medium we can use as well
    auto &attachedMedia = prov.attachedMediaInfos ();

    for ( auto &medium : attachedMedia ) {
      if ( medium.isSameMedium ( _mirrorList, _initialSpec ) ) {
        finishWithSuccess ( medium );
        return;
      }
    }

    for ( auto &otherItem : prov.items() ) {
      auto attachIt = std::dynamic_pointer_cast<AttachMediaItem>(otherItem);
      if ( !attachIt // not the right type
           || attachIt.get()     == this            // do not attach to ourselves
           || attachIt->state () == Uninitialized   // item was not initialized
           || attachIt->state () == Finalizing      // item cleaning up
           || attachIt->state () == Finished )      // item done
        continue;

      // does this Item attach the same medium?
      const auto sameMedium = attachIt->_initialSpec.isSameMedium( _initialSpec);
      if ( zypp::indeterminate(sameMedium) ) {
        // check the primary URLs ( should we do a full list compare? )
        if ( attachIt->_mirrorList.front() != _mirrorList.front() )
          continue;
      }
      else if ( !(bool)sameMedium )
        continue;

      MIL << "Found item providing the same medium, attaching to finished signal and waiting for it to be finished" << std::endl;

      // it does, connect to its ready signal and just wait
      _masterItemConn = connect( *attachIt, &AttachMediaItem::sigReady, *this, &AttachMediaItem::onMasterItemReady );
      return;
    }

    _workerType = scheme->worker_type();

    switch( _workerType ) {
      case ProvideQueue::Config::Downloading: {

        // if the media file is empty in the spec we can not do anything
        // simply pretend attach worked
        if( _initialSpec.mediaFile().empty() ) {
          finishWithSuccess( prov.addMedium( _workerType, _mirrorList.front(), _initialSpec ) );
          return;
        }

        // prepare the verifier with the data
        auto smvDataLocal  = MediaDataVerifier::createVerifier("SuseMediaV1");
        if ( !smvDataLocal ) {
          cancelWithError( ZYPP_EXCPT_PTR( zypp::media::MediaException("Unable to verify the medium, no verifier instance was returned.")) );
          return;
        }

        if ( !smvDataLocal->load( _initialSpec.mediaFile() ) ) {
          cancelWithError( ZYPP_EXCPT_PTR( zypp::media::MediaException("Unable to verify the medium, unable to load local verify data.")) );
          return;
        }

        _verifier = smvDataLocal;

        std::vector<zypp::Url> urls;
        urls.reserve( _mirrorList.size () );

        for ( zypp::Url url : _mirrorList ) {
          url.appendPathName ( ( zypp::str::Format("/media.%d/media") % _initialSpec.medianr() ).asString() );
          urls.push_back(url);
        }

        // for downloading schemes we ask for the /media.x/media file and check the data manually
        ProvideFileSpec spec;
        spec.customHeaders() = _initialSpec.customHeaders();

        // disable metalink
        spec.customHeaders().set( std::string(NETWORK_METALINK_ENABLED), false );

        auto req = ProvideRequest::create( *this, urls, spec );
        if ( !req ) {
          cancelWithError( req.error() );
          return;
        }
        if ( !enqueueRequest( *req ) ) {
          cancelWithError( ZYPP_EXCPT_PTR(zypp::media::MediaException("Failed to queue request")) );
          return;
        }
        updateState ( Downloading );
        break;
      }
      case ProvideQueue::Config::VolatileMount:
      case ProvideQueue::Config::SimpleMount: {

        const auto &newId = provider().nextMediaId();
        auto req = ProvideRequest::create( *this, _mirrorList, newId, _initialSpec );
        if ( !req ) {
          cancelWithError( req.error() );
          return;
        }
        if ( !enqueueRequest( *req ) ) {
          ERR << "Failed to queue request" << std::endl;
          cancelWithError( ZYPP_EXCPT_PTR(zypp::media::MediaException("Failed to queue request")) );
          return;
        }
        break;
      }
      default: {
        auto prom = promise();
        if ( prom ) {
          try {
            prom->setReady( expected<Provide::MediaHandle>::error( ZYPP_EXCPT_PTR ( zypp::media::MediaException("URL scheme does not support attaching.") )) );
          } catch( const zypp::Exception &e ) {
            ZYPP_CAUGHT(e);
          }
        }
        updateState( Finished );
        return;
      }
    }
  }

  void AttachMediaItem::finishWithSuccess( AttachedMediaInfo &medium )
  {

    updateState(Finalizing);

    // aquire a ref to keep the medium around until we notified all dependant attach operations
    // currently not really required because only the next schedule run will clean up attached medias
    // but in case that ever changes the code is safe already
    medium.ref();
    zypp::OnScopeExit autoUnref([&]{
      medium.unref();
    });

    auto prom = promise();
    try {
      if ( prom ) {
        // the ref for the result we are giving out
        medium.ref();
        try {
          prom->setReady( expected<Provide::MediaHandle>::success( Provide::MediaHandle( *static_cast<Provide*>( provider().z_func() ), medium._name) ) );
        } catch( const zypp::Exception &e ) {
          ZYPP_CAUGHT(e);
        }
      }
    }  catch ( const std::exception &e ) {
      ERR << "WTF " << e.what () << std::endl;
    }  catch ( ... ) {
      ERR << "WTF " << std::endl;
    }

    // tell others as well
    _sigReady.emit( zyppng::expected<AttachedMediaInfo *>::success(&medium) );

    prom->isReady ();

    MIL << "Before setFinished" << std::endl;
    updateState( Finished );
    return;
  }

  void AttachMediaItem::cancelWithError( std::exception_ptr error )
  {
    MIL << "Cancelling Item with error" << std::endl;
    updateState(Finalizing);

    // tell children
    _sigReady.emit( expected<AttachedMediaInfo *>::error(error) );

    if ( _runningReq ) {
      // we might get deleted when calling dequeueRequest
      auto weakThis = weak_from_this ();
      provider().dequeueRequest ( _runningReq, error );
      if ( weakThis.expired () )
        return;
    }

    // if we reach this place we had no runningReq, clean up manually
    _runningReq.reset();
    _masterItemConn.disconnect();

    auto p = promise();
    if ( p ) {
      try {
        p->setReady( expected<zyppng::Provide::MediaHandle>::error( error ) );
      } catch( const zypp::Exception &e ) {
        ZYPP_CAUGHT(e);
      }
    }
    updateState( Finished );
  }

  void AttachMediaItem::onMasterItemReady( const zyppng::expected<AttachedMediaInfo *> &result )
  {

    _masterItemConn.disconnect();

    if ( result ) {
      AttachedMediaInfo &medium = *result.get();
      finishWithSuccess(medium);
    } else {
      try {
        std::rethrow_exception ( result.error() );
      } catch ( const zypp::media::MediaRequestCancelledException & e) {
        // in case a item was cancelled, we revert to Pending state and trigger the scheduler.
        // This will make sure that all our sibilings that also depend on the master
        // can revert to pending state and we only get one new master in the next schedule run
        MIL_PRV << "Master item was cancelled, reverting to Uninitialized state and waiting for scheduler to run again" << std::endl;
        updateState (Uninitialized);
        provider().schedule( ProvidePrivate::RestartAttach );

      } catch ( ... ) {
        cancelWithError( std::current_exception() );
      }
    }
  }

  AttachMediaItemRef AttachMediaItem::create( const std::vector<zypp::Url> &urls, const ProvideMediaSpec &request, ProvidePrivate &parent )
  {
    return AttachMediaItemRef( new AttachMediaItem(urls, request, parent) );
  }

  SignalProxy<void (const zyppng::expected<AttachedMediaInfo *> &)> AttachMediaItem::sigReady()
  {
    return _sigReady;
  }

  void AttachMediaItem::finishReq ( ProvideQueue &queue, ProvideRequestRef finishedReq, const ProvideMessage &msg )
  {
    if ( finishedReq != _runningReq ) {
      WAR << "Received event for unknown request, ignoring" << std::endl;
      return;
    }

    if( _workerType == ProvideQueue::Config::Downloading ) {
      // success
      if ( msg.code() == ProvideMessage::Code::ProvideFinished ) {

        updateState(Finalizing);

        zypp::Url baseUrl = *finishedReq->activeUrl();
        // remove /media.n/media
        baseUrl.setPathName( zypp::Pathname(baseUrl.getPathName()).dirname().dirname() );

        // we got the file, lets parse it
        auto smvDataRemote = MediaDataVerifier::createVerifier("SuseMediaV1");
        if ( !smvDataRemote ) {
          return cancelWithError( ZYPP_EXCPT_PTR( zypp::media::MediaException("Unable to verify the medium, no verifier instance was returned.")) );
        }

        if ( !smvDataRemote->load( msg.value( ProvideFinishedMsgFields::LocalFilename ).asString() ) ) {
          return cancelWithError( ZYPP_EXCPT_PTR( zypp::media::MediaException("Unable to verify the medium, unable to load remote verify data.")) );
        }

        // check if we got a valid media file
        if ( !smvDataRemote->valid () ) {
          return cancelWithError( ZYPP_EXCPT_PTR( zypp::media::MediaException("Unable to verify the medium, unable to load local verify data.")) );
        }

        // check if the received file matches with the one we have in the spec
        if (! _verifier->matches( smvDataRemote ) ) {
          DBG << "expect: " << _verifier      << " medium " << _initialSpec.medianr() << std::endl;
          DBG << "remote: " << smvDataRemote  << std::endl;
          return cancelWithError( ZYPP_EXCPT_PTR( zypp::media::MediaNotDesiredException( *finishedReq->activeUrl() ) ) );
        }

        // all good, register the medium and tell all child items
        _runningReq.reset();
        return finishWithSuccess( provider().addMedium( _workerType, baseUrl, _initialSpec ) );

      } else if ( msg.code() == ProvideMessage::Code::NotFound ) {

        // simple downloading attachment we need to check the media file contents
        // in case of a error we might tolerate a file not found error in certain situations
        if ( _verifier->totalMedia () == 1 ) {
          // relaxed , tolerate a vanished media file
          _runningReq.reset();
          return finishWithSuccess( provider().addMedium( _workerType, _mirrorList.front(), _initialSpec) );
        } else {
          return ProvideItem::finishReq ( queue, finishedReq, msg );
        }
      } else {
        return ProvideItem::finishReq ( queue, finishedReq, msg );
      }
    } else {
      // real device attach
      if ( msg.code() == ProvideMessage::Code::AttachFinished ) {
        _runningReq.reset();
        return finishWithSuccess( provider().addMedium( _workerType
          , queue.weak_this<ProvideQueue>()
          , finishedReq->provideMessage().value( AttachMsgFields::AttachId ).asString()
          , *finishedReq->activeUrl()
          , _initialSpec ) );
      }
    }

    // unhandled message , let the base impl do it
    return ProvideItem::finishReq ( queue, finishedReq, msg );
  }

  expected<zypp::media::AuthData> AttachMediaItem::authenticationRequired ( ProvideQueue &queue, ProvideRequestRef req, const zypp::Url &effectiveUrl, int64_t lastTimestamp, const std::map<std::string, std::string> &extraFields )
  {
    zypp::Url baseUrl = effectiveUrl;
    if( _workerType == ProvideQueue::Config::Downloading ) {
        // remove /media.n/media
        baseUrl.setPathName( zypp::Pathname(baseUrl.getPathName()).dirname().dirname() );
    }
    return ProvideItem::authenticationRequired( queue, req, baseUrl, lastTimestamp, extraFields );
  }

}
