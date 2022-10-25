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
#ifndef ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_PREPAREMULTI_P_H_INCLUDED
#define ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_PREPAREMULTI_P_H_INCLUDED

#include "base_p.h"
#include <zypp-core/zyppng/base/statemachine.h>

#include <zypp-curl/parser/MediaBlockList>

namespace zyppng {

  struct DlNormalFileState;
  struct DlMetalinkState;
  struct FinishedState;

#if ENABLE_ZCHUNK_COMPRESSION
  struct DLZckHeadState;
#endif

  /*!
   * Metalink peparation state implementation, this state parses the downloaded Metalink file
   * and registeres all mirrors in \ref MirrorControl. Once the first mirror becomes ready it passes on
   * to the next state.
   */
  struct PrepareMultiState : public zyppng::SimpleState< DownloadPrivate, Download::PrepareMulti, false > {

    using Request = DownloadPrivateBase::Request;

    enum Mode {
      Zsync,
      Metalink
    };

    PrepareMultiState ( std::shared_ptr<Request> oldReq, Mode m, DownloadPrivate &parent );

    void enter ();
    void exit ();

    const NetworkRequestError &error () const {
      return _error;
    }

    SignalProxy< void () > sigFinished() {
      return _sigFinished;
    }
    SignalProxy< void () > sigFailed() {
      return _sigFailed;
    }
    SignalProxy< void () > sigFallback() {
      return _sigFallback;
    }

    std::shared_ptr<DlNormalFileState>  fallbackToNormalTransition ();
    std::shared_ptr<DlMetalinkState>    transitionToMetalinkDl ();
    std::shared_ptr<FinishedState>      transitionToFinished ();
#if ENABLE_ZCHUNK_COMPRESSION
    std::shared_ptr<DLZckHeadState>     transitionToZckHeadDl ();
    bool toZckHeadDownloadGuard () const;
#endif

    bool toMetalinkDownloadGuard () const;

    std::vector<Url> _mirrors;
    zypp::media::MediaBlockList _blockList;

  private:
    sigc::connection _mirrorControlReadyConn;

    void onMirrorsReady ();
#if ENABLE_ZCHUNK_COMPRESSION
    bool _haveZckData = false; //< do we have zck data ready
#endif
    Mode _mode; //< wether we should expect a metalink or zsync file
    std::shared_ptr<Request> _oldRequest; //< exising request of previous states, that the next states might reuse
    NetworkRequestError _error;
    Signal< void () > _sigFinished;
    Signal< void () > _sigFallback;
    Signal< void () > _sigFailed;
  };

}

#endif
