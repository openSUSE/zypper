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
#include "zypp/target/hal/HalContext.h"

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
      if( url_r.getScheme() != "dvd" && url_r.getScheme() != "cdrom")
      {
	ERR << "Unsupported schema in the Url: " << url_r.asString()
	                                         << std::endl;
	ZYPP_THROW(MediaUnsupportedUrlSchemeException(_url));
      }

      MIL << "MediaCD::MediaCD(" << url_r << ", " << attach_point_hint_r << ")" << endl;

      DeviceList detected( detectDevices(
	url_r.getScheme() == "dvd" ? true : false
      ));

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
	      bool is_ok = false;
	      PathInfo dinfo(device);
	      if( dinfo.isBlk())
	      {
		MediaSource media("cdrom", device, dinfo.major(),
	                                           dinfo.minor());

	        DeviceList::const_iterator d( detected.begin());
	        for( ; d != detected.end(); ++d)
	        {
	          if( media.equals( *d))
		  {
		    is_ok = true;
	            _devices.push_back( *d);
    		    DBG << "use device " << device << endl;
		  }
	        }
	      }

	      if( !is_ok)
	      {
	        ERR << "Device " << device << " is not acceptable "
	            << "for " << _url.getScheme() << std::endl;
	        ZYPP_THROW(MediaBadUrlEmptyDestinationException(_url));
	      }
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
        string device( "/dev/cdrom" );
    	if ( _url.getScheme() == "dvd" && PathInfo( "/dev/dvd" ).isBlk() ) {
    	  device = "/dev/dvd";
    	}

    	DBG << "going to use default device list" << endl;
	PathInfo dinfo(device);
	if( dinfo.isBlk())
	{
	  MediaSource media("cdrom", device, dinfo.major(), dinfo.minor());

	  DeviceList::const_iterator d( detected.begin());
	  for( ; d != detected.end(); ++d)
	  {
	    // /dev/cdrom or /dev/dvd to the front
	    if( media.equals( *d))
	      _devices.push_front( *d);
	    else
	      _devices.push_back( *d);
	  }
	}
	else
	{
	  // no /dev/cdrom or /dev/dvd link
	  _devices = detected;
	}
      }

      if( _devices.empty())
      {
	ERR << "Unable to find any cdrom drive for " << _url.asString()
	                                             << std::endl;
	ZYPP_THROW(MediaBadUrlEmptyDestinationException(_url));
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

    MediaCD::DeviceList
    MediaCD::detectDevices(bool supportingDVD)
    {
      using namespace zypp::target::hal;

      DeviceList devices;
      try
      {
	HalContext hal(true);

	std::vector<std::string> drv_udis;
	drv_udis = hal.findDevicesByCapability("storage.cdrom");

	DBG << "Found " << drv_udis.size() << " cdrom drive udis" << std::endl;
	for(size_t d = 0; d < drv_udis.size(); d++)
	{
	  HalDrive drv( hal.getDriveFromUDI( drv_udis[d]));

	  if( drv)
	  {
	    if( supportingDVD)
	    {
	      std::vector<std::string> caps;
	      try {
		caps = drv.getCdromCapabilityNames();
	      }
	      catch(const HalException &e)
	      {
		ZYPP_CAUGHT(e);
	      }

	      bool found=false;
	      std::vector<std::string>::const_iterator ci;
	      for( ci=caps.begin(); ci != caps.end(); ++ci)
	      {
		if( *ci == "dvd")
		  found = true;
	      }
	      if( !found)
		continue;
	    }

	    MediaSource media("cdrom", drv.getDeviceFile(),
				       drv.getDeviceMajor(),
				       drv.getDeviceMinor());
	    DBG << "Found " << drv_udis[d] << ": "
			    << media.asString() << std::endl;
	    devices.push_back(media);
	  }
	}
      }
      catch(const zypp::target::hal::HalException &e)
      {
	ZYPP_CAUGHT(e);
      }
      return devices;
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

	MediaSourceRef media( new MediaSource( *it));

	AttachedMedia ret( findAttachedMedia( media));

	if( ret.mediaSource && ret.attachPoint &&
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
	  _lastdev = count;
	  mountsucceeded = true;
	  break;
	}
	// FIXME: hmm... we may also
	// - check against hal/mtab if still mounted
	// - if !ret, check if already mounted (e.g.
	//   by automounter) and reuse (!temp) ?

	// close tray
	closeTray( it->name );

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

    	    mount.mount (it->name, mountpoint.c_str(), *fsit, options);

	    _lastdev = count;
	    setMediaSource(media);
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
      Mount mount;
      mount.umount(attachPoint().asString());
    
      // eject device
      if (eject)
      {
        openTray( mediaSourceName() );
      }
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
      if ( !isAttached()) {	// no device mounted in this instance
	for ( DeviceList::iterator it = _devices.begin(); it != _devices.end(); ++it ) {
	  MediaSourceRef media( new MediaSource( *it));

	  // FIXME: we have also to check if it is mounted in the system
	  AttachedMedia ret( findAttachedMedia( media));
	  if( !ret.mediaSource)
	  {
	    if ( openTray( it->name ) )
	      break; // on 1st success
	  }
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
