/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaNFS.cc
 *
*/

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/media/MediaNFS.h"
#include "zypp/media/Mount.h"

#include <dirent.h>

using namespace std;

namespace zypp {
  namespace media {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaNFS
    //
    ///////////////////////////////////////////////////////////////////
    
    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaNFS::MediaNFS
    //	METHOD TYPE : Constructor
    //
    //	DESCRIPTION :
    //
    MediaNFS::MediaNFS( const Url &      url_r,
			const Pathname & attach_point_hint_r )
      : MediaHandler( url_r, attach_point_hint_r,
		      "/", // urlpath at attachpoint
		      false ) // does_download
    {
	MIL << "MediaNFS::MediaNFS(" << url_r << ", " << attach_point_hint_r << ")" << endl;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaNFS::attachTo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that not already attached, and attachPoint is a directory.
    //
    void MediaNFS::attachTo(bool next)
    {
      if(_url.getHost().empty())
    	ZYPP_THROW(MediaBadUrlEmptyHostException(_url));
      if(next)
	ZYPP_THROW(MediaNotSupportedException(_url));

      string path = _url.getHost();
      path += ':';
      path += _url.getPathName();

      MediaSourceRef media( new MediaSource("nfs", path));
      AttachedMedia  ret( findAttachedMedia( media));

      if( ret.mediaSource &&
	  ret.attachPoint &&
	  !ret.attachPoint->empty())
      {
	DBG << "Using a shared media "
	    << ret.mediaSource->name
	    << " attached on "
	    << ret.attachPoint->path
	    << endl;

	removeAttachPoint();
	setAttachPoint(ret.attachPoint);
	setMediaSource(ret.mediaSource);
	return;
      }

      const char* const filesystem = "nfs";
      std::string       mountpoint = attachPoint().asString();
      Mount mount;

      if( !isUseableAttachPoint(attachPoint()))
      {
	mountpoint = createAttachPoint().asString();
	if( mountpoint.empty())
	  ZYPP_THROW( MediaBadAttachPointException(url()));
	setAttachPoint( mountpoint, true);
      }

      string options = _url.getQueryParam("mountoptions");
      if(options.empty())
      {
    	options="ro";
      }
    
      // Add option "nolock", unless option "lock" or "unlock" is already set.
      // This should prevent the mount command hanging when the portmapper isn't
      // running.
      vector<string> optionList;
      str::split( options, std::back_inserter(optionList), "," );
      vector<string>::const_iterator it;
      for( it = optionList.begin(); it != optionList.end(); ++it ) {
    	if ( *it == "lock" || *it == "nolock" ) break;
      }
      if ( it == optionList.end() ) {
    	optionList.push_back( "nolock" );
    	options = str::join( optionList, "," );
      }

      mount.mount(path,mountpoint.c_str(),filesystem,options);

      setMediaSource(media);

      // wait for /etc/mtab update ...
      // (shouldn't be needed)
      int limit = 10;
      bool mountsucceeded;
      while( !(mountsucceeded=isAttached()) && limit--)
      {
        sleep(1);
      }

      if( !mountsucceeded)
      {
        setMediaSource(MediaSourceRef());
        try
        {
          mount.umount(attachPoint().asString());
        }
        catch (const MediaException & excpt_r)
        {
          ZYPP_CAUGHT(excpt_r);
        }
        ZYPP_THROW(MediaMountException(path, mountpoint,
          "Unable to verify that the media was mounted"
        ));
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaNFS::isAttached
    //	METHOD TYPE : bool
    //
    //	DESCRIPTION : Override check if media is attached.
    //
    bool
    MediaNFS::isAttached() const
    {
      return checkAttached(false);
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaNFS::releaseFrom
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaNFS::releaseFrom( bool eject )
    {
      Mount mount;
      mount.umount(attachPoint().asString());
    }


    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaNFS::getFile
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaNFS::getFile (const Pathname & filename) const
    {
      MediaHandler::getFile( filename );;
    }
    
    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaNFS::getDir
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaNFS::getDir( const Pathname & dirname, bool recurse_r ) const
    {
      MediaHandler::getDir( dirname, recurse_r );
    }
    
    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaNFS::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaNFS::getDirInfo( std::list<std::string> & retlist,
    			      const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }
    
    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaNFS::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaNFS::getDirInfo( filesystem::DirContent & retlist,
    			   const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }

  } // namespace media
} // namespace zypp
