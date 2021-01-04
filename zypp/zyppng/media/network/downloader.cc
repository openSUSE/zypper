#include <zypp/zyppng/media/network/private/downloader_p.h>
#include <zypp/zyppng/media/network/private/request_p.h>
#include <zypp/zyppng/media/network/private/networkrequesterror_p.h>
#include <zypp/zyppng/media/network/networkrequestdispatcher.h>
#include <zypp/zyppng/media/network/request.h>
#include <zypp/zyppng/base/Timer>
#include <zypp/zyppng/base/EventDispatcher>
#include <zypp/Pathname.h>
#include <zypp/media/TransferSettings.h>
#include <zypp/media/MetaLinkParser.h>
#include <zypp/ByteCount.h>
#include <zypp/base/String.h>
#include <zypp/PathInfo.h>
#include <zypp/media/CurlHelper.h>
#include <zypp/media/CredentialManager.h>
#include <zypp/ZConfig.h>
#include <zypp/base/Logger.h>
#include <zypp/ZYppFactory.h>

#if ENABLE_ZCHUNK_COMPRESSION
extern "C" {
  #include <zck.h>
}
#endif

#include <queue>
#include <fcntl.h>
#include <iostream>
#include <fstream>

#define BLKSIZE		131072

namespace  {
  constexpr auto minMetalinkProbeSize = 256; //< The maximum probe size we download before we decide we really got no metalink file

  bool looks_like_metalink_data( const std::vector<char> &data )
  {
    if ( data.empty() )
      return false;

    const char *p = data.data();
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
      p++;

    if (!strncasecmp(p, "<?xml", 5))
    {
      while (*p && *p != '>')
        p++;
      if (*p == '>')
        p++;
      while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;
    }
    bool ret = !strncasecmp( p, "<metalink", 9 ) ? true : false;
    return ret;
  }

  bool looks_like_metalink_file( const zypp::Pathname &file )
  {
    std::unique_ptr<FILE, decltype(&fclose)> fd( fopen( file.c_str(), "r" ), &fclose );
    if ( !fd )
      return false;
    return looks_like_metalink_data( zyppng::peek_data_fd( fd.get(), 0, minMetalinkProbeSize ) );
  }

#if ENABLE_ZCHUNK_COMPRESSION
  bool isZchunkFile ( const zypp::Pathname &file ) {
    std::ifstream dFile( file.c_str() );
    if ( !dFile.is_open() )
      return false;

    constexpr std::string_view magic("\0ZCK1", 5);

    std::array< char, magic.size() > lead;
    lead.fill('\0');
    dFile.read( lead.data(), lead.size() );
    return ( magic == std::string_view( lead.data(), lead.size()) );
  }
#endif
}

namespace zyppng {

  DownloadPrivateBase::DownloadPrivateBase(Downloader &parent, std::shared_ptr<NetworkRequestDispatcher> requestDispatcher, std::shared_ptr<MirrorControl> mirrors, DownloadSpec &&spec, Download &p)
    : BasePrivate(p)
    , _requestDispatcher ( std::move(requestDispatcher) )
    , _mirrorControl( std::move(mirrors) )
    , _spec( std::move(spec) )
    , _parent( &parent )
    , _sigStarted(p)
    , _sigStateChanged(p)
    , _sigAlive(p)
    , _sigProgress(p)
    , _sigFinished(p)
    , _sigAuthRequired(p)
  {}

  DownloadPrivateBase::~DownloadPrivateBase()
  { }


  // Initial state functions, this state is used only to kickstart the statemachine
  void InitialState::enter(){ MIL << "Entering initial state"  << std::endl; }

  void InitialState::exit(){  MIL << "Leaving initial state"  << std::endl;  }

  void DownloadPrivateBase::InitialState::initiate()
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;

    if ( spec.checkExistsOnly() ) {
      MIL << "Check exists only enabled" << std::endl;
      return _sigTransitionToDlNormalFileState.emit();
    }

#if ENABLE_ZCHUNK_COMPRESSION
    bool deltaZck = isZchunkFile( spec.deltaFile() );
#endif
    if ( spec.metalinkEnabled() ) {
#if ENABLE_ZCHUNK_COMPRESSION
      if ( deltaZck && spec.headerSize() > 0 ) {
        MIL << "We might have a zck file, detecting metalink first" << std::endl;
        return _sigTransitionToDetectMetalinkState.emit();
      }
#endif
      MIL << "No zchunk data available but metalink requested, going to download metalink directly." << std::endl;
      return _sigTransitionToDlMetaLinkInfoState.emit();
    }

#if ENABLE_ZCHUNK_COMPRESSION
    // no Metalink, maybe we can directly download zck
    if ( deltaZck && spec.headerSize() > 0 ) {
      MIL << "No metalink but zckunk data availble trying to download ZckHead directly." << std::endl;
      return _sigTransitionToDLZckHeaderState.emit();
    }
#endif
    MIL << "Fallback to normal DL" << std::endl;
    _sigTransitionToDlNormalFileState.emit();
  }

#if ENABLE_ZCHUNK_COMPRESSION
  std::shared_ptr<DownloadPrivateBase::DLZckHeadState> InitialState::toDLZckHeadState()
  {
    // we have no mirrors, the range downloader would need to fall back to using the base URL
    return std::make_shared<DLZckHeadState>( std::vector<Url> { stateMachine()._spec.url() }, stateMachine() );
  }
#endif

  // Metalink detection state, we query the type of file from the server, in order to use metalink support the server
  // needs to correctly return the metalink file content type, otherwise we proceed to not downloading a metalink file
  DetectMetalinkState::DetectMetalinkState(DownloadPrivate &parent) : SimpleState( parent ){
    MIL << "Creating DetectMetalinkState" << std::endl;
  }

  void DownloadPrivateBase::DetectMetalinkState::enter()
  {
    _request.reset();
    _gotMetalink = false;

    auto &sm = stateMachine();
    const auto &url = sm._spec.url();

    MIL << "Detecting if metalink is available on " << url << std::endl;

    _request = std::make_shared<Request>( ::internal::clearQueryString( url ), zypp::Pathname("/dev/null") );

    _request->_originalUrl = url;
    _request->transferSettings() = sm._spec.settings();
    _request->transferSettings().addHeader("Accept: */*, application/metalink+xml, application/metalink4+xml");
    _request->setOptions( _request->options() | NetworkRequest::HeadRequest );

    _request->connectSignals( *this );
    sm._requestDispatcher->enqueue( _request );
  }

  void DownloadPrivateBase::DetectMetalinkState::exit()
  {
    _request->disconnectSignals();
    _request.reset();
  }

  void DownloadPrivateBase::DetectMetalinkState::onRequestStarted( NetworkRequest & )
  {
    stateMachine()._sigStarted.emit( *stateMachine().z_func() );
  }

  void DownloadPrivateBase::DetectMetalinkState::onRequestProgress( NetworkRequest &, off_t, off_t dlnow, off_t, off_t )
  {
    stateMachine()._sigAlive.emit( *stateMachine().z_func(), dlnow );
  }

  void DownloadPrivateBase::DetectMetalinkState::onRequestFinished( NetworkRequest &req, const NetworkRequestError &err )
  {
    auto lck = stateMachine().z_func()->shared_from_this();
    if ( req.hasError() ) {
      MIL << "Detecing if metalink is possible for url " << req.url() << " failed with error " << err.toString() << " falling back to download without metalink." << std::endl;
      _error = err;
      _gotMetalink = false;
      return _sigFinished.emit();
    }

    std::string cType = req.contentType();
    _gotMetalink = ( cType.find("application/metalink+xml") == 0 || cType.find("application/metalink4+xml") == 0 );
    MIL << "Metalink detection result on url " << req.url() << " is " << _gotMetalink << std::endl;
    _sigFinished.emit();
  }

  bool DownloadPrivateBase::DetectMetalinkState::toSimpleDownloadGuard() const
  {
#if ENABLE_ZCHUNK_COMPRESSION
    return !toMetalinkGuard() && !toZckHeadDownloadGuard();
#else
    return !toMetalinkGuard();
#endif
  }

#if ENABLE_ZCHUNK_COMPRESSION
  bool DownloadPrivateBase::DetectMetalinkState::toZckHeadDownloadGuard() const
  {
    return !toMetalinkGuard() && stateMachine().hasZckInfo();
  }

  std::shared_ptr<DownloadPrivateBase::DLZckHeadState> DetectMetalinkState::toDLZckHeadState()
  {
    // we have no mirrors, the range downloader would need to fall back to using the base URL
    return std::make_shared<DLZckHeadState>( std::vector<Url> { stateMachine()._spec.url() }, stateMachine() );
  }
#endif

  void DownloadPrivateBase::BasicDownloaderStateBase::enter()
  {
    _request.reset();

    auto &sm = stateMachine();
    const auto &spec = sm._spec;
    auto url = spec.url();

    TransferSettings set = spec.settings();
    NetworkRequestError dummyErr;
    MirrorControl::MirrorHandle mirror;

    if ( _mirrors.size() )
      mirror = sm.findNextMirror( _mirrors, url, set, dummyErr );

    if ( !mirror ) {
      url = spec.url();
      set = spec.settings();
      auto err = sm.safeFillSettingsFromURL( url, set );
      if ( err.isError() )
        return failed( std::move(err) );
    }

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

  void DownloadPrivateBase::BasicDownloaderStateBase::exit()
  {
    _request->disconnectSignals();
    _request.reset();
  }

  bool DownloadPrivateBase::BasicDownloaderStateBase::initializeRequest( std::shared_ptr<DownloadPrivateBase::Request> )
  {
    return true;
  }

  void DownloadPrivateBase::BasicDownloaderStateBase::gotFinished()
  {
    _sigFinished.emit();
  }

  void DownloadPrivateBase::BasicDownloaderStateBase::failed( std::string &&str )
  {
    failed( NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, std::move(str) ) );
  }

  void DownloadPrivateBase::BasicDownloaderStateBase::failed( NetworkRequestError &&err )
  {
    _error = std::move( err );
    _sigFailed.emit();
  }

  void DownloadPrivateBase::BasicDownloaderStateBase::onRequestStarted( NetworkRequest & )
  {
    auto &sm = stateMachine();
    if ( !sm._emittedSigStart ) {
      sm._emittedSigStart = true;
      stateMachine()._sigStarted.emit( *stateMachine().z_func() );
    }
    if ( _request->_myMirror )
      _request->_myMirror->startTransfer();
  }

  void DownloadPrivateBase::BasicDownloaderStateBase::handleRequestProgress( NetworkRequest &req, off_t dltotal, off_t dlnow )
  {
    auto &sm = stateMachine();
    const off_t expFSize = sm._spec.expectedFileSize();
    if ( expFSize  > 0 && expFSize < dlnow ) {
      sm._requestDispatcher->cancel( req, NetworkRequestErrorPrivate::customError( NetworkRequestError::ExceededMaxLen ) );
      return;
    }
    return sm._sigProgress.emit( *sm.z_func(), (expFSize > 0 ? expFSize : dltotal), dlnow );
  }

  void DownloadPrivateBase::BasicDownloaderStateBase::onRequestProgress( NetworkRequest &req, off_t dltotal, off_t dlnow, off_t, off_t )
  {
    handleRequestProgress( req, dltotal, dlnow );
  }

  void DownloadPrivateBase::BasicDownloaderStateBase::onRequestFinished( NetworkRequest &req, const NetworkRequestError &err )
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
      MIL << "Downloading on " << stateMachine()._spec.url() << " failed with error "<< err.toString() << " " << err.nativeErrorString() << std::endl;
      return failed( NetworkRequestError(err) );
    }

    gotFinished();
  }

  DlMetaLinkInfoState::DlMetaLinkInfoState(DownloadPrivate &parent) : BasicDownloaderStateBase( parent )
  {
    MIL << "Downloading metalink on " << parent._spec.url() << std::endl;
  }

  std::shared_ptr<DownloadPrivateBase::FinishedState> DownloadPrivateBase::DlMetaLinkInfoState::transitionToFinished()
  {
    MIL << "Downloading on " << stateMachine()._spec.url() << " transition to final state. " << std::endl;
    return std::make_shared<FinishedState>( std::move(_error), stateMachine() );
  }

  bool DlMetaLinkInfoState::initializeRequest( std::shared_ptr<Request> r )
  {
    r->transferSettings().addHeader("Accept: */*, application/metalink+xml, application/metalink4+xml");
    return BasicDownloaderStateBase::initializeRequest(r);
  }

  void DlMetaLinkInfoState::gotFinished()
  {
    // some proxies do not store the content type, so also look at the file to find
    // out if we received a metalink (bnc#649925)
    if ( !_isMetalink )
      _isMetalink = looks_like_metalink_file( _request->targetFilePath() );
    if ( !_isMetalink ) {
      // Move to finished state
      MIL << "Downloading on " << stateMachine()._spec.url() << " was successful, no metalink data. " << std::endl;
      return BasicDownloaderStateBase::gotFinished();
    }

    // Move to Prepare Multi state
    MIL << "Downloading on " << stateMachine()._spec.url() << " returned a Metalink " << std::endl;
    _sigGotMetalink.emit();
  }

  void DlMetaLinkInfoState::handleRequestProgress(NetworkRequest &req, off_t dltotal, off_t dlnow)
  {
    auto &sm = stateMachine();

    if ( !_isMetalink && dlnow < minMetalinkProbeSize ) {
      // can't tell yet, ...
      return sm._sigAlive.emit( *sm.z_func(), dlnow );
    }

    if ( !_isMetalink ) {
      std::string cType = req.contentType();
      _isMetalink = ( cType.find("application/metalink+xml") == 0 || cType.find("application/metalink4+xml") == 0 );
    }

    if ( !_isMetalink ) {
      _isMetalink = looks_like_metalink_data( req.peekData( 0, minMetalinkProbeSize ) );
    }

    if ( _isMetalink ) {
      // this is a metalink file change the expected filesize
      if ( zypp::ByteCount( 2, zypp::ByteCount::MB) < static_cast<zypp::ByteCount::SizeType>( dlnow ) ) {
        MIL << "Metalink file exceeds 2MB in filesize, aborting."<<std::endl;
        sm._requestDispatcher->cancel( req, NetworkRequestErrorPrivate::customError( NetworkRequestError::ExceededMaxLen ) );
        return;
      }
    } else {
      // still no metalink, we assume a normal download, not perfect though
      return BasicDownloaderStateBase::handleRequestProgress( req, dltotal, dlnow );
    }
  }

  PrepareMultiState::PrepareMultiState(DownloadPrivate &parent) : SimpleState( parent )
  {
    MIL << "About to enter PrepareMultiState" << std::endl;
  }

  void DownloadPrivateBase::PrepareMultiState::enter( )
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;
    const auto &url = spec.url();
    const auto &targetPath = spec.targetPath();
#if ENABLE_ZCHUNK_COMPRESSION
    _haveZckData = (isZchunkFile( spec.deltaFile() )  && spec.headerSize() > 0);
    DBG << " Upgrading request for URL: "<< url << " to multipart download , which zckunk=" << _haveZckData << std::endl;
#else
    DBG << " Upgrading request for URL: "<< url << " to multipart download , which zckunk=false" << std::endl;
#endif


    //we have a metalink download, lets parse it and see what we got
    _mirrors.clear();

    std::vector<zypp::media::MetalinkMirror> mirrs;

    try {
      zypp::media::MetaLinkParser parser;
      parser.parse( targetPath );

      // we only care about the metalink chunks if we have no zchunk data
#if ENABLE_ZCHUNK_COMPRESSION
      if ( !_haveZckData ) {
#else
      if ( true ) {
#endif
        auto bl = parser.getBlockList();
        if ( !bl.haveBlocks() )
          MIL << "HERE! Got no blocks for URL " << spec.url() << " but got filesize? " << bl.getFilesize() << std::endl;
        if ( bl.haveBlocks() || bl.haveFilesize() )
          _blockList = std::move(bl);
      }

      //migrate some settings from the base url to the mirror
      mirrs = parser.getMirrors();
      for ( auto urliter = mirrs.begin(); urliter != mirrs.end(); ++urliter ) {
        try {
          const std::string scheme = urliter->url.getScheme();
          if (scheme == "http" || scheme == "https" || scheme == "ftp" || scheme == "tftp") {
            if ( !sm._requestDispatcher->supportsProtocol( urliter->url )) {
              urliter = mirrs.erase( urliter );
              continue;
            }
            urliter->url = ::internal::propagateQueryParams( urliter->url, url );
            _mirrors.push_back( urliter->url );
          }
        }
        catch (...) {  }
      }

      if ( mirrs.empty() ) {
        mirrs.push_back( { 0, -1, url } );
        _mirrors.push_back( url );
      }

    } catch ( const zypp::Exception &ex ) {
      _error = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, zypp::str::Format("Failed to parse metalink information.(%1%)" ) % ex.asUserString() );
      _sigFailed.emit();
      return;
    }

    if ( mirrs.size() == 0 ) {
      _error = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, zypp::str::Format("Invalid metalink information.( No mirrors in metalink file)" ) );
      _sigFailed.emit();
      return;
    }

    //remove the metalink file
    zypp::filesystem::unlink( targetPath );
    _mirrorControlReadyConn = sm._mirrorControl->connect( &MirrorControl::sigAllMirrorsReady, *this, &PrepareMultiState::onMirrorsReady );

    // this will emit a mirrorsReady signal once all connection tests have been done
    sm._mirrorControl->registerMirrors( mirrs );
  }

  void DownloadPrivateBase::PrepareMultiState::onMirrorsReady()
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;
    const auto &url = spec.url();
    _mirrorControlReadyConn.disconnect();

#if ENABLE_ZCHUNK_COMPRESSION
    if ( _haveZckData  ) {
      _sigFinished.emit();
      return;
    }
#endif

    // we have no zchunk data, so for a multi download we need a blocklist
    if ( !_blockList.haveBlocks() )  {
      //if we have no filesize we can not generate a blocklist, we need to fall back to normal download
      if ( !_blockList.haveFilesize() ) {

        //fall back to normal download but use a mirror from the mirror list
        //otherwise we get HTTPS to HTTP redirect errors
        _sigFallback.emit();
        return;
      } else {
        //we generate a blocklist on the fly based on the filesize

        XXX << "Generate blocklist, since there was none in the metalink file." << url  << std::endl;

        off_t currOff = 0;
        off_t filesize = _blockList.getFilesize();
        while ( currOff <  filesize )  {

          auto blksize = filesize - currOff ;
          if ( blksize > sm._spec.preferredChunkSize() )
            blksize = sm._spec.preferredChunkSize();

          _blockList.addBlock( currOff, blksize );
          currOff += blksize;
        }

        XXX << "Generated blocklist: " << std::endl << _blockList << std::endl << " End blocklist " << std::endl;
      }
    }

    _sigFinished.emit();
    XXX << "Got control back" << std::endl;
  }

  std::shared_ptr<DownloadPrivateBase::DlNormalFileState> DownloadPrivateBase::PrepareMultiState::fallbackToNormalTransition()
  {
    DBG << "No blocklist and no filesize, falling back to normal download for URL " << stateMachine()._spec.url() << std::endl;
    auto ptr = std::make_shared<DlNormalFileState>( stateMachine() );
    ptr->_mirrors = std::move(_mirrors);

    if ( _blockList.haveFileChecksum() ) {
      ptr->_chksumtype = _blockList.fileChecksumType();
      ptr->_chksumVec  = _blockList.getFileChecksum();
    }

    return ptr;
  }

  std::shared_ptr<DownloadPrivateBase::DlMetalinkState> PrepareMultiState::transitionToMetalinkDl()
  {
    return std::make_shared<DlMetalinkState>( std::move(_blockList), std::move(_mirrors), stateMachine() );
  }

  std::shared_ptr<DownloadPrivateBase::FinishedState> DownloadPrivateBase::PrepareMultiState::transitionToFinished()
  {
    return std::make_shared<FinishedState>( std::move(_error), stateMachine() );
  }

#if ENABLE_ZCHUNK_COMPRESSION
  std::shared_ptr<DownloadPrivateBase::DLZckHeadState> PrepareMultiState::transitionToZckHeadDl()
  {
    return std::make_shared<DLZckHeadState>( std::move(_mirrors), stateMachine() );
  }

  bool DownloadPrivateBase::PrepareMultiState::toZckHeadDownloadGuard() const
  {
    return ( stateMachine().hasZckInfo() );
  }
#endif

  bool DownloadPrivateBase::PrepareMultiState::toMetalinkDownloadGuard() const
  {
#if ENABLE_ZCHUNK_COMPRESSION
    return (!toZckHeadDownloadGuard());
#else
    return true;
#endif
  }

  void DownloadPrivateBase::RangeDownloaderBaseState::onRequestStarted( NetworkRequest & )
  { }

  void DownloadPrivateBase::RangeDownloaderBaseState::onRequestProgress( NetworkRequest &, off_t , off_t, off_t , off_t  )
  {
    off_t dlnowMulti = _downloadedMultiByteCount;
    for( const auto &req : _runningRequests ) {
      dlnowMulti += req->downloadedByteCount();
    }
    stateMachine()._sigProgress.emit( *stateMachine().z_func(), _fileSize, dlnowMulti );
  }

  void DownloadPrivateBase::RangeDownloaderBaseState::onRequestFinished( NetworkRequest &req, const zyppng::NetworkRequestError &err )
  {
    auto lck = stateMachine().z_func()->shared_from_this();
    auto it = std::find_if( _runningRequests.begin(), _runningRequests.end(), [ &req ]( const std::shared_ptr<Request> &r ) {
      return ( r.get() == &req );
    });
    if ( it == _runningRequests.end() )
      return;

    auto reqLocked = *it;

    //remove from running
    _runningRequests.erase( it );

    //feed the working URL back into the mirrors in case there are still running requests that might fail
    // @TODO , finishing the transfer might never be called in case of cancelling the request, need a better way to track running transfers
    if ( reqLocked->_myMirror )
      reqLocked->_myMirror->finishTransfer( !err.isError() );

    if ( err.isError() ) {
      return handleRequestError( reqLocked, err );
    }

    _downloadedMultiByteCount += req.downloadedByteCount();

    DBG << "Request finished "<<std::endl;
    const auto &rngs = reqLocked->requestedRanges();
    std::for_each( rngs.begin(), rngs.end(), []( const auto &b ){ DBG << "-> Block " << b.start << " finished." << std::endl; } );

    auto restartReqWithBlock = [ this ]( std::shared_ptr<Request> &req, std::vector<Block> &&blocks ) {
      DBG << "Reusing Request to download blocks:"<<std::endl;
      if ( !addBlockRanges( req, std::move( blocks ) ) )
        return false;

      //this is not a new request, only add to queues but do not connect signals again
      addNewRequest( req, false );
      return true;
    };

    //check if we already have enqueued all blocks if not reuse the request
    if ( _ranges.size() ) {
      DBG << "Reusing to download blocks: "<<std::endl;
      if ( !restartReqWithBlock( reqLocked, getNextBlocks( reqLocked->url().getScheme() ) ) ) {
        return setFailed( "Failed to restart request with new blocks." );
      }
      return;

    } else {
      //if we have failed blocks, try to download them with this mirror
      if ( !_failedRanges.empty() ) {

        auto fblks = getNextFailedBlocks( reqLocked->url().getScheme() );
        DBG << "Reusing to download failed blocks: "<<std::endl;
        if ( !restartReqWithBlock( reqLocked, std::move(fblks) ) ) {
            return setFailed( "Failed to restart request with previously failed blocks." );
        }
        return;
      }
    }

    //feed the working URL back into the mirrors in case there are still running requests that might fail
    _mirrors.push_back( reqLocked->_originalUrl );

    // make sure downloads are running, at this p
    ensureDownloadsRunning();
  }

  void DownloadPrivateBase::RangeDownloaderBaseState::handleRequestError( std::shared_ptr<Request> req, const zyppng::NetworkRequestError &err )
  {
    bool retry = false;
    auto &parent = stateMachine();


    //Handle the auth errors explicitely, we need to give the user a way to put in new credentials
    //if we get valid new credentials we can retry the request
    if ( err.type() == NetworkRequestError::Unauthorized || err.type() == NetworkRequestError::AuthFailed ) {
      retry = parent.handleRequestAuthError( req, err );
    } else {

      //if a error happens during a multi download we try to use another mirror to download the failed block
      DBG << "Request failed " << req->extendedErrorString() << "(" << req->url() << ")" << std::endl;

      NetworkRequestError dummyErr;

      const auto &fRanges = req->failedRanges();
      try {
        std::transform( fRanges.begin(), fRanges.end(), std::back_inserter(_failedRanges), [ &req ]( const auto &r ){
          Block b = std::any_cast<Block>(r.userData);;
          b._failedWithErr = req->error();
          DBG << "Adding failed block to failed blocklist: " << b.start << " " << b.len << " (" << req->error().toString() << " [" << req->error().nativeErrorString()<< "])" << std::endl;
          return b;
        });

        //try to init a new multi request, if we have leftover mirrors we get a valid one
        auto newReq = initMultiRequest( dummyErr, true );
        if ( newReq ) {
          addNewRequest( newReq );
          return;
        }
        // we got no mirror this time, but since we still have running requests there is hope this will be finished later
        if ( _runningRequests.size() && _failedRanges.size() )
          return;

      } catch ( const zypp::Exception &ex ) {
        //we just log the exception and fall back to a normal download
        WAR << "Multipart download failed: " << ex.asString() << std::endl;
      }
    }

    //if rety is true we just enqueue the request again, usually this means authentication was updated
    if ( retry ) {
      //make sure this request will run asap
      req->setPriority( parent._defaultSubRequestPriority );

      //this is not a new request, only add to queues but do not connect signals again
      addNewRequest( req, false );
      return;
    }

    //we do not have more mirrors left we can try
    cancelAll ( err );

    // not all hope is lost, maybe a normal download can work out?
    // fall back to normal download
    _sigFailed.emit();
  }

  void DownloadPrivateBase::RangeDownloaderBaseState::ensureDownloadsRunning()
  {
    //check if there is still work to do
    if ( _ranges.size() && _runningRequests.size() < 10 ) {

      NetworkRequestError lastErr = _error;

      //we try to allocate as many requests as possible but stop if we cannot find a valid mirror for one
      while( _runningRequests.size() < 10 ) {
        auto req = initMultiRequest( lastErr );
        if (!req ) {
          break;
        }
        addNewRequest( req );
      }

      while ( lastErr.type() == NetworkRequestError::NoError && _failedRanges.size() ) {

        if ( _runningRequests.size() >= 10 )
          break;

        auto req = initMultiRequest( lastErr, true );
        if ( !req ) {
          break;
        }

        addNewRequest( req );
      }

      if ( _runningRequests.empty() && lastErr.type()!= NetworkRequestError::NoError )  {
        //we found no mirrors -> fail
        setFailed( std::move( lastErr) );
        return;
      }
    }

    if ( _runningRequests.empty() ) {

      if ( _failedRanges.size() || _ranges.size() ) {
        setFailed( NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, "Unable to download all blocks." ) );
        return;
      }

      // seems we were successfull , transition to finished state
      setFinished();
    }
  }

  void DownloadPrivateBase::RangeDownloaderBaseState::reschedule()
  {
    bool triggerResched = false;
    for ( auto &req : _runningRequests ) {
      if ( req->state() == NetworkRequest::Pending ) {
        triggerResched = true;
        req->setPriority( NetworkRequest::Critical, false );
      }
    }
    if ( triggerResched )
      stateMachine()._requestDispatcher->reschedule();
  }

  void DownloadPrivateBase::RangeDownloaderBaseState::addNewRequest(std::shared_ptr<Request> req , const bool connectSignals)
  {
    if ( connectSignals )
      req->connectSignals( *this );

    _runningRequests.push_back( req );
    stateMachine()._requestDispatcher->enqueue( req );

    if ( req->_myMirror )
      req->_myMirror->startTransfer();
  }

  std::shared_ptr<DownloadPrivateBase::Request> DownloadPrivateBase::RangeDownloaderBaseState::initMultiRequest( NetworkRequestError &err, bool useFailed )
  {
    auto &parent = stateMachine();
    Url myUrl;
    TransferSettings settings;

    MirrorControl::MirrorHandle mirr;
    if (  _mirrors.size() ) {
      if ( !( mirr = parent.findNextMirror( _mirrors, myUrl, settings, err ) ) )
        return nullptr;
    } else {
      MIL << "We did run out of mirrors, aborting block download." << std::endl;
      return nullptr;
    }

    auto blocks = useFailed ?  getNextFailedBlocks( myUrl.getScheme() ) : getNextBlocks( myUrl.getScheme() );
    if ( !blocks.size() ) {

      // we do not use the mirror, give it back to the list
      if ( mirr )
        _mirrors.push_back( myUrl );

      err = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, "Did not find blocks to download." );
      return nullptr;
    }

    const auto &spec = parent._spec;

    std::shared_ptr<Request> req = std::make_shared<Request>( ::internal::clearQueryString( myUrl ), spec.targetPath(), NetworkRequest::WriteShared );
    req->_myMirror = mirr;
    req->_originalUrl = myUrl;
    req->setPriority( parent._defaultSubRequestPriority );
    req->transferSettings() = settings;

    // if we download chunks we do not want to wait for too long on mirrors that are slow to answer
    req->transferSettings().setTimeout( 2 );

    DBG << "Creating Request to download blocks:"<<std::endl;
    if ( !addBlockRanges( req, std::move(blocks) ) ) {
      err = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, "Failed to add blocks to request." );
      return nullptr;
    }

    return req;
  }

  /**
   * Just initialize the requests ranges from the internal blocklist
   */
  bool DownloadPrivateBase::RangeDownloaderBaseState::addBlockRanges ( std::shared_ptr<DownloadPrivateBase::Request> req , std::vector<Block> &&blocks ) const
  {
    req->resetRequestRanges();
    for ( const auto &block : blocks ) {
      if ( block.chksumVec && block.chksumtype.size() ) {
        std::shared_ptr<zypp::Digest> dig = std::make_shared<zypp::Digest>();
        if ( !dig->create( block.chksumtype ) ) {
          WAR << "Trying to create Digest with chksum type " << block.chksumtype << " failed " << std::endl;
          return false;
        }

        DBG << "Starting block " << block.start << " with checksum " << zypp::Digest::digestVectorToString( *block.chksumVec ) << "." << std::endl;
        req->addRequestRange( block.start, block.len, dig, *block.chksumVec, std::any( block ), block.chksumCompareLen );
      } else {
        DBG << "Starting block " << block.start << " without checksum." << std::endl;
        req->addRequestRange( block.start, block.len, {}, {}, std::any( block ) );
      }
    }
    return true;
  }

  void zyppng::DownloadPrivateBase::RangeDownloaderBaseState::setFailed(NetworkRequestError &&err)
  {
    _error = std::move( err );
    zypp::filesystem::unlink( stateMachine()._spec.targetPath() );
    _sigFailed.emit();
  }

  void DownloadPrivateBase::RangeDownloaderBaseState::setFailed(std::string &&reason)
  {
    setFailed( NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, std::move(reason) ) );
  }

  void DownloadPrivateBase::RangeDownloaderBaseState::setFinished()
  {
    _error = NetworkRequestError();
    _sigFinished.emit();
  }

  void DownloadPrivateBase::RangeDownloaderBaseState::cancelAll(const NetworkRequestError &err)
  {
    while( _runningRequests.size() ) {
      auto req = _runningRequests.back();
      req->disconnectSignals();
      _runningRequests.pop_back();
      stateMachine()._requestDispatcher->cancel( *req, err );
      if ( req->_myMirror )
        req->_myMirror->cancelTransfer();
    }
  }

  std::vector<DownloadPrivateBase::Block> DownloadPrivateBase::RangeDownloaderBaseState::getNextBlocks( const std::string &urlScheme )
  {
    std::vector<DownloadPrivateBase::Block> blocks;    
    const auto prefSize = static_cast<size_t>( stateMachine()._spec.preferredChunkSize() );
    size_t accumulatedSize = 0;

    bool canDoRandomBlocks = ( zypp::str::hasPrefixCI( urlScheme, "http") );
    
    std::optional<size_t> lastBlockEnd;
    while ( _ranges.size() && accumulatedSize < prefSize ) {
      const auto &r = _ranges.front();
      
      if ( !canDoRandomBlocks && lastBlockEnd ) {
        if ( static_cast<const size_t>(r.start) != (*lastBlockEnd)+1 )
          break;
      }
      
      lastBlockEnd = r.start + r.len - 1;
      accumulatedSize += r.len;
      
      blocks.push_back( std::move( _ranges.front() ) );
      _ranges.pop_front();
      
    }
    XXX << "Accumulated " << blocks.size() <<  " blocks with accumulated size of: " << accumulatedSize << "." << std::endl;
    return blocks;
  }

  std::vector<DownloadPrivateBase::Block> DownloadPrivateBase::RangeDownloaderBaseState::getNextFailedBlocks( const std::string &urlScheme )
  {
    const auto prefSize = static_cast<size_t>( stateMachine()._spec.preferredChunkSize() );
    // sort the failed requests by block number, this should make sure get them in offset order as well
    _failedRanges.sort( []( const auto &a , const auto &b ){ return a.start < b.start; } );

    bool canDoRandomBlocks = ( zypp::str::hasPrefixCI( urlScheme, "http") );

    std::vector<DownloadPrivateBase::Block> fblks;
    std::optional<size_t> lastBlockEnd;
    size_t accumulatedSize = 0;
    while ( _failedRanges.size() ) {
      
      const auto &block =_failedRanges.front();
      
      //we need to check if we have consecutive blocks because only http mirrors support random request ranges
      if ( !canDoRandomBlocks && lastBlockEnd ) {
        if ( static_cast<const size_t>(block.start) != (*lastBlockEnd)+1 )
          break;
      }
      
      lastBlockEnd = block.start + block.len - 1;
      accumulatedSize += block.len;
      
      fblks.push_back( std::move( _failedRanges.front() ));
      _failedRanges.pop_front();
      
      fblks.back()._retryCount += 1;

      if ( accumulatedSize >= prefSize )
        break;
    }

    return fblks;
  }

  DlMetalinkState::DlMetalinkState(zypp::media::MediaBlockList &&blockList, std::vector<Url> &&mirrors, DownloadPrivate &parent)
  : RangeDownloaderBaseState( std::move(mirrors), parent ), _blockList( std::move(blockList) )
  {
    MIL << "About to enter DlMetalinkState for url " << parent._spec.url() << std::endl;
  }

  void DownloadPrivateBase::DlMetalinkState::enter()
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;

    //first we try to reuse blocks from the deltafile , if we have one
    if ( !spec.deltaFile().empty() ) {
      zypp::PathInfo dFileInfo ( spec.deltaFile() );
      if ( dFileInfo.isFile() && dFileInfo.isR() ) {
        FILE *f = fopen( spec.targetPath().asString().c_str(), "w+b" );
        if ( !f ) {
          setFailed( NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, zypp::str::Format("Failed to open target file.(errno %1%)" ) % errno ) );
          return;
        }

        try {
          _blockList.reuseBlocks ( f, spec.deltaFile().asString() );
        } catch ( ... ) { }

        fclose( f );
      }
    }

    // setup the base downloader
    _error = {};
    _ranges.clear();
    _failedRanges.clear();
    _downloadedMultiByteCount = 0;

    if ( _blockList.haveFileChecksum() ) {
      _fileChecksumType = _blockList.fileChecksumType();
      _fileChksumVec    = _blockList.getFileChecksum();
    }

    const size_t fLen = _blockList.getFilesize();
    if ( _fileSize > 0 ) {
      // check if the file size as reported by zchunk is equal to the one we expect
      if ( _fileSize != fLen ) {
        return setFailed( NetworkRequestErrorPrivate::customError(
          NetworkRequestError::ExceededMaxLen,
          zypp::str::Format("Metalink file reports a different filesize than what was expected ( Meta: %1% != Exp: %2%).") % fLen % _fileSize )
          );
      }
    } else {
      _fileSize = fLen;
    }

    // remember how many bytes we need to download
    size_t bytesToDl = 0;
    for ( size_t i = 0; i < _blockList.numBlocks(); i++ ) {
      const auto &mediaBlock = _blockList.getBlock( i );
      _ranges.push_back(
        Block{
          .start = mediaBlock.off,
          .len   = mediaBlock.size,
          .chksumtype = _blockList.getChecksumType(),
          .chksumVec  = _blockList.getChecksum ( i )
        }
      );

      bytesToDl += mediaBlock.size;
    }
    // substract the length of the blocks we have to download from the overall file size
    _downloadedMultiByteCount = fLen - bytesToDl;

    ensureDownloadsRunning();
  }

  void DownloadPrivateBase::DlMetalinkState::exit()
  {
    cancelAll( NetworkRequestError() );
  }

  void DownloadPrivateBase::DlMetalinkState::setFinished()
  {
    if ( _fileChecksumType.size() && _fileChksumVec ) {
      //TODO move this into a external application so we do not need to block on it
      //need to check file digest
      zypp::Digest dig;
      dig.create( _fileChecksumType );

      std::ifstream istrm( stateMachine()._spec.targetPath().asString(), std::ios::binary);
      if ( !istrm.is_open() ) {
        setFailed( "Failed to verify file digest (Could not open target file)." );
        return;
      }
      if ( !dig.update( istrm ) ) {
        setFailed( "Failed to verify file digest (Could not read target file)." );
        return;
      }

      const auto &calculatedChksum = dig.digestVector();
      if ( *_fileChksumVec != calculatedChksum ) {
        setFailed( "Failed to verify file digest (Checksum did not match)." );
        return;
      }
    }
    RangeDownloaderBaseState::setFinished();
  }

  std::shared_ptr<DownloadPrivateBase::FinishedState> DlMetalinkState::transitionToFinished()
  {
    return std::make_shared<FinishedState>( std::move(_error), stateMachine() );
  }

  DlNormalFileState::DlNormalFileState(DownloadPrivate &parent) : BasicDownloaderStateBase( parent )
  {
    MIL << "About to enter DlNormalFileState for url " << parent._spec.url() << std::endl;
  }

  std::shared_ptr<DownloadPrivateBase::FinishedState> DlNormalFileState::transitionToFinished()
  {
    return std::make_shared<FinishedState>( std::move(_error), stateMachine() );
  }

#if ENABLE_ZCHUNK_COMPRESSION
  DLZckHeadState::DLZckHeadState(std::vector<Url> &&mirrors, DownloadPrivate &parent) :
    BasicDownloaderStateBase( parent )
  {
    _mirrors = std::move(mirrors);
    MIL << "About to enter DlZckHeadState for url " << parent._spec.url() << std::endl;
  }

  bool DownloadPrivateBase::DLZckHeadState::initializeRequest(std::shared_ptr<DownloadPrivateBase::Request> r)
  {
    BasicDownloaderStateBase::initializeRequest( r );

    const auto &s = stateMachine()._spec;
    if ( s.headerSize() == 0 ) {
      ERR << "Downloading the zck header was requested, but headersize is zero." << std::endl;
      return false;
    }

    std::shared_ptr<zypp::Digest> digest;
    NetworkRequest::CheckSumBytes sum;

    const auto &headerSum = s.headerChecksum();
    if ( headerSum ) {
      digest = std::make_shared<zypp::Digest>();
      if ( !digest->create( headerSum->type() ) ) {
        ERR << "Unknown header checksum type " << headerSum->type() << std::endl;
        return false;
      }
      sum = zypp::Digest::hexStringToUByteArray( headerSum->checksum() );
    }

    r->addRequestRange( 0, s.headerSize(), digest, sum );
    return true;
  }

  void DownloadPrivateBase::DLZckHeadState::gotFinished()
  {
    if ( isZchunkFile( stateMachine()._spec.targetPath() ) )
      return BasicDownloaderStateBase::gotFinished();
    failed ( "Downloaded header is not a zchunk header");
  }

  std::shared_ptr<DownloadPrivateBase::DLZckState> DLZckHeadState::transitionToDlZckState()
  {
    MIL << "Downloaded the header of size: " << _request->downloadedByteCount() << std::endl;
    return std::make_shared<DLZckState>( std::move(_mirrors), stateMachine() );
  }

  DLZckState::DLZckState(std::vector<Url> &&mirrors, DownloadPrivate &parent)
  : RangeDownloaderBaseState( std::move(mirrors), parent )
  {
    MIL << "About to enter DLZckState for url " << parent._spec.url() << std::endl;
  }

  void DownloadPrivateBase::DLZckState::enter()
  {
    const auto &spec = stateMachine()._spec;

    // setup the base downloader
    _error = {};
    _ranges.clear();
    _failedRanges.clear();

    // @TODO get this from zchunk file?
    _fileSize = spec.expectedFileSize();

    zypp::AutoFD src_fd = open( spec.deltaFile().asString().c_str(), O_RDONLY);
    if(src_fd < 0)
      return setFailed ( zypp::str::Format("Unable to open %1%") % spec.deltaFile() );

    zypp::AutoDispose<zckCtx *> zck_src ( zck_create(), []( auto ptr ) { if ( ptr ) zck_free( &ptr ); } );
    if( !zck_src )
      return setFailed ( zypp::str::Format("%1%") % zck_get_error(NULL) );

    if(!zck_init_read(zck_src, src_fd))
      return setFailed ( zypp::str::Format( "Unable to open %1%: %2%") %  spec.deltaFile() % zck_get_error(zck_src) );

    zypp::AutoFD target_fd = open( spec.targetPath().asString().c_str(), O_RDWR);
    if(target_fd < 0)
      return setFailed ( zypp::str::Format("Unable to open %1%") % spec.targetPath() );

    zypp::AutoDispose<zckCtx *> zckTarget ( zck_create(), []( auto ptr ) { if ( ptr ) zck_free( &ptr ); } );
    if( !zckTarget )
      return setFailed ( zypp::str::Format("%1%") % zck_get_error(NULL) );

    if(!zck_init_read(zckTarget, target_fd))
      return setFailed ( zypp::str::Format( "Unable to open %1%: %2%") %  spec.targetPath() % zck_get_error(zckTarget) );

    // Returns 0 for error, -1 for invalid checksum and 1 for valid checksum
    switch ( zck_find_valid_chunks(zckTarget) ) {
      case 0: // Returns 0 if there was a error
        return setFailed ( zypp::str::Format( "Unable to open %1%: %2%") %  spec.targetPath() % zck_get_error(zckTarget) );
      case 1: // getting a 1 would mean the file is already complete, basically impossible but lets handle it anyway
        return setFinished();
    }

    const auto srcHashType = zck_get_chunk_hash_type( zckTarget );
    const auto targetHashType = zck_get_chunk_hash_type( zckTarget );

    const size_t fLen = zck_get_length( zckTarget );
    if ( _fileSize > 0 ) {
      // check if the file size as reported by zchunk is equal to the one we expect
      if ( _fileSize != fLen ) {
        return setFailed( NetworkRequestErrorPrivate::customError(
          NetworkRequestError::ExceededMaxLen,
          zypp::str::Format("Zchunk header reports a different filesize than what was expected ( Zck: %1% != Exp: %2%).") % fLen % _fileSize )
        );
      }
    } else {
      _fileSize = fLen;
    }

    if( srcHashType != targetHashType )
      return setFailed ( zypp::str::Format( "ERROR: Chunk hash types don't match. Source Hash: %1% vs Target Hash: %2%")
                        % zck_hash_name_from_type ( srcHashType )
                        % zck_hash_name_from_type ( targetHashType ) );

    if(!zck_copy_chunks( zck_src, zckTarget ))
      return setFailed ( zypp::str::Format( "Unable to copy chunks from deltafile.") );

    // will reset all chunks that are marked as failed back to missing
    zck_reset_failed_chunks( zckTarget );


    // we calculate what is already downloaded by substracting the block sizes we still need to download from the full file size
    _downloadedMultiByteCount = _fileSize;

    auto chunk = zck_get_first_chunk( zckTarget );
    do {
      // Get validity of current chunk: 1 = valid, 0 = missing, -1 = invalid
      if ( zck_get_chunk_valid( chunk ) == 1 )
        continue;

      zypp::AutoFREE<char> zckDigest( zck_get_chunk_digest( chunk ) );
      UByteArray chksumVec = zypp::Digest::hexStringToUByteArray( std::string_view( zckDigest.value() ) );
      std::string chksumName;
      std::optional<size_t> chksumCompareLen;

      switch ( targetHashType ) {
        case ZCK_HASH_SHA1: {
          chksumName = zypp::Digest::sha1();
          break;
        }
        case ZCK_HASH_SHA256: {
          chksumName = zypp::Digest::sha256();
          break;
        }
        case ZCK_HASH_SHA512: {
          chksumName = zypp::Digest::sha512();
          break;
        }
        case ZCK_HASH_SHA512_128: {
          // defined in zchunk as
          // SHA-512/128 (first 128 bits of SHA-512 checksum)
          chksumName = zypp::Digest::sha512();
          chksumCompareLen = chksumVec.size();
          break;
        }
        default: {
          return setFailed ( zypp::str::Format( "Unsupported chunk hash type: %1%.") % zck_hash_name_from_type( targetHashType ) );
        }
      }

      const auto s = static_cast<off_t>( zck_get_chunk_start( chunk ) );
      const auto l = static_cast<size_t>( zck_get_chunk_comp_size ( chunk ) );

      MIL << "Downloading block " << s << " with length " << l << " checksum " << zckDigest.value() << " type " << chksumName << std::endl;

      _ranges.push_back( Block{
        .start = s,
        .len   = l,
        .chksumtype = chksumName,
        .chksumVec  = std::move( chksumVec ),
        .chksumCompareLen = std::move(chksumCompareLen)
      } );

      // substract the block length from the already downloaded bytes size
      _downloadedMultiByteCount -= l;

    } while ( (chunk = zck_get_next_chunk( chunk )) );

    ensureDownloadsRunning();
  }

  void DownloadPrivateBase::DLZckState::exit()
  {
    cancelAll( NetworkRequestError() );
  }

  std::shared_ptr<DownloadPrivateBase::FinishedState> DLZckState::transitionToFinished()
  {
    return std::make_shared<FinishedState>( std::move(_error), stateMachine() );
  }

  void DownloadPrivateBase::DLZckState::setFinished()
  {
    const auto &spec = stateMachine()._spec;

    zypp::AutoFD target_fd = open( spec.targetPath().asString().c_str(), O_RDONLY );
    if( target_fd < 0 )
      return setFailed ( zypp::str::Format("Unable to open %1%") % spec.targetPath() );

    zypp::AutoDispose<zckCtx *> zckTarget ( zck_create(), []( auto ptr ) { if ( ptr ) zck_free( &ptr ); } );
    if( !zckTarget )
      return setFailed ( zypp::str::Format("%1%") % zck_get_error(nullptr) );

    if(!zck_init_read(zckTarget, target_fd))
      return setFailed ( zypp::str::Format( "Unable to open %1%: %2%") %  spec.targetPath() % zck_get_error(zckTarget) );

    /* Validate the chunk and data checksums for the current file.
     * Returns 0 for error, -1 for invalid checksum and 1 for valid checksum */
    const auto res = zck_validate_checksums( zckTarget );
    if ( res == 0 || res == -1 ) {
      if( zck_is_error(nullptr) ) {
        std::string err = zck_get_error(NULL);
        zck_clear_error(NULL);
        return setFailed( std::move(err) );
      }
      if( zck_is_error(zckTarget) )
        return setFailed( zck_get_error(zckTarget) );
      return setFailed( "zck_validate_checksums returned a unknown error." );
    }

    // everything is valid
    RangeDownloaderBaseState::setFinished();
  }
#endif

  FinishedState::FinishedState(NetworkRequestError &&error, DownloadPrivate &parent) : SimpleState( parent ),_error( std::move(error) )
  {
    MIL << "About to enter FinishedState for url " << parent._spec.url() << std::endl;
  }

  bool DownloadPrivateBase::handleRequestAuthError( std::shared_ptr<Request> req, const zyppng::NetworkRequestError &err )
  {
    //Handle the auth errors explicitely, we need to give the user a way to put in new credentials
    //if we get valid new credentials we can retry the request
    bool retry = false;
    if ( err.type() == NetworkRequestError::Unauthorized || err.type() == NetworkRequestError::AuthFailed ) {

      MIL << "Authentication failed for " << req->url() << " trying to recover." << std::endl;

      zypp::media::CredentialManager cm( zypp::media::CredManagerOptions( zypp::ZConfig::instance().repoManagerRoot()) );
      auto authDataPtr = cm.getCred( req->url() );

      // get stored credentials
      NetworkAuthData_Ptr cmcred( authDataPtr ? new NetworkAuthData( *authDataPtr ) : new NetworkAuthData() );
      TransferSettings &ts = req->transferSettings();

      // We got credentials from store, _triedCredFromStore makes sure we just try the auth from store once
      // if its timestamp does not change between 2 retries
      if ( cmcred && ( !req->_triedCredFromStore || req->_authTimestamp < cmcred->lastDatabaseUpdate() ) ) {
        MIL << "got stored credentials:" << std::endl << *cmcred << std::endl;
        ts.setUsername( cmcred->username() );
        ts.setPassword( cmcred->password() );
        retry = true;
        req->_triedCredFromStore = true;
        _lastTriedAuthTime = req->_authTimestamp = cmcred->lastDatabaseUpdate();
      } else {

        //we did not get credentials from the store, emit a signal that allows
        //setting new auth data

        NetworkAuthData credFromUser;
        credFromUser.setUrl( req->url() );

        //in case we got a auth hint from the server the error object will contain it
        std::string authHint = err.extraInfoValue("authHint", std::string());

        //preset from store if we found something
        if ( cmcred && !cmcred->username().empty() )
          credFromUser.setUsername( cmcred->username() );

        _sigAuthRequired.emit( *z_func(), credFromUser, authHint );
        if ( credFromUser.valid() ) {
          MIL << "Got user provided credentials" << std::endl;
          ts.setUsername( credFromUser.username() );
          ts.setPassword( credFromUser.password() );

          // set available authentication types from the error
          if ( credFromUser.authType() == CURLAUTH_NONE )
            credFromUser.setAuthType( authHint );

          // set auth type (seems this must be set _after_ setting the userpwd)
          if ( credFromUser.authType()  != CURLAUTH_NONE ) {
            // FIXME: only overwrite if not empty?
            req->transferSettings().setAuthType(credFromUser.authTypeAsString());
          }

          cm.addCred( credFromUser );
          cm.save();

          // potentially setting this after the file has been touched we might miss a change
          // in a later loop, if another update to the store happens right in the few ms difference
          // between the actual file write and calling time() here. However this is highly unlikely
          _lastTriedAuthTime = req->_authTimestamp = time( nullptr ) ;

          retry = true;
        }
      }
    }
    return retry;
  }

#if ENABLE_ZCHUNK_COMPRESSION
  bool DownloadPrivateBase::hasZckInfo() const
  {
    if ( zypp::indeterminate(_specHasZckInfo) )
      _specHasZckInfo = ( _spec.headerSize() > 0 && isZchunkFile( _spec.deltaFile() ) );
    return bool(_specHasZckInfo);
  }
#endif

  void DownloadPrivateBase::Request::disconnectSignals()
  {
    _sigStartedConn.disconnect();
    _sigProgressConn.disconnect();
    _sigFinishedConn.disconnect();
  }

  DownloadPrivate::DownloadPrivate(Downloader &parent, std::shared_ptr<NetworkRequestDispatcher> requestDispatcher, std::shared_ptr<MirrorControl> mirrors, DownloadSpec &&spec, Download &p) :
    DownloadPrivateBase( parent, std::move(requestDispatcher), std::move(mirrors), std::move(spec), p)
  {
    Base::connectFunc( *this, &DownloadStatemachine<DownloadPrivate>::sigFinished, [this](){
      DownloadPrivateBase::_sigFinished.emit( *z_func() );
    });

    Base::connectFunc( *this, &DownloadStatemachine<DownloadPrivate>::sigStateChanged, [this]( const auto state ){
      DownloadPrivateBase::_sigStateChanged.emit( *z_func(), state );
    });

    //DownloadStatemachine<DownloadPrivate>::start();
  }

  void DownloadPrivate::start()
  {
    auto cState = currentState();
    if ( !cState )
      DownloadStatemachine<DownloadPrivate>::start();

    cState = currentState();
    if ( *cState != Download::InitialState && *cState != Download::Finished ) {
      // the state machine has advaned already, we can only restart it in a finished state
        return;
    }

    //reset state variables
    _emittedSigStart = false;
    _specHasZckInfo  = zypp::indeterminate;

    // restart the statemachine
    if ( cState == Download::Finished )
      DownloadStatemachine<DownloadPrivate>::start();

    //jumpstart the process
    state<InitialState>().initiate();
  }


  NetworkRequestError DownloadPrivateBase::safeFillSettingsFromURL( const Url &url, TransferSettings &set)
  {
    auto buildExtraInfo = [this, &url](){
      std::map<std::string, boost::any> extraInfo;
      extraInfo.insert( {"requestUrl", url } );
      extraInfo.insert( {"filepath", _spec.targetPath() } );
      return extraInfo;
    };

    NetworkRequestError res;
    try {
      ::internal::fillSettingsFromUrl( url, set );
      if ( _spec.settings().proxy().empty() )
        ::internal::fillSettingsSystemProxy( url, set );

      /* Fixes bsc#1174011 "auth=basic ignored in some cases"
       * We should proactively add the password to the request if basic auth is configured
       * and a password is available in the credentials but not in the URL.
       *
       * We will be a bit paranoid here and require that the URL has a user embedded, otherwise we go the default route
       * and ask the server first about the auth method
       */
      if ( set.authType() == "basic"
           && set.username().size()
           && !set.password().size() ) {
        zypp::media::CredentialManager cm( zypp::media::CredManagerOptions( zypp::ZConfig::instance().repoManagerRoot()) );
        const auto cred = cm.getCred( url );
        if ( cred && cred->valid() ) {
          if ( !set.username().size() )
            set.setUsername(cred->username());
          set.setPassword(cred->password());
        }
      }

    } catch ( const zypp::media::MediaBadUrlException & e ) {
      res = NetworkRequestErrorPrivate::customError( NetworkRequestError::MalformedURL, e.asString(), buildExtraInfo() );
    } catch ( const zypp::media::MediaUnauthorizedException & e ) {
      res = NetworkRequestErrorPrivate::customError( NetworkRequestError::AuthFailed, e.asString(), buildExtraInfo() );
    } catch ( const zypp::Exception & e ) {
      res = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, e.asString(), buildExtraInfo() );
    }
    return res;
  }

  MirrorControl::MirrorHandle DownloadPrivateBase::findNextMirror( std::vector<Url> &_mirrors, Url &url, TransferSettings &set, NetworkRequestError &err)
  {
    Url myUrl;
    TransferSettings settings;
    MirrorControl::MirrorHandle mirror;

    while ( !mirror ) {

      const auto nextBest = _mirrorControl->pickBestMirror( _mirrors );
      if ( !nextBest.second )
        break;

      myUrl = *nextBest.first;

      settings = _spec.settings();
      //if this is a different host than the initial request, we reset username/password
      if ( myUrl.getHost() != _spec.url().getHost() ) {
        settings.setUsername( std::string() );
        settings.setPassword( std::string() );
        settings.setAuthType( std::string() );
      }

      err = safeFillSettingsFromURL( myUrl, settings );
      if ( err.type() != NetworkRequestError::NoError ) {
        continue;
      }

      _mirrors.erase( nextBest.first );
      mirror = nextBest.second;
      break;
    }

    if ( !mirror ) {
      err = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, "No valid mirror found" );
      return nullptr;
    }

    url = myUrl;
    set = settings;
    return mirror;
  }

  Download::Download(zyppng::Downloader &parent, std::shared_ptr<zyppng::NetworkRequestDispatcher> requestDispatcher, std::shared_ptr<zyppng::MirrorControl> mirrors, zyppng::DownloadSpec &&spec)
    : Base( *new DownloadPrivate( parent, std::move(requestDispatcher), std::move(mirrors), std::move(spec), *this )  )
  { }

  Download::~Download()
  {
    cancel();
  }

  Download::State Download::state() const
  {
    const auto &s = d_func()->currentState();
    if ( !s )
      return Download::InitialState;
    return *s;
  }

  NetworkRequestError Download::lastRequestError() const
  {
    if ( state() == Finished ) {
      return d_func()->state<DownloadPrivateBase::FinishedState>()._error;
    }
    return NetworkRequestError();
  }

  bool Download::hasError() const
  {
    return lastRequestError().isError();
  }

  std::string Download::errorString() const
  {
    const auto &lReq = lastRequestError();
    if (! lReq.isError() ) {
      return {};
    }

    return ( zypp::str::Format("%1%(%2%)") % lReq.toString() % lReq.nativeErrorString() );
  }

  void Download::start()
  {
    d_func()->start();
  }

  void Download::prioritize()
  {
    Z_D();

    if ( !d->_requestDispatcher )
      return;

    d->_defaultSubRequestPriority = NetworkRequest::Critical;

    // we only reschedule requests when we are in a state that downloads in blocks
    d->visitState( []( auto &s ){
      using T = std::decay_t<decltype (s)>;
      if constexpr ( std::is_same_v<T, DlMetalinkState>
#if ENABLE_ZCHUNK_COMPRESSION
          || std::is_same_v<T, DLZckState>
#endif
      ) {
        s.reschedule();
      }
    });
  }

  void Download::cancel()
  {
    Z_D();
    d->forceState ( std::make_unique<FinishedState>( NetworkRequestErrorPrivate::customError( NetworkRequestError::Cancelled, "Download was cancelled explicitely" ), *d_func() ) );
  }

  DownloadSpec &Download::spec()
  {
    return d_func()->_spec;
  }

  const DownloadSpec &Download::spec() const
  {
    return d_func()->_spec;
  }

  uint64_t Download::lastAuthTimestamp() const
  {
    return d_func()->_lastTriedAuthTime;
  }

  zyppng::NetworkRequestDispatcher &Download::dispatcher() const
  {
    return *d_func()->_requestDispatcher;
  }

  SignalProxy<void (Download &req)> Download::sigStarted()
  {
    return d_func()->_sigStarted;
  }

  SignalProxy<void (Download &req, Download::State state)> Download::sigStateChanged()
  {
    return d_func()->DownloadPrivateBase::_sigStateChanged;
  }

  SignalProxy<void (zyppng::Download &req, off_t dlnow)> zyppng::Download::sigAlive()
  {
    return d_func()->_sigAlive;
  }

  SignalProxy<void (Download &req, off_t dltotal, off_t dlnow)> Download::sigProgress()
  {
    return d_func()->_sigProgress;
  }

  SignalProxy<void (Download &req)> Download::sigFinished()
  {
    return d_func()->DownloadPrivateBase::_sigFinished;
  }

  SignalProxy<void (zyppng::Download &req, zyppng::NetworkAuthData &auth, const std::string &availAuth)> Download::sigAuthRequired()
  {
    return d_func()->_sigAuthRequired;
  }

  DownloaderPrivate::DownloaderPrivate(std::shared_ptr<MirrorControl> mc, Downloader &p)
    : BasePrivate(p)
    , _sigStarted(p)
    , _sigFinished(p)
    , _queueEmpty(p)
    , _mirrors( std::move(mc) )
  {
    _requestDispatcher = std::make_shared<NetworkRequestDispatcher>( );
    if ( !_mirrors ) {
      _mirrors = MirrorControl::create();
    }
  }

  void DownloaderPrivate::onDownloadStarted(Download &download)
  {
    _sigStarted.emit( *z_func(), download );
  }

  void DownloaderPrivate::onDownloadFinished( Download &download )
  {
    _sigFinished.emit( *z_func(), download );

    auto it = std::find_if( _runningDownloads.begin(), _runningDownloads.end(), [ &download ]( const std::shared_ptr<Download> &dl){
      return dl.get() == &download;
    });

    if ( it != _runningDownloads.end() ) {
      //make sure this is not deleted before all user code was done
      // EventDispatcher::unrefLater( *it );
      _runningDownloads.erase( it );
    }

    if ( _runningDownloads.empty() )
      _queueEmpty.emit( *z_func() );
  }

  Downloader::Downloader( )
    : Base ( *new DownloaderPrivate( {}, *this ) )
  {

  }

  Downloader::Downloader( std::shared_ptr<MirrorControl> mc )
    : Base ( *new DownloaderPrivate( mc, *this ) )
  { }

  Downloader::~Downloader()
  {
    Z_D();
    while ( d->_runningDownloads.size() ) {
      d->_runningDownloads.back()->cancel();
      d->_runningDownloads.pop_back();
    }
  }

  std::shared_ptr<Download> Downloader::downloadFile(const zyppng::DownloadSpec &spec )
  {
    Z_D();
    std::shared_ptr<Download> dl ( new Download ( *this, d->_requestDispatcher, d->_mirrors, DownloadSpec(spec) ) );

    d->_runningDownloads.push_back( dl );
    dl->connect( &Download::sigFinished, *d, &DownloaderPrivate::onDownloadFinished );
    d->_requestDispatcher->run();

    return dl;
  }

  std::shared_ptr<NetworkRequestDispatcher> Downloader::requestDispatcher() const
  {
    return d_func()->_requestDispatcher;
  }

  SignalProxy<void (Downloader &parent, Download &download)> Downloader::sigStarted()
  {
    return d_func()->_sigStarted;
  }

  SignalProxy<void (Downloader &parent, Download &download)> Downloader::sigFinished()
  {
    return d_func()->_sigFinished;
  }

  SignalProxy<void (Downloader &parent)> Downloader::queueEmpty()
  {
    return d_func()->_queueEmpty;
  }

}
