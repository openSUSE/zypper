/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaDISK.cc
 *
*/

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/media/Mount.h"
#include "zypp/media/MediaDISK.h"
#include "zypp/media/MediaManager.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <sys/types.h>
#include <sys/mount.h>
#include <errno.h>
#include <dirent.h>

#define DELAYED_VERIFY    1

#define VOL_ID_TOOL_PATHS { "/sbin/vol_id", "/lib/udev/vol_id", NULL}

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
      MIL << "MediaDISK::MediaDISK(" << url_r << ", " << attach_point_hint_r << ")" << endl;

      _device = Pathname(_url.getQueryParam("device")).asString();
      if( _device.empty())
      {
	ERR << "Media url does not contain a device specification" << std::endl;
	ZYPP_THROW(MediaBadUrlEmptyDestinationException(_url));
      }
#if DELAYED_VERIFY
      DBG << "Verify of " << _device << " delayed" << std::endl;
#else
      if( !verifyIfDiskVolume( _device))
      {
	ZYPP_THROW(MediaBadUrlEmptyDestinationException(_url));
      }
#endif

      _filesystem = _url.getQueryParam("filesystem");
      if(_filesystem.empty())
	_filesystem="auto";

    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaDISK::verifyIfDiskVolume
    //	METHOD TYPE : void
    //
    //	DESCRIPTION : Check if specified device file name is
    //                a disk volume device or throw an error.
    //
    bool MediaDISK::verifyIfDiskVolume(const Pathname &dev_name)
    {
      if( dev_name.empty() ||
	  dev_name.asString().compare(0, sizeof("/dev/")-1, "/dev/"))
      {
	ERR << "Specified device name " << dev_name
	    << " is not allowed" << std::endl;
	return false;
      }

      PathInfo dev_info(dev_name);
      if( !dev_info.isBlk())
      {
	ERR << "Specified device name " << dev_name
	    << " is not a block device" << std::endl;
	return false;
      }

      // check if a volume using /dev/disk/by-uuid links first
      {
	Pathname            dpath("/dev/disk/by-uuid");
	std::list<Pathname> dlist;
	if( zypp::filesystem::readdir(dlist, dpath) == 0)
	{
	  std::list<Pathname>::const_iterator it;
	  for(it = dlist.begin(); it != dlist.end(); ++it)
	  {
	    PathInfo vol_info(*it);
	    if( vol_info.isBlk() && vol_info.major() == dev_info.major() &&
	                            vol_info.minor() == dev_info.minor())
	    {
	      DBG << "Specified device name " << dev_name
		  << " is a volume (disk/by-uuid link "
		  << vol_info.path() << ")"
		  << std::endl;
	      return true;
	    }
	  }
	}
      }

      // check if a volume using /dev/disk/by-label links
      // (e.g. vbd mapped volumes in a XEN vm)
      {
	Pathname            dpath("/dev/disk/by-label");
	std::list<Pathname> dlist;
	if( zypp::filesystem::readdir(dlist, dpath) == 0)
	{
	  std::list<Pathname>::const_iterator it;
	  for(it = dlist.begin(); it != dlist.end(); ++it)
	  {
	    PathInfo vol_info(*it);
	    if( vol_info.isBlk() && vol_info.major() == dev_info.major() &&
	                            vol_info.minor() == dev_info.minor())
	    {
	      DBG << "Specified device name " << dev_name
		  << " is a volume (disk/by-label link "
		  << vol_info.path() << ")"
		  << std::endl;
	      return true;
	    }
	  }
	}
      }

      // check if a filesystem volume using the /sbin/vol_id tool
      // (there is no /dev/disk link for some of them)
      for(const char *vol_id_paths[] = VOL_ID_TOOL_PATHS,
	             **vol_id_path    = vol_id_paths;
	  vol_id_path != NULL && *vol_id_path != NULL;
	  vol_id_path++)
      {
	PathInfo vol_id_info(*vol_id_path);
	if( !vol_id_info.isFile() || !vol_id_info.isXUsr())
	  continue;

	const char *cmd[3];
	cmd[0] = *vol_id_path;
	cmd[1] = dev_name.asString().c_str();
	cmd[2] = NULL;

	ExternalProgram vol_id(cmd, ExternalProgram::Stderr_To_Stdout);

	std::string vol_fs_usage;
	std::string vol_fs_uuid;
	std::string vol_fs_type;

	for(std::string out( vol_id.receiveLine());
	    out.length(); out = vol_id.receiveLine())
	{
	  out = str::rtrim(out);

	  if( out.compare(0, sizeof("ID_FS_USAGE=")-1, "ID_FS_USAGE=") == 0)
	  {
	    vol_fs_usage = out.substr(sizeof("ID_FS_USAGE=")-1);
	  }
	  else
	  if( out.compare(0, sizeof("ID_FS_TYPE=")-1, "ID_FS_TYPE=")   == 0)
	  {
	    vol_fs_type  = out.substr(sizeof("ID_FS_TYPE=")-1);
	  }
	  else
	  if( out.compare(0, sizeof("ID_FS_UUID=")-1, "ID_FS_UUID=")   == 0)
	  {
	    vol_fs_uuid  = out.substr(sizeof("ID_FS_UUID=")-1);
	  }
	}

	if( vol_id.close() == 0)
	{
	  if( vol_fs_usage == "filesystem")
	  {
	    if(vol_fs_type == "iso9660" || vol_fs_type == "udf")
	    {
	      DBG << "Specified device name " << dev_name
	          << " is a CD/DVD volume (type " << vol_fs_type << ")"
	          << std::endl;
	      return true;
	    }
	    else
	    if(!vol_fs_type.empty() && !vol_fs_uuid.empty())
	    {
	      DBG << "Specified device name " << dev_name
	          << " is a volume (type " << vol_fs_type
		  << ", uuid " << vol_fs_uuid << ")"
	          << std::endl;
	      return true;
	    }
	  }
	}
      }

      ERR << "Specified device name " << dev_name
	  << " is not a usable disk volume"
          << std::endl;
      return false;
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
    
      PathInfo dev_info(_device);
      if(!dev_info.isBlk())
        ZYPP_THROW(MediaBadUrlEmptyDestinationException(url()));
#if DELAYED_VERIFY
      DBG << "Verifying " << _device << " ..." << std::endl;
      if( !verifyIfDiskVolume( _device))
      {
	ZYPP_THROW(MediaBadUrlEmptyDestinationException(_url));
      }
#endif

      if(_filesystem.empty())
	ZYPP_THROW(MediaBadUrlEmptyFilesystemException(url()));

      MediaSourceRef media( new MediaSource(
      	"disk", _device, dev_info.major(), dev_info.minor()
      ));
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

      MediaManager  manager;
      MountEntries  entries( manager.getMountEntries());
      MountEntries::const_iterator e;
      for( e = entries.begin(); e != entries.end(); ++e)
      {
	bool        is_device = false;
	std::string dev_path(Pathname(e->src).asString());
	PathInfo    dev_info;

	if( dev_path.compare(0, sizeof("/dev/")-1, "/dev/") == 0 &&
	    dev_info(e->src) && dev_info.isBlk())
	{
	  is_device = true;
	}

	if( is_device && media->maj_nr == dev_info.major() &&
	                 media->min_nr == dev_info.minor())
	{
	  /*
	  if( _filesystem != "auto" && _filesystem != e->type)
	  {
	    ZYPP_THROW();
	  }
	  */
	  media->bdir = e->dir;
	}
      }

      Mount mount;
      std::string mountpoint = attachPoint().asString();
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
    	options = "ro";
      }

      if( !media->bdir.empty())
      {
	options += ",bind";
	mount.mount(media->bdir, mountpoint, "none", options);
      }
      else
      {
      	mount.mount(_device, mountpoint, _filesystem, options);
      }

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
	  _device, mountpoint
        ));
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaDISK::isAttached
    //	METHOD TYPE : bool
    //
    //	DESCRIPTION : Override check if media is attached.
    //
    bool
    MediaDISK::isAttached() const
    {
      return checkAttached(false);
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaDISK::releaseFrom
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaDISK::releaseFrom( const std::string & ejectDev )
    {
      Mount mount;
      mount.umount(attachPoint().asString());
    }

    void MediaDISK::releaseFrom( bool eject )
    {
      releaseFrom("");
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

    bool MediaDISK::getDoesFileExist( const Pathname & filename ) const
    {
      return MediaHandler::getDoesFileExist( filename );
    }    
    
  } // namespace media
} // namespace zypp
// vim: set ts=8 sts=2 sw=2 ai noet:
