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

  DownloadPrivate::DownloadPrivate(Downloader &parent, std::shared_ptr<NetworkRequestDispatcher> requestDispatcher, Url &&file, zypp::filesystem::Pathname &&targetPath, zypp::ByteCount &&expectedFileSize )
    : _requestDispatcher ( requestDispatcher )
    , _url( std::move(file) )
    , _targetPath( std::move(targetPath) )
    , _expectedFileSize( std::move(expectedFileSize) )
    , _parent( &parent )
  { }

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
    _multiPartMirrors.clear();
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

    if ( err.isError() ) {

      bool retry = false;

      //Handle the auth errors explicitely, we need to give the user a way to put in new credentials
      //if we get valid new credentials we can retry the request
      if ( err.type() == NetworkRequestError::Unauthorized || err.type() == NetworkRequestError::AuthFailed ) {

        zypp::media::CredentialManager cm( zypp::media::CredManagerOptions( zypp::ZConfig::instance().repoManagerRoot()) );
        auto authDataPtr = cm.getCred( req.url() );

        // get stored credentials
        NetworkAuthData_Ptr cmcred( authDataPtr ? new NetworkAuthData( *authDataPtr ) : new NetworkAuthData() );
        TransferSettings &ts = req.transferSettings();

        // We got credentials from store, _triedCredFromStore makes sure we just try the auth from store once
        if ( cmcred && !reqLocked->_triedCredFromStore ) {
          DBG << "got stored credentials:" << std::endl << *cmcred << std::endl;
          ts.setUsername( cmcred->username() );
          ts.setPassword( cmcred->password() );
          retry = true;
          reqLocked->_triedCredFromStore = true;
        } else {

          //we did not get credentials from the store, emit a signal that allows
          //setting new auth data

          NetworkAuthData credFromUser;
          credFromUser.setUrl( req.url() );

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
              req.transferSettings().setAuthType(credFromUser.authTypeAsString());
            }

            cm.addCred( credFromUser );
            cm.save();

            retry = true;
          }
        }
      } else if ( _state == Download::RunningMulti ) {

        //if a error happens during a multi download we try to use another mirror to download the failed block
        DBG << "Request failed " << reqLocked->_myBlock << " " << reqLocked->extendedErrorString() << std::endl;
        NetworkRequestError dummyErr;

        //try to init a new multi request, if we have leftover mirrors we get a valid one
        auto newReq = initMultiRequest( reqLocked->_myBlock, dummyErr );
        if ( newReq ) {
          newReq->_retryCount = reqLocked->_retryCount + 1;
          addNewRequest( newReq );
          return;
        } else {
          //no mirrors left but if we still have running requests, there is hope to finish the block
          if ( !_runningRequests.empty() ) {
            DBG << "Adding to failed blocklist " << reqLocked->_myBlock <<std::endl;
            _failedBlocks.push_back( FailedBlock{ reqLocked->_myBlock, reqLocked->_retryCount, err } );
            return;
          }
        }
      }

      //if rety is true we just enqueue the request again, usually this means authentication was updated
      if ( retry ) {
        //make sure this request will run asap
        reqLocked->setPriority( NetworkRequest::High );

        //this is not a new request, only add to queues but do not connect signals again
        _runningRequests.push_back( reqLocked );
        _requestDispatcher->enqueue( reqLocked );
        return;
      }

      //we do not have more mirrors left we can try or we do not have a multi download, abort
      while( _runningRequests.size() ) {
        auto req = _runningRequests.back();
        req->disconnectSignals();
        _runningRequests.pop_back();
        _requestDispatcher->cancel( *req, err );
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
      return;
    }

    if ( _state == Download::Initializing || _state == Download::Running ) {
      if ( _isMultiPartEnabled && !_isMultiDownload )
        _isMultiDownload = looks_like_metalink_file( req.targetFilePath() );
      if ( !_isMultiDownload ) {
        setFinished();
        return;
      }

      DBG << " Upgrading request for URL: "<< req.url() << " to multipart download " << std::endl;

      //we have a metalink download, lets parse it and see what we got
      _multiPartMirrors.clear();

      try {
        zypp::media::MetaLinkParser parser;
        parser.parse( req.targetFilePath() );

        _blockList = parser.getBlockList();

        //migrate some settings from the base url to the mirror
        std::vector<Url> urllist = parser.getUrls();
        for (std::vector<Url>::iterator urliter = urllist.begin(); urliter != urllist.end(); ++urliter) {
          try {
            std::string scheme = urliter->getScheme();
            if (scheme == "http" || scheme == "https" || scheme == "ftp" || scheme == "tftp") {
              if ( !_requestDispatcher->supportsProtocol( *urliter ))
                continue;
              _multiPartMirrors.push_back(internal::propagateQueryParams(*urliter, _url));
            }
          }
          catch (...) {  }
        }

        if ( _multiPartMirrors.empty() )
          _multiPartMirrors.push_back( _url );

      } catch ( const zypp::Exception &ex ) {
        setFailed( zypp::str::Format("Failed to parse metalink information.(%1%)" ) % ex.asUserString() );
        return;
      }

      if ( _multiPartMirrors.size() == 0 ) {
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
          if ( !findNextMirror( url, set, dummyErr ) ) {
            url = _url;
            set = _transferSettings;
          }

          auto req = std::make_shared<Request>( internal::clearQueryString(url), _targetPath ) ;

          if ( _blockList.haveFileChecksum() ) {
            std::shared_ptr<zypp::Digest> fileDigest = std::make_shared<zypp::Digest>();
            if ( _blockList.createFileDigest( *fileDigest ) ) {
              req->setDigest( fileDigest );
              req->setExpectedChecksum( _blockList.getFileChecksum() );
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

            size_t blksize = static_cast<size_t>( filesize - currOff );
            if ( blksize > BLKSIZE)
              blksize = BLKSIZE;

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

      setState( Download::RunningMulti );
      _blockIter = 0;

    } else if ( _state == Download::RunningMulti ) {
      _downloadedMultiByteCount += req.downloadedByteCount();

      DBG << "Request finished " << reqLocked->_myBlock <<std::endl;

      auto restartReqWithBlock = [ this ]( std::shared_ptr<Request> &req, size_t block, int retryCount ) {
        zypp::media::MediaBlock blk = _blockList.getBlock( block );
        req->_myBlock = block;
        req->_retryCount = retryCount;
        req->setRequestRange( blk.off, static_cast<off_t>( blk.size ) );
        req->setExpectedChecksum( _blockList.getChecksum( block ) );

        //this is not a new request, only add to queues but do not connect signals again
        _runningRequests.push_back( req );
        _requestDispatcher->enqueue( req );
      };

      //check if we already have enqueued all blocks if not reuse the request
      if ( _blockIter  < _blockList.numBlocks() ) {

        DBG << "Reusing to download block: " << _blockIter <<std::endl;
        restartReqWithBlock( reqLocked, _blockIter, 0 );
        _blockIter++;
        return;

      } else {
        //if we have failed blocks, try to download them with this mirror
        if ( !_failedBlocks.empty() ) {

          FailedBlock blk = std::move( _failedBlocks.front() );
          _failedBlocks.pop_front();

          DBG << "Reusing to download failed block: " << blk._block <<std::endl;

          restartReqWithBlock( reqLocked, blk._block, blk._retryCount+1 );
          return;
        }

        //feed the working URL back into the mirrors in case there are still running requests that might fail
        _multiPartMirrors.push_front( reqLocked->_originalUrl );
      }
    }

    //check if there is still work to do
    if ( _runningRequests.size() < 10 ) {

      NetworkRequestError lastErr = _requestError;

      //we try to allocate as many requests as possible but stop if we cannot find a valid mirror for one
      for ( ; _blockIter < _blockList.numBlocks(); _blockIter++ ){

        if ( _runningRequests.size() >= 10 )
          break;

        std::shared_ptr<Request> req = initMultiRequest( _blockIter, lastErr );
        if ( !req )
          break;

        addNewRequest( req );
      }

      while ( _failedBlocks.size() ) {

        if ( _runningRequests.size() >= 10 )
          break;

        FailedBlock blk = std::move( _failedBlocks.front() );
        _failedBlocks.pop_front();

        auto req = initMultiRequest( blk._block, lastErr );
        if ( !req )
          break;

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

  void DownloadPrivate::addNewRequest( std::shared_ptr<Request> req )
  {
    auto slot = _sigStarted.slots().front();
    req->connectSignals( *this );
    _runningRequests.push_back( req );
    _requestDispatcher->enqueue( req );
  }

  std::shared_ptr<DownloadPrivate::Request> DownloadPrivate::initMultiRequest( size_t block, NetworkRequestError &err )
  {
    zypp::media::MediaBlock blk = _blockList.getBlock( block );

    Url myUrl;
    TransferSettings settings;
    if ( !findNextMirror( myUrl, settings, err ) )
      return nullptr;

    DBG << "Starting block " << block << std::endl;

    std::shared_ptr<Request> req = std::make_shared<Request>( internal::clearQueryString( myUrl ), _targetPath, blk.off, blk.size, NetworkRequest::WriteShared );
    req->_originalUrl = myUrl;
    req->_myBlock = block;
    req->setPriority( NetworkRequest::High );
    req->transferSettings() = settings;

    if ( _blockList.haveChecksum( block ) ) {
      std::shared_ptr<zypp::Digest> dig = std::make_shared<zypp::Digest>();
      _blockList.createDigest( *dig );
      req->setDigest( dig );
      std::vector<unsigned char> checksumVec = _blockList.getChecksum( block );
      req->setExpectedChecksum( checksumVec );
      DBG << "Starting block  " << block << " with checksum " << zypp::Digest::digestVectorToString( checksumVec ) << std::endl;
    } else {
      DBG << "Block " << block << " has no checksum." << std::endl;
    }
    return req;
  }

  bool DownloadPrivate::findNextMirror( Url &url, TransferSettings &set, NetworkRequestError &err )
  {
    Url myUrl;
    bool foundMirror = false;
    TransferSettings settings;
    while ( _multiPartMirrors.size() ) {
      myUrl = _multiPartMirrors.front();
      _multiPartMirrors.pop_front();

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

      foundMirror = true;
      break;
    }

    if ( !foundMirror )
      return false;

    url = myUrl;
    set = settings;
    return true;
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
    if ( this->state() > Download::InitialState && this->state() < Download::Success )
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
    //we do not have more mirrors left we can try or we do not have a multi download, abort
    while( d->_runningRequests.size() ) {
      auto req = d->_runningRequests.back();
      req->disconnectSignals();
      d->_runningRequests.pop_back();
      d->_requestDispatcher->cancel( *req, "Download cancelled" );
    }
    d->setFailed( "Download was cancelled explicitely" );
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

  DownloaderPrivate::DownloaderPrivate( )
  {
    _requestDispatcher = std::make_shared<NetworkRequestDispatcher>(  );
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
    std::shared_ptr<Download> dl = std::make_shared<Download>( std::move( *new DownloadPrivate( *this, d->_requestDispatcher, std::move(file), std::move(targetPath), std::move(expectedFileSize) ) ) );

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
