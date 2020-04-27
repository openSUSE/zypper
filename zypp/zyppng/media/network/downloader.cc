#include <zypp/zyppng/media/network/private/downloader_p.h>
#include <zypp/zyppng/media/network/private/request_p.h>
#include <zypp/zyppng/media/network/private/networkrequesterror_p.h>
#include <zypp/zyppng/media/network/networkrequestdispatcher.h>
#include <zypp/zyppng/media/network/request.h>
#include <zypp/zyppng/base/Timer>
#include <zypp/zyppng/base/EventDispatcher>
#include <zypp/zyppng/Context>
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

#include <queue>
#include <fcntl.h>
#include <iostream>
#include <fstream>

#define BLKSIZE		131072

namespace  {
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
    return looks_like_metalink_data( zyppng::peek_data_fd( fd.get(), 0, 256 ) );
  }
}

namespace zyppng {

  DownloadPrivate::DownloadPrivate(Downloader &parent, std::shared_ptr<NetworkRequestDispatcher> requestDispatcher, std::shared_ptr<MirrorControl> mirrors, Url &&file, zypp::filesystem::Pathname &&targetPath, zypp::ByteCount &&expectedFileSize )
    : _requestDispatcher ( std::move(requestDispatcher) )
    , _mirrorControl( std::move(mirrors) )
    , _url( std::move(file) )
    , _targetPath( std::move(targetPath) )
    , _expectedFileSize( std::move(expectedFileSize) )
  , _parent( &parent )
  { }

  DownloadPrivate::~DownloadPrivate()
  {
    if ( _mirrorControlReadyConn )
      _mirrorControlReadyConn.disconnect();
  }

  void DownloadPrivate::Request::connectSignals(DownloadPrivate &dl)
  {
    _sigStartedConn  = sigStarted().connect ( sigc::mem_fun( dl, &DownloadPrivate::onRequestStarted) );
    _sigProgressConn = sigProgress().connect( sigc::mem_fun( dl, &DownloadPrivate::onRequestProgress) );
    _sigFinishedConn = sigFinished().connect( sigc::mem_fun( dl, &DownloadPrivate::onRequestFinished) );
  }

  void DownloadPrivate::Request::disconnectSignals()
  {
    _sigStartedConn.disconnect();
    _sigProgressConn.disconnect();
    _sigFinishedConn.disconnect();
  }

  void DownloadPrivate::start()
  {
    if ( _state == Download::Initializing ||
         _state == Download::Running ||
         _state == Download::RunningMulti )
      return;

    //reset state variables
    _isMultiDownload = false;
    _downloadedMultiByteCount = 0;
    _mirrors.clear();
    _mirrorControlReadyConn.disconnect();
    _blockList    = zypp::media::MediaBlockList ();
    _blockIter    = 0;
    _errorString  = std::string();
    _requestError = NetworkRequestError();

    setState( Download::Initializing );

    _requestError = safeFillSettingsFromURL( _url , _transferSettings );
    if ( _requestError.type() != NetworkRequestError::NoError ) {
      setFailed( "Failed to read settings from URL " );
      return;
    }

    std::shared_ptr<Request> initialRequest = std::make_shared<Request>( internal::clearQueryString(_url), _targetPath );

    initialRequest->_originalUrl = _url;
    initialRequest->transferSettings() = _transferSettings;

    if ( _isMultiPartEnabled ) {
      if ( !_checkExistsOnly )
        initialRequest->transferSettings().addHeader("Accept: */*, application/metalink+xml, application/metalink4+xml");
      else
        WAR << "Ignoring enabled multi part option for check only download of : " << internal::clearQueryString(_url) << std::endl;
    }

    if ( _checkExistsOnly )
      initialRequest->setOptions( initialRequest->options() | NetworkRequest::HeadRequest );

    addNewRequest( initialRequest );
  }

  void DownloadPrivate::setState(Download::State newState)
  {
    if ( _state == newState )
      return;
    _state = newState;
    _sigStateChanged.emit( *z_func(), newState );
  }

  void DownloadPrivate::onRequestStarted( NetworkRequest & )
  {
    if ( _state == Download::Initializing )
      _sigStarted.emit( *z_func() );
  }

  void DownloadPrivate::onRequestProgress( NetworkRequest &req, off_t dltotal, off_t dlnow, off_t , off_t  )
  {
    //we are not sure yet if we are downloading a Metalink, for now just send alive messages
    if ( _state == Download::Initializing ) {
      if ( !_isMultiPartEnabled ) {
        setState( Download::Running );
      } else {
        if ( !_isMultiDownload ) {
          std::string cType = req.contentType();
          if ( cType.find("application/metalink+xml") == 0 || cType.find("application/metalink4+xml") == 0 )
            _isMultiDownload = true;
        }

        if ( !_isMultiDownload && dlnow < 256 ) {
          // can't tell yet, ...
          return _sigAlive.emit( *z_func(), dlnow );
        }

        if ( !_isMultiDownload )
        {
          _isMultiDownload = looks_like_metalink_data( req.peekData( 0, 256 ) );
        }

        if ( _isMultiDownload )
        {
          // this is a metalink file change the expected filesize
          if ( zypp::ByteCount( 2, zypp::ByteCount::MB) < static_cast<zypp::ByteCount::SizeType>( dlnow ) ) {
            _requestDispatcher->cancel( req, NetworkRequestErrorPrivate::customError( NetworkRequestError::ExceededMaxLen ) );
            return;
          }
          return _sigAlive.emit( *z_func(), dlnow );
        }

        //if we reach here we have a normal file ( or a multi download with more than 256 byte of comment in the beginning )
        setState( Download::Running );
      }
    }

    if ( _state == Download::Running ) {
      if ( _expectedFileSize > 0 && _expectedFileSize < dlnow ) {
        _requestDispatcher->cancel( req, NetworkRequestErrorPrivate::customError( NetworkRequestError::ExceededMaxLen ) );
        return;
      }
      return _sigProgress.emit( *z_func(), dltotal, dlnow );

    } else if ( _state == Download::RunningMulti ) {
      off_t dlnowMulti = _downloadedMultiByteCount;
      for( const auto &req : _runningRequests ) {
        dlnowMulti += req->downloadedByteCount();
      }
      _sigProgress.emit( *z_func(), _blockList.getFilesize(), dlnowMulti );
    }
  }

  void DownloadPrivate::onRequestFinished( NetworkRequest &req, const zyppng::NetworkRequestError &err )
  {

    auto it = std::find_if( _runningRequests.begin(), _runningRequests.end(), [ &req ]( const std::shared_ptr<Request> &r ) {
      return ( r.get() == &req );
    });
    if ( it == _runningRequests.end() )
      return;

    auto reqLocked = *it;

    //remove from running
    _runningRequests.erase( it );

    if ( reqLocked->_myMirror )
      reqLocked->_myMirror->finishTransfer( !err.isError() );

    if ( err.isError() ) {
      return handleRequestError( reqLocked, err );
    }

    if ( _state == Download::Initializing || _state == Download::Running ) {

      if ( _isMultiPartEnabled && !_isMultiDownload )
        _isMultiDownload = looks_like_metalink_file( req.targetFilePath() );
      if ( !_isMultiDownload ) {
        setFinished();
        return;
      }

      //if we get to here we have a multipart request
      return upgradeToMultipart( req );

    } else if ( _state == Download::RunningMulti ) {
      _downloadedMultiByteCount += req.downloadedByteCount();

      DBG << "Request finished "<<std::endl;
      std::for_each( reqLocked->_myBlocks.begin(), reqLocked->_myBlocks.end(), []( const auto &b ){ DBG << "-> Block " << b._block << " finished." << std::endl; } );

      auto restartReqWithBlock = [ this ]( std::shared_ptr<Request> &req, std::vector<Block> &&blocks ) {
        DBG << "Reusing Request to download blocks:"<<std::endl;
        req->_myBlocks = std::move( blocks );
        addBlockRanges( req );

        //this is not a new request, only add to queues but do not connect signals again
        addNewRequest( req, false );
      };

      //check if we already have enqueued all blocks if not reuse the request
      if ( _blockIter  < _blockList.numBlocks() ) {
        DBG << "Reusing to download blocks: "<<std::endl;
        restartReqWithBlock( reqLocked, getNextBlocks( reqLocked->url().getScheme() ) );
        return;

      } else {
        //if we have failed blocks, try to download them with this mirror
        if ( !_failedBlocks.empty() ) {

          auto fblks = getNextFailedBlocks( reqLocked->url().getScheme() );
          DBG << "Reusing to download failed blocks: "<<std::endl;
          restartReqWithBlock( reqLocked, std::move(fblks) );
          return;
        }

        //feed the working URL back into the mirrors in case there are still running requests that might fail
        _mirrors.push_back( reqLocked->_originalUrl );
      }
    }

    // make sure downloads are running, at this p
    ensureDownloadsRunning();
  }

  void DownloadPrivate::handleRequestError( std::shared_ptr<Request> req, const zyppng::NetworkRequestError &err )
  {
    bool retry = false;

    //Handle the auth errors explicitely, we need to give the user a way to put in new credentials
    //if we get valid new credentials we can retry the request
    if ( err.type() == NetworkRequestError::Unauthorized || err.type() == NetworkRequestError::AuthFailed ) {

      zypp::media::CredentialManager cm( zypp::media::CredManagerOptions( zypp::ZConfig::instance().repoManagerRoot()) );
      auto authDataPtr = cm.getCred( req->url() );

      // get stored credentials
      NetworkAuthData_Ptr cmcred( authDataPtr ? new NetworkAuthData( *authDataPtr ) : new NetworkAuthData() );
      TransferSettings &ts = req->transferSettings();

      // We got credentials from store, _triedCredFromStore makes sure we just try the auth from store once
      if ( cmcred && !req->_triedCredFromStore ) {
        DBG << "got stored credentials:" << std::endl << *cmcred << std::endl;
        ts.setUsername( cmcred->username() );
        ts.setPassword( cmcred->password() );
        retry = true;
        req->_triedCredFromStore = true;
      } else {

        //we did not get credentials from the store, emit a signal that allows
        //setting new auth data

        NetworkAuthData credFromUser;
        credFromUser.setUrl( req->url() );

        //in case we got a auth hint from the server the error object will contain it
        auto authHintIt = err.extraInfo().find("authHint");
        std::string authHint;

        if ( authHintIt != err.extraInfo().end() ){
          try {
            authHint = boost::any_cast<std::string>( authHintIt->second );
          } catch ( const boost::bad_any_cast &) { }
        }

        //preset from store if we found something
        if ( cmcred && !cmcred->username().empty() )
          credFromUser.setUsername( cmcred->username() );

        _sigAuthRequired.emit( *z_func(), credFromUser, authHint );
        if ( credFromUser.valid() ) {
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

          retry = true;
        }
      }
    } else if ( _state == Download::RunningMulti ) {

      //if a error happens during a multi download we try to use another mirror to download the failed block
      DBG << "Request failed " << req->extendedErrorString() << std::endl;
      auto failed = req->failedRanges();

      NetworkRequestError dummyErr;

      const auto fRanges = req->failedRanges();
      std::vector<Block> blocks;

      try {
        std::transform( fRanges.begin(), fRanges.end(), std::back_inserter(_failedBlocks), [ &req ]( const auto &r ){
          auto ind = std::find_if( req->_myBlocks.begin(), req->_myBlocks.end(), [ &r ](const auto &elem){ return elem._block == std::any_cast<size_t>(r.userData); } );

          if ( ind == req->_myBlocks.end() ){
            throw zypp::Exception( "User data did not point to a valid block in the request set, this is a bug" );
          }

          Block b = *ind;
          b._failedWithErr = req->error();
          DBG << "Adding failed block to failed blocklist: " << b._block << " " << r.start << " " << r.len << " (" << req->error().toString() << ")" << std::endl;
          return b;
        });

        //try to init a new multi request, if we have leftover mirrors we get a valid one
        auto newReq = initMultiRequest( dummyErr, true );
        if ( newReq ) {
          addNewRequest( newReq );
          return;
        }
        // we got no mirror this time, but since we still have running requests there is hope this will be finished later
        if ( _runningRequests.size() && _failedBlocks.size() )
          return;

      } catch ( const zypp::Exception &ex ) {
        //we just log the exception and fall back to a normal download
        WAR << "Multipart download failed: " << ex.asString() << std::endl;
      }
    }

    //if rety is true we just enqueue the request again, usually this means authentication was updated
    if ( retry ) {
      //make sure this request will run asap
      req->setPriority( NetworkRequest::High );

      //this is not a new request, only add to queues but do not connect signals again
      addNewRequest( req, false );
      return;
    }

    //we do not have more mirrors left we can try or we do not have a multi download, abort
    while( _runningRequests.size() ) {
      auto req = _runningRequests.back();
      req->disconnectSignals();
      _runningRequests.pop_back();
      _requestDispatcher->cancel( *req, err );
      if ( req->_myMirror )
        req->_myMirror->cancelTransfer();
    }

    //not all hope is lost, maybe a normal download can work out?
    if ( _state == Download::RunningMulti ) {
      //fall back to normal download
      DBG << "Falling back to download from initial URL." << std::endl;
      _isMultiDownload = false;
      _isMultiPartEnabled = false;
      setState( Download::Running );

      auto req = std::make_shared<Request>( internal::clearQueryString(_url), _targetPath ) ;
      req->transferSettings() = _transferSettings;
      addNewRequest( req );
      return;
    }

    _requestError = err;
    setFailed( "Download failed ");
  }


  void DownloadPrivate::upgradeToMultipart( NetworkRequest &req )
  {
    DBG << " Upgrading request for URL: "<< req.url() << " to multipart download " << std::endl;

    //we have a metalink download, lets parse it and see what we got
    _mirrors.clear();

    std::vector<zypp::media::MetalinkMirror> mirrs;

    try {
      zypp::media::MetaLinkParser parser;
      parser.parse( req.targetFilePath() );

      _blockList = parser.getBlockList();

      //migrate some settings from the base url to the mirror
      mirrs = parser.getMirrors();
      for ( auto urliter = mirrs.begin(); urliter != mirrs.end(); ++urliter ) {
        try {
          const std::string scheme = urliter->url.getScheme();
          if (scheme == "http" || scheme == "https" || scheme == "ftp" || scheme == "tftp") {
            if ( !_requestDispatcher->supportsProtocol( urliter->url )) {
              urliter = mirrs.erase( urliter );
              continue;
            }
            urliter->url = internal::propagateQueryParams( urliter->url, _url );
            _mirrors.push_back( urliter->url );
          }
        }
        catch (...) {  }
      }

      if ( mirrs.empty() ) {
        mirrs.push_back( { 0, -1, _url } );
        _mirrors.push_back( _url );
      }

    } catch ( const zypp::Exception &ex ) {
      setFailed( zypp::str::Format("Failed to parse metalink information.(%1%)" ) % ex.asUserString() );
      return;
    }

    if ( mirrs.size() == 0 ) {
      setFailed( zypp::str::Format("Invalid metalink information.( No mirrors in metalink file)" ) );
      return;
    }

    if ( !_blockList.haveBlocks() ) {

      //if we have no filesize we can not generate a blocklist, we need to fall back to normal download
      if ( !_blockList.haveFilesize() ) {
        DBG << "No blocklist and no filesize, falling back to normal download for URL " << _url << std::endl;

        //fall back to normal download but use a mirror from the mirror list
        //otherwise we get HTTPS to HTTP redirect errors
        _isMultiDownload = false;
        _isMultiPartEnabled = false;

        Url url;
        TransferSettings set;
        NetworkRequestError dummyErr;
        MirrorControl::MirrorHandle mirror = findNextMirror( url, set, dummyErr );
        if ( !mirror ) {
          url = _url;
          set = _transferSettings;
        }

        auto req = std::make_shared<Request>( internal::clearQueryString(url), _targetPath ) ;
        req->_myMirror = mirror;

        if ( _blockList.haveFileChecksum() ) {
          std::shared_ptr<zypp::Digest> fileDigest = std::make_shared<zypp::Digest>();
          if ( _blockList.createFileDigest( *fileDigest ) ) {
            // to run the checksum for the full file we need to request one big range with open end
            req->addRequestRange( 0, 0, fileDigest, _blockList.getFileChecksum() );
          }
        }

        req->transferSettings() = set;
        addNewRequest( req );
        return;

      } else {
        //we generate a blocklist on the fly based on the filesize

        DBG << "Generate blocklist, since there was none in the metalink file." << _url  << std::endl;

        off_t currOff = 0;
        off_t filesize = _blockList.getFilesize();
        while ( currOff <  filesize )  {

          auto blksize = filesize - currOff ;
          if ( blksize > _preferredChunkSize )
            blksize = _preferredChunkSize;

          _blockList.addBlock( currOff, blksize );
          currOff += blksize;
        }

        XXX << "Generated blocklist: " << std::endl << _blockList << std::endl << " End blocklist " << std::endl;
      }
    }

    //remove the metalink file
    zypp::filesystem::unlink( _targetPath );

    if ( !_deltaFilePath.empty() ) {
      zypp::PathInfo dFileInfo ( _deltaFilePath );
      if ( dFileInfo.isFile() && dFileInfo.isR() ) {
        FILE *f = fopen( _targetPath.asString().c_str(), "w+b" );
        if ( !f ) {
          setFailed( zypp::str::Format("Failed to open target file.(errno %1%)" ) % errno );
          return;
        }

        try {
          _blockList.reuseBlocks ( f, _deltaFilePath.asString() );
        } catch ( ... ) { }

        fclose( f );
      }
    }

    setState( Download::PrepareMulti );
    _mirrorControlReadyConn = _mirrorControl->sigAllMirrorsReady().connect( track_obj( [ this ](){
      _mirrorControlReadyConn.disconnect();
      mirrorsReady();
    }, this ) );

    // this will emit a mirrorsReady signal once all connection tests have been done
    _mirrorControl->registerMirrors( mirrs );
  }

  void DownloadPrivate::mirrorsReady()
  {
    setState( Download::RunningMulti );
    _blockIter = 0;
    ensureDownloadsRunning();
  }

  void DownloadPrivate::ensureDownloadsRunning()
  {
    //check if there is still work to do
    if ( _runningRequests.size() < 10 ) {

      NetworkRequestError lastErr = _requestError;

      //we try to allocate as many requests as possible but stop if we cannot find a valid mirror for one
      while( _runningRequests.size() < 10 ) {
        std::shared_ptr<Request> req = initMultiRequest( lastErr );
        if (!req ) {
          break;
        }
        addNewRequest( req );
      }

      while ( lastErr.type() == NetworkRequestError::NoError && _failedBlocks.size() ) {

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
        _requestError = lastErr;
        setFailed( "Unable to use mirror" );
      }
    }

    if ( _runningRequests.empty() ) {

      if ( _failedBlocks.size() || ( _blockIter < _blockList.numBlocks() )) {
        setFailed( "Unable to download all blocks." );
        return;
      }

      if ( _state == Download::RunningMulti && _blockList.haveFileChecksum() ) {
        //TODO move this into a external application so we do not need to block on it
        //need to check file digest
        zypp::Digest dig;
        _blockList.createFileDigest( dig );

        std::ifstream istrm( _targetPath.asString(), std::ios::binary);
        if ( !istrm.is_open() ) {
          setFailed( "Failed to verify file digest (Could not open target file)." );
          return;
        }
        if ( !dig.update( istrm ) ) {
          setFailed( "Failed to verify file digest (Could not read target file)." );
          return;
        }
        if ( !_blockList.verifyFileDigest( dig ) ) {
          setFailed( "Failed to verify file digest (Checksum did not match)." );
          return;
        }
      }
      //TODO implement digest check for non multi downloads
      setFinished();
    }
  }

  void DownloadPrivate::addNewRequest(std::shared_ptr<Request> req , const bool connectSignals)
  {
    if ( connectSignals )
      req->connectSignals( *this );

    _runningRequests.push_back( req );
    _requestDispatcher->enqueue( req );

    if ( req->_myMirror )
      req->_myMirror->startTransfer();
  }

  std::shared_ptr<DownloadPrivate::Request> DownloadPrivate::initMultiRequest( NetworkRequestError &err, bool useFailed )
  {
    Url myUrl;
    TransferSettings settings;
    if ( !findNextMirror( myUrl, settings, err ) )
      return nullptr;

    auto blocks = useFailed ?  getNextFailedBlocks( myUrl.getScheme() ) : getNextBlocks( myUrl.getScheme() );
    if ( !blocks.size() ) {
      _mirrors.push_back( myUrl );
      return nullptr;
    }

    std::shared_ptr<Request> req = std::make_shared<Request>( internal::clearQueryString( myUrl ), _targetPath, NetworkRequest::WriteShared );
    req->_originalUrl = myUrl;
    req->setPriority( NetworkRequest::High );
    req->transferSettings() = settings;
    req->_myBlocks = std::move(blocks);

    DBG << "Creating Request to download blocks:"<<std::endl;
    addBlockRanges( req );

    return req;
  }

  /**
   * Just initialize the requests ranges from the internal blocklist
   */
  void DownloadPrivate::addBlockRanges( std::shared_ptr<DownloadPrivate::Request> req ) const
  {
    req->resetRequestRanges();
    for ( const auto &block : req->_myBlocks ) {
      zypp::media::MediaBlock blk = _blockList.getBlock( block._block );
      std::shared_ptr<zypp::Digest> dig;
      std::vector<unsigned char> checksumVec;
      if ( _blockList.haveChecksum( block._block ) ) {
        dig = std::make_shared<zypp::Digest>();
        _blockList.createDigest( *dig );
        checksumVec = _blockList.getChecksum( block._block);
        DBG << "Starting block " << block._block << " with checksum " << zypp::Digest::digestVectorToString( checksumVec ) << "." << std::endl;
      } else {
        DBG << "Starting block " << block._block << " without checksum." << std::endl;
      }
      req->addRequestRange( blk.off, blk.size, dig, checksumVec, block._block );
    }
  }

  MirrorControl::MirrorHandle DownloadPrivate::findNextMirror( Url &url, TransferSettings &set, NetworkRequestError &err )
  {

    Url myUrl;
    TransferSettings settings;
    MirrorControl::MirrorHandle mirror;

    while ( !mirror ) {

      const auto nextBest = _mirrorControl->pickBestMirror( _mirrors );
      if ( !nextBest.second )
        return nullptr;

      myUrl = *nextBest.first;
      _mirrors.erase( nextBest.first );

      settings = _transferSettings;
      //if this is a different host than the initial request, we reset username/password
      if ( myUrl.getHost() != _url.getHost() ) {
        settings.setUsername( std::string() );
        settings.setPassword( std::string() );
        settings.setAuthType( std::string() );
      }

      err = safeFillSettingsFromURL( myUrl, settings );
      if ( err.type() != NetworkRequestError::NoError ) {
        continue;
      }

      mirror = nextBest.second;
      break;
    }

    if ( !mirror )
      return nullptr;

    url = myUrl;
    set = settings;
    return mirror;
  }

  void DownloadPrivate::setFailed(std::string &&reason)
  {
    _errorString = reason;
    zypp::filesystem::unlink( _targetPath );
    setFinished( false );
  }

  void DownloadPrivate::setFinished(bool success)
  {
    setState( success ? Download::Success : Download::Failed );
    _sigFinished.emit( *z_func() );
  }

  std::vector<DownloadPrivate::Block> DownloadPrivate::getNextBlocks( const std::string &urlScheme )
  {
    std::vector<DownloadPrivate::Block> blocks;
    size_t accumulatedSize = 0;

    bool canDoRandomBlocks = ( zypp::str::hasPrefixCI( urlScheme, "http") );
    std::optional<size_t> lastBlockEnd;
    for (; _blockIter < _blockList.numBlocks() &&  accumulatedSize < static_cast<size_t>( _preferredChunkSize ); _blockIter++ ){
      const auto block = _blockList.getBlock( _blockIter );

      if ( !canDoRandomBlocks && lastBlockEnd ) {
        if ( static_cast<const size_t>(block.off) != (*lastBlockEnd)+1 )
          break;
      }

      lastBlockEnd = block.off + block.size - 1;
      accumulatedSize += block.size;
      blocks.push_back( { _blockIter } );
    }
    return blocks;
  }

  std::vector<DownloadPrivate::Block> DownloadPrivate::getNextFailedBlocks( const std::string &urlScheme )
  {
    // sort the failed requests by block number, this should make sure get them in offset order as well
    std::sort( _failedBlocks.begin(), _failedBlocks.end(), []( const auto &a , const auto &b ){ return a._block < b._block; } );

    bool canDoRandomBlocks = ( zypp::str::hasPrefixCI( urlScheme, "http") );

    std::vector<DownloadPrivate::Block> fblks;
    std::optional<size_t> lastBlockEnd;
    size_t accumulatedSize = 0;
    while ( _failedBlocks.size() ) {

      const auto block = _blockList.getBlock( _failedBlocks.front()._block );

      //we need to check if we have consecutive blocks because only http mirrors support random request ranges
      if ( !canDoRandomBlocks && lastBlockEnd ) {
        if ( static_cast<const size_t>(block.off) != (*lastBlockEnd)+1 )
          break;
      }

      fblks.push_back( std::move( _failedBlocks.front() ));
      _failedBlocks.pop_front();

      fblks.back()._retryCount += 1;

      lastBlockEnd = block.off + block.size - 1;
      accumulatedSize += block.size;

      if ( accumulatedSize >= static_cast<size_t>( _preferredChunkSize ) )
        break;
    }

    return fblks;
  }

  NetworkRequestError DownloadPrivate::safeFillSettingsFromURL( const Url &url, TransferSettings &set)
  {
    auto buildExtraInfo = [this, &url](){
      std::map<std::string, boost::any> extraInfo;
      extraInfo.insert( {"requestUrl", url } );
      extraInfo.insert( {"filepath", _targetPath } );
      return extraInfo;
    };

    NetworkRequestError res;
    try {
      internal::fillSettingsFromUrl( url, set );
      if ( _transferSettings.proxy().empty() )
        internal::fillSettingsSystemProxy( url, set );

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

  Download::Download(zyppng::DownloadPrivate &&prv)
  : Base( prv )
  { }

  Download::~Download()
  {
    cancel();
  }

  Url Download::url() const
  {
    return d_func()->_url;
  }

  zypp::filesystem::Pathname Download::targetPath() const
  {
    return d_func()->_targetPath;
  }

  Download::State Download::state() const
  {
    return d_func()->_state;
  }

  NetworkRequestError Download::lastRequestError() const
  {
    return d_func()->_requestError;
  }

  std::string Download::errorString() const
  {
    Z_D();
    if (! d->_requestError.isError() ) {
      return d->_errorString;
    }

    return ( zypp::str::Format("%1%(%2% %3%)") % d->_errorString % d->_requestError.toString() % d->_requestError.nativeErrorString() );
  }

  TransferSettings &Download::settings()
  {
    return d_func()->_transferSettings;
  }

  void Download::start()
  {
    d_func()->start();
  }

  void Download::cancel()
  {
    Z_D();
    if ( this->state() >= Download::InitialState && this->state() < Download::Success ) {
      //we do not have more mirrors left we can try or we do not have a multi download, abort
      while( d->_runningRequests.size() ) {
        auto req = d->_runningRequests.back();
        req->disconnectSignals();
        d->_runningRequests.pop_back();
        d->_requestDispatcher->cancel( *req, "Download cancelled" );
      }
      d->setFailed( "Download was cancelled explicitely" );
    }
  }

  void Download::setMultiPartHandlingEnabled( bool enable )
  {
    d_func()->_isMultiPartEnabled = enable;
  }

  void Download::setCheckExistsOnly(bool set)
  {
    Z_D();
    if ( d->_checkExistsOnly != set ) {
      d_func()->_checkExistsOnly = set;
      if ( set == true )
        d_func()->_isMultiPartEnabled = false;
    }
  }

  void Download::setDeltaFile( const zypp::filesystem::Pathname &file )
  {
    d_func()->_deltaFilePath = file;
  }

  void Download::setPreferredChunkSize(const zypp::ByteCount &bc)
  {
    d_func()->_preferredChunkSize = bc;
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
    return d_func()->_sigStateChanged;
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
    return d_func()->_sigFinished;
  }

  SignalProxy<void (zyppng::Download &req, zyppng::NetworkAuthData &auth, const std::string &availAuth)> Download::sigAuthRequired()
  {
    return d_func()->_sigAuthRequired;
  }

  DownloaderPrivate::DownloaderPrivate(std::shared_ptr<MirrorControl> mc)
    : _mirrors( std::move(mc) )
  {
    _requestDispatcher = std::make_shared<NetworkRequestDispatcher>( );

    if ( !_mirrors ) {
      auto zyppContext = zypp::getZYpp()->ngContext();
      _mirrors = zyppContext->mirrorControl();
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
      EventDispatcher::unrefLater( *it );
      _runningDownloads.erase( it );
    }

    if ( _runningDownloads.empty() )
      _queueEmpty.emit( *z_func() );
  }

  Downloader::Downloader( )
    : Base ( *new DownloaderPrivate( ) )
  {

  }

  Downloader::Downloader( std::shared_ptr<MirrorControl> mc )
    : Base ( *new DownloaderPrivate( mc ) )
  { }

  Downloader::~Downloader()
  {
    Z_D();
    while ( d->_runningDownloads.size() ) {
      d->_runningDownloads.back()->cancel();
      d->_runningDownloads.pop_back();
    }
  }

  std::shared_ptr<Download> Downloader::downloadFile(zyppng::Url file, zypp::filesystem::Pathname targetPath, zypp::ByteCount expectedFileSize )
  {
    Z_D();
    std::shared_ptr<Download> dl = std::make_shared<Download>( std::move( *new DownloadPrivate( *this, d->_requestDispatcher, d->_mirrors, std::move(file), std::move(targetPath), std::move(expectedFileSize) ) ) );

    d->_runningDownloads.push_back( dl );
    dl->sigFinished().connect( sigc::mem_fun( *d , &DownloaderPrivate::onDownloadFinished ) );

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
