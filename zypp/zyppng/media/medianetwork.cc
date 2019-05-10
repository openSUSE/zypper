#include "medianetwork.h"

#include <zypp/base/Logger.h>
#include <zypp/media/MediaException.h>
#include <zypp/zyppng/base/EventDispatcher>
#include <zypp/zyppng/media/network/downloader.h>
#include <zypp/zyppng/media/network/networkrequestdispatcher.h>
#include <zypp/zyppng/media/network/request.h>
#include <zypp/zyppng/media/network/AuthData>
#include <zypp/media/mediacurlprefetcher.h>
#include <zypp/media/CurlHelper.h>
#include <zypp/media/MediaUserAuth.h>
#include <zypp/media/MediaException.h>
#include <zypp/media/CredentialManager.h>
#include <zypp/ZConfig.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/base/String.h>
#include <zypp/base/Gettext.h>

#include "zypp/ZYppCallbacks.h"


namespace  {
  struct ProgressData
  {
    ProgressData( const zyppng::Url & _url = zyppng::Url(),
      zypp::callback::SendReport<zypp::media::DownloadProgressReport> *_report = nullptr )
      : url( _url )
      , report( _report )
    {}

    zypp::Url	url;
    zypp::callback::SendReport<zypp::media::DownloadProgressReport> *report;

    time_t _timeStart	= 0;	///< Start total stats
    time_t _timeLast	= 0;	///< Start last period(~1sec)

    double _dnlTotal	= 0.0;	///< Bytes to download or 0 if unknown
    double _dnlLast	= 0.0;	///< Bytes downloaded at period start
    double _dnlNow	= 0.0;	///< Bytes downloaded now

    int    _dnlPercent= 0;	///< Percent completed or 0 if _dnlTotal is unknown

    double _drateTotal= 0.0;	///< Download rate so far
    double _drateLast	= 0.0;	///< Download rate in last period

    void updateStats( double dltotal = 0.0, double dlnow = 0.0 )
    {
      time_t now = time(0);

      // If called without args (0.0), recompute based on the last values seen
      if ( dltotal && dltotal != _dnlTotal )
        _dnlTotal = dltotal;

      if ( dlnow && dlnow != _dnlNow )
      {
        _dnlNow = dlnow;
      }
      else if ( !_dnlNow && !_dnlTotal )
      {
        // Start time counting as soon as first data arrives.
        // Skip the connection / redirection time at begin.
        return;
      }

      // init or reset if time jumps back
      if ( !_timeStart || _timeStart > now )
        _timeStart = _timeLast = now;

      // percentage:
      if ( _dnlTotal )
        _dnlPercent = int(_dnlNow * 100 / _dnlTotal);

      // download rates:
      _drateTotal = _dnlNow / std::max( int(now - _timeStart), 1 );

      if ( _timeLast < now )
      {
        _drateLast = (_dnlNow - _dnlLast) / int(now - _timeLast);
        // start new period
        _timeLast  = now;
        _dnlLast   = _dnlNow;
      }
      else if ( _timeStart == _timeLast )
        _drateLast = _drateTotal;
    }

    int reportProgress() const
    {
      if ( report && !(*report)->progress( _dnlPercent, url, _drateTotal, _drateLast ) )
        return 1;	// user requested abort
      return 0;
    }
  };
}

namespace zyppng {

MediaHandlerNetwork::MediaHandlerNetwork(
  const Url & url_r,
  const zypp::Pathname & attach_point_hint_r )
  : MediaHandler( url_r, attach_point_hint_r,
      "/", // urlpath at attachpoint
      true ) // does_download
{

  MIL << "MediaHandlerNetwork::MediaHandlerNetwork(" << url_r << ", " << attach_point_hint_r << ")" << std::endl;

  if( !attachPoint().empty()){
    zypp::PathInfo ainfo(attachPoint());
    zypp::Pathname apath(attachPoint() + "XXXXXX");
    char    *atemp = ::strdup( apath.asString().c_str());
    char    *atest = NULL;
    if( !ainfo.isDir() || !ainfo.userMayRWX() ||
         atemp == NULL || (atest=::mkdtemp(atemp)) == NULL) {
      WAR << "attach point " << ainfo.path()
          << " is not useable for " << url_r.getScheme() << std::endl;
      setAttachPoint("", true);
    }
    else if( atest != NULL)
      ::rmdir(atest);

    if( atemp != NULL)
      ::free(atemp);
  }
}

TransferSettings &MediaHandlerNetwork::settings()
{
  return _settings;
}


void MediaHandlerNetwork::disconnectFrom()
{
  if ( _prefetchCacheId ) {
    zypp::media::MediaCurlPrefetcher::instance().closeCache( *_prefetchCacheId );
    _prefetchCacheId.reset();
  }
}

void MediaHandlerNetwork::attachTo(bool next)
{
  if ( next )
    ZYPP_THROW( zypp::media::MediaNotSupportedException(_url) );

  if ( !_url.isValid() )
    ZYPP_THROW( zypp::media::MediaBadUrlException(_url) );

  if( !NetworkRequestDispatcher::supportsProtocol( _url ) ) {
    std::string msg("Unsupported protocol '");
    msg += _url.getScheme();
    msg += "'";
    ZYPP_THROW( zypp::media::MediaBadUrlException(_url, msg) );
  }

  if( !isUseableAttachPoint( attachPoint() ) )
  {
    setAttachPoint( createAttachPoint(), true );
  }

  disconnectFrom();

  _prefetchCacheId = zypp::media::MediaCurlPrefetcher::instance().createCache();

  // FIXME: need a derived class to propelly compare url's
  zypp::media::MediaSourceRef media( new zypp::media::MediaSource(_url.getScheme(), _url.asString()) );
  setMediaSource(media);
}

void MediaHandlerNetwork::releaseFrom(const std::string &)
{
  disconnect();
}

Url MediaHandlerNetwork::getFileUrl( const zypp::Pathname & filename_r ) const
{
  // Simply extend the URLs pathname. An 'absolute' URL path
  // is achieved by encoding the leading '/' in an URL path:
  //   URL: ftp://user@server		-> ~user
  //   URL: ftp://user@server/		-> ~user
  //   URL: ftp://user@server//		-> ~user
  //   URL: ftp://user@server/%2F	-> /
  //                         ^- this '/' is just a separator
  Url newurl( _url );
  newurl.setPathName( ( zypp::Pathname("./"+_url.getPathName()) / filename_r ).asString().substr(1) );
  return newurl;
}

void MediaHandlerNetwork::handleRequestResult( const Download &req ) const
{
  if ( req.state() == Download::Success )
    return;

  const NetworkRequestError &err = req.lastRequestError();
  if ( err.type() != NetworkRequestError::NoError ) {

    Url reqUrl = err.extraInfoValue<Url>("requestUrl", req.url());

    switch ( err.type() )
    {
      case NetworkRequestError::Unauthorized: {
        std::string hint = err.extraInfoValue<std::string>("authHint");
        ZYPP_THROW( zypp::media::MediaUnauthorizedException(
          reqUrl, err.toString(), err.nativeErrorString(), hint
          ));
        break;
      }
      case NetworkRequestError::TemporaryProblem:
        ZYPP_THROW( zypp::media::MediaTemporaryProblemException(reqUrl) );
        break;
      case NetworkRequestError::Timeout:
        ZYPP_THROW( zypp::media::MediaTimeoutException(reqUrl) );
        break;
      case NetworkRequestError::Forbidden:
        ZYPP_THROW( zypp::media::MediaForbiddenException(reqUrl, err.toString()));
        break;
      case NetworkRequestError::NotFound:
        ZYPP_THROW( zypp::media::MediaFileNotFoundException(reqUrl, req.targetPath()) );
        break;
      default:
        break;
    }
    ZYPP_THROW( zypp::media::MediaCurlException( reqUrl, err.toString(), err.nativeErrorString() ) );
  }

  ZYPP_THROW( zypp::media::MediaCurlException( req.url(), req.errorString(), "" ) );
}

void MediaHandlerNetwork::authenticate( const Download &, NetworkAuthData &auth, const std::string &availAuth ) const
{

  zypp::callback::SendReport<zypp::media::AuthenticationReport> auth_report;

  NetworkAuthData authCopy = auth;

  // preset the username if present in current url
  if ( !_url.getUsername().empty() && authCopy.password().empty() )
    authCopy.setUsername( _url.getUsername() );

  std::string prompt_msg = zypp::str::Format( _("Authentication required for '%s'") ) % _url.asString();

  // set available authentication types from the exception
  // might be needed in prompt
  authCopy.setAuthType(availAuth);

  // ask user
  if ( auth_report->prompt(_url, prompt_msg, authCopy) ) {
    DBG << "callback answer: retry" << std::endl
        << "CurlAuthData: " << authCopy << std::endl;

    if ( authCopy.valid() ) {
      auth = authCopy;
    }
  }
}

std::shared_ptr<Download> MediaHandlerNetwork::prepareRequest( Downloader &dlManager, const zypp::filesystem::Pathname &filename, const zypp::ByteCount &expectedFileSize_r ) const
{
  DBG << filename.asString() << std::endl;

  if(!_url.isValid())
    ZYPP_THROW(zypp::media::MediaBadUrlException(_url));

  if(_url.getHost().empty())
    ZYPP_THROW(zypp::media::MediaBadUrlEmptyHostException(_url));

  Url url( getFileUrl(filename) );

  zypp::filesystem::Pathname target = localPath(filename);
  zypp::filesystem::Pathname dest = target.absolutename();
  if( assert_dir( dest.dirname() ) )
  {
    DBG << "assert_dir " << dest.dirname() << " failed" << std::endl;
    Url url(getFileUrl(filename));
    ZYPP_THROW( zypp::media::MediaSystemException(url, "System error on " + dest.dirname().asString()) );
  }

  DBG << "URL: " << url.asString() << std::endl;

  Download::Ptr dl = dlManager.downloadFile( url, localPath(filename), expectedFileSize_r );
  dl->settings() = _settings;
  dl->sigAuthRequired().connect( sigc::mem_fun( *this, &MediaHandlerNetwork::authenticate ) );
  return dl;
}

bool MediaHandlerNetwork::getDoesFileExist( const zypp::filesystem::Pathname &filename ) const
{
  auto ev = EventDispatcher::createForThread();
  Downloader::Ptr dlManager = std::make_shared<Downloader>();

  Download::Ptr dl = prepareRequest( *dlManager, filename );
  dl->sigFinished().connect( [&ev]( zyppng::Download & ){
    ev->quit();
  });


  dl->setCheckExistsOnly( true );
  dl->start();
  ev->run();

  try
  {
    //this will throw if the file does not exist
    handleRequestResult( *dl );
  } catch ( const zypp::media::MediaFileNotFoundException &e ) {
    return false;
  } catch ( const zypp::media::MediaException &e ) {
    // some error, we are not sure about file existence, rethrw
    ZYPP_RETHROW(e);
  }

  return true;
}


void MediaHandlerNetwork::getFile(const zypp::filesystem::Pathname &filename, const zypp::ByteCount &expectedFileSize_r) const
{

  zypp::callback::SendReport<zypp::media::DownloadProgressReport> report;
  Url fileurl(getFileUrl(filename));

  if ( _prefetchCacheId && zypp::media::MediaCurlPrefetcher::instance().requireFile( *_prefetchCacheId, fileurl, localPath( filename ), report ) ) {
    MIL << "Got file " << filename << " from precache." << std::endl;
    return;
  } else {
    MIL << "Precache failed for file " << filename << std::endl;
  }

  auto ev = EventDispatcher::createForThread();
  Downloader::Ptr dlManager = std::make_shared<Downloader>();

  ProgressData data( fileurl, &report );

  Download::Ptr dl = prepareRequest( *dlManager, filename, expectedFileSize_r );
  dl->setDeltaFile( deltafile() );
  dl->sigFinished().connect( [&ev]( zyppng::Download & ){
    ev->quit();
  });
  dl->sigAlive().connect( [&data]( Download &, off_t dlnow ){
    data.updateStats( 0.0, dlnow );
    data.reportProgress();
  });
  dl->sigProgress().connect( [&data]( Download &, off_t dltotal, off_t dlnow ){
    data.updateStats( dltotal, dlnow );
    data.reportProgress();
  });

  dl->start();
  ev->run();

  try {
    //this will throw if the file does not exist
    handleRequestResult( *dl );
  } catch ( zypp::media::MediaUnauthorizedException & ex_r ) {
    report->finish(fileurl, zypp::media::DownloadProgressReport::ACCESS_DENIED, ex_r.asUserHistory());
    ZYPP_RETHROW(ex_r);
  }
  // unexpected exception
  catch ( zypp::media::MediaException & excpt_r)
  {
    zypp::media::DownloadProgressReport::Error reason = zypp::media::DownloadProgressReport::ERROR;
    if( typeid(excpt_r) == typeid( zypp::media::MediaFileNotFoundException )  ||
         typeid(excpt_r) == typeid( zypp::media::MediaNotAFileException ) )
    {
      reason = zypp::media::DownloadProgressReport::NOT_FOUND;
    }
    report->finish(fileurl, reason, excpt_r.asUserHistory());
    ZYPP_RETHROW(excpt_r);
  }

  report->finish(fileurl, zypp::media::DownloadProgressReport::NO_ERROR, "");
}

void MediaHandlerNetwork::getFiles( const std::vector<std::pair<zypp::filesystem::Pathname, zypp::ByteCount> > &files ) const
{
  return MediaHandler::getFiles( files );
}

void MediaHandlerNetwork::getDir(const zypp::filesystem::Pathname &dirname, bool recurse_r) const
{
  //we could make this download concurrently, but its not used anywhere in the code, so why bother
  zypp::filesystem::DirContent content;
  getDirInfo( content, dirname, /*dots*/false );

  for ( zypp::filesystem::DirContent::const_iterator it = content.begin(); it != content.end(); ++it ) {
    zypp::Pathname filename = dirname + it->name;
    int res = 0;

    switch ( it->type ) {
      case zypp::filesystem::FT_NOT_AVAIL: // old directory.yast contains no typeinfo at all
      case zypp::filesystem::FT_FILE:
        getFile( filename, 0 );
        break;
      case zypp::filesystem::FT_DIR: // newer directory.yast contain at least directory info
        if ( recurse_r ) {
          getDir( filename, recurse_r );
        } else {
          res = assert_dir( localPath( filename ) );
          if ( res ) {
            WAR << "Ignore error (" << res <<  ") on creating local directory '" << localPath( filename ) << "'" << std::endl;
          }
        }
        break;
      default:
        // don't provide devices, sockets, etc.
        break;
    }
  }
}

void MediaHandlerNetwork::precacheFiles(const std::vector<zypp::OnMediaLocation> &files)
{
  if ( !_prefetchCacheId )
    _prefetchCacheId = zypp::media::MediaCurlPrefetcher::instance().createCache();
  std::vector< zypp::media::MediaCurlPrefetcher::Request > prefetch;
  std::for_each( files.begin(), files.end(), [this,  &prefetch ]( const zypp::OnMediaLocation &elem ){
    zypp::media::MediaCurlPrefetcher::Request r;
    r.cache = *_prefetchCacheId;
    r.url = getFileUrl( elem.filename() );
    r.settings = settings();
    r.expectedFileSize = elem.downloadSize();
    prefetch.push_back( std::move(r) );
  });
  zypp::media::MediaCurlPrefetcher::instance().precacheFiles( std::move(prefetch) );
}

void MediaHandlerNetwork::getDirInfo(std::list<std::string> &retlist, const zypp::filesystem::Pathname &dirname, bool dots) const
{
  getDirectoryYast( retlist, dirname, dots );
}

void MediaHandlerNetwork::getDirInfo(zypp::filesystem::DirContent &retlist, const zypp::filesystem::Pathname &dirname, bool dots) const
{
  getDirectoryYast( retlist, dirname, dots );
}

bool MediaHandlerNetwork::checkAttachPoint(const zypp::Pathname &apoint) const
{
  return MediaHandler::checkAttachPoint( apoint, true, true);
}

}
