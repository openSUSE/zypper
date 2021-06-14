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
#ifndef ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADER_P_H_INCLUDED
#define ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADER_P_H_INCLUDED

#include <zypp-core/zyppng/base/statemachine.h>

#include "downloaderstates/base_p.h"
#include "downloaderstates/initial_p.h"
#include "downloaderstates/detectmeta_p.h"
#include "downloaderstates/metalinkinfo_p.h"
#include "downloaderstates/preparemulti_p.h"
#include "downloaderstates/metalink_p.h"
#include "downloaderstates/normal_p.h"
#include "downloaderstates/final_p.h"
#if ENABLE_ZCHUNK_COMPRESSION
#include "downloaderstates/zck_p.h"
#endif

namespace zyppng {

  /**
   * our known states:
   * InitialState         //< initial state before we start downloading
   * DetectMetalinkState  //< First attempt to get the zchunk header, but we might receive metalink data instead
   * DlMetaLinkInfoState  //< We got Metalink, lets get the full metalink file or we got no zchunk in the first place
   * PrepareMultiState    //< Parsing the metalink file and preparing the mirrors
   * DLZckHeadState       //< Download the ZChunk Header
   * DLZckState           //< Download the File in ZChunk Mode
   * DlMetalinkState      //< Download the File in Metalink Mode
   * DlNormalFileState    //< Simple Plain download, no chunking
   * FinishedState        //< We are done
   */

  template <typename Derived>
  using DownloadStatemachine = Statemachine< Derived, Download::State,
    //          Source State,             State Change Event                     TargetState,    Transition Condition,  Transition operation
    Transition< InitialState, &InitialState::sigTransitionToDetectMetalinkState, DetectMetalinkState >,
    Transition< InitialState, &InitialState::sigTransitionToDlMetaLinkInfoState, DlMetaLinkInfoState >,
#if ENABLE_ZCHUNK_COMPRESSION
    Transition< InitialState, &InitialState::sigTransitionToDLZckHeaderState,    DLZckHeadState, DefaultStateCondition, &InitialState::toDLZckHeadState >,
#endif
    Transition< InitialState, &InitialState::sigTransitionToDlNormalFileState,   DlNormalFileState >,

    Transition< DetectMetalinkState, &DetectMetalinkState::sigFinished,   DlMetaLinkInfoState, &DetectMetalinkState::toMetalinkGuard >,
#if ENABLE_ZCHUNK_COMPRESSION
    Transition< DetectMetalinkState, &DetectMetalinkState::sigFinished,   DLZckHeadState,      &DetectMetalinkState::toZckHeadDownloadGuard, &DetectMetalinkState::toDLZckHeadState  >,
#endif
    Transition< DetectMetalinkState, &DetectMetalinkState::sigFinished,   DlNormalFileState,   &DetectMetalinkState::toSimpleDownloadGuard >,

    Transition< DlMetaLinkInfoState, &DlMetaLinkInfoState::sigFinished,    FinishedState, DefaultStateCondition, &DlMetaLinkInfoState::transitionToFinished >,
    Transition< DlMetaLinkInfoState, &DlMetaLinkInfoState::sigGotMetalink, PrepareMultiState>,
    Transition< DlMetaLinkInfoState, &DlMetaLinkInfoState::sigFailed,      FinishedState, DefaultStateCondition, &DlMetaLinkInfoState::transitionToFinished >,

    Transition< PrepareMultiState, &PrepareMultiState::sigFinished,   DlMetalinkState,  &PrepareMultiState::toMetalinkDownloadGuard , &PrepareMultiState::transitionToMetalinkDl >,
#if ENABLE_ZCHUNK_COMPRESSION
    Transition< PrepareMultiState, &PrepareMultiState::sigFinished,   DLZckHeadState,   &PrepareMultiState::toZckHeadDownloadGuard, &PrepareMultiState::transitionToZckHeadDl >,
#endif
    Transition< PrepareMultiState, &PrepareMultiState::sigFallback,   DlNormalFileState, DefaultStateCondition, &PrepareMultiState::fallbackToNormalTransition >,
    Transition< PrepareMultiState, &PrepareMultiState::sigFailed,     DlNormalFileState >,

#if ENABLE_ZCHUNK_COMPRESSION
    Transition< DLZckHeadState, &DLZckHeadState::sigFinished, DLZckState, DefaultStateCondition, &DLZckHeadState::transitionToDlZckState >,
    Transition< DLZckHeadState, &DLZckHeadState::sigFailed,   DlNormalFileState >,

    Transition< DLZckState, &DLZckState::sigFinished, FinishedState, DefaultStateCondition, &DLZckState::transitionToFinished >,
    Transition< DLZckState, &DLZckState::sigFallback, DlNormalFileState >,
#endif

    Transition< DlMetalinkState, &DlMetalinkState::sigFinished, FinishedState, DefaultStateCondition, &DlMetalinkState::transitionToFinished >,
    Transition< DlMetalinkState, &DlMetalinkState::sigFailed, FinishedState, &DlMetalinkState::toFinalStateCondition, &DlMetalinkState::transitionToFinished   >,
    Transition< DlMetalinkState, &DlMetalinkState::sigFailed, DlNormalFileState, &DlMetalinkState::toSimpleDownloadCondition >,

    Transition< DlNormalFileState, &DlNormalFileState::sigFinished, FinishedState, DefaultStateCondition, &DlNormalFileState::transitionToFinished >,
    Transition< DlNormalFileState, &DlNormalFileState::sigFailed, FinishedState, DefaultStateCondition, &DlNormalFileState::transitionToFinished  >
    >;

  class DownloadPrivate : public DownloadPrivateBase, public DownloadStatemachine<DownloadPrivate>
  {
  public:
    DownloadPrivate ( Downloader &parent, std::shared_ptr<NetworkRequestDispatcher> requestDispatcher, std::shared_ptr<MirrorControl> mirrors, DownloadSpec &&spec, Download &p );
    void start ();
    void init() override;
  };

  class DownloaderPrivate : public BasePrivate
  {
    ZYPP_DECLARE_PUBLIC(Downloader)
  public:
    DownloaderPrivate( std::shared_ptr<MirrorControl> mc, Downloader &p );

    std::vector< std::shared_ptr<Download> > _runningDownloads;
    std::shared_ptr<NetworkRequestDispatcher> _requestDispatcher;

    void onDownloadStarted ( Download &download );
    void onDownloadFinished ( Download &download );

    Signal< void ( Downloader &parent, Download& download )> _sigStarted;
    Signal< void ( Downloader &parent, Download& download )> _sigFinished;
    Signal< void ( Downloader &parent )> _queueEmpty;
    std::shared_ptr<MirrorControl> _mirrors;
  };

}

#endif
