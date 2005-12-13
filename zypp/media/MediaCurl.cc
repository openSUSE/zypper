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
//#include <y2util/SysConfig.h>

#include "zypp/media/MediaCurl.h"
//#include "zypp/media/MediaCallbacks.h"

#include <sys/types.h>
#include <sys/mount.h>
#include <errno.h>
#include <dirent.h>

#include "config.h"

using namespace std;
using namespace zypp::base;
//using namespace MediaCallbacks;

namespace zypp {
  namespace media {

Pathname MediaCurl::_cookieFile = "/var/lib/YaST2/cookies";

MediaCurl::Callbacks *MediaCurl::_callbacks = 0;

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
    ZYPP_THROW( MediaException("Error::E_not_supported_by_media") );

#warning FIXME implement check for URL validity
#if 0
  if ( !_url.isValid() )
    ZYPP_THROW( MediaException("Error::E_bad_url") );
#endif

  _curl = curl_easy_init();
  if ( !_curl ) {
    ERR << "curl easy init failed" << endl;
    ZYPP_THROW( MediaException("Error::E_error") );
  }

  _connected = true;

  CURLcode ret = curl_easy_setopt( _curl, CURLOPT_ERRORBUFFER, _curlError );
  if ( ret != 0 ) {
    ERR << "Error setting error buffer" << endl;
    ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
  }

  ret = curl_easy_setopt( _curl, CURLOPT_FAILONERROR, true );
  if ( ret != 0 ) {
    ERR << _curlError << endl;
    ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
  }

  if ( _url.getScheme() == "http" ) {
    // follow any Location: header that the server sends as part of
    // an HTTP header (#113275)
    ret = curl_easy_setopt ( _curl, CURLOPT_FOLLOWLOCATION, true );
    if ( ret != 0) {
      ERR << _curlError << endl;
      ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
    }
    ret = curl_easy_setopt ( _curl, CURLOPT_MAXREDIRS, 3L );
    if ( ret != 0) {
      ERR << _curlError << endl;
      ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
    }
  }

  // XXX: wasn't that the wrong fix for some problem? this should be
  // removed
  if ( _url.getScheme() == "https" ) {
    WAR << "Disable certificate verification for https." << endl;
    ret = curl_easy_setopt( _curl, CURLOPT_SSL_VERIFYPEER, 0 );
    if ( ret != 0 ) {
      ERR << _curlError << endl;
      ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
    }

    ret = curl_easy_setopt( _curl, CURLOPT_SSL_VERIFYHOST, 0 );
    if ( ret != 0 ) {
      ERR << _curlError << endl;
      ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
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
      ERR << _curlError << endl;
      ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
    }
  }

  /*---------------------------------------------------------------*
   CURLOPT_PROXY: host[:port]

   Url::option(proxy and proxyport) -> CURLOPT_PROXY
   If not provided, /etc/sysconfig/proxy is evaluated
   *---------------------------------------------------------------*/

#warning FIX once I understand how to get this info
#warning FIXME enable proxy via sysconfig somehow
#if 0
  _proxy = _url.option( "proxy" );

  if ( ! _proxy.empty() ) {
    string proxyport( _url.option( "proxyport" ) );
    if ( ! proxyport.empty() ) {
      _proxy += ":" + proxyport;
    }
  } else {

    SysConfig cfg( "proxy" );

    if ( cfg.readBoolEntry( "PROXY_ENABLED", false ) ) {
      bool useproxy = true;

      std::vector<std::string> nope;
      stringutil::split( cfg.readEntry( "NO_PROXY" ), nope, ", \t" );
      for ( unsigned i = 0; i < nope.size(); ++i ) {
	// no proxy: if nope equals host,
	// or is a suffix preceeded by a '.'
	string::size_type pos = _url.getHost().find( nope[i] );
	if ( pos != string::npos
	     && ( pos + nope[i].size() == _url.getHost().size() )
	     && ( pos == 0 || _url.getHost()[pos -1] == '.' ) ) {
	  DBG << "NO_PROXY: " << nope[i] << " matches host " << _url.getHost() << endl;
	  useproxy = false;
	  break;
	}
      }

      if ( useproxy ) {
	if ( _url.getScheme() == Url::ftp ) {
	  _proxy = cfg.readEntry( "FTP_PROXY" );
	} else if ( _url.getScheme() == Url::http ) {
	  _proxy = cfg.readEntry( "HTTP_PROXY" );
	} else if ( _url.getScheme() == Url::https ) {
	  _proxy = cfg.readEntry( "HTTPS_PROXY" );
	}
      }
    }
  }
#endif


  DBG << "Proxy: " << _proxy << endl;

  if ( ! _proxy.empty() ) {

    ret = curl_easy_setopt( _curl, CURLOPT_PROXY, _proxy.c_str() );
    if ( ret != 0 ) {
      ERR << _curlError << endl;
      ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
    }

    /*---------------------------------------------------------------*
     CURLOPT_PROXYUSERPWD: [user name]:[password]

     Url::option(proxyuser and proxypassword) -> CURLOPT_PROXYUSERPWD
     If not provided, $HOME/.curlrc is evaluated
     *---------------------------------------------------------------*/

#warning FIXME once I know how to get this info
#warning FIXME enable proxy via sysconfig somehow
#if 0
    _proxyuserpwd = _url.option( "proxyuser" );

    if ( ! _proxyuserpwd.empty() ) {

      string proxypassword( _url.option( "proxypassword" ) );
      if ( ! proxypassword.empty() ) {
	_proxyuserpwd += ":" + proxypassword;
      }

    } else {

      string curlrcFile = string( getenv("HOME") ) + string( "/.curlrc" );
      SysConfig curlrc( curlrcFile );
      _proxyuserpwd = curlrc.readEntry( "proxy-user" );

    }
#endif

    _proxyuserpwd = unEscape( _proxyuserpwd );
    ret = curl_easy_setopt( _curl, CURLOPT_PROXYUSERPWD, _proxyuserpwd.c_str() );
    if ( ret != 0 ) {
      ERR << _curlError << endl;
      ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
    }

  }

  /*---------------------------------------------------------------*
   *---------------------------------------------------------------*/

  _currentCookieFile = _cookieFile.asString();

  ret = curl_easy_setopt( _curl, CURLOPT_COOKIEFILE,
                          _currentCookieFile.c_str() );
  if ( ret != 0 ) {
    ERR << _curlError << endl;
    ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
  }

  ret = curl_easy_setopt( _curl, CURLOPT_COOKIEJAR,
                          _currentCookieFile.c_str() );
  if ( ret != 0 ) {
    ERR << _curlError << endl;
    ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
  }

  ret = curl_easy_setopt( _curl, CURLOPT_PROGRESSFUNCTION,
                          &MediaCurl::progressCallback );
  if ( ret != 0 ) {
    ERR << _curlError << endl;
    ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
  }

  ret = curl_easy_setopt( _curl, CURLOPT_NOPROGRESS, false );
  if ( ret != 0 ) {
    ERR << _curlError << endl;
    ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
  }
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
    DBG << filename.asString() << endl;

#warning FIXME implement check for URL validity
#if 0
    if(!_url.isValid())
      ZYPP_THROW( MediaException(string("Error::E_bad_url") + " " + _url.toString()) );
#endif

    if(_url.getHost().empty())
      ZYPP_THROW( MediaException("Error::E_no_host_specified") );

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
    url.setPathName( escapedPath(path) );

    Pathname dest = target.absolutename();
    string destNew = target.asString() + ".new.yast.37456";

    DBG << "dest: " << dest << endl;
    DBG << "destNew: " << destNew << endl;

    if( assert_dir( dest.dirname() ) )
    {
      DBG << "assert_dir " << dest.dirname() << " failed" << endl;
      ZYPP_THROW( MediaException(string("Error::E_system") + string(" ") + dest.dirname().asString()) );
    }

    DBG << "URL: " << url.toString().c_str() << endl;
    // Use URL without options (not RFC conform) and without
    // username and passwd (some proxies dislike them in the URL.
    // Curloptions for these were set in attachTo().
    Url curlUrl( url );
    curlUrl.setUsername( "" );
    curlUrl.setPassword( "" );
#warning Check whether the call is correct
//    string urlBuffer = curlUrl.toString(true,false,true); // without options
    string urlBuffer = curlUrl.toString(); // without options

    CURLcode ret = curl_easy_setopt( _curl, CURLOPT_URL,
                                     urlBuffer.c_str() );
    if ( ret != 0 ) {
      ERR << _curlError << endl;
      ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
    }

    FILE *file = fopen( destNew.c_str(), "w" );
    if ( !file ) {
      ERR << "fopen failed for file '" << destNew << "'" << endl;
      ZYPP_THROW( MediaException(string("Error::E_write_error") + string(" ") + destNew) );
    }

    ret = curl_easy_setopt( _curl, CURLOPT_WRITEDATA, file );
    if ( ret != 0 ) {
      fclose( file );
      ERR << _curlError << endl;
      ZYPP_THROW( MediaException("Error::E_curl_setopt_failed") );
    }

    // Set callback and perform.
#warning FIXME reenable callback
#if 0
    DownloadProgressReport::Send report( downloadProgressReport );
    report->start( url, dest );
    if ( curl_easy_setopt( _curl, CURLOPT_PROGRESSDATA, &report ) != 0 ) {
      WAR << "Can't set CURLOPT_PROGRESSDATA: " << _curlError << endl;;
    }
#endif

    ret = curl_easy_perform( _curl );
    fclose( file );

    if ( curl_easy_setopt( _curl, CURLOPT_PROGRESSDATA, NULL ) != 0 ) {
      WAR << "Can't unset CURLOPT_PROGRESSDATA: " << _curlError << endl;;
    }

    if ( ret != 0 ) {
      filesystem::unlink( destNew );

      ERR << "curl error: " << ret << ": " << _curlError << endl;
      std::string err;
      switch ( ret ) {
        case CURLE_UNSUPPORTED_PROTOCOL:
        case CURLE_URL_MALFORMAT:
        case CURLE_URL_MALFORMAT_USER:
          err = "Error::E_bad_url";
          break;
        case CURLE_HTTP_NOT_FOUND:
          {
            long httpReturnCode;
            CURLcode infoRet = curl_easy_getinfo( _curl, CURLINFO_HTTP_CODE,
                                                  &httpReturnCode );
            if ( infoRet == CURLE_OK ) {
              string msg = "HTTP return code: " +
                           str::numstring( httpReturnCode ) +
                           " (URL: " + url.toString() + ")";
              DBG << msg << endl;
              if ( httpReturnCode == 401 )
	      {
		msg = "URL: " + url.toString();
                err = "Error::E_login_failed";
	      }
              else
	      {
		err = "Error::E_file_not_found";
	      }
#warning FIXME reenable change report
#if 0
	      report->stop( err );
#endif
	      ZYPP_THROW( MediaException(err + string(" ") + _curlError) );
            }
          }
          break;
        case CURLE_FTP_COULDNT_RETR_FILE:
        case CURLE_FTP_ACCESS_DENIED:
          err = "Error::E_file_not_found";
          break;
        case CURLE_BAD_PASSWORD_ENTERED:
        case CURLE_FTP_USER_PASSWORD_INCORRECT:
          err = "Error::E_login_failed";
          break;
        case CURLE_COULDNT_RESOLVE_PROXY:
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        case CURLE_FTP_CANT_GET_HOST:
          err = "Error::E_connection_failed";
          break;
        case CURLE_WRITE_ERROR:
          err = "Error::E_write_error";
          break;
        case CURLE_ABORTED_BY_CALLBACK:
          err = "Error::E_user_abort";
          break;
        case CURLE_SSL_PEER_CERTIFICATE:
        default:
          err = "Error::E_error";
          break;
      }

#warning FIXME reenable change report
#if 0
      report->stop( err );
#endif
      ZYPP_THROW( MediaException(err + string(" ") + _curlError) );
    }

    if ( rename( destNew, dest ) != 0 ) {
      ERR << "Rename failed" << endl;
#warning FIXME reenable change report
#if 0
      report->stop( Error::E_write_error );
#endif
      ZYPP_THROW( MediaException("Error::E_write_error") );
    }

#warning FIXME reenable change report
#if 0
    report->stop( Error::E_ok );
#endif
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
  try {
    getDirInfo( content, dirname, /*dots*/false );
  }
  catch (const MediaException & excpt_r)
  {
#warning FIXME rethrow
#if 0
    ZYPP_RETHROW(excpt_r);
#endif
  }

  for ( filesystem::DirContent::const_iterator it = content.begin(); it != content.end(); ++it ) {
    try {
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
    catch (const MediaException & excpt_r)
    {
#warning FIXME rethrow
#if 0
      ZYPP_RETHROW(excpt_r);
#endif
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
  try {
    getDirectoryYast( retlist, dirname, dots );
  }
  catch (const MediaException & excpt_r)
  {
    ZYPP_THROW( MediaException("Error::E_not_supported_by_media") );
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
void MediaCurl::getDirInfo( filesystem::DirContent & retlist,
                            const Pathname & dirname, bool dots ) const
{
  try {
    getDirectoryYast( retlist, dirname, dots );
  }
  catch (const MediaException & excpt_r)
  {
    ZYPP_THROW( MediaException("Error::E_not_supported_by_media") );
  }
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
#warning FIXME this function
#if 0
  DownloadProgressReport::Send * reportP = reinterpret_cast<DownloadProgressReport::Send*>( clientp );
  if ( reportP ) {
    ProgressData pd( 0, int(dltotal), int(dlnow) );
    if ( (*reportP)->progress( pd ) == false )
      return 1; // abort requested by user
  }

#warning YOU callbacks still active
  if ( _callbacks ) {
    if ( _callbacks->progress( int( dlnow * 100 / dltotal ) ) ) return 0;
    else return 1;
  }
#endif
  return 0;
}

  } // namespace media
} // namespace zypp
