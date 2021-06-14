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

#include "mirrorhandling_p.h"

namespace zyppng {

  MirrorHandlingStateBase::MirrorHandlingStateBase( DownloadPrivate &parent )
    : BasicState(parent)
  { }

  MirrorHandlingStateBase::~MirrorHandlingStateBase()
  {
    _sigMirrorsReadyConn.disconnect();
  }

  MirrorHandlingStateBase::PrepareResult MirrorHandlingStateBase::prepareNextMirror()
  {
    auto &sm = stateMachine();
    auto res = sm._mirrorControl->pickBestMirror( _fileMirrors );
    if ( res.code == MirrorControl::PickResult::Again ) {
      if ( !_sigMirrorsReadyConn )
        _sigMirrorsReadyConn = sm._mirrorControl->connectFunc( &MirrorControl::sigNewMirrorsReady, [this](){
          _sigMirrorsReadyConn.disconnect();
          prepareNextMirror();
        }, *this );
      return Delayed;
    } else if ( res.code == MirrorControl::PickResult::Unknown ) {
      failedToPrepare();
      return Failed;
    }
    mirrorReceived( res.result );
    return Ok;
  }

  NetworkRequestError MirrorHandlingStateBase::setupMirror( const MirrorControl::MirrorPick &pick, Url &url, TransferSettings &set )
  {
    auto &sm = stateMachine();
    Url myUrl;
    TransferSettings settings;

    myUrl = *pick.first;

    settings = sm._spec.settings();
    //if this is a different host than the initial request, we reset username/password
    if ( myUrl.getHost() != sm._spec.url().getHost() ) {
      settings.setUsername( std::string() );
      settings.setPassword( std::string() );
      settings.setAuthType( std::string() );
    }

    NetworkRequestError err = sm.safeFillSettingsFromURL( myUrl, settings );
    if ( err.type() != NetworkRequestError::NoError )
      return err;

    url = myUrl;
    set = settings;
    return err;
  }

}
