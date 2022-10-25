/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/

#include <zypp-curl/ng/network/private/downloader_p.h>
#include <zypp-curl/ng/network/private/mediadebug_p.h>

#include "initial_p.h"
#if ENABLE_ZCHUNK_COMPRESSION
#include "zck_p.h"
#endif

namespace zyppng {

  void InitialState::enter(){ MIL_MEDIA << "Entering initial state"  << std::endl; }

  void InitialState::exit(){  MIL_MEDIA << "Leaving initial state"  << std::endl;  }

  void InitialState::initiate()
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;

    if ( spec.checkExistsOnly() ) {
      MIL_MEDIA << "Check exists only enabled" << std::endl;
      return _sigTransitionToDlNormalFileState.emit();
    }

#if ENABLE_ZCHUNK_COMPRESSION
    bool deltaZck = isZchunkFile( spec.deltaFile() );
#endif
    if ( spec.metalinkEnabled() ) {
#if ENABLE_ZCHUNK_COMPRESSION
      if ( deltaZck && spec.headerSize() > 0 ) {
        MIL_MEDIA << "We might have a zck file, detecting metalink first" << std::endl;
        return _sigTransitionToDetectMetalinkState.emit();
      }
#endif
      MIL_MEDIA << "No zchunk data available but metalink requested, going to download metalink directly." << std::endl;
      return _sigTransitionToDlMetaLinkInfoState.emit();
    }

#if ENABLE_ZCHUNK_COMPRESSION
    // no Metalink, maybe we can directly download zck
    if ( deltaZck && spec.headerSize() > 0 ) {
      MIL_MEDIA << "No metalink but zckunk data availble trying to download ZckHead directly." << std::endl;
      return _sigTransitionToDLZckHeaderState.emit();
    }
#endif
    MIL_MEDIA << "Fallback to normal DL" << std::endl;
    _sigTransitionToDlNormalFileState.emit();
  }

#if ENABLE_ZCHUNK_COMPRESSION
  std::shared_ptr<DLZckHeadState> InitialState::toDLZckHeadState()
  {
    // we have no mirrors, the range downloader would need to fall back to using the base URL
    return std::make_shared<DLZckHeadState>( std::vector<Url> { stateMachine()._spec.url() }, stateMachine() );
  }
#endif


}
