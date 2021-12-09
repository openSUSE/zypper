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
#ifndef ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_ZCK_P_H_INCLUDED
#define ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_ZCK_P_H_INCLUDED

#include "base_p.h"
#include "basicdownloader_p.h"
#include "rangedownloader_p.h"
#include <zypp-core/zyppng/base/statemachine.h>

namespace zyppng {

#if ENABLE_ZCHUNK_COMPRESSION

  struct DLZckState;
  struct FinishedState;

  bool isZchunkFile ( const zypp::Pathname &file );

  /*!
   * Zchunk header download state implementation. This downloads only the
   * zck header to calculate which ranges/blocks are required for download.
   */
  struct DLZckHeadState : public BasicDownloaderStateBase {
    static constexpr auto stateId = Download::DlZChunkHead;

    DLZckHeadState( std::vector<Url> &&mirrors, DownloadPrivate &parent );
    DLZckHeadState( std::vector<Url> &&mirrors, std::shared_ptr<Request> &&oldReq, DownloadPrivate &parent );

    virtual bool initializeRequest( std::shared_ptr<Request> &r ) override;
    virtual void gotFinished () override;

    std::shared_ptr<DLZckState> transitionToDlZckState ();

    SignalProxy< void () > sigFinished() {
      return _sigFinished;
    }
    SignalProxy< void () > sigFailed() {
      return _sigFailed;
    }
  };

  /*!
   * State implementation for the actual zck download. This downloads the
   * zck file from different mirrors in ranges.
   */
  struct DLZckState : public RangeDownloaderBaseState {

    static constexpr auto stateId = Download::DlZChunk;

    DLZckState ( std::vector<Url> &&mirrors, DownloadPrivate &parent );

    void enter ();
    void exit ();

    std::shared_ptr<FinishedState> transitionToFinished ();

    SignalProxy< void () > sigFinished() {
      return _sigFinished;
    }

    SignalProxy< void () > sigFallback() {
      return _sigFailed;
    }

    void setFinished() override;

  };

#endif

}

#endif
