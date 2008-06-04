/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaCurl.cc
 *
*/

#include <iostream>
#include <list>

#include "zypp/base/Logger.h"
#include "zypp/ExternalProgram.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Sysconfig.h"
#include "zypp/base/Gettext.h"

#include "zypp/media/MediaCurl.h"
#include "zypp/media/proxyinfo/ProxyInfos.h"
#include "zypp/media/ProxyInfo.h"
#include "zypp/media/MediaUserAuth.h"
#include "zypp/media/CurlConfig.h"
#include "zypp/thread/Once.h"
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <boost/format.hpp>

#define  DETECT_DIR_INDEX       0
#define  CONNECT_TIMEOUT        60
#define  TRANSFER_TIMEOUT       60 * 3
#define  TRANSFER_TIMEOUT_MAX   60 * 60


using namespace std;
using namespace zypp::base;

namespace
{
  zypp::thread::OnceFlag g_InitOnceFlag = PTHREAD_ONCE_INIT;
  zypp::thread::OnceFlag g_FreeOnceFlag = PTHREAD_ONCE_INIT;

  extern "C" void _do_free_once()
  {
    curl_global_cleanup();
  }

  extern "C" void globalFreeOnce()
  {
    zypp::thread::callOnce(g_FreeOnceFlag, _do_free_once);
  }

  extern "C" void _do_init_once()
  {
    CURLcode ret = curl_global_init( CURL_GLOBAL_ALL );
    if ( ret != 0 )
    {
      WAR << "curl global init failed" << endl;
    }

    //
    // register at exit handler ?
    // this may cause trouble, because we can protect it
    // against ourself only.
    // if the app sets an atexit handler as well, it will
    // cause a double free while the second of them runs.
    //
    //std::atexit( globalFreeOnce);
  }

  inline void globalInitOnce()
  {
    zypp::thread::callOnce(g_InitOnceFlag, _do_init_once);
  }

  int log_curl(CURL *curl, curl_infotype info,
               char *ptr, size_t len, void *max_lvl)
  {
    std::string pfx(" ");
    long        lvl = 0;
    switch( info)
    {
      case CURLINFO_TEXT:       lvl = 1; pfx = "*"; break;
      case CURLINFO_HEADER_IN:  lvl = 2; pfx = "<"; break;
      case CURLINFO_HEADER_OUT: lvl = 2; pfx = ">"; break;
      default:                                      break;
    }
    if( lvl > 0 && max_lvl != NULL && lvl <= *((long *)max_lvl))
    {
      std::string                            msg(ptr, len);
      std::list<std::string>                 lines;
      std::list<std::string>::const_iterator line;
      zypp::str::split(msg, std::back_inserter(lines), "\r\n");
      for(line = lines.begin(); line != lines.end(); ++line)
      {
        DBG << pfx << " " << *line << endl;
      }
    }
    return 0;
  }
}

namespace zypp {
  namespace media {

  namespace {
    struct ProgressData
    {
      ProgressData(const long _timeout, const zypp::Url &_url = zypp::Url(),
                   callback::SendReport<DownloadProgressReport> *_report=NULL)
        : timeout(_timeout)
        , reached(false)
        , report(_report)
        , drate_period(-1)
        , dload_period(0)
        , secs(0)
        , drate_avg(-1)
        , ltime( time(NULL))
        , dload( 0)
        , uload( 0)
        , url(_url)
      {}
      long                                          timeout;
      bool                                          reached;
      callback::SendReport<DownloadProgressReport> *report;
      // download rate of the last period (cca 1 sec)
      double                                        drate_period;
      // bytes downloaded at the start of the last period
      double                                        dload_period;
      // seconds from the start of the download
      long                                          secs;
      // average download rate
      double                                        drate_avg;
      // last time the progress was reported
      time_t                                        ltime;
      // bytes downloaded at the moment the progress was last reported
      double                                        dload;
      // bytes uploaded at the moment the progress was last reported
      double                                        uload;
      zypp::Url                                     url;
    };

    ///////////////////////////////////////////////////////////////////

    inline void escape( string & str_r,
                        const char char_r, const string & escaped_r ) {
      for ( string::size_type pos = str_r.find( char_r );
            pos != string::npos; pos = str_r.find( char_r, pos ) ) {
              str_r.replace( pos, 1, escaped_r );
            }
    }

    inline string escapedPath( string path_r ) {
      escape( path_r, ' ', "%20" );
      return path_r;
    }

    inline string unEscape( string text_r ) {
      char * tmp = curl_unescape( text_r.c_str(), 0 );
      string ret( tmp );
      curl_free( tmp );
      return ret;
    }

  }

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : MediaCurl
//
///////////////////////////////////////////////////////////////////

Pathname MediaCurl::_cookieFile = "/var/lib/YaST2/cookies";

const char *const MediaCurl::agentString()
{
  static const std::string _value( str::form( "ZYpp %s (curl %s)",
                                              VERSION,
                                              curl_version_info(CURLVERSION_NOW)->version ) );
  return _value.c_str();
}


MediaCurl::MediaCurl( const Url &      url_r,
                      const Pathname & attach_point_hint_r )
    : MediaHandler( url_r, attach_point_hint_r,
                    "/", // urlpath at attachpoint
                    true ), // does_download
      _curl( NULL )
{
  _curlError[0] = '\0';
  _curlDebug = 0L;

  MIL << "MediaCurl::MediaCurl(" << url_r << ", " << attach_point_hint_r << ")" << endl;

  globalInitOnce();

  if( !attachPoint().empty())
  {
    PathInfo ainfo(attachPoint());
    Pathname apath(attachPoint() + "XXXXXX");
    char    *atemp = ::strdup( apath.asString().c_str());
    char    *atest = NULL;
    if( !ainfo.isDir() || !ainfo.userMayRWX() ||
         atemp == NULL || (atest=::mkdtemp(atemp)) == NULL)
    {
      WAR << "attach point " << ainfo.path()
          << " is not useable for " << url_r.getScheme() << endl;
      setAttachPoint("", true);
    }
    else if( atest != NULL)
      ::rmdir(atest);

    if( atemp != NULL)
      ::free(atemp);
  }
}

void MediaCurl::setCookieFile( const Pathname &fileName )
{
  _cookieFile = fileName;
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : MediaCurl::attachTo
//        METHOD TYPE : PMError
//
//        DESCRIPTION : Asserted that not already attached, and attachPoint is a directory.
//
void MediaCurl::attachTo (bool next)
{
  if ( next )
    ZYPP_THROW(MediaNotSupportedException(_url));

  if ( !_url.isValid() )
    ZYPP_THROW(MediaBadUrlException(_url));

  CurlConfig curlconf;
  CurlConfig::parseConfig(curlconf); // parse ~/.curlrc

  curl_version_info_data *curl_info = NULL;
  curl_info = curl_version_info(CURLVERSION_NOW);
  // curl_info does not need any free (is static)
  if (curl_info->protocols)
  {
    const char * const *proto;
    std::string        scheme( _url.getScheme());
    bool               found = false;
    for(proto=curl_info->protocols; !found && *proto; ++proto)
    {
      if( scheme == std::string((const char *)*proto))
        found = true;
    }
    if( !found)
    {
      std::string msg("Unsupported protocol '");
      msg += scheme;
      msg += "'";
      ZYPP_THROW(MediaBadUrlException(_url, msg));
    }
  }

  if( !isUseableAttachPoint(attachPoint()))
  {
    std::string mountpoint = createAttachPoint().asString();

    if( mountpoint.empty())
      ZYPP_THROW( MediaBadAttachPointException(url()));

    setAttachPoint( mountpoint, true);
  }

  disconnectFrom(); // clean _curl if needed
  _curl = curl_easy_init();
  if ( !_curl ) {
    ZYPP_THROW(MediaCurlInitException(_url));
  }

  {
    char *ptr = getenv("ZYPP_MEDIA_CURL_DEBUG");
    _curlDebug = (ptr && *ptr) ? str::strtonum<long>( ptr) : 0L;
    if( _curlDebug > 0)
    {
      curl_easy_setopt( _curl, CURLOPT_VERBOSE, 1);
      curl_easy_setopt( _curl, CURLOPT_DEBUGFUNCTION, log_curl);
      curl_easy_setopt( _curl, CURLOPT_DEBUGDATA, &_curlDebug);
    }
  }

  CURLcode ret = curl_easy_setopt( _curl, CURLOPT_ERRORBUFFER, _curlError );
  if ( ret != 0 ) {
    disconnectFrom();
    ZYPP_THROW(MediaCurlSetOptException(_url, "Error setting error buffer"));
  }

  ret = curl_easy_setopt( _curl, CURLOPT_FAILONERROR, true );
  if ( ret != 0 ) {
    disconnectFrom();
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  ret = curl_easy_setopt( _curl, CURLOPT_NOSIGNAL, 1 );
  if ( ret != 0 ) {
    disconnectFrom();
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  /**
   * Transfer timeout
   */
  {
    _xfer_timeout = TRANSFER_TIMEOUT;

    std::string param(_url.getQueryParam("timeout"));
    if( !param.empty())
    {
      long num = str::strtonum<long>( param);
      if( num >= 0 && num <= TRANSFER_TIMEOUT_MAX)
        _xfer_timeout = num;
    }
  }

  /*
  ** Connect timeout
  */
  ret = curl_easy_setopt( _curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
  if ( ret != 0 ) {
    disconnectFrom();
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  if ( _url.getScheme() == "http" ) {
    // follow any Location: header that the server sends as part of
    // an HTTP header (#113275)
    ret = curl_easy_setopt ( _curl, CURLOPT_FOLLOWLOCATION, true );
    if ( ret != 0) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
    ret = curl_easy_setopt ( _curl, CURLOPT_MAXREDIRS, 3L );
    if ( ret != 0) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }

    ret = curl_easy_setopt ( _curl, CURLOPT_USERAGENT, agentString() );


    if ( ret != 0) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
  }

  if ( _url.getScheme() == "https" )
  {
    bool verify_peer = false;
    bool verify_host = false;

    std::string verify( _url.getQueryParam("ssl_verify"));
    if( verify.empty() ||
        verify == "yes")
    {
      verify_peer = true;
      verify_host = true;
    }
    else
    if( verify == "no")
    {
      verify_peer = false;
      verify_host = false;
    }
    else
    {
      std::vector<std::string>                 flags;
      std::vector<std::string>::const_iterator flag;
      str::split( verify, std::back_inserter(flags), ",");
      for(flag = flags.begin(); flag != flags.end(); ++flag)
      {
        if( *flag == "host")
        {
          verify_host = true;
        }
        else
        if( *flag == "peer")
        {
          verify_peer = true;
        }
        else
        {
                disconnectFrom();
          ZYPP_THROW(MediaBadUrlException(_url, "Unknown ssl_verify flag"));
        }
      }
    }

    _ca_path = Pathname(_url.getQueryParam("ssl_capath")).asString();
    if( _ca_path.empty())
    {
        _ca_path = "/etc/ssl/certs/";
    }
    else
    if( !PathInfo(_ca_path).isDir() || !Pathname(_ca_path).absolute())
    {
        disconnectFrom();
        ZYPP_THROW(MediaBadUrlException(_url, "Invalid ssl_capath path"));
    }

    if( verify_peer || verify_host)
    {
      ret = curl_easy_setopt( _curl, CURLOPT_CAPATH, _ca_path.c_str());
      if ( ret != 0 ) {
        disconnectFrom();
        ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
      }
    }

    ret = curl_easy_setopt( _curl, CURLOPT_SSL_VERIFYPEER, verify_peer ? 1L : 0L);
    if ( ret != 0 ) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
    ret = curl_easy_setopt( _curl, CURLOPT_SSL_VERIFYHOST, verify_host ? 2L : 0L);
    if ( ret != 0 ) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }

    ret = curl_easy_setopt ( _curl, CURLOPT_USERAGENT, agentString() );
    if ( ret != 0) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
  }


  /*---------------------------------------------------------------*
   CURLOPT_USERPWD: [user name]:[password]

   Url::username/password -> CURLOPT_USERPWD
   If not provided, anonymous FTP identification
   *---------------------------------------------------------------*/

  if ( _url.getUsername().empty() ) {
    if ( _url.getScheme() == "ftp" ) {
      string id = "yast2@";
      id += VERSION;
      DBG << "Anonymous FTP identification: '" << id << "'" << endl;
      _userpwd = "anonymous:" + id;
    }
  } else {
    _userpwd = _url.getUsername();
    if ( _url.getPassword().size() ) {
      _userpwd += ":" + _url.getPassword();
    }
  }

  if ( _userpwd.size() ) {
    _userpwd = unEscape( _userpwd );
    ret = curl_easy_setopt( _curl, CURLOPT_USERPWD, _userpwd.c_str() );
    if ( ret != 0 ) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }

    // HTTP authentication type
    if(_url.getScheme() == "http" || _url.getScheme() == "https")
    {
      string use_auth = _url.getQueryParam("auth");
      if( use_auth.empty())
        use_auth = "digest,basic";

      try
      {
        long auth = CurlAuthData::auth_type_str2long(use_auth);
        if( auth != CURLAUTH_NONE)
        {
          DBG << "Enabling HTTP authentication methods: " << use_auth
              << " (CURLOPT_HTTPAUTH=" << auth << ")" << std::endl;

          ret = curl_easy_setopt( _curl, CURLOPT_HTTPAUTH, auth);
          if ( ret != 0 ) {
            disconnectFrom();
            ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
          }
        }
      }
      catch (MediaException & ex_r)
      {
        string auth_hint = getAuthHint();

        DBG << "Rethrowing as MediaUnauthorizedException. auth hint: '"
            << auth_hint << "'" << endl;

        ZYPP_THROW(MediaUnauthorizedException(
          _url, ex_r.msg(), _curlError, auth_hint
        ));
      }
    }
  }

  /*---------------------------------------------------------------*
   CURLOPT_PROXY: host[:port]

   Url::option(proxy and proxyport) -> CURLOPT_PROXY
   If not provided, /etc/sysconfig/proxy is evaluated
   *---------------------------------------------------------------*/

  _proxy = _url.getQueryParam( "proxy" );

  if ( ! _proxy.empty() ) {
    string proxyport( _url.getQueryParam( "proxyport" ) );
    if ( ! proxyport.empty() ) {
      _proxy += ":" + proxyport;
    }
  } else {

    ProxyInfo proxy_info (ProxyInfo::ImplPtr(new ProxyInfoSysconfig("proxy")));

    if ( proxy_info.enabled())
    {
      bool useproxy = true;

      std::list<std::string> nope = proxy_info.noProxy();
      for (ProxyInfo::NoProxyIterator it = proxy_info.noProxyBegin();
           it != proxy_info.noProxyEnd();
           it++)
      {
        std::string host( str::toLower(_url.getHost()));
        std::string temp( str::toLower(*it));

        // no proxy if it points to a suffix
        // preceeded by a '.', that maches
        // the trailing portion of the host.
        if( temp.size() > 1 && temp.at(0) == '.')
        {
          if(host.size() > temp.size() &&
             host.compare(host.size() - temp.size(), temp.size(), temp) == 0)
          {
            DBG << "NO_PROXY: '" << *it  << "' matches host '"
                                 << host << "'" << endl;
            useproxy = false;
            break;
          }
        }
        else
        // no proxy if we have an exact match
        if( host == temp)
        {
          DBG << "NO_PROXY: '" << *it  << "' matches host '"
                               << host << "'" << endl;
          useproxy = false;
          break;
        }
      }

      if ( useproxy ) {
        _proxy = proxy_info.proxy(_url.getScheme());
      }
    }
  }


  DBG << "Proxy: " << (_proxy.empty() ? "-none-" : _proxy) << endl;

  if ( ! _proxy.empty() ) {

    ret = curl_easy_setopt( _curl, CURLOPT_PROXY, _proxy.c_str() );
    if ( ret != 0 ) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }

    /*---------------------------------------------------------------*
     CURLOPT_PROXYUSERPWD: [user name]:[password]

     Url::option(proxyuser and proxypassword) -> CURLOPT_PROXYUSERPWD
     If not provided, $HOME/.curlrc is evaluated
     *---------------------------------------------------------------*/

    _proxyuserpwd = _url.getQueryParam( "proxyuser" );

    if ( ! _proxyuserpwd.empty() ) {
      string proxypassword( _url.getQueryParam( "proxypassword" ) );
      if ( ! proxypassword.empty() ) {
        _proxyuserpwd += ":" + proxypassword;
      }
    } else {
      if (curlconf.proxyuserpwd.empty())
        DBG << "~/.curlrc does not contain the proxy-user option" << endl;
      else
      {
        _proxyuserpwd = curlconf.proxyuserpwd;
        DBG << "using proxy-user from ~/.curlrc" << endl;
      }
    }

    _proxyuserpwd = unEscape( _proxyuserpwd );
    ret = curl_easy_setopt( _curl, CURLOPT_PROXYUSERPWD, _proxyuserpwd.c_str() );
    if ( ret != 0 ) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
  }

  /*---------------------------------------------------------------*
   *---------------------------------------------------------------*/

  _currentCookieFile = _cookieFile.asString();

  ret = curl_easy_setopt( _curl, CURLOPT_COOKIEFILE,
                          _currentCookieFile.c_str() );
  if ( ret != 0 ) {
    disconnectFrom();
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  ret = curl_easy_setopt( _curl, CURLOPT_COOKIEJAR,
                          _currentCookieFile.c_str() );
  if ( ret != 0 ) {
    disconnectFrom();
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  ret = curl_easy_setopt( _curl, CURLOPT_PROGRESSFUNCTION,
                          &progressCallback );
  if ( ret != 0 ) {
    disconnectFrom();
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  ret = curl_easy_setopt( _curl, CURLOPT_NOPROGRESS, false );
  if ( ret != 0 ) {
    disconnectFrom();
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  // FIXME: need a derived class to propelly compare url's
  MediaSourceRef media( new MediaSource(_url.getScheme(), _url.asString()));
  setMediaSource(media);
}

bool
MediaCurl::checkAttachPoint(const Pathname &apoint) const
{
  return MediaHandler::checkAttachPoint( apoint, true, true);
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : MediaCurl::disconnectFrom
//        METHOD TYPE : PMError
//
void MediaCurl::disconnectFrom()
{
  if ( _curl )
  {
    curl_easy_cleanup( _curl );
    _curl = NULL;
  }
}


///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : MediaCurl::releaseFrom
//        METHOD TYPE : void
//
//        DESCRIPTION : Asserted that media is attached.
//
void MediaCurl::releaseFrom( const std::string & ejectDev )
{
  disconnect();
}

static Url getFileUrl(const Url & url, const Pathname & filename)
{
  Url newurl(url);
  string path = url.getPathName();
  if ( !path.empty() && path != "/" && *path.rbegin() == '/' &&
       filename.absolute() )
  {
    // If url has a path with trailing slash, remove the leading slash from
    // the absolute file name
    path += filename.asString().substr( 1, filename.asString().size() - 1 );
  }
  else if ( filename.relative() )
  {
    // Add trailing slash to path, if not already there
    if (path.empty()) path = "/";
    else if (*path.rbegin() != '/' ) path += "/";
    // Remove "./" from begin of relative file name
    path += filename.asString().substr( 2, filename.asString().size() - 2 );
  }
  else
  {
    path += filename.asString();
  }

  newurl.setPathName(path);
  return newurl;
}


///////////////////////////////////////////////////////////////////
//
//        METHOD NAME : MediaCurl::getFile
//        METHOD TYPE : void
//
void MediaCurl::getFile( const Pathname & filename ) const
{
    // Use absolute file name to prevent access of files outside of the
    // hierarchy below the attach point.
    getFileCopy(filename, localPath(filename).absolutename());
}

///////////////////////////////////////////////////////////////////
//
//        METHOD NAME : MediaCurl::getFileCopy
//        METHOD TYPE : void
//
void MediaCurl::getFileCopy( const Pathname & filename , const Pathname & target) const
{
  callback::SendReport<DownloadProgressReport> report;

  Url fileurl(getFileUrl(_url, filename));

  bool retry = false;
  CurlAuthData auth_data;

  do
  {
    try
    {
      doGetFileCopy(filename, target, report);
      retry = false;
    }
    // retry with proper authentication data
    catch (MediaUnauthorizedException & ex_r)
    {
      callback::SendReport<AuthenticationReport> auth_report;

      if (!_url.getUsername().empty() && !retry)
        auth_data.setUserName(_url.getUsername());

      string prompt_msg;
      if (retry || !_url.getUsername().empty())
        prompt_msg = _("Invalid user name or password.");
      else // first prompt
        prompt_msg = boost::str(boost::format(
          _("Authentication required for '%s'")) % _url.asString());

      // set available authentication types from the exception
      auth_data.setAuthType(ex_r.hint());

      if (auth_report->prompt(_url, prompt_msg, auth_data))
      {
        DBG << "callback answer: retry" << endl
            << "CurlAuthData: " << auth_data << endl;

        if (auth_data.valid()) {
          _userpwd = auth_data.getUserPwd();

          // set username and password
          CURLcode ret = curl_easy_setopt(_curl, CURLOPT_USERPWD, _userpwd.c_str());
          if ( ret != 0 ) ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));

          // set auth type
          ret = curl_easy_setopt(_curl, CURLOPT_HTTPAUTH, auth_data.authType());
          if ( ret != 0 ) ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
        }

        retry = true;
      }
      else
      {
        DBG << "callback answer: cancel" << endl;
        report->finish(fileurl, zypp::media::DownloadProgressReport::ACCESS_DENIED, ex_r.asUserString());
        ZYPP_RETHROW(ex_r);
      }
    }
    // unexpected exception
    catch (MediaException & excpt_r)
    {
      // FIXME: error number fix
      report->finish(fileurl, zypp::media::DownloadProgressReport::ERROR, excpt_r.asUserString());
      ZYPP_RETHROW(excpt_r);
    }
  }
  while (retry);

  report->finish(fileurl, zypp::media::DownloadProgressReport::NO_ERROR, "");
}

bool MediaCurl::getDoesFileExist( const Pathname & filename ) const
{
  bool retry = false;
  CurlAuthData auth_data;

  do
  {
    try
    {
      return doGetDoesFileExist( filename );
    }
    // authentication problem, retry with proper authentication data
    catch (MediaUnauthorizedException & ex_r)
    {
      callback::SendReport<AuthenticationReport> auth_report;

      if (!_url.getUsername().empty() && !retry)
        auth_data.setUserName(_url.getUsername());

      string prompt_msg;
      if (retry || !_url.getUsername().empty())
        prompt_msg = _("Invalid user name or password.");
      else // first prompt
        prompt_msg = boost::str(boost::format(
          _("Authentication required for '%s'")) % _url.asString());

      // set available authentication types from the exception
      auth_data.setAuthType(ex_r.hint());

      if (auth_report->prompt(_url, prompt_msg, auth_data))
      {
        DBG << "callback answer: retry" << endl
            << "CurlAuthData: " << auth_data << endl;

        if (auth_data.valid()) {
          _userpwd = auth_data.getUserPwd();

          // set username and password
          CURLcode ret = curl_easy_setopt(_curl, CURLOPT_USERPWD, _userpwd.c_str());
          if ( ret != 0 ) ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));

          // set auth type
          ret = curl_easy_setopt(_curl, CURLOPT_HTTPAUTH, auth_data.authType());
          if ( ret != 0 ) ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
        }

        retry = true;
      }
      else
      {
        DBG << "callback answer: cancel" << endl;
        ZYPP_RETHROW(ex_r);
      }
    }
    // unexpected exception
    catch (MediaException & excpt_r)
    {
      ZYPP_RETHROW(excpt_r);
    }
  }
  while (retry);

  return false;
}

bool MediaCurl::doGetDoesFileExist( const Pathname & filename ) const
{
  DBG << filename.asString() << endl;

  if(!_url.isValid())
    ZYPP_THROW(MediaBadUrlException(_url));

  if(_url.getHost().empty())
    ZYPP_THROW(MediaBadUrlEmptyHostException(_url));

  string path = _url.getPathName();
  if ( !path.empty() && path != "/" && *path.rbegin() == '/' &&
        filename.absolute() ) {
      // If url has a path with trailing slash, remove the leading slash from
      // the absolute file name
    path += filename.asString().substr( 1, filename.asString().size() - 1 );
  } else if ( filename.relative() ) {
      // Add trailing slash to path, if not already there
    if ( !path.empty() && *path.rbegin() != '/' ) path += "/";
    // Remove "./" from begin of relative file name
    path += filename.asString().substr( 2, filename.asString().size() - 2 );
  } else {
    path += filename.asString();
  }

  Url url( _url );
  url.setPathName( path );

  DBG << "URL: " << url.asString() << endl;
    // Use URL without options and without username and passwd
    // (some proxies dislike them in the URL).
    // Curl seems to need the just scheme, hostname and a path;
    // the rest was already passed as curl options (in attachTo).
  Url curlUrl( url );

    // Use asString + url::ViewOptions instead?
  curlUrl.setUsername( "" );
  curlUrl.setPassword( "" );
  curlUrl.setPathParams( "" );
  curlUrl.setQueryString( "" );
  curlUrl.setFragment( "" );

  //
    // See also Bug #154197 and ftp url definition in RFC 1738:
    // The url "ftp://user@host/foo/bar/file" contains a path,
    // that is relative to the user's home.
    // The url "ftp://user@host//foo/bar/file" (or also with
    // encoded slash as %2f) "ftp://user@host/%2ffoo/bar/file"
    // contains an absolute path.
  //
  string urlBuffer( curlUrl.asString());
  CURLcode ret = curl_easy_setopt( _curl, CURLOPT_URL,
                                   urlBuffer.c_str() );
  if ( ret != 0 ) {
    ZYPP_THROW(MediaCurlSetOptException(url, _curlError));
  }

  // instead of returning no data with NOBODY, we return
  // little data, that works with broken servers, and
  // works for ftp as well, because retrieving only headers
  // ftp will return always OK code ?
  if (  _url.getScheme() == "http" ||  _url.getScheme() == "https" )
    ret = curl_easy_setopt( _curl, CURLOPT_NOBODY, 1 );
  else
    ret = curl_easy_setopt( _curl, CURLOPT_RANGE, "0-1" );

  if ( ret != 0 ) {
    curl_easy_setopt( _curl, CURLOPT_NOBODY, NULL );
    curl_easy_setopt( _curl, CURLOPT_RANGE, NULL );
    /* yes, this is why we never got to get NOBODY working before,
       because setting it changes this option too, and we also
       need to reset it
       See: http://curl.haxx.se/mail/archive-2005-07/0073.html
    */
    curl_easy_setopt( _curl, CURLOPT_HTTPGET, 1 );
    ZYPP_THROW(MediaCurlSetOptException(url, _curlError));
  }


  FILE *file = ::fopen( "/dev/null", "w" );
  if ( !file ) {
      ::fclose(file);
      ERR << "fopen failed for /dev/null" << endl;
      curl_easy_setopt( _curl, CURLOPT_NOBODY, NULL );
      curl_easy_setopt( _curl, CURLOPT_RANGE, NULL );
      /* yes, this is why we never got to get NOBODY working before,
       because setting it changes this option too, and we also
       need to reset it
       See: http://curl.haxx.se/mail/archive-2005-07/0073.html
      */
      curl_easy_setopt( _curl, CURLOPT_HTTPGET, 1 );
      if ( ret != 0 ) {
          ZYPP_THROW(MediaCurlSetOptException(url, _curlError));
      }
      ZYPP_THROW(MediaWriteException("/dev/null"));
  }

  ret = curl_easy_setopt( _curl, CURLOPT_WRITEDATA, file );
  if ( ret != 0 ) {
      ::fclose(file);
      std::string err( _curlError);
      curl_easy_setopt( _curl, CURLOPT_RANGE, NULL );
      curl_easy_setopt( _curl, CURLOPT_NOBODY, NULL );
      /* yes, this is why we never got to get NOBODY working before,
       because setting it changes this option too, and we also
       need to reset it
       See: http://curl.haxx.se/mail/archive-2005-07/0073.html
      */
      curl_easy_setopt( _curl, CURLOPT_HTTPGET, 1 );
      if ( ret != 0 ) {
          ZYPP_THROW(MediaCurlSetOptException(url, _curlError));
      }
      ZYPP_THROW(MediaCurlSetOptException(url, err));
  }
    // Set callback and perform.
  //ProgressData progressData(_xfer_timeout, url, &report);
  //report->start(url, dest);
  //if ( curl_easy_setopt( _curl, CURLOPT_PROGRESSDATA, &progressData ) != 0 ) {
  //  WAR << "Can't set CURLOPT_PROGRESSDATA: " << _curlError << endl;;
  //}

  CURLcode ok = curl_easy_perform( _curl );
  MIL << "perform code: " << ok << " [ " << curl_easy_strerror(ok) << " ]" << endl;

  // reset curl settings
  if (  _url.getScheme() == "http" ||  _url.getScheme() == "https" )
  {
    ret = curl_easy_setopt( _curl, CURLOPT_NOBODY, NULL );
    /* yes, this is why we never got to get NOBODY working before,
       because setting it changes this option too, and we also
       need to reset it
       See: http://curl.haxx.se/mail/archive-2005-07/0073.html
    */
    ret = curl_easy_setopt( _curl, CURLOPT_HTTPGET, 1 );
  }
  else
    ret = curl_easy_setopt( _curl, CURLOPT_RANGE, NULL );

  if ( ret != 0 )
  {
    ZYPP_THROW(MediaCurlSetOptException(url, _curlError));
  }

  if ( ok != 0 )
  {
    ::fclose( file );

    std::string err;
    try
    {
      bool err_file_not_found = false;
      switch ( ok )
      {
      case CURLE_FTP_COULDNT_RETR_FILE:
      case CURLE_FTP_ACCESS_DENIED:
        err_file_not_found = true;
        break;
      case CURLE_HTTP_RETURNED_ERROR:
        {
          long httpReturnCode = 0;
          CURLcode infoRet = curl_easy_getinfo( _curl,
                                                CURLINFO_RESPONSE_CODE,
                                                &httpReturnCode );
          if ( infoRet == CURLE_OK )
          {
            string msg = "HTTP response: " +
                          str::numstring( httpReturnCode );
            if ( httpReturnCode == 401 )
            {
              std::string auth_hint = getAuthHint();

              DBG << msg << " Login failed (URL: " << url.asString() << ")" << std::endl;
              DBG << "MediaUnauthorizedException auth hint: '" << auth_hint << "'" << std::endl;

              ZYPP_THROW(MediaUnauthorizedException(
                url, "Login failed.", _curlError, auth_hint
              ));
            }
            else
            if ( httpReturnCode == 403)
            {
               ZYPP_THROW(MediaForbiddenException(url));
            }
            else
            if ( httpReturnCode == 404)
            {
               err_file_not_found = true;
               break;
            }

            msg += err;
            DBG << msg << " (URL: " << url.asString() << ")" << std::endl;
            ZYPP_THROW(MediaCurlException(url, msg, _curlError));
          }
          else
          {
            string msg = "Unable to retrieve HTTP response:";
            msg += err;
            DBG << msg << " (URL: " << url.asString() << ")" << std::endl;
            ZYPP_THROW(MediaCurlException(url, msg, _curlError));
          }
        }
        break;
      case CURLE_UNSUPPORTED_PROTOCOL:
      case CURLE_URL_MALFORMAT:
      case CURLE_URL_MALFORMAT_USER:
      case CURLE_BAD_PASSWORD_ENTERED:
      case CURLE_FTP_USER_PASSWORD_INCORRECT:
        err = "Login failed";
        break;
      case CURLE_COULDNT_RESOLVE_PROXY:
      case CURLE_COULDNT_RESOLVE_HOST:
      case CURLE_COULDNT_CONNECT:
      case CURLE_FTP_CANT_GET_HOST:
        err = "Connection failed";
        break;
      case CURLE_WRITE_ERROR:
        err = "Write error";
        break;
      case CURLE_ABORTED_BY_CALLBACK:
      case CURLE_OPERATION_TIMEOUTED:
        err  = "Timeout reached";
        ZYPP_THROW(MediaTimeoutException(url));
        break;
      case CURLE_SSL_CACERT:
        ZYPP_THROW(MediaBadCAException(url,_curlError));
      case CURLE_SSL_PEER_CERTIFICATE:
      default:
        err = curl_easy_strerror(ok);
        if (err.empty())
          err = "Unrecognized error";
        break;
      }

      if( err_file_not_found)
      {
        // file does not exists
        return false;
      }
      else
      {
        // there was an error
        ZYPP_THROW(MediaCurlException(url, string(), _curlError));
      }
    }
    catch (const MediaException & excpt_r)
    {
      ZYPP_RETHROW(excpt_r);
    }
  }

  // exists
  return ( ok == CURLE_OK );
  //if ( curl_easy_setopt( _curl, CURLOPT_PROGRESSDATA, NULL ) != 0 ) {
  //  WAR << "Can't unset CURLOPT_PROGRESSDATA: " << _curlError << endl;;
  //}
}


void MediaCurl::doGetFileCopy( const Pathname & filename , const Pathname & target, callback::SendReport<DownloadProgressReport> & report) const
{
    DBG << filename.asString() << endl;

    if(!_url.isValid())
      ZYPP_THROW(MediaBadUrlException(_url));

    if(_url.getHost().empty())
      ZYPP_THROW(MediaBadUrlEmptyHostException(_url));

    Url url(getFileUrl(_url, filename));

    Pathname dest = target.absolutename();
    if( assert_dir( dest.dirname() ) )
    {
      DBG << "assert_dir " << dest.dirname() << " failed" << endl;
      ZYPP_THROW( MediaSystemException(url, "System error on " + dest.dirname().asString()) );
    }

    DBG << "URL: " << url.asString() << endl;
    // Use URL without options and without username and passwd
    // (some proxies dislike them in the URL).
    // Curl seems to need the just scheme, hostname and a path;
    // the rest was already passed as curl options (in attachTo).
    Url curlUrl( url );

    // Use asString + url::ViewOptions instead?
    curlUrl.setUsername( "" );
    curlUrl.setPassword( "" );
    curlUrl.setPathParams( "" );
    curlUrl.setQueryString( "" );
    curlUrl.setFragment( "" );

    //
    // See also Bug #154197 and ftp url definition in RFC 1738:
    // The url "ftp://user@host/foo/bar/file" contains a path,
    // that is relative to the user's home.
    // The url "ftp://user@host//foo/bar/file" (or also with
    // encoded slash as %2f) "ftp://user@host/%2ffoo/bar/file"
    // contains an absolute path.
    //
    string urlBuffer( curlUrl.asString());
    CURLcode ret = curl_easy_setopt( _curl, CURLOPT_URL,
                                     urlBuffer.c_str() );
    if ( ret != 0 ) {
      ZYPP_THROW(MediaCurlSetOptException(url, _curlError));
    }

    // set IFMODSINCE time condition (no download if not modified)
    curl_easy_setopt(_curl, CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE);
    curl_easy_setopt(_curl, CURLOPT_TIMEVALUE, PathInfo(target).mtime());

    string destNew = target.asString() + ".new.zypp.XXXXXX";
    char *buf = ::strdup( destNew.c_str());
    if( !buf)
    {
      ERR << "out of memory for temp file name" << endl;
      ZYPP_THROW(MediaSystemException(
        url, "out of memory for temp file name"
      ));
    }

    int tmp_fd = ::mkstemp( buf );
    if( tmp_fd == -1)
    {
      free( buf);
      ERR << "mkstemp failed for file '" << destNew << "'" << endl;
      ZYPP_THROW(MediaWriteException(destNew));
    }
    destNew = buf;
    free( buf);

    FILE *file = ::fdopen( tmp_fd, "w" );
    if ( !file ) {
      ::close( tmp_fd);
      filesystem::unlink( destNew );
      ERR << "fopen failed for file '" << destNew << "'" << endl;
      ZYPP_THROW(MediaWriteException(destNew));
    }

    DBG << "dest: " << dest << endl;
    DBG << "temp: " << destNew << endl;

    ret = curl_easy_setopt( _curl, CURLOPT_WRITEDATA, file );
    if ( ret != 0 ) {
      ::fclose( file );
      filesystem::unlink( destNew );
      ZYPP_THROW(MediaCurlSetOptException(url, _curlError));
    }

    // Set callback and perform.
    ProgressData progressData(_xfer_timeout, url, &report);
    report->start(url, dest);
    if ( curl_easy_setopt( _curl, CURLOPT_PROGRESSDATA, &progressData ) != 0 ) {
      WAR << "Can't set CURLOPT_PROGRESSDATA: " << _curlError << endl;;
    }

    ret = curl_easy_perform( _curl );

    if ( curl_easy_setopt( _curl, CURLOPT_PROGRESSDATA, NULL ) != 0 ) {
      WAR << "Can't unset CURLOPT_PROGRESSDATA: " << _curlError << endl;;
    }

    if ( ret != 0 ) {
      ERR << "curl error: " << ret << ": " << _curlError
          << ", temp file size " << PathInfo(destNew).size()
          << " byte." << endl;

      ::fclose( file );
      filesystem::unlink( destNew );

      std::string err;
      try {
       bool err_file_not_found = false;
       switch ( ret ) {
        case CURLE_UNSUPPORTED_PROTOCOL:
        case CURLE_URL_MALFORMAT:
        case CURLE_URL_MALFORMAT_USER:
          err = " Bad URL";
        case CURLE_HTTP_RETURNED_ERROR:
          {
            long httpReturnCode = 0;
            CURLcode infoRet = curl_easy_getinfo( _curl,
                                                  CURLINFO_RESPONSE_CODE,
                                                  &httpReturnCode );
            if ( infoRet == CURLE_OK ) {
              string msg = "HTTP response: " +
                           str::numstring( httpReturnCode );
              if ( httpReturnCode == 401 )
              {
                std::string auth_hint = getAuthHint();

                DBG << msg << " Login failed (URL: " << url.asString() << ")" << std::endl;
                DBG << "MediaUnauthorizedException auth hint: '" << auth_hint << "'" << std::endl;

                ZYPP_THROW(MediaUnauthorizedException(
                  url, "Login failed.", _curlError, auth_hint
                ));
              }
              else
              if ( httpReturnCode == 403)
              {
                 ZYPP_THROW(MediaForbiddenException(url));
              }
              else
              if ( httpReturnCode == 404)
              {
                 ZYPP_THROW(MediaFileNotFoundException(_url, filename));
              }

              msg += err;
              DBG << msg << " (URL: " << url.asString() << ")" << std::endl;
              ZYPP_THROW(MediaCurlException(url, msg, _curlError));
            }
            else
            {
              string msg = "Unable to retrieve HTTP response:";
              msg += err;
              DBG << msg << " (URL: " << url.asString() << ")" << std::endl;
              ZYPP_THROW(MediaCurlException(url, msg, _curlError));
            }
          }
          break;
        case CURLE_FTP_COULDNT_RETR_FILE:
        case CURLE_FTP_ACCESS_DENIED:
          err = "File not found";
          err_file_not_found = true;
          break;
        case CURLE_BAD_PASSWORD_ENTERED:
        case CURLE_FTP_USER_PASSWORD_INCORRECT:
          err = "Login failed";
          break;
        case CURLE_COULDNT_RESOLVE_PROXY:
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        case CURLE_FTP_CANT_GET_HOST:
          err = "Connection failed";
          break;
        case CURLE_WRITE_ERROR:
          err = "Write error";
          break;
        case CURLE_ABORTED_BY_CALLBACK:
        case CURLE_OPERATION_TIMEDOUT:
          if( progressData.reached)
          {
            err  = "Timeout reached";
            ZYPP_THROW(MediaTimeoutException(url));
          }
          else
          {
            err = "User abort";
          }
          break;
        case CURLE_SSL_PEER_CERTIFICATE:
        default:
          err = "Unrecognized error";
          break;
       }
       if( err_file_not_found)
       {
         ZYPP_THROW(MediaFileNotFoundException(_url, filename));
       }
       else
       {
         ZYPP_THROW(MediaCurlException(url, err, _curlError));
       }
      }
      catch (const MediaException & excpt_r)
      {
        ZYPP_RETHROW(excpt_r);
      }
    }
#if DETECT_DIR_INDEX
    else
    if(curlUrl.getScheme() == "http" ||
       curlUrl.getScheme() == "https")
    {
      //
      // try to check the effective url and set the not_a_file flag
      // if the url path ends with a "/", what usually means, that
      // we've received a directory index (index.html content).
      //
      // Note: This may be dangerous and break file retrieving in
      //       case of some server redirections ... ?
      //
      bool      not_a_file = false;
      char     *ptr = NULL;
      CURLcode  ret = curl_easy_getinfo( _curl,
                                         CURLINFO_EFFECTIVE_URL,
                                         &ptr);
      if ( ret == CURLE_OK && ptr != NULL)
      {
        try
        {
          Url         eurl( ptr);
          std::string path( eurl.getPathName());
          if( !path.empty() && path != "/" && *path.rbegin() == '/')
          {
            DBG << "Effective url ("
                << eurl
                << ") seems to provide the index of a directory"
                << endl;
            not_a_file = true;
          }
        }
        catch( ... )
        {}
      }

      if( not_a_file)
      {
        ::fclose( file );
        filesystem::unlink( destNew );
        ZYPP_THROW(MediaNotAFileException(_url, filename));
      }
    }
#endif // DETECT_DIR_INDEX

    long httpReturnCode = 0;
    CURLcode infoRet = curl_easy_getinfo(_curl,
                                         CURLINFO_RESPONSE_CODE,
                                         &httpReturnCode);
    bool modified = true;
    if (infoRet == CURLE_OK)
    {
      DBG << "HTTP response: " + str::numstring(httpReturnCode);
      if ( httpReturnCode == 304 ) // not modified
      {
        DBG << " Not modified.";
        modified = false;
      }
      DBG << endl;
    }
    else
    {
      WAR << "Could not get the reponse code." << endl;
    }

    if (modified || infoRet != CURLE_OK)
    {
      // apply umask
      if ( ::fchmod( ::fileno(file), filesystem::applyUmaskTo( 0644 ) ) )
      {
        ERR << "Failed to chmod file " << destNew << endl;
      }
      ::fclose( file );

      // move the temp file into dest
      if ( rename( destNew, dest ) != 0 ) {
        ERR << "Rename failed" << endl;
        ZYPP_THROW(MediaWriteException(dest));
      }
    }
    else
    {
      // close and remove the temp file
      ::fclose( file );
      filesystem::unlink( destNew );
    }

    DBG << "done: " << PathInfo(dest) << endl;
}


///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : MediaCurl::getDir
//        METHOD TYPE : PMError
//
//        DESCRIPTION : Asserted that media is attached
//
void MediaCurl::getDir( const Pathname & dirname, bool recurse_r ) const
{
  filesystem::DirContent content;
  getDirInfo( content, dirname, /*dots*/false );

  for ( filesystem::DirContent::const_iterator it = content.begin(); it != content.end(); ++it ) {
      Pathname filename = dirname + it->name;
      int res = 0;

      switch ( it->type ) {
      case filesystem::FT_NOT_AVAIL: // old directory.yast contains no typeinfo at all
      case filesystem::FT_FILE:
        getFile( filename );
        break;
      case filesystem::FT_DIR: // newer directory.yast contain at least directory info
        if ( recurse_r ) {
          getDir( filename, recurse_r );
        } else {
          res = assert_dir( localPath( filename ) );
          if ( res ) {
            WAR << "Ignore error (" << res <<  ") on creating local directory '" << localPath( filename ) << "'" << endl;
          }
        }
        break;
      default:
        // don't provide devices, sockets, etc.
        break;
      }
  }
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : MediaCurl::getDirInfo
//        METHOD TYPE : PMError
//
//        DESCRIPTION : Asserted that media is attached and retlist is empty.
//
void MediaCurl::getDirInfo( std::list<std::string> & retlist,
                               const Pathname & dirname, bool dots ) const
{
  getDirectoryYast( retlist, dirname, dots );
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : MediaCurl::getDirInfo
//        METHOD TYPE : PMError
//
//        DESCRIPTION : Asserted that media is attached and retlist is empty.
//
void MediaCurl::getDirInfo( filesystem::DirContent & retlist,
                            const Pathname & dirname, bool dots ) const
{
  getDirectoryYast( retlist, dirname, dots );
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : MediaCurl::progressCallback
//        METHOD TYPE : int
//
//        DESCRIPTION : Progress callback triggered from MediaCurl::getFile
//
int MediaCurl::progressCallback( void *clientp,
                                 double dltotal, double dlnow,
                                 double ultotal, double ulnow)
{
  ProgressData *pdata = reinterpret_cast<ProgressData *>(clientp);
  if( pdata)
  {
    time_t now   = time(NULL);
    if( now > 0)
    {
    	// reset time of last change in case initial time()
	// failed or the time was adjusted (goes backward)
	if( pdata->ltime <= 0 || pdata->ltime > now)
	{
	  pdata->ltime = now;
	}

	// start time counting as soon as first data arrives
	// (skip the connection / redirection time at begin)
	time_t dif = 0;
	if (dlnow > 0 || ulnow > 0)
	{
    	  dif = (now - pdata->ltime);
	  dif = dif > 0 ? dif : 0;

	  pdata->secs += dif;
	}

	// update the drate_avg and drate_period only after a second has passed
	// (this callback is called much more often than a second)
	// otherwise the values would be far from accurate when measuring
	// the time in seconds
	//! \todo more accurate download rate computationn, e.g. compute average value from last 5 seconds, or work with milliseconds instead of seconds

        if ( pdata->secs > 1 && (dif > 0 || dlnow == dltotal ))
          pdata->drate_avg = (dlnow / pdata->secs);

	if ( dif > 0 )
	{
	  pdata->drate_period = ((dlnow - pdata->dload_period) / dif);
	  pdata->dload_period = dlnow;
	}
    }

    // send progress report first, abort transfer if requested
    if( pdata->report)
    {
      if (!(*(pdata->report))->progress(int( dlnow * 100 / dltotal ),
	                                pdata->url,
	                                pdata->drate_avg,
	                                pdata->drate_period))
      {
        return 1; // abort transfer
      }
    }

    // check if we there is a timeout set
    if( pdata->timeout > 0)
    {
      if( now > 0)
      {
        bool progress = false;

        // update download data if changed, mark progress
        if( dlnow != pdata->dload)
        {
          progress     = true;
          pdata->dload = dlnow;
          pdata->ltime = now;
        }
        // update upload data if changed, mark progress
        if( ulnow != pdata->uload)
        {
          progress     = true;
          pdata->uload = ulnow;
          pdata->ltime = now;
        }

        if( !progress && (now >= (pdata->ltime + pdata->timeout)))
        {
          pdata->reached = true;
          return 1; // aborts transfer
        }
      }
    }
  }
  return 0;
}

string MediaCurl::getAuthHint() const
{
  long auth_info = CURLAUTH_NONE;

  CURLcode infoRet =
    curl_easy_getinfo(_curl, CURLINFO_HTTPAUTH_AVAIL, &auth_info);

  if(infoRet == CURLE_OK)
  {
    return CurlAuthData::auth_type_long2str(auth_info);
  }

  return "";
}

  } // namespace media
} // namespace zypp
//
