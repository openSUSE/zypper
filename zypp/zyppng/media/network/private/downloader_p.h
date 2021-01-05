#ifndef ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADER_P_H_INCLUDED
#define ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADER_P_H_INCLUDED

#include <zypp/zyppng/base/private/base_p.h>
#include <zypp/zyppng/base/signals.h>
#include <zypp/zyppng/core/ByteArray>
#include <zypp/zyppng/media/network/downloader.h>
#include <zypp/zyppng/media/network/downloadspec.h>
#include <zypp/zyppng/media/network/request.h>
#include <zypp/zyppng/media/network/TransferSettings>
#include <zypp/zyppng/media/network/networkrequesterror.h>
#include <zypp/zyppng/media/network/private/mirrorcontrol_p.h>
#include <zypp/media/MediaBlockList.h>
#include <zypp/zyppng/base/statemachine.h>
#include <zypp/TriBool.h>

#include <deque>

namespace zyppng {

  class NetworkRequestDispatcher;

  class DownloadPrivate;


  class DownloadPrivateBase : public BasePrivate
  {
    ZYPP_DECLARE_PUBLIC(Download)
  public:
    DownloadPrivateBase ( Downloader &parent, std::shared_ptr<NetworkRequestDispatcher> requestDispatcher, std::shared_ptr<MirrorControl> mirrors, DownloadSpec &&spec, Download &p );
    ~DownloadPrivateBase ();

    struct Block {
      off_t  start = 0;
      size_t len = 0;

      std::string chksumtype;
      std::optional<UByteArray> chksumVec;
      std::optional<size_t> chksumCompareLen; //< initialized if only the first few bytes of the checksum should be considered

      int _retryCount = 0;  //< how many times was this request restarted
      NetworkRequestError _failedWithErr; //< what was the error this request failed with
    };

    struct Request : public NetworkRequest {

      using NetworkRequest::NetworkRequest;
      using Ptr = std::shared_ptr<Request>;
      using WeakPtr = std::shared_ptr<Request>;

      template <typename Receiver>
      void connectSignals ( Receiver &dl ) {
        _sigStartedConn  = connect ( &NetworkRequest::sigStarted,  dl, &Receiver::onRequestStarted );
        _sigProgressConn = connect ( &NetworkRequest::sigProgress, dl, &Receiver::onRequestProgress );
        _sigFinishedConn = connect ( &NetworkRequest::sigFinished, dl, &Receiver::onRequestFinished );
      }
      void disconnectSignals ();

      bool _triedCredFromStore = false; //< already tried to authenticate from credential store?
      time_t _authTimestamp = 0; //< timestamp of the AuthData we tried from the store
      Url _originalUrl;  //< The unstripped URL as it was passed to Download , before transfer settings are removed
      MirrorControl::MirrorHandle _myMirror;

      connection _sigStartedConn;
      connection _sigProgressConn;
      connection _sigFinishedConn;
    };

    struct InitialState;         //< initial state before we start downloading
    struct DetectMetalinkState;  //< First attempt to get the zchunk header, but we might receive metalink data instead
    struct DlMetaLinkInfoState;  //< We got Metalink, lets get the full metalink file or we got no zchunk in the first place
    struct PrepareMultiState;    //< Parsing the metalink file and preparing the mirrors
#if ENABLE_ZCHUNK_COMPRESSION
    struct DLZckHeadState;       //< Download the ZChunk Header
    struct DLZckState;           //< Download the File in ZChunk Mode
#endif
    struct DlMetalinkState;      //< Download the File in Metalink Mode
    struct DlNormalFileState;    //< Simple Plain download, no chunking
    struct FinishedState;        //< We are done

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

    struct DetectMetalinkState : public zyppng::SimpleState< DownloadPrivate, Download::DetectMetaLink, false > {

      DetectMetalinkState ( DownloadPrivate &parent );

      void enter ();
      void exit ();

      void onRequestStarted  ( NetworkRequest & );
      void onRequestProgress ( NetworkRequest &, off_t, off_t dlnow, off_t, off_t );
      void onRequestFinished ( NetworkRequest &req , const NetworkRequestError &err );


      const NetworkRequestError &error () const {
        return _error;
      }

      SignalProxy< void () > sigFinished() {
        return _sigFinished;
      }

      bool toMetalinkGuard () const {
        return _gotMetalink;
      }

      bool toSimpleDownloadGuard () const;

#if ENABLE_ZCHUNK_COMPRESSION
      bool toZckHeadDownloadGuard () const;
      std::shared_ptr<DLZckHeadState> toDLZckHeadState();
#endif

      std::shared_ptr<Request> _request;

    private:
      NetworkRequestError _error;
      bool _gotMetalink = false;
      Signal< void () > _sigFinished;
    };

    struct BasicDownloaderStateBase : public zyppng::BasicState< DownloadPrivate, false > {

      BasicDownloaderStateBase ( DownloadPrivate &parent ) : BasicState( parent ){}

      void enter ();
      void exit ();

      virtual bool initializeRequest ( std::shared_ptr<Request> r );
      virtual void gotFinished ();
      virtual void failed(NetworkRequestError &&err);
      void failed (std::string &&str );

      void onRequestStarted  ( NetworkRequest & );
      void onRequestProgress ( NetworkRequest &, off_t dltotal, off_t dlnow, off_t, off_t );
      void onRequestFinished ( NetworkRequest &req , const NetworkRequestError &err );

      const NetworkRequestError &error () const {
        return _error;
      }

      std::shared_ptr<Request> _request;
      std::vector<Url> _mirrors;
      std::optional<std::string> _chksumtype; //< The file checksum type if available
      std::optional<UByteArray>  _chksumVec;  //< The file checksum if available

    protected:
      virtual void handleRequestProgress (NetworkRequest &req, off_t dltotal, off_t dlnow );
      NetworkRequestError _error;
      Signal< void () > _sigFinished;
      Signal< void () > _sigFailed;
    };

    /*!
     * State to download the metalink file, we can however not be 100% sure that we actually
     * will get a metalink file, so we need to check the content type or in bad cases the
     * data we get from the server.
     */
    struct DlMetaLinkInfoState : public BasicDownloaderStateBase {
      static constexpr auto stateId = Download::DlMetaLinkInfo;
      DlMetaLinkInfoState( DownloadPrivate &parent );

      SignalProxy< void () > sigFinished() {
        return _sigFinished;
      }
      SignalProxy< void () > sigGotMetalink() {
        return _sigGotMetalink;
      }
      SignalProxy< void () > sigFailed() {
        return _sigFailed;
      }

      std::shared_ptr<FinishedState> transitionToFinished ();

      bool initializeRequest( std::shared_ptr<Request> r ) override;
      virtual void gotFinished () override;

    protected:
      bool _isMetalink = false;
      Signal< void () > _sigGotMetalink;

      virtual void handleRequestProgress ( NetworkRequest &req, off_t dltotal, off_t dlnow ) override;
    };


    /*!
     * Parses the downloaded Metalink file and sets up the mirror database
     */
    struct PrepareMultiState : public zyppng::SimpleState< DownloadPrivate, Download::PrepareMulti, false > {

      PrepareMultiState ( DownloadPrivate &parent );

      void enter ();
      void exit () {}

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
      NetworkRequestError _error;
      Signal< void () > _sigFinished;
      Signal< void () > _sigFallback;
      Signal< void () > _sigFailed;
    };


    /*!
     * Base type for all block downloader, here we handle all the nasty details of downloading
     * a file in blocks
     */
    struct RangeDownloaderBaseState : public BasicState< DownloadPrivate, false  > {

      RangeDownloaderBaseState ( std::vector<Url> &&mirrors, DownloadPrivate &parent ) :
        BasicState(parent),
        _mirrors( std::move(mirrors) ){}

      void ensureDownloadsRunning ();
      void reschedule ();

      const NetworkRequestError &error () const {
        return _error;
      }

      void setFailed  ( NetworkRequestError &&err );
      void setFailed  ( std::string && reason );
      virtual void setFinished ( );
      void cancelAll  ( const NetworkRequestError &err  );

      void onRequestStarted  ( NetworkRequest & );
      void onRequestProgress ( NetworkRequest &, off_t, off_t, off_t, off_t );
      void onRequestFinished ( NetworkRequest &req , const NetworkRequestError &err );

    protected:
      std::vector<Url> _mirrors;
      NetworkRequestError _error;

      size_t             _fileSize = 0; //< The expected filesize, this is used to make sure we do not write after the end offset of the expected file size
      std::list<Block>   _ranges;

      //keep a list with failed blocks in case we run out of mirrors,
      //in that case we can retry to download them once we have a finished download
      std::list<Block>   _failedRanges;

      off_t _downloadedMultiByteCount = 0; //< the number of bytes that were already fetched in RunningMulti state

      std::vector< std::shared_ptr<Request> > _runningRequests;

      // we only define the signals here and add the accessor functions in the subclasses, static casting of
      // the class type is not allowed at compile time, so they would not be useable in the transition table otherwise
      Signal< void () > _sigFinished;
      Signal< void () > _sigFailed;

private:
      void handleRequestError( std::shared_ptr<Request> req, const zyppng::NetworkRequestError &err );
      bool addBlockRanges( std::shared_ptr<Request> req, std::vector<Block> &&blocks ) const;
      void addNewRequest     (std::shared_ptr<Request> req, const bool connectSignals = true );

      std::shared_ptr<Request>    initMultiRequest ( NetworkRequestError &err, bool useFailed = false );

      std::vector<Block> getNextBlocks ( const std::string &urlScheme );
      std::vector<Block> getNextFailedBlocks( const std::string &urlScheme );
    };

    struct DlMetalinkState : public RangeDownloaderBaseState {

      static constexpr auto stateId = Download::DlMetalink;

      DlMetalinkState ( zypp::media::MediaBlockList &&blockList, std::vector<Url> &&mirrors, DownloadPrivate &parent );

      void enter ();
      void exit ();
      virtual void setFinished () override;

      std::shared_ptr<FinishedState> transitionToFinished ();

      // in case of error we might fall back, except for the errors listed here
      bool toFinalStateCondition () {
        return (  _error.type() == NetworkRequestError::Unauthorized
                 || _error.type() == NetworkRequestError::AuthFailed );
      }

      bool toSimpleDownloadCondition () {
        return !toFinalStateCondition();
      }

      SignalProxy< void () > sigFinished() {
        return _sigFinished;
      }

      SignalProxy< void () > sigFailed() {
        return _sigFailed;
      }

    private:
      zypp::media::MediaBlockList _blockList;
      std::string        _fileChecksumType;
      std::optional<std::vector<unsigned char>> _fileChksumVec;
    };

    /*!
     * Just a plain normal file download, no metalink, nothing fancy.
     * If this fails we have no more fallbacks
     */
    struct DlNormalFileState : public BasicDownloaderStateBase {
      static constexpr auto stateId = Download::DlSimple;
      DlNormalFileState( DownloadPrivate &parent );

      std::shared_ptr<FinishedState> transitionToFinished ();

      SignalProxy< void () > sigFinished() {
        return _sigFinished;
      }
      SignalProxy< void () > sigFailed() {
        return _sigFailed;
      }
    };

#if ENABLE_ZCHUNK_COMPRESSION

    /*!
     * State that downloads the ZckHeader
     */
    struct DLZckHeadState : public BasicDownloaderStateBase {
      static constexpr auto stateId = Download::DlZChunkHead;

      DLZckHeadState( std::vector<Url> &&mirrors, DownloadPrivate &parent );

      virtual bool initializeRequest( std::shared_ptr<Request> r ) override;
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
     * State that downloads the Zck data
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

    struct FinishedState : public SimpleState< DownloadPrivate, Download::Finished, true >
    {
      FinishedState ( NetworkRequestError &&error, DownloadPrivate &parent );

      void enter (){}
      void exit (){}

      NetworkRequestError _error;
    };

    bool _emittedSigStart = false;
    bool handleRequestAuthError(std::shared_ptr<Request> req, const zyppng::NetworkRequestError &err);

#if ENABLE_ZCHUNK_COMPRESSION
    bool hasZckInfo () const;
#endif

    std::shared_ptr<NetworkRequestDispatcher> _requestDispatcher;
    std::shared_ptr<MirrorControl> _mirrorControl;

    DownloadSpec _spec; // the download settings
    mutable zypp::TriBool _specHasZckInfo = zypp::indeterminate;

    Downloader *_parent = nullptr;

    time_t _lastTriedAuthTime = 0; //< if initialized this shows the last timestamp that we loaded a cred for the given URL from CredentialManager
    NetworkRequest::Priority _defaultSubRequestPriority = NetworkRequest::High;

    Signal< void ( Download &req )> _sigStarted;
    Signal< void ( Download &req, Download::State state )> _sigStateChanged;
    Signal< void ( Download &req, off_t dlnow  )> _sigAlive;
    Signal< void ( Download &req, off_t dltotal, off_t dlnow )> _sigProgress;
    Signal< void ( Download &req )> _sigFinished;
    Signal< void ( zyppng::Download &req, zyppng::NetworkAuthData &auth, const std::string &availAuth )> _sigAuthRequired;

  protected:
    NetworkRequestError safeFillSettingsFromURL ( const Url &url, TransferSettings &set );
    MirrorControl::MirrorHandle findNextMirror(std::vector<Url> &_mirrors, Url &url, TransferSettings &set, NetworkRequestError &err );
  };

  using InitialState = DownloadPrivateBase::InitialState;
  using DetectMetalinkState = DownloadPrivateBase::DetectMetalinkState;
  using DlMetaLinkInfoState = DownloadPrivateBase::DlMetaLinkInfoState;
  using PrepareMultiState = DownloadPrivateBase::PrepareMultiState;
  using DlMetalinkState = DownloadPrivateBase::DlMetalinkState;
  using DlNormalFileState = DownloadPrivateBase::DlNormalFileState;
  using FinishedState = DownloadPrivateBase::FinishedState;

#if ENABLE_ZCHUNK_COMPRESSION
  using DLZckHeadState = DownloadPrivateBase::DLZckHeadState;
  using DLZckState = DownloadPrivateBase::DLZckState;
#endif

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
