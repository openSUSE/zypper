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

#include <zypp/base/Logger.h>
#include <zypp/media/MediaDIR.h>

#include <sys/mount.h>
#include <errno.h>

using std::endl;

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
    //	DESCRIPTION : Attach point is always url_r.getPathName(),
    //                as files are not copied.
    //                Thus attach_point_hint_r is ignored.
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

      // fetch attach point from url again if needed ...
      // it may happen that attachPointHint (and attachPoint())
      // does not contain any path, because the directory has
      // not existed while the handler was constructed.
      if( attachPoint().empty() && !url().getPathName().empty())
      {
	Pathname real( getRealPath(url().getPathName()));

	PathInfo adir( real);
	if( adir.isDir())
	{
	  // set attachpoint only if the dir exists
	  setAttachPoint( real, false);
	}
	else
	{
          ZYPP_THROW(MediaBadUrlException(url(),
            "Specified path '" + url().getPathName() + "' is not a directory"
	  ));
	}
      }

      // attach point is same as source path... we do not mount here
      if(attachPoint().empty())
      {
        ZYPP_THROW(MediaBadUrlException(url(),
          "The media URL does not provide any useable directory path"
	));
      }
      else
      if(!PathInfo(attachPoint()).isDir())
      {
        ZYPP_THROW(MediaBadUrlException(url(),
	  "Specified path '" + attachPoint().asString() + "' is not a directory"
	));
      }

      MediaSourceRef media(new MediaSource("dir", attachPoint().asString()));
      setMediaSource(media);
    }


    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDIR::releaseFrom
    //	METHOD TYPE : void
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaDIR::releaseFrom( const std::string & ejectDev )
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
    void MediaDIR::getFile( const OnMediaLocation &file, const ByteCount &expectedFileSize_r ) const
    {
      MediaHandler::getFile( file, expectedFileSize_r );
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

    bool MediaDIR::getDoesFileExist( const Pathname & filename ) const
    {
      return MediaHandler::getDoesFileExist( filename );
    }

  } // namespace media
} // namespace zypp
