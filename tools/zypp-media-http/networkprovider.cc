/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include "networkprovider.h"

#include <zypp-curl/ng/network/Downloader>
#include <zypp-curl/ng/network/NetworkRequestDispatcher>
#include <zypp-curl/ng/network/DownloadSpec>
#include <zypp-curl/parser/MetaLinkParser>
#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/CheckSum.h>
#include <zypp-media/ng/private/providedbg_p.h>


#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "NetworkProvider"


NetworkProvideItem::NetworkProvideItem(NetworkProvider &parent, zyppng::ProvideMessage &&spec)
  : zyppng::worker::ProvideWorkerItem( std::move(spec) )
  , _parent(parent)
{}

NetworkProvideItem::~NetworkProvideItem()
{
  clearConnections();
  if ( _dl ) {
    _dl->cancel();
  }
}

void NetworkProvideItem::startDownload( std::shared_ptr<zyppng::Download> &&dl )
{
  if ( _state == ProvideWorkerItem::Running || _dl )
    throw std::runtime_error("Can not start a already running Download");

  MIL_PRV << "Starting download of : " << dl->spec().url() << "with target path: " << dl->spec().targetPath() << std::endl;

  _connections.clear ();
  // we set the state to running immediately but only send the message to controller once the item actually started
  _state = ProvideWorkerItem::Running;
  _dl = std::move(dl);
  _connections.emplace_back( connect( *_dl, &zyppng::Download::sigStarted, *this, &NetworkProvideItem::onStarted ) );
  _connections.emplace_back( connect( *_dl, &zyppng::Download::sigFinished, *this, &NetworkProvideItem::onFinished ) );
  _connections.emplace_back( connect( *_dl, &zyppng::Download::sigAuthRequired, *this, &NetworkProvideItem::onAuthRequired ) );

  _dl->setStopOnMetalink( true );
  _dl->start();
}

void NetworkProvideItem::cancelDownload()
{
  _state = zyppng::worker::ProvideWorkerItem::Finished;
  if ( _dl ) {
    _dl->cancel();
    clearConnections();
    _dl.reset();
  }
}

void NetworkProvideItem::clearConnections()
{
  for ( auto c : _connections)
    c.disconnect();
  _connections.clear();
}

void NetworkProvideItem::onStarted( zyppng::Download & )
{
  _parent.itemStarted( shared_this<NetworkProvideItem>() );
}

void NetworkProvideItem::onFinished( zyppng::Download & )
{
  _parent.itemFinished( shared_this<NetworkProvideItem>() );
}

void NetworkProvideItem::onAuthRequired( zyppng::Download &, zyppng::NetworkAuthData &auth, const std::string &availAuth )
{
  _parent.itemAuthRequired( shared_this<NetworkProvideItem>(), auth, availAuth );
}


NetworkProvider::NetworkProvider( std::string_view workerName )
  : zyppng::worker::ProvideWorker( workerName )
  , _dlManager( std::make_shared<zyppng::Downloader>() )
{
  // we only want to hear about new provides
  setProvNotificationMode( ProvideWorker::ONLY_NEW_PROVIDES );
}

zyppng::expected<zyppng::worker::WorkerCaps> NetworkProvider::initialize( const zyppng::worker::Configuration &conf )
{
  const auto &values = conf.values();
  const auto iEnd = values.end();
  if ( const auto &i = values.find( std::string(zyppng::AGENT_STRING_CONF) ); i != iEnd ) {
    const auto &val = i->second;
    MIL << "Setting agent string to: " << val << std::endl;
    _dlManager->requestDispatcher()->setAgentString( val );
  }
  if ( const auto &i = values.find( std::string(zyppng::DISTRO_FLAV_CONF) ); i != iEnd ) {
    const auto &val = i->second;
    MIL << "Setting distro flavor header to: " << val << std::endl;
    _dlManager->requestDispatcher()->setHostSpecificHeader("download.opensuse.org", "X-ZYpp-DistributionFlavor", val );
  }
  if ( const auto &i = values.find( std::string(zyppng::ANON_ID_CONF) ); i != iEnd ) {
    const auto &val = i->second;
    MIL << "Got anonymous ID setting from controller" << std::endl;
    _dlManager->requestDispatcher()->setHostSpecificHeader("download.opensuse.org", "X-ZYpp-AnonymousId", val );
  }
  if ( const auto &i = values.find( std::string(zyppng::ATTACH_POINT) ); i != iEnd ) {
    const auto &val = i->second;
    MIL << "Got attachpoint from controller: " << val << std::endl;
    _attachPoint = val;
  } else {
    return zyppng::expected<zyppng::worker::WorkerCaps>::error(ZYPP_EXCPT_PTR( zypp::Exception("Attach point required to work.") ));
  }

  zyppng::worker::WorkerCaps caps;
  caps.set_worker_type ( zyppng::worker::WorkerCaps::Downloading );
  caps.set_cfg_flags(
    zyppng::worker::WorkerCaps::Flags (
      zyppng::worker::WorkerCaps::Pipeline
      | zyppng::worker::WorkerCaps::ZyppLogFormat
      )
    );

  return zyppng::expected<zyppng::worker::WorkerCaps>::success(caps);
}

void NetworkProvider::provide()
{
  auto &queue = requestQueue();

  if ( !queue.size() )
    return;

  const auto now = std::chrono::steady_clock::now();
  auto &io = controlIO();

  while ( io.readFdOpen() ) {
    // find next pending item
    auto i = std::find_if( queue.begin(), queue.end(), [&now]( const zyppng::worker::ProvideWorkerItemRef &req ){
      NetworkProvideItemRef nReq = std::static_pointer_cast<NetworkProvideItem>(req);
      return nReq && nReq->_state == zyppng::worker::ProvideWorkerItem::Pending && nReq->_scheduleAfter < now;
    });

    // none left, bail out
    if ( i == queue.end () )
      break;

    auto req = std::static_pointer_cast<NetworkProvideItem>( *i );
    if ( req->_state != zyppng::worker::ProvideWorkerItem::Pending )
      return;

    MIL_PRV << "About to schedule: " << req->_spec.code() << " with ID: "<< req->_spec.requestId() << std::endl;

    // here we only receive request codes, we only support Provide messages, all others are rejected
    // Cancel is never to be received here
    if ( req->_spec.code() != zyppng::ProvideMessage::Code::Provide ) {
      req->_state = zyppng::worker::ProvideWorkerItem::Finished;
      provideFailed( req->_spec.requestId()
        , zyppng::ProvideMessage::Code::BadRequest
        , "Request type not implemented"
        , false
        , {} );

      queue.erase(i);
      continue;
    }

    zypp::Url url;
    const auto &urlVal = req->_spec.value( zyppng::ProvideMsgFields::Url );
    try {
      url = zypp::Url( urlVal.asString() );
    }  catch ( const zypp::Exception &excp ) {
      ZYPP_CAUGHT(excp);

      std::string err = zypp::str::Str() << "Invalid URL in request: " << urlVal.asString();
      ERR << err << std::endl;

      req->_state = zyppng::worker::ProvideWorkerItem::Finished;
      provideFailed( req->_spec.requestId()
        , zyppng::ProvideMessage::Code::BadRequest
        , err
        , false
        , {} );

      queue.erase(i);
      continue;
    }

    const auto &fPath = zypp::Pathname( url.getPathName() );

    /*
     * We download into the staging directory first, using the md5sum of the full path as filename.
     * Once the download is finished its transferred into the cache directory. This way we can clearly
     * distinguish between cancelled downloads and downloads we have completely finished.
     */
    zypp::Pathname localPath   = zypp::Pathname( _attachPoint ) / "cache" / fPath;
    zypp::Pathname stagingPath = ( zypp::Pathname( _attachPoint ) / "staging" / zypp::CheckSum::md5FromString( fPath.asString() ).asString() ).extend(".part");

    MIL_PRV << "Providing " << url << " to local: " << localPath << " staging: " << stagingPath << std::endl;

    // Cache hit?
    zypp::PathInfo cacheFile(localPath);
    if ( cacheFile.isExist () ) {

      // could be a directory
      if ( !cacheFile.isFile() ) {
        MIL_PRV << "Path " << url << " exists in Cache but its not a file!" << std::endl;
        req->_state = zyppng::worker::ProvideWorkerItem::Finished;
        provideFailed( req->_spec.requestId()
          , zyppng::ProvideMessage::Code::NotAFile
          , "Not a file"
          , false
          , {} );

        queue.erase(i);
        continue;
      }

      MIL_PRV << "Providing " << url << "Cache HIT" << std::endl;
      req->_state = zyppng::worker::ProvideWorkerItem::Finished;
      provideSuccess ( req->_spec.requestId(), true, localPath );
      queue.erase(i);
      continue;
    }

    if ( zypp::PathInfo(stagingPath).isExist () ) {
      // check if there is another request running with this staging file name
      auto runningIt = std::find_if( queue.begin (), queue.end(), [&]( const zyppng::worker::ProvideWorkerItemRef &queueItem ){
        if ( req == queueItem || queueItem->_state != zyppng::worker::ProvideWorkerItem::Running )
          return false;
        return ( static_cast<NetworkProvideItem*>( queueItem.get() )->_stagingFileName == stagingPath );
      });

      // we schedule that later again which should result in a immediate cache hit
      if ( runningIt != queue.end() ) {
        MIL << "Found a existing request that results in the same staging file, postponing the request for later" << std::endl;
        req->_scheduleAfter = now;
        continue;
      }

      MIL<< "Removing broken staging file" << stagingPath << std::endl;

      // here this is most likely a broken download
      zypp::filesystem::unlink( stagingPath );
    }

    auto errCode = zypp::filesystem::assert_dir( localPath.dirname() );
    if( errCode ) {
      std::string err = zypp::str::Str() << "assert_dir " << localPath.dirname() << " failed";
      DBG << err << std::endl;

      req->_state = zyppng::worker::ProvideWorkerItem::Finished;
      provideFailed( req->_spec.requestId()
        , zyppng::ProvideMessage::Code::InternalError
        , err
        , false
        , {} );
      queue.erase(i);
      continue;
    }

    errCode = zypp::filesystem::assert_dir( stagingPath.dirname() );
    if( errCode ) {
      std::string err = zypp::str::Str() << "assert_dir " << stagingPath.dirname() << " failed";
      DBG << err << std::endl;

      req->_state = zyppng::worker::ProvideWorkerItem::Finished;
      provideFailed( req->_spec.requestId()
        , zyppng::ProvideMessage::Code::InternalError
        , err
        , false
        , {} );
      queue.erase(i);
      continue;
    }

    bool doMetalink = true;
    const auto &metalinkVal = req->_spec.value( zyppng::NETWORK_METALINK_ENABLED );
    if ( metalinkVal.valid() && metalinkVal.isBool () ) {
      doMetalink = metalinkVal.asBool();
      MIL << "Explicity setting metalink " << ( doMetalink ? "enabled" : "disabled" ) << std::endl;
    }

    req->_targetFileName  = localPath;
    req->_stagingFileName = stagingPath;

    const auto &expFilesize = req->_spec.value( zyppng::ProvideMsgFields::ExpectedFilesize );
    const auto &checkExistsOnly = req->_spec.value( zyppng::ProvideMsgFields::CheckExistOnly );
    const auto &deltaFile = req->_spec.value( zyppng::ProvideMsgFields::DeltaFile );

    zyppng::DownloadSpec spec(
      url
      , stagingPath
      , expFilesize.valid() ? zypp::ByteCount( expFilesize.asInt64() ) : zypp::ByteCount()
    );
    spec
      .setCheckExistsOnly( checkExistsOnly.valid() ? checkExistsOnly.asBool() : false )
      .setDeltaFile ( deltaFile.valid() ? deltaFile.asString() : zypp::Pathname() )
      .setMetalinkEnabled ( doMetalink );

    req->startDownload( _dlManager->downloadFile ( spec ) );
  }
}

void NetworkProvider::cancel( const std::deque<zyppng::worker::ProvideWorkerItemRef>::iterator &i )
{
  auto &queue = requestQueue ();
  if ( i == queue.end() ) {
    ERR << "Unknown request ID, ignoring." << std::endl;
    return;
  }

  const auto &req = std::static_pointer_cast<NetworkProvideItem>(*i);
  req->cancelDownload();
  queue.erase(i);
}

void NetworkProvider::immediateShutdown()
{
  for ( const auto &pItem : requestQueue () ) {
    auto ref = std::static_pointer_cast<NetworkProvideItem>( pItem );
    ref->cancelDownload();
  }
}

zyppng::worker::ProvideWorkerItemRef NetworkProvider::makeItem(zyppng::ProvideMessage &&spec )
{
  return std::make_shared<NetworkProvideItem>( *this, std::move(spec) );
}

void NetworkProvider::itemStarted( NetworkProvideItemRef item )
{
  provideStart( item->_spec.requestId(), item->_dl->spec().url().asCompleteString(), item->_targetFileName.asString(), item->_stagingFileName.asString() );
}

void NetworkProvider::itemFinished( NetworkProvideItemRef item )
{
  auto &queue = requestQueue ();
  auto i = std::find( queue.begin(), queue.end(), item );
  if ( i == queue.end() ) {
    ERR << "Unknown request finished. This is a BUG!" << std::endl;
    return;
  }

  item->_state = NetworkProvideItem::Finished;
  if ( !item->_dl->hasError() ) {

    if ( item->_dl->stoppedOnMetalink () ) {
      // need to read the metalink info and send back a redirect
      try {
        zypp::media::MetaLinkParser pars;
        pars.parse( item->_stagingFileName  );

        std::vector<zypp::Url> urls;

        for ( const auto &mirr : pars.getMirrors () ) {
          try {
            zypp::Url url( mirr.url.asCompleteString () );
            urls.push_back(url);

            if ( urls.size() == 10 )
              break;

          }  catch ( const zypp::Exception &e ) {
            ZYPP_CAUGHT (e);
          }
        }

        // @TODO , restart the download without metalink?
        if ( urls.size() == 0 )
          throw zypp::Exception("No usable mirrors in Mirrorlink file");

        if ( !messageStream()->sendMessage( zyppng::ProvideMessage::createMetalinkRedir( item->_spec.requestId(), urls ).impl() ) ) {
          ERR << "Failed to send ProvideSuccess message" << std::endl;
        }

      }  catch ( const zypp::Exception &exp ) {
        provideFailed( item->_spec.requestId(), zyppng::ProvideMessage::Code::InternalError, exp.asUserString (), false );
      }

      zypp::filesystem::unlink( item->_stagingFileName );

    } else {
      const auto errCode = zypp::filesystem::rename( item->_stagingFileName, item->_targetFileName );
      if( errCode ) {

        zypp::filesystem::unlink( item->_stagingFileName );

        std::string err = zypp::str::Str() << "Renaming " << item->_stagingFileName << " to " << item->_targetFileName << " failed!";
        DBG << err << std::endl;

        provideFailed( item->_spec.requestId()
          , zyppng::ProvideMessage::Code::InternalError
          , err
          , false
          , {} );

      } else {
        provideSuccess( item->_spec.requestId(), false, item->_targetFileName );
      }
    }
  } else {
    zyppng::HeaderValueMap extra;
    bool isTransient = false;
    auto errCode = zyppng::ProvideMessage::Code::InternalError;

    const auto &error = item->_dl->lastRequestError();
    switch( error.type() ) {
      case zyppng::NetworkRequestError::NoError: { throw std::runtime_error("DownloadError info broken"); break;}
      case zyppng::NetworkRequestError::InternalError: { errCode = zyppng::ProvideMessage::Code::InternalError; break;}
      case zyppng::NetworkRequestError::RangeFail: { errCode = zyppng::ProvideMessage::Code::InternalError; break;}
      case zyppng::NetworkRequestError::Cancelled: { errCode = zyppng::ProvideMessage::Code::Cancelled; break;}
      case zyppng::NetworkRequestError::PeerCertificateInvalid: { errCode = zyppng::ProvideMessage::Code::PeerCertificateInvalid; break;}
      case zyppng::NetworkRequestError::ConnectionFailed: { errCode = zyppng::ProvideMessage::Code::ConnectionFailed; break;}
      case zyppng::NetworkRequestError::ExceededMaxLen: { errCode = zyppng::ProvideMessage::Code::ExpectedSizeExceeded; break;}
      case zyppng::NetworkRequestError::InvalidChecksum: { errCode = zyppng::ProvideMessage::Code::InvalidChecksum; break;}
      case zyppng::NetworkRequestError::UnsupportedProtocol: {  errCode = zyppng::ProvideMessage::Code::BadRequest; break; }
      case zyppng::NetworkRequestError::MalformedURL: { errCode = zyppng::ProvideMessage::Code::BadRequest; break; }
      case zyppng::NetworkRequestError::TemporaryProblem: { errCode = zyppng::ProvideMessage::Code::InternalError; isTransient = true; break;}
      case zyppng::NetworkRequestError::Timeout: { errCode = zyppng::ProvideMessage::Code::Timeout; break;}
      case zyppng::NetworkRequestError::Forbidden: { errCode = zyppng::ProvideMessage::Code::Forbidden; break;}
      case zyppng::NetworkRequestError::NotFound: { errCode = zyppng::ProvideMessage::Code::NotFound; break;}
      case zyppng::NetworkRequestError::Unauthorized: {
        errCode = zyppng::ProvideMessage::Code::Unauthorized;
        const auto &authHint = error.extraInfoValue("authHint", std::string());
        if ( authHint.size () )
          extra.set ( "authhint", authHint );
        break;
      }
      case zyppng::NetworkRequestError::AuthFailed: {
        errCode = zyppng::ProvideMessage::Code::NoAuthData;
        const auto &authHint = error.extraInfoValue("authHint", std::string());
        if ( authHint.size () )
          extra.set ( "authhint", authHint );
        break;
      }
      //@TODO add extra codes for handling these
      case zyppng::NetworkRequestError::Http2Error:
      case zyppng::NetworkRequestError::Http2StreamError:
      case zyppng::NetworkRequestError::ServerReturnedError: { errCode = zyppng::ProvideMessage::Code::InternalError; break; }
      case zyppng::NetworkRequestError::MissingData: { errCode = zyppng::ProvideMessage::Code::BadRequest; break; }
    }

    provideFailed( item->_spec.requestId(), errCode, item->_dl->errorString(), isTransient, extra );
  }
  queue.erase(i);

  // pick the next request
  provide();
}

void NetworkProvider::itemAuthRequired( NetworkProvideItemRef item, zyppng::NetworkAuthData &auth, const std::string & availAuth )
{
  auto res = requireAuthorization( item->_spec.requestId(), auth.url(), auth.username(), auth.lastDatabaseUpdate(), { { std::string( zyppng::AuthDataRequestMsgFields::AuthHint ), availAuth } } );
  if ( !res ) {
    auth = zyppng::NetworkAuthData();
    return;
  }

  auth.setUrl ( item->_dl->spec().url() );
  auth.setUsername ( res->username );
  auth.setPassword ( res->password );
  auth.setLastDatabaseUpdate ( res->last_auth_timestamp );
  auth.extraValues() = res->extraKeys;

  const auto &keyStr = std::string(zyppng::AuthInfoMsgFields::AuthType);
  if ( res->extraKeys.count( keyStr ) ) {
    auth.setAuthType( res->extraKeys[keyStr] );
  }

  //@todo migrate away from using NetworkAuthData, AuthData now has a map of extra values that can carry the AuthType
  const auto &authTypeKey = std::string( zyppng::AuthInfoMsgFields::AuthType );
  if ( res->extraKeys.count( authTypeKey ) )
    auth.setAuthType( res->extraKeys[ authTypeKey ] );
}
