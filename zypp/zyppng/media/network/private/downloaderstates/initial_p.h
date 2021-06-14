/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_INITIAL_P_H_INCLUDED
#define ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_INITIAL_P_H_INCLUDED

#include "base_p.h"
#include <zypp-core/zyppng/base/statemachine.h>

namespace zyppng {

#if ENABLE_ZCHUNK_COMPRESSION
  struct DLZckHeadState;
#endif

  /*!
   * Initial state implementation, this state is used only to kickstart the statemachine
   */
  struct InitialState : public zyppng::SimpleState< DownloadPrivate, Download::InitialState, false > {

    InitialState ( DownloadPrivate &parent ) : SimpleState( parent ){}

    void enter ();;
    void exit ();

    void initiate();

    SignalProxy< void () > sigTransitionToDetectMetalinkState() {
      return _sigTransitionToDetectMetalinkState;
    }

    SignalProxy< void () > sigTransitionToDlMetaLinkInfoState() {
      return _sigTransitionToDlMetaLinkInfoState;
    }

#if ENABLE_ZCHUNK_COMPRESSION
    SignalProxy< void () > sigTransitionToDLZckHeaderState() {
      return _sigTransitionToDLZckHeaderState;
    }
#endif

    SignalProxy< void () > sigTransitionToDlNormalFileState() {
      return _sigTransitionToDlNormalFileState;
    }

#if ENABLE_ZCHUNK_COMPRESSION
    std::shared_ptr<DLZckHeadState> toDLZckHeadState ();
#endif

  private:
    Signal<void()> _sigTransitionToDetectMetalinkState;
    Signal<void()> _sigTransitionToDlMetaLinkInfoState;
#if ENABLE_ZCHUNK_COMPRESSION
    Signal<void()> _sigTransitionToDLZckHeaderState;
#endif
    Signal<void()> _sigTransitionToDlNormalFileState;
  };

}

#endif
