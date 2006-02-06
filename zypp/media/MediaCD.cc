/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaCD.cc
 *
*/

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/ExternalProgram.h"
#include "zypp/media/Mount.h"
#include "zypp/media/MediaCD.h"
#include "zypp/media/MediaManager.h"
#include "zypp/Url.h"

#include <cstring> // strerror

#include <errno.h>
#include <dirent.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/cdrom.h>


using namespace std;

namespace zypp {
  namespace media {

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : MediaCD
//
///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCD::MediaCD
    //	METHOD TYPE : Constructor
    //
    //	DESCRIPTION :
    //
    MediaCD::MediaCD( const Url &      url_r,
    		  const Pathname & attach_point_hint_r )
      : MediaHandler( url_r, attach_point_hint_r,
    		    url_r.getPathName(), // urlpath below attachpoint
    		    false ), // does_download
      _lastdev(-1)
    {
	MIL << "MediaCD::MediaCD(" << url_r << ", " << attach_point_hint_r << ")" << endl;

      string devices = _url.getQueryParam("devices");
      if (!devices.empty())
      {
   	string::size_type pos;
    	DBG << "parse " << devices << endl;
    	while(!devices.empty())
    	{
    	    pos = devices.find(',');
    	    string device = devices.substr(0,pos);
    	    if (!device.empty())
    	    {
    		_devices.push_back(device);
    		DBG << "use device " << device << endl;
    	    }
    	    if (pos!=string::npos)
    		devices=devices.substr(pos+1);
    	    else
    		devices.erase();
    	}
      }
      else
      {
    	//default is /dev/cdrom; for dvd: /dev/dvd if it exists
    	//TODO: make configurable
          string device( "/dev/cdrom" );
    	if ( _url.getScheme() == "dvd" && PathInfo( "/dev/dvd" ).isBlk() ) {
    	  device = "/dev/dvd";
    	}
    	DBG << "use default device " << device << endl;
    	_devices.push_back(device);
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCD::openTray
    //	METHOD TYPE : bool
    //
    bool MediaCD::openTray( const string & device_r )
    {
      int fd = ::open( device_r.c_str(), O_RDONLY|O_NONBLOCK );
      if ( fd == -1 ) {
        WAR << "Unable to open '" << device_r << "' (" << ::strerror( errno ) << ")" << endl;
        return false;
      }
      int res = ::ioctl( fd, CDROMEJECT );
      ::close( fd );
      if ( res ) {
        WAR << "Eject " << device_r << " failed (" << ::strerror( errno ) << ")" << endl;
        return false;
      }
      MIL << "Eject " << device_r << endl;
      return true;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCD::closeTray
    //	METHOD TYPE : bool
    //
    bool MediaCD::closeTray( const string & device_r )
    {
      int fd = ::open( device_r.c_str(), O_RDONLY|O_NONBLOCK );
      if ( fd == -1 ) {
        WAR << "Unable to open '" << device_r << "' (" << ::strerror( errno ) << ")" << endl;
        return false;
      }
      int res = ::ioctl( fd, CDROMCLOSETRAY );
      ::close( fd );
      if ( res ) {
        WAR << "Close tray " << device_r << " failed (" << ::strerror( errno ) << ")" << endl;
        return false;
      }
      DBG << "Close tray " << device_r << endl;
      return true;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCD::attachTo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that not already attached, and attachPoint is a directory.
    //
    void MediaCD::attachTo(bool next)
    {
      DBG << "next " << next << " last " << _lastdev << endl;
      if (next && _lastdev == -1)
	ZYPP_THROW(MediaNotSupportedException(url()));

      Mount mount;
      string mountpoint = attachPoint().asString();
      bool mountsucceeded = false;
      int count = 0;

      string options = _url.getQueryParam("mountoptions");
      if (options.empty())
      {
    	options="ro";
      }
    
      //TODO: make configurable
      list<string> filesystems;
    
      // if DVD, try UDF filesystem before iso9660
      if ( _url.getScheme() == "dvd" )
    	filesystems.push_back("udf");
    
      filesystems.push_back("iso9660");
    
      // try all devices in sequence
      for (DeviceList::iterator it = _devices.begin()
    	; !mountsucceeded && it != _devices.end()
    	; ++it, count++ )
      {
    	DBG << "count " << count << endl;
    	if (next && count<=_lastdev )
    	{
    		DBG << "skip" << endl;
    		continue;
    	}
	PathInfo dev_info( *it);
	if( !dev_info.isBlk())
	{
    		DBG << "skip " << *it << " - is not a block device" << endl;
		continue;
	}

	MediaSourceRef media( new MediaSource(
	  "cdrom", *it, dev_info.major(), dev_info.minor()
	));

	AttachedMedia ret( findAttachedMedia( media));

	if( ret.attachPoint &&
	    ret.mediaSource &&
	   !ret.attachPoint->empty() &&
	    media->equals( *ret.mediaSource))
	{
    		DBG << "Using a shared media "
		    << ret.mediaSource->name
		    << " attached on "
		    << ret.attachPoint->path
		    << endl;
                removeAttachPoint();
		setAttachPoint(ret.attachPoint);
		_mediaSource = ret.mediaSource;
    		_lastdev = count;
		mountsucceeded = true;
		break;
	}
	// FIXME: hmm... we may also
	// - check against hal/mtab if still mounted
	// - if !ret, check if already mounted (e.g.
	//   by automounter) and reuse (!temp) ?

    	// close tray
    	closeTray( *it );
    
    	// try all filesystems in sequence
    	for(list<string>::iterator fsit = filesystems.begin()
    	    ; !mountsucceeded && fsit != filesystems.end()
    	    ; ++fsit)
    	{
	  try {
	    // FIXME: verify, if this mountpoint isn't already in use.
	    if( mountpoint.empty() || mountpoint == "/")
	    {
	      mountpoint = createAttachPoint().asString();
	      setAttachPoint( mountpoint, true);
	      if( mountpoint.empty())
	      {
		ZYPP_THROW( MediaBadAttachPointException(url()));
	      }
	    }

    	    mount.mount (*it, mountpoint.c_str(), *fsit, options);

	    _lastdev = count;
	    _mediaSource = media;
	    mountsucceeded = true;
	  }
	  catch (const MediaException & excpt_r)
	  {
	    ZYPP_CAUGHT(excpt_r);
	  }
    	}
      }
    
      if (!mountsucceeded)
      {
    	_lastdev = -1;
        ZYPP_THROW(MediaMountException(_url.asString(), mountpoint, "Mounting media failed"));
      }
      DBG << _lastdev << " " << count << endl;
    }


    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCD::releaseFrom
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaCD::releaseFrom( bool eject )
    {
      DBG << "Release CD ... use count = "
          << _mediaSource.use_count()
	  << (_mediaSource.unique() ? ", unique" : ", not unique")
	  << std::endl;

      // check if a device is mounted
      if ( !_mediaSource)
      {
    	if (eject)			// eject wanted -> eject all devices
    	{
    	    for (DeviceList::iterator it = _devices.begin()
    		; it != _devices.end()
    		; ++it )
    	    {
    	        openTray( *it );
    	    }
    	    return;
	}
	ZYPP_THROW(MediaNotAttachedException(url()));
      }
      else
      if( _mediaSource.unique())
      {
        Mount mount;
        mount.umount(attachPoint().asString());
    
        // eject device
        if (eject)
        {
	  openTray( _mediaSource->name );
        }
      }

      _mediaSource.reset();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCD::forceEject
    //	METHOD TYPE : void
    //
    // Asserted that media is not attached.
    //
    void MediaCD::forceEject()
    {
      if ( !_mediaSource) {	// no device mounted
	for ( DeviceList::iterator it = _devices.begin(); it != _devices.end(); ++it ) {
	  if ( openTray( *it ) )
	    break; // on 1st success
	}
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaCD::getFile
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaCD::getFile( const Pathname & filename ) const
    {
      MediaHandler::getFile( filename );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaCD::getDir
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaCD::getDir( const Pathname & dirname, bool recurse_r ) const
    {
      MediaHandler::getDir( dirname, recurse_r );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCD::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaCD::getDirInfo( std::list<std::string> & retlist,
			      const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCD::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaCD::getDirInfo( filesystem::DirContent & retlist,
			      const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }

  } // namespace media
} // namespace zypp
// vim: set ts=8 sts=2 sw=2 ai noet:
