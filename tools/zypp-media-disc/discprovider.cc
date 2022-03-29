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
#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/AutoDispose.h>
#include <zypp-core/base/StringV.h>
#include <zypp-media/Mount>
#include <zypp-media/ng/MediaVerifier>
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

DiscProvider::DiscProvider(std::string_view workerName) : zyppng::worker::ProvideWorker( workerName )
{ }

DiscProvider::~DiscProvider()
{}

zyppng::expected<zyppng::worker::WorkerCaps> DiscProvider::initialize(const zyppng::worker::Configuration &conf)
{
  const auto &values = conf.values();
  if ( const auto &i = values.find( std::string(zyppng::ATTACH_POINT) ); i != values.end() ) {
    const auto &val = i->second;
    MIL << "Got attachpoint from controller: " << val << std::endl;
    _attachRoot = val;
  } else {
    return zyppng::expected<zyppng::worker::WorkerCaps>::error(ZYPP_EXCPT_PTR( zypp::Exception("Attach point required to work.") ));
  }

  zyppng::worker::WorkerCaps caps;
  caps.set_worker_type ( zyppng::worker::WorkerCaps::VolatileMount );
  caps.set_cfg_flags(
    zyppng::worker::WorkerCaps::Flags (
      zyppng::worker::WorkerCaps::Pipeline
      | zyppng::worker::WorkerCaps::ZyppLogFormat
      | zyppng::worker::WorkerCaps::SingleInstance
      )
    );

  return zyppng::expected<zyppng::worker::WorkerCaps>::success(caps);
}

void DiscProvider::provide()
{
  detectDevices();
  auto &queue = requestQueue();

  if ( !queue.size() )
    return;

  auto req = queue.front();
  queue.pop_front();

  MIL_PRV << "Received provide: " << req->_spec.code() << std::endl;

  try {
    switch ( req->_spec.code () ) {
      case zyppng::ProvideMessage::Code::Attach: {

        std::vector<std::string> devs;
        for ( const auto &d : req->_spec.values( zyppng::AttachMsgFields::Device ) ) {
          if ( d.isString() ) {
            MIL << "Got device from user we are allowed to use: " << d.asString() << std::endl;
            devs.push_back(d.asString());
          }
        }

        std::vector<std::shared_ptr<Device>> possibleDevs;
        if ( devs.size () ) {
          for ( const auto &d : devs ) {
            zypp::PathInfo p(d);
            if ( !p.isBlk () )
              continue;

            std::shared_ptr<Device> device;
            auto i = std::find_if( _sysDevs.begin(), _sysDevs.end(), [&p]( const auto &sysDev ){ return ( sysDev->_maj_nr == p.devMajor() && sysDev->_min_nr == p.devMinor() ); } );
            if ( i == _sysDevs.end() ) {
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

              auto dev = std::make_shared<Device>( Device{ p.path().asString(), p.devMajor(), p.devMinor() } );
              DBG << "Registering device (REQUEST): " << dev->_path << " " << dev->_maj_nr << ":" << dev->_min_nr << std::endl;
              _sysDevs.push_back ( dev );
              device = dev;
#endif
            } else {
              device = *i;
            }

            if ( device )
              possibleDevs.push_back(device);
          }
        } else {
          possibleDevs = _sysDevs;
        }

        // if we have no devices at this point controller gets a error
        if ( possibleDevs.empty () ) {
          req->_state = zyppng::worker::ProvideWorkerItem::Finished;
          provideFailed( req->_spec.requestId()
            , zyppng::ProvideMessage::Code::MountFailed
            , "No useable device found"
            , false
            , {} );
          return;
        }

        const auto attachUrl = zypp::Url( req->_spec.value( zyppng::AttachMsgFields::Url ).asString() );
        const auto attachRoot = zypp::Pathname( attachUrl.getPathName() );
        const auto attachMediaNr = req->_spec.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt();

        // set up the verifier
        zyppng::MediaDataVerifierRef verifier;
        zyppng::MediaDataVerifierRef devVerifier;
        if ( req->_spec.value( zyppng::AttachMsgFields::VerifyType ).valid() ) {
          verifier = zyppng::MediaDataVerifier::createVerifier( req->_spec.value(zyppng::AttachMsgFields::VerifyType).asString() );
          devVerifier = zyppng::MediaDataVerifier::createVerifier( req->_spec.value(zyppng::AttachMsgFields::VerifyType).asString() );
          if ( !verifier || !devVerifier ) {
            provideFailed( req->_spec.requestId()
              , zyppng::ProvideMessage::Code::MountFailed
              , "Invalid verifier type"
              , false
              , {} );
            return;
          }

          if ( !verifier->load( req->_spec.value(zyppng::AttachMsgFields::VerifyData).asString() ) ) {
            provideFailed( req->_spec.requestId()
              , zyppng::ProvideMessage::Code::MountFailed
              , "Failed to create verifier from file"
              , false
              , {} );
            return;
          }
        }

        //first check if any of the mounted devices are what we want
        for( const auto &dev : possibleDevs ) {
          if ( dev->_mountPoint.empty() )
            continue;

          if ( verifier ) {
            if ( !devVerifier->loadFromMedium( dev->_mountPoint / attachRoot , req->_spec.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt() ) ) {
              continue;
            }

            if ( !verifier->matches(devVerifier) )
              continue;

            MIL << "Found requested medium in dev " << dev->_path << std::endl;

            // we found the device we want!
            _attachedMedia.insert( std::make_pair( req->_spec.value(zyppng::AttachMsgFields::AttachId ).asString(), AttachedMedia{ dev, attachRoot } ) );
            attachSuccess( req->_spec.requestId() );
            return;
          }
        }

        std::list<std::string> filesystems;

        filesystems.push_back("iso9660");

        // if DVD, try UDF filesystem after iso9660
        if ( attachUrl.getScheme() == "dvd" )
          filesystems.push_back("udf");

        std::string options;
        auto optVal = req->_spec.value( "mountoptions"sv );
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

            MIL << "Trying to mount dev " << dev->_path << std::endl;

            // make sure the tray is closed
            zypp::media::CDTools::closeTray( dev->_path );

            devicesTested++;
            zypp::media::Mount mount;
            bool mountsucceeded = false;
            std::exception_ptr lastErr;
            for( auto fsit = filesystems.begin() ; !mountsucceeded && fsit != filesystems.end() ; ++fsit) {

              zypp::Pathname newAp;
              try {
                newAp = createAttachPoint( _attachRoot );
                if ( newAp.empty() ) {
                  provideFailed( req->_spec.requestId()
                    , zyppng::ProvideMessage::Code::MountFailed
                    , "Failed to create mount directory."
                    , false
                    , {} );
                  return;
                }

                mount.mount( dev->_path, newAp.asString(), *fsit, options);

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
                    dev->_path, newAp.asString()
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
              if ( verifier && devVerifier ) {

                if ( !devVerifier->loadFromMedium( dev->_mountPoint / attachRoot, attachMediaNr ) ) {
                  unmountDevice( *dev );
                  continue;
                }

                if ( !verifier->matches(devVerifier) ) {
                  unmountDevice( *dev );
                  continue;
                }

                MIL << "Found requested medium in dev " << dev->_path << std::endl;

                canUseDevice = true;
              } else {
                // if we have no verifier we use the first device that mounts and has the attach root
                zypp::PathInfo p( dev->_mountPoint / attachRoot );
                canUseDevice = p.isExist() && p.isDir();
              }

              // we found a compatible medium
              if ( canUseDevice ) {
                _attachedMedia.insert( std::make_pair( req->_spec.value(zyppng::AttachMsgFields::AttachId ).asString(), AttachedMedia{ dev, attachRoot } ) );
                attachSuccess( req->_spec.requestId() );
                return;

              } else {
                unmountDevice( *dev );
              }
            }
          } // for each device

          // we did go through all devices, but didn't find any.
          if ( devicesTested == 0 ) {
            // no devices are free
            provideFailed( req->_spec.requestId()
              , zyppng::ProvideMessage::Code::Jammed
              , "No free ressources available"
              , false
              , {} );
            return;
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
                MIL << "Adding " << dev->_path << " to list of free devs" << std::endl;
                freeDevs.push_back( dev->_path );
              }
            }

            // if there are no devices free, we are jammed
            if ( freeDevs.empty() ) {
              provideFailed( req->_spec.requestId()
                , zyppng::ProvideMessage::Code::Jammed
                , "No free ressources available"
                , false
                , {} );
              return;
            }

            const auto &changeRes = requestMediaChange ( req->_spec.requestId (), req->_spec.value( zyppng::AttachMsgFields::Label, "No label" ).asString(), attachMediaNr, freeDevs );
            if ( changeRes != SUCCESS ) {
              if ( changeRes == SKIP)
                provideFailed( req->_spec.requestId()
                  , zyppng::ProvideMessage::Code::MediaChangeSkip
                  , "User asked to skip the media change."
                  , false
                  , {} );
              else
                provideFailed( req->_spec.requestId()
                  , zyppng::ProvideMessage::Code::MediaChangeAbort
                  , "User asked to abort the media change."
                , false
                  , {} );
              return;
            }
          }
        }
        break;
      }
      case zyppng::ProvideMessage::Code::Detach: {

        const auto url = zypp::Url( req->_spec.value( zyppng::DetachMsgFields::Url ).asString() );
        const auto &attachId = url.getAuthority();
        auto i = _attachedMedia.find( attachId );
        if ( i == _attachedMedia.end() ) {
          provideFailed( req->_spec.requestId()
            , zyppng::ProvideMessage::Code::NotFound
            , "Attach ID not known."
            , false
            , {} );
          return;
        }

        _attachedMedia.erase(i);
        detachSuccess ( req->_spec.requestId() );
        // now unmount all others
        for ( auto i = _sysDevs.begin (); i != _sysDevs.end(); i++ ) {
          if ( i->use_count() == 1 && !(*i)->_mountPoint.empty() ) {
            MIL << "Unmounting device " << (*i)->_path << " since its not used anymore" << std::endl;
            unmountDevice(*(*i));
          }
        }
        return;
      }
      case zyppng::ProvideMessage::Code::Provide: {

        const auto url = zypp::Url( req->_spec.value( zyppng::DetachMsgFields::Url ).asString() );
        const auto &attachId = url.getAuthority();
        const auto &path = zypp::Pathname(url.getPathName());

        auto i = _attachedMedia.find( attachId );
        if ( i == _attachedMedia.end() ) {
          ERR << "Unknown Attach ID " << attachId << std::endl;
          provideFailed( req->_spec.requestId()
            , zyppng::ProvideMessage::Code::NotFound
            , "Attach ID not known."
            , false
            , {} );
          return;
        }

        const auto &locPath = i->second._dev->_mountPoint / i->second._attachRoot / path;

        MIL << "Trying to find file: " << locPath << std::endl;

        zypp::PathInfo info( locPath );
        if( info.isFile() ) {
          provideSuccess ( req->_spec.requestId(), false, locPath );
          return;
        }

        if (info.isExist())
          provideFailed( req->_spec.requestId()
            , zyppng::ProvideMessage::Code::NotAFile
            , zypp::str::Str() << "Path " << path << " exists, but its not a file"
            , false
            , {} );
        else
          provideFailed( req->_spec.requestId()
            , zyppng::ProvideMessage::Code::NotFound
            , zypp::str::Str() << "File " << path << " not found on medium"
            , false
            , {} );


        break;
      }
      default: {
        req->_state = zyppng::worker::ProvideWorkerItem::Finished;
        provideFailed( req->_spec.requestId()
          , zyppng::ProvideMessage::Code::BadRequest
          , "Request type not implemented"
          , false
          , {} );
        return;
      }
    }
  }  catch ( const zypp::Exception &e  ) {
    req->_state = zyppng::worker::ProvideWorkerItem::Finished;
    provideFailed( req->_spec.requestId()
      , zyppng::ProvideMessage::Code::BadRequest
      , e.asString()
      , false
      , {} );
    return;
  }  catch ( const std::exception &e  ) {
    req->_state = zyppng::worker::ProvideWorkerItem::Finished;
    provideFailed( req->_spec.requestId()
      , zyppng::ProvideMessage::Code::BadRequest
      , e.what()
      , false
      , {} );
    return;
  }  catch ( ... ) {
    req->_state = zyppng::worker::ProvideWorkerItem::Finished;
    provideFailed( req->_spec.requestId()
      , zyppng::ProvideMessage::Code::BadRequest
      , "Unknown exception"
      , false
      , {} );
    return;
  }
}

void DiscProvider::cancel( const std::deque<zyppng::worker::ProvideWorkerItemRef>::iterator & )
{
  ERR << "Bug, cancel should never be called for running items" << std::endl;
}

void DiscProvider::immediateShutdown()
{
  // here we need to unmount everything
  for ( auto &dev : _sysDevs )
    unmountDevice(*dev);
  _attachedMedia.clear();
}

void DiscProvider::unmountDevice( Device &dev )
{
  if ( dev._mountPoint.empty () )
    return;
  try {
    zypp::media::Mount mount;
    mount.umount( dev._mountPoint.asString() );
    removeAttachPoint( dev._mountPoint );
  } catch (const zypp::media::MediaException & excpt_r) {
    ERR << "Failed to unmount device: " << dev._path << std::endl;
    ZYPP_CAUGHT(excpt_r);
  }
  dev._mountPoint = zypp::Pathname();
}

void DiscProvider::detectDevices( )
{
  // helper lambda to add a new device to the internal registry, returning it if its either created or already known
  const auto registerDevice = [ this ]( const zypp::PathInfo &devnode, std::string_view reason ) {
    if ( devnode.isBlk() ) {
      auto i = std::find_if( _sysDevs.begin(), _sysDevs.end(), [&]( const auto &dev ){
        return ( dev->_maj_nr == devnode.devMajor() && dev->_min_nr == devnode.devMinor () );
      });

      // we already know the device
      if ( i != _sysDevs.end() )
        return i;

      auto dev = std::make_shared<Device>( Device{ devnode.path().asString(), devnode.devMajor(), devnode.devMinor() } );
      DBG << "Found (" << reason << "): " << dev->_path << " " << dev->_maj_nr << ":" << dev->_min_nr << std::endl;

      _sysDevs.push_back(dev);
      return ( --_sysDevs.end() );
    }
    return _sysDevs.end();
  };

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
          if ( dev != _sysDevs.end() ) {
            if ( ::udev_device_get_property_value( device, "ID_CDROM_DVD" ) ) {
              (*dev)->_properties["DVD"] = true;
            } else {
              (*dev)->_properties["DVD"] = false;
            }
          }
        }
      }
#endif
      if ( _sysDevs.empty() )
      {
        WAR << "CD/DVD drive detection with UDEV failed! Guessing..." << std::endl;
        auto dev = registerDevice( zypp::PathInfo ( "/dev/dvd" ), "GUESS" );
        if ( dev != _sysDevs.end() && (*dev)->_properties.count("DVD") == 0 )
          (*dev)->_properties["DVD"] = true;
        dev = registerDevice( zypp::PathInfo ( "/dev/cdrom" ), "GUESS" );
        if ( dev != _sysDevs.end() && (*dev)->_properties.count("DVD") == 0 )
          (*dev)->_properties["DVD"] = false;
      }
    }
  }
}
