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
#include <zypp/media/CurlHelper.h>

#include "detectmeta_p.h"
#include "metalinkinfo_p.h"

namespace zyppng {

  DetectMetalinkState::DetectMetalinkState(DownloadPrivate &parent) : SimpleState( parent ){
    MIL_MEDIA << "Creating DetectMetalinkState" << std::endl;
  }

  void DetectMetalinkState::enter()
  {
    _request.reset();
    _gotMetalink = false;

    auto &sm = stateMachine();
    const auto &url = sm._spec.url();

    MIL_MEDIA << "Detecting if metalink is available on " << url << std::endl;

    _request = std::make_shared<Request>( ::internal::clearQueryString( url ), zypp::Pathname("/dev/null") );

    _request->_originalUrl = url;
    _request->transferSettings() = sm._spec.settings();
    _request->transferSettings().addHeader("Accept: */*, application/metalink+xml, application/metalink4+xml");
    _request->setOptions( _request->options() | NetworkRequest::HeadRequest );

    _request->connectSignals( *this );
    sm._requestDispatcher->enqueue( _request );
  }

  void DetectMetalinkState::exit()
  {
    if ( _request ) {
      _request->disconnectSignals();
      _request.reset();
    }
  }

  void DetectMetalinkState::onRequestStarted( NetworkRequest & )
  {
    stateMachine()._sigStarted.emit( *stateMachine().z_func() );
  }

  void DetectMetalinkState::onRequestProgress( NetworkRequest &, off_t, off_t dlnow, off_t, off_t )
  {
    stateMachine()._sigAlive.emit( *stateMachine().z_func(), dlnow );
  }

  void DetectMetalinkState::onRequestFinished( NetworkRequest &req, const NetworkRequestError &err )
  {
    auto lck = stateMachine().z_func()->shared_from_this();
    if ( req.hasError() ) {
      WAR << "Detecing if metalink is possible for url " << req.url() << " failed with error " << err.toString() << " falling back to download without metalink." << std::endl;
      _error = err;
      _gotMetalink = false;
      return _sigFinished.emit();
    }

    std::string cType = req.contentType();
    _gotMetalink = ( cType.find("application/metalink+xml") == 0 || cType.find("application/metalink4+xml") == 0 );
    MIL << "Metalink detection result on url " << req.url() << " is " << _gotMetalink << std::endl;
    _sigFinished.emit();
  }

  std::shared_ptr<DlMetaLinkInfoState> DetectMetalinkState::toDlMetaLinkInfoState()
  {
    _request->disconnectSignals();
    auto nState = std::make_shared<DlMetaLinkInfoState>( std::move( _request ), stateMachine() );
    _request = nullptr;
    return nState;
  }

  bool DetectMetalinkState::toSimpleDownloadGuard() const
  {
#if ENABLE_ZCHUNK_COMPRESSION
    return !toMetalinkGuard() && !toZckHeadDownloadGuard();
#else
    return !toMetalinkGuard();
#endif
  }

#if ENABLE_ZCHUNK_COMPRESSION
  bool DetectMetalinkState::toZckHeadDownloadGuard() const
  {
    return !toMetalinkGuard() && stateMachine().hasZckInfo();
  }

  std::shared_ptr<DLZckHeadState> DetectMetalinkState::toDLZckHeadState()
  {
    // we have no mirrors, the range downloader would need to fall back to using the base URL
    if ( _error.isError() || !_request )
      return std::make_shared<DLZckHeadState>( std::vector<Url> { stateMachine()._spec.url() }, stateMachine() );
    else {
      // reuse our request
      _request->disconnectSignals();
      auto nstate = std::make_shared<DLZckHeadState>( std::vector<Url> { stateMachine()._spec.url() }, std::move(_request), stateMachine() );
      _request = nullptr;
      return nstate;
    }
  }
#endif

}
