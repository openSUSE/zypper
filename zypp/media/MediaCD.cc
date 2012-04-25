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
extern "C"
{
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#if HAVE_UDEV
#include <libudev.h>
#endif
}

#ifndef HAVE_UDEV
#if HAVE_HAL
#include "zypp/target/hal/HalContext.h"
#endif
#endif

#include <cstring> // strerror
#include <cstdlib> // getenv
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/ExternalProgram.h"
#include "zypp/media/Mount.h"
#include "zypp/media/MediaCD.h"
#include "zypp/media/MediaManager.h"
#include "zypp/Url.h"
#include "zypp/AutoDispose.h"



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

//////////////////////////////////////////////////////////////////
namespace zypp
{
  //////////////////////////////////////////////////////////////////
  namespace media
  {

    //////////////////////////////////////////////////////////////////
    namespace
    {
      typedef std::list<MediaSource> DeviceList;

      //////////////////////////////////////////////////////////////////
      /// \brief Try to detect cd/dvd devices using hal/udev
      ///
      /// Returns an empty device list on error.
      ///
      /// \todo I took the code more or less as it was from MediaCD::detectDevices
      /// into this function. Semantic between HAL and UDEV seems to be slightly
      /// different, esp. in supportingDVD mode. This should be investigated and fixed.
      //////////////////////////////////////////////////////////////////
      DeviceList systemDetectDevices( bool supportingDVD_r )
      {
	DeviceList detected;

#ifdef HAVE_UDEV
	// http://www.kernel.org/pub/linux/utils/kernel/hotplug/libudev/index.html
	zypp::AutoDispose<struct udev *> udev( ::udev_new(), ::udev_unref );
	if ( ! udev )
	{
	  ERR << "Can't create udev context." << endl;
	  return DeviceList();
	}

	zypp::AutoDispose<struct udev_enumerate *> enumerate( ::udev_enumerate_new(udev), ::udev_enumerate_unref );
	if ( ! enumerate )
	{
	  ERR << "Can't create udev list entry." << endl;
	  return DeviceList();
	}

	::udev_enumerate_add_match_subsystem( enumerate, "block" );
	::udev_enumerate_add_match_property( enumerate, "ID_CDROM", "1" );
	::udev_enumerate_scan_devices( enumerate );

	struct udev_list_entry * entry = 0;
	udev_list_entry_foreach( entry, ::udev_enumerate_get_list_entry( enumerate ) )
	{
	  zypp::AutoDispose<struct udev_device *> device( ::udev_device_new_from_syspath( ::udev_enumerate_get_udev( enumerate ),
											  ::udev_list_entry_get_name( entry ) ),
							  ::udev_device_unref );
	  if ( ! device )
	  {
	    ERR << "Can't create udev device." << endl;
	    continue;
	  }

	  if ( supportingDVD_r && ! ::udev_device_get_property_value( device, "ID_CDROM_DVD" ) )
	  {
	    continue;	// looking for dvd only
	  }

	  const char * devnodePtr( ::udev_device_get_devnode( device ) );
	  if ( ! devnodePtr )
	  {
	    ERR << "Got NULL devicenode." << endl;
	    continue;
	  }

	  // In case we need it someday:
	  //const char * mountpath = ::udev_device_get_property_value( device, "FSTAB_DIR" );

	  PathInfo devnode( devnodePtr );
	  if ( devnode.isBlk() )
	  {
	    MediaSource media( "cdrom", devnode.path().asString(), devnode.major(), devnode.minor() );
	    DBG << "Found (udev): " << media << std::endl;
	    detected.push_back( media );
	  }
	}
	if ( detected.empty() )
	{
	  WAR << "Did not find any CD/DVD device." << endl;
	}
#elif HAVE_HAL
	using namespace zypp::target::hal;
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
	      if( supportingDVD_r)
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
		  if( supportingDVD_r && supportsDVD)
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
#endif
	return detected;
      }

    } // namespace
    //////////////////////////////////////////////////////////////////


  MediaCD::MediaCD( const Url & url_r, const Pathname & attach_point_hint_r )
    : MediaHandler( url_r, attach_point_hint_r, url_r.getPathName(), false )
    , _lastdev( -1 )
    , _lastdev_tried( -1 )
  {
    MIL << "MediaCD::MediaCD(" << url_r << ", " << attach_point_hint_r << ")" << endl;

    if ( url_r.getScheme() != "dvd" && url_r.getScheme() != "cd" )
    {
      ERR << "Unsupported schema in the Url: " << url_r.asString() << endl;
      ZYPP_THROW(MediaUnsupportedUrlSchemeException(_url));
    }

    string devices = _url.getQueryParam( "devices" );
    if ( ! devices.empty() )
    {
      std::vector<std::string> words;
      str::split( devices, std::back_inserter(words), "," );
      for ( const std::string & device : words )
      {
	if ( device.empty() )
	  continue;

	MediaSource media( "cdrom", device, 0, 0 );
	_devices.push_back( media );
	DBG << "use device (delayed verify)" << device << endl;
      }
    }
    else
    {
      DBG << "going to use on-demand device list" << endl;
      return;
    }

    if ( _devices.empty() )
    {
      ERR << "Unable to find any cdrom drive for " << _url.asString() << endl;
      ZYPP_THROW(MediaBadUrlEmptyDestinationException(_url));
    }
  }

  ///////////////////////////////////////////////////////////////////
  //
  //
  //  METHOD NAME : MediaCD::openTray
  //  METHOD TYPE : bool
  //
  bool MediaCD::openTray( const std::string & device_r )
  {
    int fd = ::open( device_r.c_str(), O_RDONLY|O_NONBLOCK|O_CLOEXEC );
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
    int fd = ::open( device_r.c_str(), O_RDONLY|O_NONBLOCK|O_CLOEXEC );
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


  MediaCD::DeviceList MediaCD::detectDevices( bool supportingDVD_r ) const
  {
    DeviceList detected( systemDetectDevices( supportingDVD_r ) );

    if ( detected.empty() )
    {
      WAR << "CD/DVD drive detection with HAL/UDEV failed! Guessing..." << std::endl;
      PathInfo dvdinfo( "/dev/dvd" );
      PathInfo cdrinfo( "/dev/cdrom" );
      if ( dvdinfo.isBlk() )
      {
	MediaSource media( "cdrom", dvdinfo.path().asString(), dvdinfo.major(), dvdinfo.minor() );
	DBG << "Found (GUESS): " << media << std::endl;
	detected.push_back( media );
      }
      if ( cdrinfo.isBlk()
	&& ! ( cdrinfo.major() == dvdinfo.major() && cdrinfo.minor() == dvdinfo.minor() ) )
      {
	MediaSource media( "cdrom", cdrinfo.path().asString(), cdrinfo.major(), cdrinfo.minor() );
	DBG << "Found (GUESS): " << media << std::endl;
	detected.push_back( media );
      }
    }

    // NOTE: On the fly build on-demand device list. Code was moved to
    // here to get rid of code duplication, while keeping the ABI. Acuallty
    // this code should be moved to a _devices accessor method.
    if ( _devices.empty() )
    {
      DBG << "creating on-demand device list" << endl;
      //default is /dev/cdrom; for dvd: /dev/dvd if it exists
      string device( "/dev/cdrom" );
      if ( _url.getScheme() == "dvd" && PathInfo( "/dev/dvd" ).isBlk() )
      {
        device = "/dev/dvd";
      }

      PathInfo dinfo( device );
      if ( dinfo.isBlk() )
      {
	MediaSource media( "cdrom", device, dinfo.major(), dinfo.minor() );
	if ( detected.empty() )
	{
	  _devices.push_front( media );	// better try this than nothing
	}
	else
	{
	  for( const auto & d : detected )
	  {
	    // /dev/cdrom or /dev/dvd to the front
	    if ( media.equals( d ) )
	      _devices.push_front( d );
	    else
	      _devices.push_back( d );
	  }
	}
      }
      else
      {
	// no /dev/cdrom or /dev/dvd link
	_devices = detected;
      }
    }

    return detected;
  }


  ///////////////////////////////////////////////////////////////////
  //
  //
  //  METHOD NAME : MediaCD::attachTo
  //  METHOD TYPE : PMError
  //
  //  DESCRIPTION : Asserted that not already attached, and attachPoint is a directory.
  //
  void MediaCD::attachTo( bool next )
  {
    DBG << "next " << next << " last " << _lastdev << " last tried " << _lastdev_tried << endl;
    if ( next && _lastdev == -1 )
      ZYPP_THROW(MediaNotSupportedException(url()));

    // This also fills the _devices list on demand
    DeviceList detected( detectDevices( _url.getScheme() == "dvd" ? true : false ) );

    Mount mount;
    MediaMountException merr;
    string mountpoint = attachPoint().asString();

    string options = _url.getQueryParam( "mountoptions" );
    if ( options.empty() )
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
    int count = 0;
    bool mountsucceeded = false;
    for ( DeviceList::iterator it = _devices.begin() ; ! mountsucceeded && it != _devices.end() ; ++it, ++count )
    {
      DBG << "count " << count << endl;
      if (next && count <=_lastdev_tried )
      {
        DBG << "skipping device " << it->name << endl;
        continue;
      }
      _lastdev_tried = count;

      // bnc#755815: _devices contains either devices passed as url option
      // 	or autodetected ones. Accept both as long as they are block
      // 	devices.
      MediaSource temp( *it );
      PathInfo dinfo( temp.name );
      if ( ! dinfo.isBlk() )
      {
	WAR <<  "skipping non block device: " << dinfo << endl;
	continue;
      }
      DBG << "trying device " << dinfo << endl;

      temp.maj_nr = dinfo.major();
      temp.min_nr = dinfo.minor();
      MediaSourceRef media( new MediaSource(temp));
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

      // close tray
      closeTray( it->name );

      // try all filesystems in sequence
      for(list<string>::iterator fsit = filesystems.begin()
          ; !mountsucceeded && fsit != filesystems.end()
          ; ++fsit)
      {
        try
        {
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
          int limit = 2;
          while( !(mountsucceeded=isAttached()) && --limit)
          {
	    WAR << "Wait for /proc/mounts update and retry...." << endl;
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
      } // for filesystems
    } // for _devices

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
  //  METHOD NAME : MediaCD::releaseFrom
  //  METHOD TYPE : PMError
  //
  //  DESCRIPTION : Asserted that media is attached.
  //
  void MediaCD::releaseFrom( const std::string & ejectDev )
  {
    Mount mount;
    try
    {
      AttachedMedia am( attachedMedia());
      if(am.mediaSource && am.mediaSource->iown)
        mount.umount(am.attachPoint->path.asString());
    }
    catch (const Exception & excpt_r)
    {
      ZYPP_CAUGHT(excpt_r);
      if (!ejectDev.empty())
      {
        forceRelaseAllMedia(false);
        if(openTray( ejectDev ))
          return;
      }
      ZYPP_RETHROW(excpt_r);
    }

    // eject device
    if (!ejectDev.empty())
    {
      forceRelaseAllMedia(false);
      if( !openTray( ejectDev ))
      {
#if REPORT_EJECT_ERRORS
        ZYPP_THROW(MediaNotEjectedException(ejectDev));
#endif
      }
    }
  }

  ///////////////////////////////////////////////////////////////////
  //
  //
  //  METHOD NAME : MediaCD::forceEject
  //  METHOD TYPE : void
  //
  // Asserted that media is not attached.
  //
  void MediaCD::forceEject( const std::string & ejectDev_r )
  {
    bool ejected = false;
    if ( ! isAttached() )	// no device mounted in this instance
    {
      // This also fills the _devices list on demand
      DeviceList detected( detectDevices( _url.getScheme() == "dvd" ? true : false ) );
      for_( it, _devices.begin(), _devices.end() )
      {
        MediaSourceRef media( new MediaSource( *it ) );
        if ( media->name != ejectDev_r )
          continue;

	// bnc#755815: _devices contains either devices passed as url option
	// 	or autodetected ones. Accept both as long as they are block
	// 	devices.
	PathInfo dinfo( media->name );
	if( ! dinfo.isBlk() )
	{
	  WAR <<  "skipping non block device: " << dinfo << endl;
	  continue;
	}
	DBG << "trying device " << dinfo << endl;

        // FIXME: we have also to check if it is mounted in the system
        AttachedMedia ret( findAttachedMedia( media));
        if( !ret.mediaSource )
        {
          forceRelaseAllMedia( media, false );
          if ( openTray( it->name ) )
          {
            ejected = true;
            break; // on 1st success
          }
        }
      }
    }
#if REPORT_EJECT_ERRORS
    if( !ejected)
    {
      ZYPP_THROW(MediaNotEjectedException());
    }
#endif
  }

  ///////////////////////////////////////////////////////////////////
  //
  //  METHOD NAME : MediaCD::isAttached
  //  METHOD TYPE : bool
  //
  //  DESCRIPTION : Override check if media is attached.
  //
  bool
  MediaCD::isAttached() const
  {
    return checkAttached(false);
  }

  ///////////////////////////////////////////////////////////////////
  //
  //  METHOD NAME : MediaCD::getFile
  //  METHOD TYPE : PMError
  //
  //  DESCRIPTION : Asserted that media is attached.
  //
  void MediaCD::getFile( const Pathname & filename ) const
  {
    MediaHandler::getFile( filename );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //  METHOD NAME : MediaCD::getDir
  //  METHOD TYPE : PMError
  //
  //  DESCRIPTION : Asserted that media is attached.
  //
  void MediaCD::getDir( const Pathname & dirname, bool recurse_r ) const
  {
    MediaHandler::getDir( dirname, recurse_r );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //
  //  METHOD NAME : MediaCD::getDirInfo
  //  METHOD TYPE : PMError
  //
  //  DESCRIPTION : Asserted that media is attached and retlist is empty.
  //
  void MediaCD::getDirInfo( std::list<std::string> & retlist,
                            const Pathname & dirname, bool dots ) const
  {
    MediaHandler::getDirInfo( retlist, dirname, dots );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //
  //  METHOD NAME : MediaCD::getDirInfo
  //  METHOD TYPE : PMError
  //
  //  DESCRIPTION : Asserted that media is attached and retlist is empty.
  //
  void MediaCD::getDirInfo( filesystem::DirContent & retlist, const Pathname & dirname, bool dots ) const
  {
    MediaHandler::getDirInfo( retlist, dirname, dots );
  }


  bool MediaCD::getDoesFileExist( const Pathname & filename ) const
  {
    return MediaHandler::getDoesFileExist( filename );
  }


  bool MediaCD::hasMoreDevices()
  {
    if (_devices.size() == 0)
      return false;
    else if (_lastdev_tried < 0)
      return true;

    return (unsigned) _lastdev_tried < _devices.size() - 1;
  }


  void MediaCD::getDetectedDevices( std::vector<std::string> & devices, unsigned int & index ) const
  {
    if ( ! devices.empty() )
      devices.clear();

    if ( _devices.empty() )
      // This also fills the _devices list on demand
      detectDevices( _url.getScheme() == "dvd" ? true : false );

    for ( const auto & it : _devices )
      devices.push_back( it.name );

    index = ( _lastdev >= 0  ? (unsigned)_lastdev : 0 );

    MIL << "got " << devices.size() << " detected devices, current: "
        << (index < devices.size() ? devices[index] : "<none>")
        << "(" << index << ")" << endl;
  }

  } // namespace media
  //////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////
