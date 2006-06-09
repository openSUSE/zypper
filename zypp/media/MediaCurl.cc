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

#include "zypp/base/Logger.h"
#include "zypp/ExternalProgram.h"
#include "zypp/base/String.h"
#include "zypp/base/Sysconfig.h"

#include "zypp/media/MediaCurl.h"
#include "zypp/media/proxyinfo/ProxyInfos.h"
#include "zypp/media/ProxyInfo.h"
#include "zypp/thread/Once.h"
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>

#include "config.h"

#define  DETECT_DIR_INDEX       0

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
}

namespace zypp {
  namespace media {

Pathname    MediaCurl::_cookieFile = "/var/lib/YaST2/cookies";
std::string MediaCurl::_agent = "Novell ZYPP Installer";

///////////////////////////////////////////////////////////////////

static inline void escape( string & str_r,
			   const char char_r, const string & escaped_r ) {
  for ( string::size_type pos = str_r.find( char_r );
	pos != string::npos; pos = str_r.find( char_r, pos ) ) {
    str_r.replace( pos, 1, escaped_r );
  }
}

static inline string escapedPath( string path_r ) {
  escape( path_r, ' ', "%20" );
  return path_r;
}

static inline string unEscape( string text_r ) {
  char * tmp = curl_unescape( text_r.c_str(), 0 );
  string ret( tmp );
  curl_free( tmp );
  return ret;
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : MediaCurl
//
///////////////////////////////////////////////////////////////////

MediaCurl::MediaCurl( const Url &      url_r,
		      const Pathname & attach_point_hint_r )
    : MediaHandler( url_r, attach_point_hint_r,
		    "/", // urlpath at attachpoint
		    true ), // does_download
      _curl( NULL )
{
  _curlError[0] = '\0';

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
//	METHOD NAME : MediaCurl::attachTo
//	METHOD TYPE : PMError
//
//	DESCRIPTION : Asserted that not already attached, and attachPoint is a directory.
//
void MediaCurl::attachTo (bool next)
{
  if ( next )
    ZYPP_THROW(MediaNotSupportedException(_url));

  if ( !_url.isValid() )
    ZYPP_THROW(MediaBadUrlException(_url));

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

  /*
  ** Don't block "forever" on system calls. Curl seems to
  ** recover nicely, if the ftp server has e.g. a 30sec
  ** timeout. If required, it closes the connection, trys
  ** to reopen and fetch it - this works in many cases
  ** without to report any error to us.
  **
  ** Disabled, because it breaks normal operations over a
  ** slow link :(
  **
  ret = curl_easy_setopt( _curl, CURLOPT_TIMEOUT, 600 );
  if ( ret != 0 ) {
    disconnectFrom();
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }
  */

  /*
  ** Connect timeout
  */
  ret = curl_easy_setopt( _curl, CURLOPT_CONNECTTIMEOUT, 60);
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
    ret = curl_easy_setopt ( _curl, CURLOPT_USERAGENT, _agent.c_str() );
    if ( ret != 0) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
  }

  if ( _url.getScheme() == "https" ) {
    ret = curl_easy_setopt( _curl, CURLOPT_SSL_VERIFYPEER, 1 );
    if ( ret != 0 ) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
    ret = curl_easy_setopt( _curl, CURLOPT_CAPATH, "/etc/ssl/certs/" );
    if ( ret != 0 ) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
    ret = curl_easy_setopt( _curl, CURLOPT_SSL_VERIFYHOST, 2 );
    if ( ret != 0 ) {
      disconnectFrom();
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
    ret = curl_easy_setopt ( _curl, CURLOPT_USERAGENT, _agent.c_str() );
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

    if( !_url.getQueryParam("auth").empty() &&
	(_url.getScheme() == "http" || _url.getScheme() == "https"))
    {
      std::vector<std::string>                 list;
      std::vector<std::string>::const_iterator it;
      str::split(_url.getQueryParam("auth"), std::back_inserter(list), ",");

      long auth = CURLAUTH_NONE;
      for(it = list.begin(); it != list.end(); ++it)
      {
	if(*it == "basic")
	{
	  auth |= CURLAUTH_BASIC;
	}
	else
	if(*it == "digest")
	{
	  auth |= CURLAUTH_DIGEST;
	}
	else
	if((curl_info && (curl_info->features & CURL_VERSION_NTLM)) &&
	   (*it == "ntlm"))
	{
	  auth |= CURLAUTH_NTLM;
	}
	else
	if((curl_info && (curl_info->features & CURL_VERSION_SPNEGO)) &&
	   (*it == "spnego" || *it == "negotiate"))
	{
	  // there is no separate spnego flag for auth
	  auth |= CURLAUTH_GSSNEGOTIATE;
	}
	else
	if((curl_info && (curl_info->features & CURL_VERSION_GSSNEGOTIATE)) &&
	   (*it == "gssnego" || *it == "negotiate"))
	{
	  auth |= CURLAUTH_GSSNEGOTIATE;
	}
	else
	{
	  std::string msg("Unsupported HTTP authentication method '");
	  msg += *it;
	  msg += "'";
      	  disconnectFrom();
	  ZYPP_THROW(MediaBadUrlException(_url, msg));
	}
      }

      if( auth != CURLAUTH_NONE)
      {
	DBG << "Enabling HTTP authentication methods: "
	    << _url.getQueryParam("auth") << std::endl;

	ret = curl_easy_setopt( _curl, CURLOPT_HTTPAUTH, auth);
	if ( ret != 0 ) {
      	  disconnectFrom();
	  ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
	}
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
      char *home = getenv("HOME");
      if( home && *home)
      {
      	Pathname curlrcFile = string( home ) + string( "/.curlrc" );

        PathInfo h_info(string(home), PathInfo::LSTAT);
	PathInfo c_info(curlrcFile,   PathInfo::LSTAT);

        if( h_info.isDir()  && h_info.owner() == getuid() &&
            c_info.isFile() && c_info.owner() == getuid())
	{
      	  map<string,string> rc_data = base::sysconfig::read( curlrcFile );

      	  map<string,string>::const_iterator it = rc_data.find("proxy-user");
      	  if (it != rc_data.end())
	    _proxyuserpwd = it->second;
        }
	else
	{
	  WAR << "Not allowed to parse '" << curlrcFile
	      << "': bad file owner" << std::endl;
	}
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
//	METHOD NAME : MediaCurl::disconnectFrom
//	METHOD TYPE : PMError
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
//	METHOD NAME : MediaCurl::releaseFrom
//	METHOD TYPE : PMError
//
//	DESCRIPTION : Asserted that media is attached.
//
void MediaCurl::releaseFrom( bool eject )
{
  disconnect();
}


///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : MediaCurl::getFile
//	METHOD TYPE : PMError
//

void MediaCurl::getFile( const Pathname & filename ) const
{
    // Use absolute file name to prevent access of files outside of the
    // hierarchy below the attach point.
    getFileCopy(filename, localPath(filename).absolutename());
}


void MediaCurl::getFileCopy( const Pathname & filename , const Pathname & target) const
{
  callback::SendReport<DownloadProgressReport> report;

  Url url( _url );

  try {
    doGetFileCopy(filename, target, report);
  }
  catch (MediaException & excpt_r)
  {
    // FIXME: this will not match the first URL
    // FIXME: error number fix
    report->finish(url, zypp::media::DownloadProgressReport::NOT_FOUND, excpt_r.msg());
    ZYPP_RETHROW(excpt_r);
  }
  report->finish(url, zypp::media::DownloadProgressReport::NO_ERROR, "");
}

void MediaCurl::doGetFileCopy( const Pathname & filename , const Pathname & target, callback::SendReport<DownloadProgressReport> & report) const
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
    report->start(url, dest);
    if ( curl_easy_setopt( _curl, CURLOPT_PROGRESSDATA, &report ) != 0 ) {
      WAR << "Can't set CURLOPT_PROGRESSDATA: " << _curlError << endl;;
    }

    ret = curl_easy_perform( _curl );

    if ( curl_easy_setopt( _curl, CURLOPT_PROGRESSDATA, NULL ) != 0 ) {
      WAR << "Can't unset CURLOPT_PROGRESSDATA: " << _curlError << endl;;
    }

    if ( ret != 0 ) {
      ::fclose( file );
      filesystem::unlink( destNew );
      ERR << "curl error: " << ret << ": " << _curlError << endl;
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
                err = " Login failed";
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
          err = "User abort";
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

    mode_t mask;
    // getumask() would be fine, but does not exist
    // [ the linker can't find it in glibc :-( ].
    mask = ::umask(0022); ::umask(mask);
    if ( ::fchmod( ::fileno(file), 0644 & ~mask))
    {
      ERR << "Failed to chmod file " << destNew << endl;
    }
    ::fclose( file );

    if ( rename( destNew, dest ) != 0 ) {
      ERR << "Rename failed" << endl;
      ZYPP_THROW(MediaWriteException(dest));
    }
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaCurl::getDir
//	METHOD TYPE : PMError
//
//	DESCRIPTION : Asserted that media is attached
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
//	METHOD NAME : MediaCurl::getDirInfo
//	METHOD TYPE : PMError
//
//	DESCRIPTION : Asserted that media is attached and retlist is empty.
//
void MediaCurl::getDirInfo( std::list<std::string> & retlist,
			       const Pathname & dirname, bool dots ) const
{
  getDirectoryYast( retlist, dirname, dots );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaCurl::getDirInfo
//	METHOD TYPE : PMError
//
//	DESCRIPTION : Asserted that media is attached and retlist is empty.
//
void MediaCurl::getDirInfo( filesystem::DirContent & retlist,
                            const Pathname & dirname, bool dots ) const
{
  getDirectoryYast( retlist, dirname, dots );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaCurl::progressCallback
//	METHOD TYPE : int
//
//	DESCRIPTION : Progress callback triggered from MediaCurl::getFile
//
int MediaCurl::progressCallback( void *clientp, double dltotal, double dlnow,
                                 double ultotal, double ulnow )
{
  callback::SendReport<DownloadProgressReport> *report
    = reinterpret_cast<callback::SendReport<DownloadProgressReport>*>( clientp );
  if (report)
  {
    // FIXME: empty URL
    if (! (*report)->progress(int( dlnow * 100 / dltotal ), Url() ))
      return 1;
  }
  return 0;
}


  } // namespace media
} // namespace zypp
