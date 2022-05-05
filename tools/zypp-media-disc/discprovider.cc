/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include "discprovider.h"
#include <zypp-media/ng/private/providedbg_p.h>
#include <zypp-media/ng/worker/ProvideWorker>
#include <zypp-media/ng/MediaVerifier>
#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/AutoDispose.h>
#include <zypp-core/base/StringV.h>
#include <zypp-media/Mount>
#include <zypp-media/CDTools>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "DiscProvider"

extern "C"
{
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#if HAVE_UDEV
#include <libudev.h>
#endif
}

using namespace std::literals;

DiscProvider::DiscProvider( ) : zyppng::worker::DeviceDriver( zyppng::worker::WorkerCaps::VolatileMount )
{ }

DiscProvider::~DiscProvider()
{}

zyppng::worker::AttachResult DiscProvider::mountDevice ( const uint32_t id, const zypp::Url &attachUrl, const std::string &attachId, const std::string &label, const zyppng::HeaderValueMap &extras )
{
  std::vector<std::string> devs;
  if ( extras.contains( zyppng::AttachMsgFields::Device ) ) {
    for ( const auto &d :extras.values( zyppng::AttachMsgFields::Device ) ) {
      if ( d.isString() ) {
        MIL << "Got device from user we are allowed to use: " << d.asString() << std::endl;
        devs.push_back(d.asString());
      }
    }
  }

  auto &sysDevs = knownDevices();

  std::vector<std::shared_ptr<zyppng::worker::Device>> possibleDevs;
  if ( devs.size () ) {
    for ( const auto &d : devs ) {
      zypp::PathInfo p(d);
      if ( !p.isBlk () )
        continue;

      std::shared_ptr<zyppng::worker::Device> device;
      auto i = std::find_if( sysDevs.begin(), sysDevs.end(), [&p]( const auto &sysDev ){ return ( sysDev->_maj_nr == p.devMajor() && sysDev->_min_nr == p.devMinor() ); } );
      if ( i == sysDevs.end() ) {
        // this is a device we did not detect before lets see if its a CDROM
#ifdef HAVE_UDEV
        zypp::AutoDispose<struct udev *> udev( ::udev_new(), ::udev_unref );
        if ( !udev ) {
          ERR << "Can't create udev context." << std::endl;
          continue;
        }

        auto devnum = makedev( p.devMajor(), p.devMinor() );
        zypp::AutoDispose<struct udev_device *> udevDevice( ::udev_device_new_from_devnum ( udev, 'b', devnum ), ::udev_device_unref );
        if ( !udevDevice ) {
          ERR << "Can't create udev device from devnum, ignoring." << std::endl;
          continue;
        }

        if ( zypp::strv::asStringView( ::udev_device_get_property_value( udevDevice, "ID_CDROM" ) ) != "1" ) {
          ERR << "Device " << d << " from Attach request is not a CDROM/DVD ignoring!" << std::endl;
          continue;
        }

        auto dev = std::make_shared<zyppng::worker::Device>( zyppng::worker::Device {
          ._name = p.path().asString(),
          ._maj_nr = p.devMajor(),
          ._min_nr = p.devMinor() } );
        DBG << "Registering device (REQUEST): " << dev->_name << " " << dev->_maj_nr << ":" << dev->_min_nr << std::endl;
        sysDevs.push_back ( dev );
        device = dev;
#endif
      } else {
        device = *i;
      }

      if ( device )
        possibleDevs.push_back(device);
    }
  } else {
    possibleDevs = sysDevs;
  }

  // if we have no devices at this point controller gets a error
  if ( possibleDevs.empty () ) {
    return zyppng::worker::AttachResult::error(
      zyppng::ProvideMessage::Code::MountFailed
      , "No useable device found"
      , false
    );
  }

  const auto attachRoot = zypp::Pathname( attachUrl.getPathName() );
  const auto attachMediaNr = extras.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt();

  // set up the verifier
  zyppng::MediaDataVerifierRef verifier;
  if ( extras.contains( zyppng::AttachMsgFields::VerifyType ) ) {
    verifier = zyppng::MediaDataVerifier::createVerifier( extras.value(zyppng::AttachMsgFields::VerifyType).asString() );
    if ( !verifier ) {
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , "Invalid verifier type"
        , false
      );
    }

    if ( !verifier->load( extras.value(zyppng::AttachMsgFields::VerifyData).asString() ) ) {
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , "Failed to create verifier from file"
        , false
      );
    }
  }

  //first check if any of the mounted devices are what we want
  for( const auto &dev : possibleDevs ) {
    if ( dev->_mountPoint.empty() )
      continue;

    auto res = isDesiredMedium( attachUrl, dev->_mountPoint / attachRoot, verifier, extras.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt() );
    if ( !res )
      continue;

    MIL << "Found requested medium in dev " << dev->_name << std::endl;

    // we found the device we want!
    attachedMedia().insert( std::make_pair( attachId, zyppng::worker::AttachedMedia{ dev, attachRoot } ) );
    return zyppng::worker::AttachResult::success();
  }

  std::list<std::string> filesystems;

  filesystems.push_back("iso9660");

  // if DVD, try UDF filesystem after iso9660
  if ( attachUrl.getScheme() == "dvd" )
    filesystems.push_back("udf");

  std::string options;
  auto optVal = extras.value( "mountoptions"sv );
  if ( optVal.valid() && optVal.isString() )
    options = optVal.asString();

  if ( options.empty() ) {
    options="ro";
  }

  while ( true ) {

    // remember how many devices we were able to test
    uint devicesTested = 0;

    // none of the already mounted devices matched, lets try what we have left
    for( const auto &dev : possibleDevs ) {
      if ( !dev->_mountPoint.empty() )
        continue;

      MIL << "Trying to mount dev " << dev->_name << std::endl;

      // make sure the tray is closed
      zypp::media::CDTools::closeTray( dev->_name );

      devicesTested++;
      zypp::media::Mount mount;
      bool mountsucceeded = false;
      std::exception_ptr lastErr;
      for( auto fsit = filesystems.begin() ; !mountsucceeded && fsit != filesystems.end() ; ++fsit) {

        zypp::Pathname newAp;
        try {
          newAp = createAttachPoint( this->attachRoot() );
          if ( newAp.empty() ) {
            return zyppng::worker::AttachResult::error(
              zyppng::ProvideMessage::Code::MountFailed
              , "Failed to create mount directory."
              , false
            );
          }

          mount.mount( dev->_name, newAp.asString(), *fsit, options);

          // wait for /etc/mtab update ...
          // (shouldn't be needed)
          int limit = 2;
          while( !(mountsucceeded=checkAttached( newAp, devicePredicate( dev->_maj_nr, dev->_min_nr ))) && --limit)
          {
            WAR << "Wait for /proc/mounts update and retry...." << std::endl;
            sleep(1);
          }

          if( !mountsucceeded) {
            try {
              mount.umount( newAp.asString() );
            } catch (const zypp::media::MediaException & excpt_r) {
              ZYPP_CAUGHT(excpt_r);
            }
            ZYPP_THROW( zypp::media::MediaMountException(
              "Unable to verify that the media was mounted",
              dev->_name, newAp.asString()
              ));
          } else {
            dev->_mountPoint = newAp;
            break;
          }
        }
        catch (const zypp::media::MediaMountException &e)
        {
          lastErr = std::current_exception ();
          removeAttachPoint(newAp);
          ZYPP_CAUGHT(e);
        }
        catch (const zypp::media::MediaException & excpt_r)
        {
          removeAttachPoint(newAp);
          ZYPP_CAUGHT(excpt_r);
        }
      } // for filesystems

      if ( mountsucceeded ) {
        // lets check if we have the correct medium
        bool canUseDevice = false;

        auto res = isDesiredMedium( attachUrl, dev->_mountPoint / attachRoot, verifier, extras.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt() );
        if ( !res ) {
          unmountDevice( *dev );
          continue;
        }

        MIL << "Found requested medium in dev " << dev->_name << std::endl;
        attachedMedia().insert( std::make_pair( attachId, zyppng::worker::AttachedMedia{ dev, attachRoot } ) );
        return zyppng::worker::AttachResult::success();
      }
    } // for each device

    // we did go through all devices, but didn't find any.
    if ( devicesTested == 0 ) {
      // no devices are free
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::Jammed
        , "No free ressources available"
        , false
      );
    } else {

      const auto &mountedDevs = zypp::media::Mount::getEntries();
      // k, we need to ask the user to give us the medium we need
      // first find the devices that are free
      std::vector<std::string> freeDevs;
      for( const auto &dev : possibleDevs ) {
        if ( !dev->_mountPoint.empty() )
          continue;

        auto i = std::find_if( mountedDevs.begin (), mountedDevs.end(), [&dev]( const zypp::media::MountEntry &e ) {
          zypp::PathInfo pi( e.src );
          return ( pi.isBlk() && pi.devMajor() == dev->_maj_nr && pi.devMinor() == dev->_min_nr );
        });

        if ( i == mountedDevs.end() ) {
          MIL << "Adding " << dev->_name << " to list of free devs" << std::endl;
          freeDevs.push_back( dev->_name );
        }
      }

      // if there are no devices free, we are jammed
      if ( freeDevs.empty() ) {
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::Jammed
          , "No free ressources available"
          , false
        );
      }

      auto changeRes = zyppng::worker::ProvideWorker::ABORT;
      auto worker = parentWorker();
      if ( worker ) {
        changeRes = worker->requestMediaChange ( id, label, attachMediaNr, freeDevs );
      }

      if ( changeRes != zyppng::worker::ProvideWorker::SUCCESS ) {
        if ( changeRes == zyppng::worker::ProvideWorker::SKIP)
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MediaChangeSkip
            , "User asked to skip the media change."
            , false);
        else
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MediaChangeAbort
            , "User asked to abort the media change."
            , false );
      }
    }
  }
}

void DiscProvider::detectDevices( )
{

  // helper lambda to add a new device to the internal registry, returning it if its either created or already known
  const auto registerDevice = [ this ]( const zypp::PathInfo &devnode, std::string_view reason ) {
    auto &sysDevs = knownDevices();
    if ( devnode.isBlk() ) {
      auto i = std::find_if( sysDevs.begin(), sysDevs.end(), [&]( const auto &dev ){
        return ( dev->_maj_nr == devnode.devMajor() && dev->_min_nr == devnode.devMinor () );
      });

      // we already know the device
      if ( i != sysDevs.end() )
        return i;

      auto dev = std::make_shared<zyppng::worker::Device>( zyppng::worker::Device{
        ._name   = devnode.path().asString(),
        ._maj_nr = devnode.devMajor(),
        ._min_nr = devnode.devMinor() } );
      DBG << "Found (" << reason << "): " << dev->_name << " " << dev->_maj_nr << ":" << dev->_min_nr << std::endl;

      sysDevs.push_back(dev);
      return ( --sysDevs.end() );
    }
    return sysDevs.end();
  };


  auto &sysDevs = knownDevices();

  // detect devices available in the system if we did not do it before:
  if ( !_devicesDetected ) {
    _devicesDetected = true;

#ifdef HAVE_UDEV
    // http://www.kernel.org/pub/linux/utils/kernel/hotplug/libudev/index.html
    zypp::AutoDispose<struct udev *> udev( ::udev_new(), ::udev_unref );
    if ( ! udev ) {
      ERR << "Can't create udev context." << std::endl;
    } else {
      zypp::AutoDispose<struct udev_enumerate *> enumerate( ::udev_enumerate_new(udev), ::udev_enumerate_unref );
      if ( ! enumerate ) {
        ERR << "Can't create udev list entry." << std::endl;
      } else {
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
            ERR << "Can't create udev device." << std::endl;
            continue;
          }

          const char * devnodePtr( ::udev_device_get_devnode( device ) );
          if ( ! devnodePtr ) {
            ERR << "Got NULL devicenode." << std::endl;
            continue;
          }

          // In case we need it someday:
          //const char * mountpath = ::udev_device_get_property_value( device, "FSTAB_DIR" );
          auto dev = registerDevice( zypp::PathInfo ( devnodePtr ), "udev" );
          if ( dev != sysDevs.end() ) {
            if ( ::udev_device_get_property_value( device, "ID_CDROM_DVD" ) ) {
              (*dev)->_properties["DVD"] = true;
            } else {
              (*dev)->_properties["DVD"] = false;
            }
          }
        }
      }
#endif
      if ( sysDevs.empty() )
      {
        WAR << "CD/DVD drive detection with UDEV failed! Guessing..." << std::endl;
        auto dev = registerDevice( zypp::PathInfo ( "/dev/dvd" ), "GUESS" );
        if ( dev != sysDevs.end() && (*dev)->_properties.count("DVD") == 0 )
          (*dev)->_properties["DVD"] = true;
        dev = registerDevice( zypp::PathInfo ( "/dev/cdrom" ), "GUESS" );
        if ( dev != sysDevs.end() && (*dev)->_properties.count("DVD") == 0 )
          (*dev)->_properties["DVD"] = false;
      }
    }
  }
}
