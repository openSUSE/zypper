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
#include <zypp/media/CurlHelper.h>

#include "basicdownloader_p.h"

namespace zyppng {

  void BasicDownloaderStateBase::enter()
  {
    _request.reset();

    auto &sm = stateMachine();
    const auto &spec = sm._spec;
    auto url = spec.url();

    TransferSettings set = spec.settings();
    NetworkRequestError dummyErr;
    MirrorControl::MirrorHandle mirror;

    if ( _fileMirrors.size() ) {
      const auto res = prepareNextMirror();
      // for Delayed or OK we can just continue here
      if ( res != MirrorHandlingStateBase::Failed )
        return;
    }
    failedToPrepare();
  }

  void BasicDownloaderStateBase::mirrorReceived( MirrorControl::MirrorPick mirror )
  {
    auto &sm = stateMachine();
    auto url = sm._spec.url();
    auto set = sm._spec.settings();

    auto err = setupMirror( mirror, url, set );
    if ( err.isError() ) {
      WAR_MEDIA << "Setting up mirror " << mirror.second->mirrorUrl << " failed with error: " << err.toString() << "(" << err.nativeErrorString() << "), falling back to original URL." << std::endl;
      failedToPrepare();
    }
    startWithMirror( mirror.second, url, set );
  }

  void BasicDownloaderStateBase::failedToPrepare()
  {
    startWithoutMirror();
  }

  void BasicDownloaderStateBase::startWithMirror( MirrorControl::MirrorHandle mirror, const zypp::Url &url, const TransferSettings &set )
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;

    _request = std::make_shared<Request>( ::internal::clearQueryString(url), spec.targetPath() ) ;
    _request->_myMirror = mirror;
    _request->_originalUrl = url;

    if ( _chksumtype  && _chksumVec ) {
      std::shared_ptr<zypp::Digest> fileDigest = std::make_shared<zypp::Digest>();
      if ( fileDigest->create( *_chksumtype ) )
        // to run the checksum for the full file we need to request one big range with open end
        _request->addRequestRange( 0, 0, fileDigest, *_chksumVec );
    }
    _request->transferSettings() = set;

    if ( !initializeRequest( _request ) ) {
      return failed( "Failed to initialize request" );
    }

    if ( stateMachine().previousState() && *stateMachine().previousState() != Download::InitialState ) {
      //make sure this request will run asap
      _request->setPriority( sm._defaultSubRequestPriority );
    }

    _request->connectSignals( *this );
    sm._requestDispatcher->enqueue( _request );
  }

  void BasicDownloaderStateBase::startWithoutMirror()
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;

    auto url = spec.url();
    auto set = spec.settings();
    auto err = sm.safeFillSettingsFromURL( url, set );
    if ( err.isError() )
      return failed( std::move(err) );
    startWithMirror( nullptr, url, set );
  }

  void BasicDownloaderStateBase::exit()
  {
    _request->disconnectSignals();
    _request.reset();
  }

  bool BasicDownloaderStateBase::initializeRequest( std::shared_ptr<Request> )
  {
    return true;
  }

  void BasicDownloaderStateBase::gotFinished()
  {
    _sigFinished.emit();
  }

  void BasicDownloaderStateBase::failed( std::string &&str )
  {
    failed( NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, std::move(str) ) );
  }

  void BasicDownloaderStateBase::failed( NetworkRequestError &&err )
  {
    _error = std::move( err );
    _sigFailed.emit();
  }

  void BasicDownloaderStateBase::onRequestStarted( NetworkRequest & )
  {
    auto &sm = stateMachine();
    if ( !sm._emittedSigStart ) {
      sm._emittedSigStart = true;
      stateMachine()._sigStarted.emit( *stateMachine().z_func() );
    }
    if ( _request->_myMirror )
      _request->_myMirror->startTransfer();
  }

  void BasicDownloaderStateBase::handleRequestProgress( NetworkRequest &req, off_t dltotal, off_t dlnow )
  {
    auto &sm = stateMachine();
    const off_t expFSize = sm._spec.expectedFileSize();
    if ( expFSize  > 0 && expFSize < dlnow ) {
      sm._requestDispatcher->cancel( req, NetworkRequestErrorPrivate::customError( NetworkRequestError::ExceededMaxLen ) );
      return;
    }
    return sm._sigProgress.emit( *sm.z_func(), (expFSize > 0 ? expFSize : dltotal), dlnow );
  }

  void BasicDownloaderStateBase::onRequestProgress( NetworkRequest &req, off_t dltotal, off_t dlnow, off_t, off_t )
  {
    handleRequestProgress( req, dltotal, dlnow );
  }

  void BasicDownloaderStateBase::onRequestFinished( NetworkRequest &req, const NetworkRequestError &err )
  {
    auto lck = stateMachine().z_func()->shared_from_this();
    auto &sm = stateMachine();

    if ( _request->_myMirror )
      _request->_myMirror->finishTransfer( !err.isError() );

    if ( req.hasError() ) {
      // if we get authentication failure we try to recover
      if ( sm.handleRequestAuthError( _request, err ) ) {
        //make sure this request will run asap
        _request->setPriority( sm._defaultSubRequestPriority );
        sm._requestDispatcher->enqueue( _request );
        return;
      }
      MIL_MEDIA << "Downloading on " << stateMachine()._spec.url() << " failed with error "<< err.toString() << " " << err.nativeErrorString() << std::endl;
      return failed( NetworkRequestError(err) );
    }

    gotFinished();
  }

}
