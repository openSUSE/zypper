/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaDIR.cc
 *
*/

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/media/MediaDIR.h"

#include <sys/mount.h>
#include <errno.h>

using namespace std;

namespace zypp {
  namespace media {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaDIR
    //
    ///////////////////////////////////////////////////////////////////
    
    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDIR::MediaDIR
    //	METHOD TYPE : Constructor
    //
    //	DESCRIPTION : Attach point is always '/', as files are not copied.
    //                    Thus attach_point_hint_r is ignored.
    //
    MediaDIR::MediaDIR( const Url &      url_r,
    			const Pathname & /*attach_point_hint_r*/ )
        : MediaHandler( url_r, url_r.getPathName(),
    		    "/",    // urlpath below attachpoint
    		    false ) // does_download
    {
	MIL << "MediaDIR::MediaDIR(" << url_r << ")" << endl;
	if( !url_r.getHost().empty())
	{
	  ZYPP_THROW(MediaBadUrlException(url_r,
	    "Hostname not allowed in the Url"
	  ));
	}
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDIR::attachTo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that not already attached, and attachPoint is a directory.
    //
    void MediaDIR::attachTo(bool next)
    {
      if(next)
	ZYPP_THROW(MediaNotSupportedException(url()));

      //
      // see ctor - attach point is equal source path
      //
      // don't allow to specify the attachpoint of an
      // another media handler as source path, e.g.:
      //   open("cd:///", "/mnt") && attach()
      //   open("dir:/mnt", "")   && attach()
      //
      // still allows to use hand mounted media paths.
      //
      if( !isUseableAttachPoint(attachPoint(), false))
      {
	ZYPP_THROW(MediaBadUrlException(url(),
	  "Specified path is not allowed as media source"
	));
      }

      MediaSourceRef media(new MediaSource("dir", attachPoint().asString()));
      setMediaSource(media);
    }


    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDIR::releaseFrom
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaDIR::releaseFrom( bool eject )
    {
      return;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDIR::getFile
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaDIR::getFile( const Pathname & filename ) const
    {
      MediaHandler::getFile( filename );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaDIR::getDir
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaDIR::getDir( const Pathname & dirname, bool recurse_r ) const
    {
      MediaHandler::getDir( dirname, recurse_r );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDIR::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaDIR::getDirInfo( std::list<std::string> & retlist,
    			       const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDIR::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaDIR::getDirInfo( filesystem::DirContent & retlist,
    			       const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }

  } // namespace media
} // namespace zypp
