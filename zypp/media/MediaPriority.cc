/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/media/MediaPriority.cc
 *
*/
#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/Url.h"
#include "zypp/ZConfig.h"

#include "zypp/media/MediaPriority.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace media
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      /**
       * 4: local:     file,dir,hd
       * 3: remote:    nfs,cifs,smb
       * ?: download:  http,https,ftp,sftp, tftp
       * ?: volatile:  cd,dvd
       * 0:            the rest
      */
      MediaPriority::value_type scheme2priority(  const std::string & scheme_r )
      {
	switch ( scheme_r[0] )
	{
#define RETURN_IF(scheme,value) \
	if ( ::strcmp( scheme+1, scheme_r.c_str()+1 ) == 0 ) return value;
	  case 'c':
	    RETURN_IF( "cd",	ZConfig::instance().download_media_prefer_download() ? 1 : 2 );
	    RETURN_IF( "cifs",	3 );
	    break;

	  case 'd':
	    RETURN_IF( "dvd",	ZConfig::instance().download_media_prefer_download() ? 1 : 2 );
	    RETURN_IF( "dir",	4 );
	    break;

	  case 'f':
	    RETURN_IF( "file",	4 );
	    RETURN_IF( "ftp",	ZConfig::instance().download_media_prefer_download() ? 2 : 1);
	    break;
	  
	  case 't':
	    RETURN_IF( "tftp",	ZConfig::instance().download_media_prefer_download() ? 2 : 1);
	    break;

	  case 'h':
	    RETURN_IF( "http",	ZConfig::instance().download_media_prefer_download() ? 2 : 1 );
	    RETURN_IF( "https",	ZConfig::instance().download_media_prefer_download() ? 2 : 1 );
	    RETURN_IF( "hd",	4 );
	    break;

	  case 'n':
	    RETURN_IF( "nfs",	3 );
	    RETURN_IF( "nfs4",	3 );
	    break;

	  case 's':
	    RETURN_IF( "sftp",	ZConfig::instance().download_media_prefer_download() ? 2 : 1 );
	    RETURN_IF( "smb",	3 );
	    break;
#undef RETURN_IF
	}
	return 0;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaPriority::MediaPriority
    //	METHOD TYPE : Ctor
    //
    MediaPriority::MediaPriority( const std::string & scheme_r )
      : _val( scheme2priority( scheme_r ) )
    {}

    MediaPriority::MediaPriority( const Url & url_r )
      : _val( scheme2priority( url_r.getScheme() ) )
    {}

    /////////////////////////////////////////////////////////////////
  } // namespace media
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
