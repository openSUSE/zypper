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
#include <sstream>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/media/MediaNFS.h>
#include <zypp/media/Mount.h>

#include <dirent.h>

using std::endl;

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

      std::string path = _url.getHost();
      path += ':';
      path += Pathname(_url.getPathName()).asString();

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

      if( !isUseableAttachPoint( attachPoint() ) )
      {
	setAttachPoint( createAttachPoint(), true );
      }
      std::string mountpoint( attachPoint().asString() );

      std::string filesystem( _url.getScheme() );
      if ( filesystem != "nfs4" && _url.getQueryParam("type") == "nfs4" )
      {
        filesystem = "nfs4";
      }

      std::string options = _url.getQueryParam("mountoptions");
      if(options.empty())
      {
    	options="ro";
      }

      std::vector<std::string> optionList;
      str::split( options, std::back_inserter(optionList), "," );
      std::vector<std::string>::const_iterator it;
      bool contains_lock  = false, contains_soft = false,
           contains_timeo = false, contains_hard = false;

      for( it = optionList.begin(); it != optionList.end(); ++it ) {
        if ( *it == "lock" || *it == "nolock" ) contains_lock = true;
        else if ( *it == "soft") contains_soft = true;
        else if ( *it == "hard") contains_hard = true;
	else if ( it->find("timeo") != std::string::npos ) contains_timeo = true;
      }

      if ( !(contains_lock && contains_soft) ) {
        // Add option "nolock", unless option "lock" or "unlock" is already set.
        // This should prevent the mount command hanging when the portmapper isn't
        // running.
        if ( !contains_lock ) {
          optionList.push_back( "nolock" );
        }
        // Add options "soft,timeo=NFS_MOUNT_TIMEOUT", unless they are set
        // already or "hard" option is explicitly specified. This prevent
        // the mount command from hanging when the nfs server is not responding
        // and file transactions from an unresponsive to throw an error after
        // a short time instead of hanging forever
        if ( !(contains_soft || contains_hard) ) {
          optionList.push_back( "soft" );
          if ( !contains_timeo ) {
	    std::ostringstream s;
            s << "timeo=" << NFS_MOUNT_TIMEOUT;
            optionList.push_back( s.str() );
          }
        }
        options = str::join( optionList, "," );
      }

      Mount mount;
      mount.mount(path,mountpoint,filesystem,options);

      setMediaSource(media);

      // wait for /etc/mtab update ...
      // (shouldn't be needed)
      int limit = 3;
      bool mountsucceeded;
      while( !(mountsucceeded=isAttached()) && --limit)
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
        ZYPP_THROW(MediaMountException(
          "Unable to verify that the media was mounted",
	  path, mountpoint
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
      return checkAttached(true);
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //  METHOD NAME : MediaNFS::releaseFrom
    //  METHOD TYPE : void
    //
    //  DESCRIPTION : Asserted that media is attached.
    //
    void MediaNFS::releaseFrom( const std::string & ejectDev )
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
    void MediaNFS::getFile (const OnMediaLocation &file, const ByteCount &expectedFileSize_r) const
    {
      MediaHandler::getFile( file, expectedFileSize_r );
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

    bool MediaNFS::getDoesFileExist( const Pathname & filename ) const
    {
      return MediaHandler::getDoesFileExist( filename );
    }


  } // namespace media
} // namespace zypp
