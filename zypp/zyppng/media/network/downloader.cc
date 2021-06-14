/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#include <zypp/zyppng/media/network/private/downloader_p.h>
#include <zypp/zyppng/media/network/private/mediadebug_p.h>
#include <zypp/zyppng/media/network/private/networkrequesterror_p.h>
#include <zypp/zyppng/media/network/networkrequestdispatcher.h>
#include <zypp/media/TransferSettings.h>
#include <zypp/media/CurlHelper.h>
#include <zypp/media/MediaException.h>
#include <zypp/base/String.h>
#include <zypp/media/CredentialManager.h>
#include <zypp/ZConfig.h>

namespace zyppng {

  DownloadPrivateBase::DownloadPrivateBase(Downloader &parent, std::shared_ptr<NetworkRequestDispatcher> requestDispatcher, std::shared_ptr<MirrorControl> mirrors, DownloadSpec &&spec, Download &p)
    : BasePrivate(p)
    , _requestDispatcher ( std::move(requestDispatcher) )
    , _mirrorControl( std::move(mirrors) )
    , _spec( std::move(spec) )
    , _parent( &parent )
  {}

  DownloadPrivateBase::~DownloadPrivateBase()
  { }

  bool DownloadPrivateBase::handleRequestAuthError( std::shared_ptr<Request> req, const zyppng::NetworkRequestError &err )
  {
    //Handle the auth errors explicitely, we need to give the user a way to put in new credentials
    //if we get valid new credentials we can retry the request
    bool retry = false;
    if ( err.type() == NetworkRequestError::Unauthorized || err.type() == NetworkRequestError::AuthFailed ) {

      MIL << "Authentication failed for " << req->url() << " trying to recover." << std::endl;

      zypp::media::CredentialManager cm( zypp::media::CredManagerOptions( zypp::ZConfig::instance().repoManagerRoot()) );
      auto authDataPtr = cm.getCred( req->url() );

      // get stored credentials
      NetworkAuthData_Ptr cmcred( authDataPtr ? new NetworkAuthData( *authDataPtr ) : new NetworkAuthData() );
      TransferSettings &ts = req->transferSettings();

      // We got credentials from store, _triedCredFromStore makes sure we just try the auth from store once
      // if its timestamp does not change between 2 retries
      if ( cmcred && ( !req->_triedCredFromStore || req->_authTimestamp < cmcred->lastDatabaseUpdate() ) ) {
        MIL << "got stored credentials:" << std::endl << *cmcred << std::endl;
        ts.setUsername( cmcred->username() );
        ts.setPassword( cmcred->password() );
        retry = true;
        req->_triedCredFromStore = true;
        _lastTriedAuthTime = req->_authTimestamp = cmcred->lastDatabaseUpdate();
      } else {

        //we did not get credentials from the store, emit a signal that allows
        //setting new auth data

        NetworkAuthData credFromUser;
        credFromUser.setUrl( req->url() );

        //in case we got a auth hint from the server the error object will contain it
        std::string authHint = err.extraInfoValue("authHint", std::string());

        //preset from store if we found something
        if ( cmcred && !cmcred->username().empty() )
          credFromUser.setUsername( cmcred->username() );

        _sigAuthRequired.emit( *z_func(), credFromUser, authHint );
        if ( credFromUser.valid() ) {
          MIL << "Got user provided credentials" << std::endl;
          ts.setUsername( credFromUser.username() );
          ts.setPassword( credFromUser.password() );

          // set available authentication types from the error
          if ( credFromUser.authType() == CURLAUTH_NONE )
            credFromUser.setAuthType( authHint );

          // set auth type (seems this must be set _after_ setting the userpwd)
          if ( credFromUser.authType()  != CURLAUTH_NONE ) {
            // FIXME: only overwrite if not empty?
            req->transferSettings().setAuthType(credFromUser.authTypeAsString());
          }

          cm.addCred( credFromUser );
          cm.save();

          // potentially setting this after the file has been touched we might miss a change
          // in a later loop, if another update to the store happens right in the few ms difference
          // between the actual file write and calling time() here. However this is highly unlikely
          _lastTriedAuthTime = req->_authTimestamp = time( nullptr ) ;

          retry = true;
        }
      }
    }
    return retry;
  }

#if ENABLE_ZCHUNK_COMPRESSION
  bool DownloadPrivateBase::hasZckInfo() const
  {
    if ( zypp::indeterminate(_specHasZckInfo) )
      _specHasZckInfo = ( _spec.headerSize() > 0 && isZchunkFile( _spec.deltaFile() ) );
    return bool(_specHasZckInfo);
  }
#endif

  void DownloadPrivateBase::Request::disconnectSignals()
  {
    _sigStartedConn.disconnect();
    _sigProgressConn.disconnect();
    _sigFinishedConn.disconnect();
  }

  DownloadPrivate::DownloadPrivate(Downloader &parent, std::shared_ptr<NetworkRequestDispatcher> requestDispatcher, std::shared_ptr<MirrorControl> mirrors, DownloadSpec &&spec, Download &p)
    : DownloadPrivateBase( parent, std::move(requestDispatcher), std::move(mirrors), std::move(spec), p )
  { }

  void DownloadPrivate::init()
  {
    Base::connectFunc( *this, &DownloadStatemachine<DownloadPrivate>::sigFinished, [this](){
      DownloadPrivateBase::_sigFinished.emit( *z_func() );
    } );

    Base::connectFunc( *this, &DownloadStatemachine<DownloadPrivate>::sigStateChanged, [this]( const auto state ){
      DownloadPrivateBase::_sigStateChanged.emit( *z_func(), state );
    } );
  }

  void DownloadPrivate::start()
  {
    auto cState = currentState();
    if ( !cState )
      DownloadStatemachine<DownloadPrivate>::start();

    cState = currentState();
    if ( *cState != Download::InitialState && *cState != Download::Finished ) {
      // the state machine has advaned already, we can only restart it in a finished state
        return;
    }

    //reset state variables
    _emittedSigStart = false;
    _specHasZckInfo  = zypp::indeterminate;

    // restart the statemachine
    if ( cState == Download::Finished )
      DownloadStatemachine<DownloadPrivate>::start();

    //jumpstart the process
    state<InitialState>()->initiate();
  }


  NetworkRequestError DownloadPrivateBase::safeFillSettingsFromURL( const Url &url, TransferSettings &set)
  {
    auto buildExtraInfo = [this, &url](){
      std::map<std::string, boost::any> extraInfo;
      extraInfo.insert( {"requestUrl", url } );
      extraInfo.insert( {"filepath", _spec.targetPath() } );
      return extraInfo;
    };

    NetworkRequestError res;
    try {
      ::internal::fillSettingsFromUrl( url, set );
      if ( _spec.settings().proxy().empty() )
        ::internal::fillSettingsSystemProxy( url, set );

      /* Fixes bsc#1174011 "auth=basic ignored in some cases"
       * We should proactively add the password to the request if basic auth is configured
       * and a password is available in the credentials but not in the URL.
       *
       * We will be a bit paranoid here and require that the URL has a user embedded, otherwise we go the default route
       * and ask the server first about the auth method
       */
      if ( set.authType() == "basic"
           && set.username().size()
           && !set.password().size() ) {
        zypp::media::CredentialManager cm( zypp::media::CredManagerOptions( zypp::ZConfig::instance().repoManagerRoot()) );
        const auto cred = cm.getCred( url );
        if ( cred && cred->valid() ) {
          if ( !set.username().size() )
            set.setUsername(cred->username());
          set.setPassword(cred->password());
        }
      }

    } catch ( const zypp::media::MediaBadUrlException & e ) {
      res = NetworkRequestErrorPrivate::customError( NetworkRequestError::MalformedURL, e.asString(), buildExtraInfo() );
    } catch ( const zypp::media::MediaUnauthorizedException & e ) {
      res = NetworkRequestErrorPrivate::customError( NetworkRequestError::AuthFailed, e.asString(), buildExtraInfo() );
    } catch ( const zypp::Exception & e ) {
      res = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, e.asString(), buildExtraInfo() );
    }
    return res;
  }

  Download::Download(zyppng::Downloader &parent, std::shared_ptr<zyppng::NetworkRequestDispatcher> requestDispatcher, std::shared_ptr<zyppng::MirrorControl> mirrors, zyppng::DownloadSpec &&spec)
    : Base( *new DownloadPrivate( parent, std::move(requestDispatcher), std::move(mirrors), std::move(spec), *this )  )
  { }

  ZYPP_IMPL_PRIVATE(Download)

  Download::~Download()
  {
    if ( state() != InitialState && state() != Finished )
      cancel();
  }

  Download::State Download::state() const
  {
    const auto &s = d_func()->currentState();
    if ( !s )
      return Download::InitialState;
    return *s;
  }

  NetworkRequestError Download::lastRequestError() const
  {
    if ( state() == Finished ) {
      return d_func()->state<FinishedState>()->_error;
    }
    return NetworkRequestError();
  }

  bool Download::hasError() const
  {
    return lastRequestError().isError();
  }

  std::string Download::errorString() const
  {
    const auto &lReq = lastRequestError();
    if (! lReq.isError() ) {
      return {};
    }

    return ( zypp::str::Format("%1%(%2%)") % lReq.toString() % lReq.nativeErrorString() );
  }

  void Download::start()
  {
    d_func()->start();
  }

  void Download::prioritize()
  {
    Z_D();

    if ( !d->_requestDispatcher )
      return;

    d->_defaultSubRequestPriority = NetworkRequest::Critical;

    // we only reschedule requests when we are in a state that downloads in blocks
    d->visitState( []( auto &s ){
      using T = std::decay_t<decltype (s)>;
      if constexpr ( std::is_same_v<T, DlMetalinkState>
#if ENABLE_ZCHUNK_COMPRESSION
          || std::is_same_v<T, DLZckState>
#endif
      ) {
        s.reschedule();
      }
    });
  }

  void Download::cancel()
  {
    Z_D();
    d->forceState ( std::make_unique<FinishedState>( NetworkRequestErrorPrivate::customError( NetworkRequestError::Cancelled, "Download was cancelled explicitely" ), *d_func() ) );
  }

  DownloadSpec &Download::spec()
  {
    return d_func()->_spec;
  }

  const DownloadSpec &Download::spec() const
  {
    return d_func()->_spec;
  }

  uint64_t Download::lastAuthTimestamp() const
  {
    return d_func()->_lastTriedAuthTime;
  }

  zyppng::NetworkRequestDispatcher &Download::dispatcher() const
  {
    return *d_func()->_requestDispatcher;
  }

  SignalProxy<void (Download &req)> Download::sigStarted()
  {
    return d_func()->_sigStarted;
  }

  SignalProxy<void (Download &req, Download::State state)> Download::sigStateChanged()
  {
    return d_func()->DownloadPrivateBase::_sigStateChanged;
  }

  SignalProxy<void (zyppng::Download &req, off_t dlnow)> zyppng::Download::sigAlive()
  {
    return d_func()->_sigAlive;
  }

  SignalProxy<void (Download &req, off_t dltotal, off_t dlnow)> Download::sigProgress()
  {
    return d_func()->_sigProgress;
  }

  SignalProxy<void (Download &req)> Download::sigFinished()
  {
    return d_func()->DownloadPrivateBase::_sigFinished;
  }

  SignalProxy<void (zyppng::Download &req, zyppng::NetworkAuthData &auth, const std::string &availAuth)> Download::sigAuthRequired()
  {
    return d_func()->_sigAuthRequired;
  }

  DownloaderPrivate::DownloaderPrivate(std::shared_ptr<MirrorControl> mc, Downloader &p)
    : BasePrivate(p)
    , _mirrors( std::move(mc) )
  {
    _requestDispatcher = std::make_shared<NetworkRequestDispatcher>( );
    if ( !_mirrors ) {
      _mirrors = MirrorControl::create();
    }
  }

  void DownloaderPrivate::onDownloadStarted(Download &download)
  {
    _sigStarted.emit( *z_func(), download );
  }

  void DownloaderPrivate::onDownloadFinished( Download &download )
  {
    _sigFinished.emit( *z_func(), download );

    auto it = std::find_if( _runningDownloads.begin(), _runningDownloads.end(), [ &download ]( const std::shared_ptr<Download> &dl){
      return dl.get() == &download;
    });

    if ( it != _runningDownloads.end() ) {
      //make sure this is not deleted before all user code was done
      _runningDownloads.erase( it );
    }

    if ( _runningDownloads.empty() )
      _queueEmpty.emit( *z_func() );
  }

  ZYPP_IMPL_PRIVATE(Downloader)

  Downloader::Downloader( )
    : Base ( *new DownloaderPrivate( {}, *this ) )
  {

  }

  Downloader::Downloader( std::shared_ptr<MirrorControl> mc )
    : Base ( *new DownloaderPrivate( mc, *this ) )
  { }

  Downloader::~Downloader()
  {
    Z_D();
    while ( d->_runningDownloads.size() ) {
      d->_runningDownloads.back()->cancel();
      d->_runningDownloads.pop_back();
    }
  }

  std::shared_ptr<Download> Downloader::downloadFile(const zyppng::DownloadSpec &spec )
  {
    Z_D();
    std::shared_ptr<Download> dl ( new Download ( *this, d->_requestDispatcher, d->_mirrors, DownloadSpec(spec) ) );

    d->_runningDownloads.push_back( dl );
    dl->connect( &Download::sigFinished, *d, &DownloaderPrivate::onDownloadFinished );
    d->_requestDispatcher->run();

    return dl;
  }

  std::shared_ptr<NetworkRequestDispatcher> Downloader::requestDispatcher() const
  {
    return d_func()->_requestDispatcher;
  }

  SignalProxy<void (Downloader &parent, Download &download)> Downloader::sigStarted()
  {
    return d_func()->_sigStarted;
  }

  SignalProxy<void (Downloader &parent, Download &download)> Downloader::sigFinished()
  {
    return d_func()->_sigFinished;
  }

  SignalProxy<void (Downloader &parent)> Downloader::queueEmpty()
  {
    return d_func()->_queueEmpty;
  }

}
