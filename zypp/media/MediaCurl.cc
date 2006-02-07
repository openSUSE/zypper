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

#include "zypp/media/MediaCurl.h"
#include "zypp/media/proxyinfo/ProxyInfos.h"
#include "zypp/media/ProxyInfo.h"

#include <sys/types.h>
#include <sys/mount.h>
#include <errno.h>
#include <dirent.h>

#include "config.h"

using namespace std;
using namespace zypp::base;

namespace zypp {
  namespace media {

Pathname MediaCurl::_cookieFile = "/var/lib/YaST2/cookies";

bool MediaCurl::_globalInit = false;

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
      _curl( 0 ), _connected( false )
{
  MIL << "MediaCurl::MediaCurl(" << url_r << ", " << attach_point_hint_r << ")" << endl;
  if ( ! _globalInit )
    {
      _globalInit = true;
      CURLcode ret = curl_global_init( CURL_GLOBAL_ALL );
      if ( ret != 0 )
        WAR << "curl global init failed" << endl;
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

  if( attachPoint().empty() || attachPoint().asString() == "/")
  {
    std::string mountpoint = createAttachPoint().asString();
    if( mountpoint.empty())
      ZYPP_THROW( MediaBadAttachPointException(url()));
      setAttachPoint( mountpoint, true);
  }

  _curl = curl_easy_init();
  if ( !_curl ) {
    ZYPP_THROW(MediaCurlInitException(_url));
  }

  _connected = true;

  CURLcode ret = curl_easy_setopt( _curl, CURLOPT_ERRORBUFFER, _curlError );
  if ( ret != 0 ) {
    ZYPP_THROW(MediaCurlSetOptException(_url, "Error setting error buffer"));
  }

  ret = curl_easy_setopt( _curl, CURLOPT_FAILONERROR, true );
  if ( ret != 0 ) {
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  if ( _url.getScheme() == "http" ) {
    // follow any Location: header that the server sends as part of
    // an HTTP header (#113275)
    ret = curl_easy_setopt ( _curl, CURLOPT_FOLLOWLOCATION, true );
    if ( ret != 0) {
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
    ret = curl_easy_setopt ( _curl, CURLOPT_MAXREDIRS, 3L );
    if ( ret != 0) {
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
  }

  if ( _url.getScheme() == "https" ) {
    ret = curl_easy_setopt( _curl, CURLOPT_SSL_VERIFYPEER, 1 );
    if ( ret != 0 ) {
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
    ret = curl_easy_setopt( _curl, CURLOPT_CAPATH, "/etc/ssl/certs/" );
    if ( ret != 0 ) {
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
    ret = curl_easy_setopt( _curl, CURLOPT_SSL_VERIFYHOST, 2 );
    if ( ret != 0 ) {
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
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
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
	// no proxy: if nope equals host,
	// or is a suffix preceeded by a '.'
	string::size_type pos = _url.getHost().find( *it );
	if ( pos != string::npos
	     && ( pos + it->size() == _url.getHost().size() )
	     && ( pos == 0 || _url.getHost()[pos -1] == '.' ) ) {
	  DBG << "NO_PROXY: " << *it << " matches host " << _url.getHost() << endl;
	  useproxy = false;
	  break;
	}
      }

      if ( useproxy ) {
	_proxy = proxy_info.proxy(_url.getScheme());
      }
    }
  }


  DBG << "Proxy: " << _proxy << endl;

  if ( ! _proxy.empty() ) {

    ret = curl_easy_setopt( _curl, CURLOPT_PROXY, _proxy.c_str() );
    if ( ret != 0 ) {
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

      string curlrcFile = string( getenv("HOME") ) + string( "/.curlrc" );
      map<string,string> rc_data
	= proxyinfo::sysconfigRead(Pathname(curlrcFile));
      map<string,string>::const_iterator it = rc_data.find("proxy-user");
      if (it != rc_data.end())
	_proxyuserpwd = it->second;
    }

    _proxyuserpwd = unEscape( _proxyuserpwd );
    ret = curl_easy_setopt( _curl, CURLOPT_PROXYUSERPWD, _proxyuserpwd.c_str() );
    if ( ret != 0 ) {
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }
  }

  /*---------------------------------------------------------------*
   *---------------------------------------------------------------*/

  _currentCookieFile = _cookieFile.asString();

  ret = curl_easy_setopt( _curl, CURLOPT_COOKIEFILE,
                          _currentCookieFile.c_str() );
  if ( ret != 0 ) {
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  ret = curl_easy_setopt( _curl, CURLOPT_COOKIEJAR,
                          _currentCookieFile.c_str() );
  if ( ret != 0 ) {
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  ret = curl_easy_setopt( _curl, CURLOPT_PROGRESSFUNCTION,
                          &progressCallback );
  if ( ret != 0 ) {
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  ret = curl_easy_setopt( _curl, CURLOPT_NOPROGRESS, false );
  if ( ret != 0 ) {
    ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
  }

  // FIXME: need a derived class to propelly compare url's
  MediaSourceRef media( new MediaSource(_url.getScheme(), _url.asString()));
  setMediaSource(media);
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaCurl::disconnectFrom
//	METHOD TYPE : PMError
//
void MediaCurl::disconnectFrom()
{
  if ( _connected ) curl_easy_cleanup( _curl );
  _connected = false ;
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
    string destNew = target.asString() + ".new.zypp.37456";

    DBG << "dest: " << dest << endl;
    DBG << "destNew: " << destNew << endl;

    if( assert_dir( dest.dirname() ) )
    {
      DBG << "assert_dir " << dest.dirname() << " failed" << endl;
      ZYPP_THROW( MediaSystemException(_url, "System error on " + dest.dirname().asString()) );
    }

    DBG << "URL: " << url.asString().c_str() << endl;
    // Use URL without options (not RFC conform) and without
    // username and passwd (some proxies dislike them in the URL.
    // Curloptions for these were set in attachTo().
    Url curlUrl( url );
    curlUrl.setUsername( "" );
    curlUrl.setPassword( "" );
#warning Check whether the call is correct
//    string urlBuffer = curlUrl.asString(true,false,true); // without options
    string urlBuffer = curlUrl.asString(); // without options

    CURLcode ret = curl_easy_setopt( _curl, CURLOPT_URL,
                                     urlBuffer.c_str() );
    if ( ret != 0 ) {
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }

    FILE *file = fopen( destNew.c_str(), "w" );
    if ( !file ) {
      ERR << "fopen failed for file '" << destNew << "'" << endl;
      ZYPP_THROW(MediaWriteException(destNew));
    }

    ret = curl_easy_setopt( _curl, CURLOPT_WRITEDATA, file );
    if ( ret != 0 ) {
      fclose( file );
      ZYPP_THROW(MediaCurlSetOptException(_url, _curlError));
    }

    // Set callback and perform.
    report->start(url, dest);
    if ( curl_easy_setopt( _curl, CURLOPT_PROGRESSDATA, &report ) != 0 ) {
      WAR << "Can't set CURLOPT_PROGRESSDATA: " << _curlError << endl;;
    }

    ret = curl_easy_perform( _curl );
    fclose( file );

    if ( curl_easy_setopt( _curl, CURLOPT_PROGRESSDATA, NULL ) != 0 ) {
      WAR << "Can't unset CURLOPT_PROGRESSDATA: " << _curlError << endl;;
    }

    if ( ret != 0 ) {
      filesystem::unlink( destNew );

      ERR << "curl error: " << ret << ": " << _curlError << endl;
      std::string err;
      try {
       switch ( ret ) {
        case CURLE_UNSUPPORTED_PROTOCOL:
        case CURLE_URL_MALFORMAT:
        case CURLE_URL_MALFORMAT_USER:
	  err = "Bad URL";
        case CURLE_HTTP_NOT_FOUND:
          {
            long httpReturnCode;
            CURLcode infoRet = curl_easy_getinfo( _curl, CURLINFO_HTTP_CODE,
                                                  &httpReturnCode );
            if ( infoRet == CURLE_OK ) {
              string msg = "HTTP return code: " +
                           str::numstring( httpReturnCode ) +
                           " (URL: " + url.asString() + ")";
              DBG << msg << endl;
              if ( httpReturnCode == 401 )
	      {
		msg = "URL: " + url.asString();
                err = "Login failed";
	      }
              else
	      {
		err = "File not found";
	      }
	      ZYPP_THROW( MediaCurlException(_url, err, _curlError));
            }
          }
          break;
        case CURLE_FTP_COULDNT_RETR_FILE:
        case CURLE_FTP_ACCESS_DENIED:
          err = "File not found";
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
       ZYPP_THROW(MediaCurlException(_url, err, _curlError));
      }
      catch (const MediaException & excpt_r)
      {
	ZYPP_RETHROW(excpt_r);
      }
    }

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
