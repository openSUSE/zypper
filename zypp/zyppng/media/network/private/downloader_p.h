#ifndef ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADER_P_H_INCLUDED
#define ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADER_P_H_INCLUDED

#include <zypp/zyppng/base/private/base_p.h>
#include <zypp/zyppng/base/signals.h>
#include <zypp/zyppng/media/network/downloader.h>
#include <zypp/zyppng/media/network/request.h>
#include <zypp/zyppng/media/network/TransferSettings>
#include <zypp/zyppng/media/network/networkrequesterror.h>
#include <zypp/zyppng/media/network/private/mirrorcontrol_p.h>
#include <zypp/media/MediaBlockList.h>

#include <deque>

namespace zyppng {

  class NetworkRequestDispatcher;

  class DownloadPrivate : public BasePrivate
  {
  public:
    ZYPP_DECLARE_PUBLIC(Download)
    DownloadPrivate ( Downloader &parent, std::shared_ptr<NetworkRequestDispatcher> requestDispatcher, std::shared_ptr<MirrorControl> mirrors, Url &&file, zypp::filesystem::Pathname &&targetPath, zypp::ByteCount &&expectedFileSize );
    ~DownloadPrivate ();

    struct Block {
      Block( size_t blockNr ) : _block(blockNr) {}
      Block( Block &&other ) = default;
      Block( const Block &other ) = default;
      Block &operator=( const Block &other) = default;

      size_t _block   = -1;
      int _retryCount = 0;  //< how many times was this request restarted
      NetworkRequestError _failedWithErr; //< what was the error this request failed with
    };

    struct Request : public NetworkRequest {

      using NetworkRequest::NetworkRequest;
      using Ptr = std::shared_ptr<Request>;
      using WeakPtr = std::shared_ptr<Request>;

      void connectSignals ( DownloadPrivate &dl );
      void disconnectSignals ();

      std::vector<Block> _myBlocks;

      bool _triedCredFromStore = false; //< already tried to authenticate from credential store?
      time_t _authTimestamp = 0; //< timestamp of the AuthData we tried from the store
      Url _originalUrl;  //< The unstripped URL as it was passed to Download , before transfer settings are removed
      MirrorControl::MirrorHandle _myMirror;

      connection _sigStartedConn;
      connection _sigProgressConn;
      connection _sigFinishedConn;
    };

    std::vector< std::shared_ptr<Request> > _runningRequests;
    std::shared_ptr<NetworkRequestDispatcher> _requestDispatcher;

    std::shared_ptr<MirrorControl> _mirrorControl;
    std::vector<Url> _mirrors;
    connection _mirrorControlReadyConn;

    Url _url;
    zypp::filesystem::Pathname _targetPath;
    zypp::Pathname _deltaFilePath;
    zypp::ByteCount _expectedFileSize;
    std::string _errorString;
    NetworkRequestError _requestError;

    TransferSettings _transferSettings;

    //data requires for multi part downloads
    off_t _downloadedMultiByteCount = 0; //< the number of bytes that were already fetched in RunningMulti state
    zypp::media::MediaBlockList _blockList;
    size_t _blockIter    = 0;
    zypp::ByteCount _preferredChunkSize = zypp::ByteCount( 4096, zypp::ByteCount::K );

    //keep a list with failed blocks in case we run out of mirrors,
    //in that case we can retry to download them once we have a finished download
    std::deque<Block> _failedBlocks;

    Downloader *_parent = nullptr;
    Download::State _state = Download::InitialState;

    bool _isMultiDownload = false;   //< State flag, shows if we are currently downloading a multi part file
    bool _isMultiPartEnabled = true; //< Enables/Disables automatic multipart downloads
    bool _checkExistsOnly = false;   //< Set to true if Downloader should only check if the URL exits
    time_t _lastTriedAuthTime = 0; //< if initialized this shows the last timestamp that we loaded a cred for the given URL from CredentialManager
    NetworkRequest::Priority _defaultSubRequestPriority = NetworkRequest::High;

    signal<void ( Download &req )> _sigStarted;
    signal<void ( Download &req, Download::State state )> _sigStateChanged;
    signal<void ( Download &req, off_t dlnow  )> _sigAlive;
    signal<void ( Download &req, off_t dltotal, off_t dlnow )> _sigProgress;
    signal<void ( Download &req )> _sigFinished;
    signal<void ( zyppng::Download &req, zyppng::NetworkAuthData &auth, const std::string &availAuth )> _sigAuthRequired;

  private:
    void start ();
    void setState ( Download::State newState );
    void onRequestStarted  ( NetworkRequest & );
    void onRequestProgress ( NetworkRequest &req, off_t dltotal, off_t dlnow, off_t, off_t );
    void onRequestFinished ( NetworkRequest &req , const NetworkRequestError &err );
    void addNewRequest     (std::shared_ptr<Request> req, const bool connectSignals = true );
    std::shared_ptr<Request> initMultiRequest( NetworkRequestError &err, bool useFailed = false );
    void addBlockRanges    ( std::shared_ptr<Request> req ) const;
    MirrorControl::MirrorHandle findNextMirror( Url &url, TransferSettings &set, NetworkRequestError &err );
    void setFailed         ( std::string && reason );
    void setFinished       ( bool success = true );
    void handleRequestError ( std::shared_ptr<Request> req, const zyppng::NetworkRequestError &err );
    void upgradeToMultipart ( NetworkRequest &req );
    void mirrorsReady ();
    void ensureDownloadsRunning ();
    void enqueueNewPartsOrFinish ();
    std::vector<Block> getNextBlocks ( const std::string &urlScheme );
    std::vector<Block> getNextFailedBlocks( const std::string &urlScheme );

    NetworkRequestError safeFillSettingsFromURL ( const Url &url, TransferSettings &set );
  };

  class DownloaderPrivate : public BasePrivate
  {
    ZYPP_DECLARE_PUBLIC(Downloader)
  public:
    DownloaderPrivate( std::shared_ptr<MirrorControl> mc = {} );

    std::vector< std::shared_ptr<Download> > _runningDownloads;
    std::shared_ptr<NetworkRequestDispatcher> _requestDispatcher;

    void onDownloadStarted ( Download &download );
    void onDownloadFinished ( Download &download );

    signal<void ( Downloader &parent, Download& download )> _sigStarted;
    signal<void ( Downloader &parent, Download& download )> _sigFinished;
    signal<void ( Downloader &parent )> _queueEmpty;
    std::shared_ptr<MirrorControl> _mirrors;
  };

}

#endif
