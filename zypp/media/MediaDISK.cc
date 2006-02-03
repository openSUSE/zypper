/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaDISK.h
 *
*/

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/media/Mount.h"
#include "zypp/media/MediaDISK.h"

#include <sys/types.h>
#include <sys/mount.h>
#include <errno.h>
#include <dirent.h>

using namespace std;

namespace zypp {
  namespace media {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaDISK
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDISK::MediaDISK
    //	METHOD TYPE : Constructor
    //
    //	DESCRIPTION :
    //
    MediaDISK::MediaDISK( const Url &      url_r,
			  const Pathname & attach_point_hint_r )
        : MediaHandler( url_r, attach_point_hint_r,
    		    url_r.getPathName(), // urlpath below attachpoint
    		    false ) // does_download
    {
      _device = _url.getQueryParam("device");
      _filesystem = _url.getQueryParam("filesystem");
      if(_filesystem.empty())
	_filesystem="auto";
	MIL << "MediaDISK::MediaDISK(" << url_r << ", " << attach_point_hint_r << ")" << endl;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDISK::attachTo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that not already attached, and attachPoint is a directory.
    //
    void MediaDISK::attachTo(bool next)
    {
      if(next)
	ZYPP_THROW(MediaNotSupportedException(url()));
      // FIXME
      // do mount --bind <partition>/<dir> to <to>
      //   mount /dev/<partition> /tmp_mount
      //   mount /tmp_mount/<dir> <to> --bind -o ro
      // FIXME: try all filesystems
    
      if(_device.empty())
	ZYPP_THROW(MediaBadUrlEmptyDestinationException(url()));
    
      if(_filesystem.empty())
	ZYPP_THROW(MediaBadUrlEmptyFilesystemException(url()));
    
      Mount mount;
      const char *mountpoint = attachPoint().asString().c_str();
      string options = _url.getQueryParam("mountoptions");
      if(options.empty())
      {
    	options="ro";
      }
    
      mount.mount(_device,mountpoint,_filesystem,options);
    }


    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDISK::releaseFrom
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaDISK::releaseFrom( bool eject )
    {
      Mount mount;
    
      mount.umount(attachPoint().asString());
    }


    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaDISK::getFile
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaDISK::getFile (const Pathname & filename) const
    {
      MediaHandler::getFile( filename );
    }
    
    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaDISK::getDir
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaDISK::getDir( const Pathname & dirname, bool recurse_r ) const
    {
      MediaHandler::getDir( dirname, recurse_r );
    }
    
    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDISK::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaDISK::getDirInfo( std::list<std::string> & retlist,
				const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }
    
    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDISK::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaDISK::getDirInfo( filesystem::DirContent & retlist,
				const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }

  } // namespace media
} // namespace zypp
