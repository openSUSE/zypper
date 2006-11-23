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

#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/ExternalProgram.h"
#include "zypp/media/Mount.h"
#include "zypp/media/MediaCD.h"
#include "zypp/media/MediaManager.h"
#include "zypp/Url.h"
#include "zypp/target/hal/HalContext.h"

#include <cstring> // strerror
#include <cstdlib> // getenv
#include <fstream>
#include <vector>
#include <list>

#include <errno.h>
#include <dirent.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // geteuid, ...

#include <linux/cdrom.h>

/*
** verify devices names as late as possible (while attach)
*/
#define  DELAYED_VERIFY          1

/*
** try umount of foreign (user/automounter) media on eject
**   0 = don't force, 1 = automounted only, 2 == all
*/
#define  FORCE_RELEASE_FOREIGN   2

/*
** Reuse foreign (user/automounter) mount points.
** 0 = don't use, 1 = automounted only, 2 = all
*/
#define  REUSE_FOREIGN_MOUNTS    2

/*
** if to throw exception on eject errors or ignore them
*/
#define  REPORT_EJECT_ERRORS     1

/*
** If defined to the full path of the eject utility,
** it will be used additionally to the eject-ioctl.
*/
#define EJECT_TOOL_PATH "/bin/eject"


using namespace std;

namespace zypp {
  namespace media {

    namespace {

      struct drive_info
      {
        drive_info(const std::string &_name = std::string(), bool _dvdr = false)
          : name(_name), dvdr(_dvdr)
        {}
        std::string name;
        bool        dvdr;
      };

      bool isNewDevice(const std::list<MediaSource> &devices,
                       const MediaSource            &media)
      {
	std::list<MediaSource>::const_iterator d( devices.begin());
	for( ; d != devices.end(); ++d)
	{
	  if( media.equals( *d))
	    return false;
	}
	return true;
      }

      inline Pathname get_sysfs_path()
      {
	Pathname sysfs_path;
	if(::getuid() == ::geteuid() && ::getgid() == ::getegid())
	{
	  const char *env = ::getenv("SYSFS_PATH");
	  if( env && *env)
	  {
	    sysfs_path = env;
	    if( PathInfo(sysfs_path, PathInfo::LSTAT).isDir())
	      return sysfs_path;
	  }
	}
	sysfs_path = "/sys";
	if( PathInfo(sysfs_path, PathInfo::LSTAT).isDir())
	  return sysfs_path;
	else
	  return Pathname();
      }

    }


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
    		    false )
      //, does_download
      , _lastdev(-1)
    {
      MIL << "MediaCD::MediaCD(" << url_r << ", "
          << attach_point_hint_r << ")" << endl;

      if( url_r.getScheme() != "dvd" && url_r.getScheme() != "cd")
      {
	ERR << "Unsupported schema in the Url: " << url_r.asString()
	                                         << std::endl;
	ZYPP_THROW(MediaUnsupportedUrlSchemeException(_url));
      }

#if !DELAYED_VERIFY
      DeviceList detected( detectDevices(
	url_r.getScheme() == "dvd" ? true : false
      ));
#endif

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
#if DELAYED_VERIFY
	      MediaSource media("cdrom", device, 0, 0);
	      _devices.push_back( media);
	       DBG << "use device (delayed verify)" << device << endl;
#else
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
	        ZYPP_THROW(MediaBadUrlException(_url,
		  "Invalid device name in URL devices argument"
		));
	      }
#endif
    	    }
    	    if (pos!=string::npos)
    		devices=devices.substr(pos+1);
    	    else
    		devices.erase();
    	}
      }
      else
      {
#if DELAYED_VERIFY
    	DBG << "going to use on-demand device list" << endl;
	return;
#else
    	DBG << "going to use default device list" << endl;
    	//default is /dev/cdrom; for dvd: /dev/dvd if it exists
        string device( "/dev/cdrom" );
    	if ( _url.getScheme() == "dvd" && PathInfo( "/dev/dvd" ).isBlk() ) {
    	  device = "/dev/dvd";
    	}

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
#endif
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
    bool MediaCD::openTray( const std::string & device_r )
    {
      int fd = ::open( device_r.c_str(), O_RDONLY|O_NONBLOCK );
      int res = -1;

      if ( fd != -1)
      {
	res = ::ioctl( fd, CDROMEJECT );
	::close( fd );
      }

      if ( res )
      {
	if( fd == -1)
	{
	  WAR << "Unable to open '" << device_r
	      << "' (" << ::strerror( errno ) << ")" << endl;
	}
	else
	{
	  WAR << "Eject " << device_r
	      << " failed (" << ::strerror( errno ) << ")" << endl;
	}

#if defined(EJECT_TOOL_PATH)
	DBG << "Try to eject " << device_r << " using "
	    << EJECT_TOOL_PATH << " utility" << std::endl;

	const char *cmd[3];
	cmd[0] = EJECT_TOOL_PATH;
	cmd[1] = device_r.c_str();
	cmd[2] = NULL;
	ExternalProgram eject(cmd, ExternalProgram::Stderr_To_Stdout);

	for(std::string out( eject.receiveLine());
	    out.length(); out = eject.receiveLine())
	{
	  DBG << " " << out;
	}

	if(eject.close() != 0)
	{
	  WAR << "Eject of " << device_r << " failed." << std::endl;
	  return false;
	}
#else
        return false;
#endif
      }
      MIL << "Eject of " << device_r << " successful." << endl;
      return true;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCD::closeTray
    //	METHOD TYPE : bool
    //
    bool MediaCD::closeTray( const std::string & device_r )
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
    //	METHOD NAME : MediaCD::detectDevices
    //	METHOD TYPE : MediaCD::DeviceList
    //
    MediaCD::DeviceList
    MediaCD::detectDevices(bool supportingDVD)
    {
      using namespace zypp::target::hal;

      DeviceList detected;
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
	    bool supportsDVD=false;
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

	      std::vector<std::string>::const_iterator ci;
	      for( ci=caps.begin(); ci != caps.end(); ++ci)
	      {
		if( *ci == "dvd")
		  supportsDVD = true;
	      }
	    }

	    MediaSource media("cdrom", drv.getDeviceFile(),
				       drv.getDeviceMajor(),
				       drv.getDeviceMinor());
	    DBG << "Found " << drv_udis[d] << ": "
			    << media.asString() << std::endl;
	    if( supportingDVD && supportsDVD)
	    {
	      detected.push_front(media);
	    }
	    else
	    {
	      detected.push_back(media);
	    }
	  }
	}
      }
      catch(const zypp::target::hal::HalException &e)
      {
	ZYPP_CAUGHT(e);
      }

      //
      // Read cdrom info from procfs as fallback ...
      //
      std::ifstream f( "/proc/sys/dev/cdrom/info");
      if( f.is_open())
      {
	std::string line;
	str::regex  rx_drive_name("^drive name:[ \t]+(.*)$");
	str::regex  rx_dvd_support("^Can read DVD:[ \t]+(.*)$");
	std::vector<drive_info> drives;

	DBG << "Collecting CD-ROM device info from /proc filesystem" << std::endl;
	while( std::getline(f, line))
	{
	  str::smatch out;
	  if( regex_match(line, out, rx_drive_name) && out.size() == 2)
	  {
	    std::list<std::string>                 vals;
	    zypp::str::split(out[1].str(), std::back_inserter(vals), " \t");

	    drives.resize( vals.size());
	    std::list<std::string>::const_iterator v(vals.begin());
	    for(size_t n = 0; v != vals.end(); ++v, n++)
	    {
              drives[n] = drive_info(*v);
	    }
	  }
	  else
	  if( regex_match(line, out, rx_dvd_support) && out.size() == 2)
	  {
	    std::list<std::string>                 vals;
	    zypp::str::split(out[1].str(), std::back_inserter(vals), " \t");
	    if( drives.size() == vals.size())
	    {
	      std::list<std::string>::const_iterator v(vals.begin());
	      for(size_t n = 0; v != vals.end(); ++v, n++)
	      {
	        drives[n].dvdr = (*v == "1");
	      }
	    }
	  }
	}
	f.close();
	for(size_t i=0; i < drives.size(); i++)
	{
	  if( drives[i].name.empty())
	    continue;

	  PathInfo dev_info("/dev/" + drives[i].name);
	  if( dev_info.isBlk())
	  {
	    MediaSource media("cdrom", dev_info.asString(),
	                               dev_info.major(),
	                               dev_info.minor());
	    if( isNewDevice(detected, media))
	    {
	      DBG << "Found a " << (drives[i].dvdr ? "DVD" : "CDROM")
	          << " " << media.asString()
	          << " info in /proc" << std::endl;
	      if( supportingDVD && drives[i].dvdr)
	      {
	        detected.push_front(media);
	      }
	      else
	      {
	        detected.push_back(media);
	      }
	    }
	  }
	}
      }


      //
      // Bug #163971
      // Hal does not include SCSI / Virtual CDROMs on iSeries ...
      //
      // Hmm... always? We can't detect DVD here.
      if( detected.empty())
      {
	Pathname    sysfs_path( get_sysfs_path());
	if(sysfs_path.empty())
	  return detected;

	std::string sys_name;
	std::string dev_name;

	// SCSI cdrom devices (/dev/sr0, ...)
	sys_name = sysfs_path.cat("block/sr").asString();
	dev_name = "/dev/sr";
	DBG << "Collecting SCSI CD-ROM devices ("
	    << dev_name << "X)" << std::endl;
	for(size_t i=0; i < 16; i++)
	{
	  PathInfo sys_info(sys_name + str::numstring(i));
	  PathInfo dev_info(dev_name + str::numstring(i));
	  if( sys_info.isDir() && dev_info.isBlk())
	  {
	    // Hmm.. how to check if it supports DVDs?
	    MediaSource media("cdrom", dev_info.asString(),
	                               dev_info.major(),
	                               dev_info.minor());
	    if( isNewDevice(detected, media))
	    {
	      DBG << "Found SCSI CDROM "
	          << media.asString()
	          << std::endl;
	      detected.push_back(media);
	    }
	  }
	}

	// IBM iSeries virtual CD-ROM devices (how many?)
#if powerpc
	sys_name = sysfs_path.cat("block/iseries!vcd").asString();
	dev_name = "/dev/iseries/vcd";
	DBG << "Collecting iSeries virtual CD-ROM devices ("
	    << dev_name << "X)" << std::endl;
	for(size_t i=0; i < 8; i++)
	{
	  char drive_letter = 'a' + i;
	  PathInfo sys_info(sys_name + drive_letter);
	  PathInfo dev_info(dev_name + drive_letter);
	  if( sys_info.isDir() && dev_info.isBlk())
	  {
	    // Hmm.. how to check if it supports DVDs?
	    MediaSource media("cdrom", dev_info.asString(),
	                               dev_info.major(),
	                               dev_info.minor());
	    if( isNewDevice(detected, media))
	    {
	      DBG << "Found iSeries virtual CDROM "
	          << media.asString()
	          << std::endl;
	      detected.push_back(media);
	    }
	  }
	}
#endif // powerpc

	// Other device types?
      }
      return detected;
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

#if DELAYED_VERIFY
      DeviceList detected( detectDevices(
	_url.getScheme() == "dvd" ? true : false
      ));

      if(_devices.empty())
      {
    	DBG << "creating on-demand device list" << endl;
    	//default is /dev/cdrom; for dvd: /dev/dvd if it exists
        string device( "/dev/cdrom" );
    	if ( _url.getScheme() == "dvd" && PathInfo( "/dev/dvd" ).isBlk() ) {
    	  device = "/dev/dvd";
    	}

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
#endif

      Mount mount;
      string mountpoint = attachPoint().asString();
      bool mountsucceeded = false;
      int count = 0;
      MediaMountException merr;

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
    		DBG << "skipping device " << it->name << endl;
    		continue;
    	}
#if DELAYED_VERIFY
	MediaSource temp( *it);
	bool        valid=false;
	PathInfo    dinfo(temp.name);
	if( dinfo.isBlk())
	{
	  temp.maj_nr = dinfo.major();
	  temp.min_nr = dinfo.minor();

	  DeviceList::const_iterator d( detected.begin());
	  for( ; d != detected.end(); ++d)
	  {
	    if( temp.equals( *d))
	    {
	      valid = true;
	      break;
	    }
	  }
	}
	if( !valid)
	{
    		DBG << "skipping invalid device: " << it->name << endl;
    		continue;
	}
	MediaSourceRef media( new MediaSource(temp));
#else
	MediaSourceRef media( new MediaSource( *it));
#endif

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

#if REUSE_FOREIGN_MOUNTS > 0
	{
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
	      AttachPointRef ap( new AttachPoint(e->dir, false));
	      AttachedMedia  am( media, ap);
	      //
	      // 1 = automounted only, 2 == all
	      //
#if REUSE_FOREIGN_MOUNTS == 1
	      if( isAutoMountedMedia(am))
#endif
	      {
		DBG << "Using a system mounted media "
		    << media->name
		    << " attached on "
		    << ap->path
		    << endl;

		media->iown = false; // mark attachment as foreign

		setMediaSource(media);
		setAttachPoint(ap);
		_lastdev = count;
		mountsucceeded = true;
		break;
	      }
	    }
	  }
	  if( mountsucceeded)
	    break;
	}
#endif  // REUSE_FOREIGN_MOUNTS

	// close tray
	closeTray( it->name );

	// try all filesystems in sequence
	for(list<string>::iterator fsit = filesystems.begin()
	    ; !mountsucceeded && fsit != filesystems.end()
	    ; ++fsit)
	{
	  try {
	    if( !isUseableAttachPoint(Pathname(mountpoint)))
	    {
	      mountpoint = createAttachPoint().asString();
	      setAttachPoint( mountpoint, true);
	      if( mountpoint.empty())
	      {
		ZYPP_THROW( MediaBadAttachPointException(url()));
	      }
	    }

    	    mount.mount(it->name, mountpoint, *fsit, options);

	    setMediaSource(media);

	    // wait for /etc/mtab update ...
	    // (shouldn't be needed)
	    int limit = 5;
	    while( !(mountsucceeded=isAttached()) && --limit)
	    {
	      sleep(1);
	    }

	    if( mountsucceeded)
	    {
	      _lastdev = count;
	    }
	    else
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
	        it->name, mountpoint
	      ));
	    }
	  }
	  catch (const MediaMountException &e)
	  {
	    merr = e;
	    removeAttachPoint();
	    ZYPP_CAUGHT(e);
	  }
	  catch (const MediaException & excpt_r)
	  {
	    removeAttachPoint();
	    ZYPP_CAUGHT(excpt_r);
	  }
    	}
      }

      if (!mountsucceeded)
      {
    	_lastdev = -1;

	if( !merr.mountOutput().empty())
	{
	  ZYPP_THROW(MediaMountException(merr.mountError(),
	                                 _url.asString(),
					 mountpoint,
	                                 merr.mountOutput()));
	}
	else
	{
	  ZYPP_THROW(MediaMountException("Mounting media failed",
	                                 _url.asString(), mountpoint));
	}
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
      try {
	AttachedMedia am( attachedMedia());
	if(am.mediaSource && am.mediaSource->iown)
	  mount.umount(am.attachPoint->path.asString());
      }
      catch (const Exception & excpt_r)
      {
	ZYPP_CAUGHT(excpt_r);
	if (eject)
	{
#if FORCE_RELEASE_FOREIGN > 0
	  /* 1 = automounted only, 2 = all */
	  forceRelaseAllMedia(false, FORCE_RELEASE_FOREIGN == 1);
#endif
	  if(openTray( mediaSourceName()))
	    return;
	}
	ZYPP_RETHROW(excpt_r);
      }

      // eject device
      if (eject)
      {
#if FORCE_RELEASE_FOREIGN > 0
	/* 1 = automounted only, 2 = all */
        forceRelaseAllMedia(false, FORCE_RELEASE_FOREIGN == 1);
#endif
        if( !openTray( mediaSourceName() ))
	{
#if REPORT_EJECT_ERRORS
	  ZYPP_THROW(MediaNotEjectedException(mediaSourceName()));
#endif
	}
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
      bool ejected=false;
      if ( !isAttached()) {	// no device mounted in this instance
#if DELAYED_VERIFY
	DeviceList detected( detectDevices(
	  _url.getScheme() == "dvd" ? true : false
	));

	if(_devices.empty())
	{
	  DBG << "creating on-demand device list" << endl;
	  //default is /dev/cdrom; for dvd: /dev/dvd if it exists
	  string device( "/dev/cdrom" );
	  if ( _url.getScheme() == "dvd" && PathInfo( "/dev/dvd" ).isBlk() ) {
	   device = "/dev/dvd";
	  }

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
#endif

	DeviceList::iterator it;
	for( it = _devices.begin(); it != _devices.end(); ++it ) {
	  MediaSourceRef media( new MediaSource( *it));
#if DELAYED_VERIFY
	  bool        valid=false;
	  PathInfo    dinfo(media->name);
	  if( dinfo.isBlk())
	  {
	    media->maj_nr = dinfo.major();
	    media->min_nr = dinfo.minor();

	    DeviceList::const_iterator d( detected.begin());
	    for( ; d != detected.end(); ++d)
	    {
	      if( media->equals( *d))
	      {
		valid = true;
		break;
	      }
	    }
	  }
	  if( !valid)
	  {
	    DBG << "skipping invalid device: " << it->name << endl;
	    continue;
	  }
#endif

	  // FIXME: we have also to check if it is mounted in the system
	  AttachedMedia ret( findAttachedMedia( media));
	  if( !ret.mediaSource)
	  {
#if FORCE_RELEASE_FOREIGN > 0
	    /* 1 = automounted only, 2 = all */
	    forceRelaseAllMedia(media, false, FORCE_RELEASE_FOREIGN == 1);
#endif
	    if ( openTray( it->name ) )
	    {
	      ejected = true;
	      break; // on 1st success
	    }
	  }
	}
      }
      if( !ejected)
      {
#if REPORT_EJECT_ERRORS
	ZYPP_THROW(MediaNotEjectedException());
#endif
      }
    }

    bool MediaCD::isAutoMountedMedia(const AttachedMedia &media)
    {
      bool is_automounted = false;
      if( media.mediaSource && !media.mediaSource->name.empty())
      {
        using namespace zypp::target::hal;

	try
	{
	  HalContext hal(true);

	  HalVolume vol = hal.getVolumeFromDeviceFile(media.mediaSource->name);
	  if( vol)
	  {
	    std::string udi = vol.getUDI();
	    std::string key;
	    std::string mnt;

	    try
	    {
	      key = "info.hal_mount.created_mount_point";
	      mnt = hal.getDevicePropertyString(udi, key);

	      if(media.attachPoint->path == mnt)
		is_automounted = true;
	    }
	    catch(const HalException &e1)
	    {
	      ZYPP_CAUGHT(e1);

	      try
	      {
		key = "volume.mount_point";
		mnt = hal.getDevicePropertyString(udi, key);

		if(media.attachPoint->path == mnt)
		  is_automounted = true;
	      }
	      catch(const HalException &e2)
	      {
		ZYPP_CAUGHT(e2);
	      }
	    }
	  }
	}
        catch(const HalException &e)
	{
	  ZYPP_CAUGHT(e);
	}
      }
      DBG << "Media "       << media.mediaSource->asString()
          << " attached on " << media.attachPoint->path
          << " is"           << (is_automounted ? "" : " not")
          << " automounted"  << std::endl;
      return is_automounted;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaCD::isAttached
    //	METHOD TYPE : bool
    //
    //	DESCRIPTION : Override check if media is attached.
    //
    bool
    MediaCD::isAttached() const
    {
      return checkAttached(false);
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

    bool MediaCD::getDoesFileExist( const Pathname & filename ) const
    {
      return MediaHandler::getDoesFileExist( filename );
    }

  } // namespace media
} // namespace zypp
// vim: set ts=8 sts=2 sw=2 ai noet:
