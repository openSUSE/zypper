/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include "testvmprovider.h"
#include <zypp-media/ng/private/providedbg_p.h>
#include <zypp-media/ng/worker/ProvideWorker>
#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/AutoDispose.h>
#include <zypp-core/base/StringV.h>
#include <zypp-media/ng/MediaVerifier>
#include <zypp-proto/tvm.pb.h>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "TestVMProvider"

#include <iostream>
#include <fstream>

extern "C"
{
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#if HAVE_UDEV
#include <libudev.h>
#endif
}

using namespace std::literals;

constexpr std::string_view CONTENTDIR_PROP = "contentDir";

TestVMProvider::TestVMProvider( ) : zyppng::worker::DeviceDriver( zyppng::worker::WorkerCaps::VolatileMount )
{ }

TestVMProvider::~TestVMProvider()
{ }

zyppng::expected<zyppng::worker::WorkerCaps> TestVMProvider::initialize(const zyppng::worker::Configuration &conf)
{
  const auto &values = conf.values();

  if ( const auto &i = values.find( std::string(zyppng::PROVIDER_ROOT) ); i != values.end() ) {
    const auto &val = i->second;
    MIL << "Got provider root from controller: " << val << std::endl;
    _provRoot = val;
  } else {
    return zyppng::expected<zyppng::worker::WorkerCaps>::error(ZYPP_EXCPT_PTR( zypp::Exception("Provider root required to work.") ));
  }

  return DeviceDriver::initialize( conf );
}

zyppng::worker::AttachResult TestVMProvider::mountDevice ( const uint32_t id, const zypp::Url &attachUrl, const std::string &attachId, const std::string &label, const zyppng::HeaderValueMap &extras )
{
  std::vector<std::string> devs;
  if ( extras.contains( zyppng::AttachMsgFields::Device ) ) {
    for ( const auto &d : extras.values( zyppng::AttachMsgFields::Device ) ) {
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
      std::shared_ptr<zyppng::worker::Device> device;
      auto i = std::find_if( sysDevs.begin(), sysDevs.end(), [&]( const auto &sysDev ){ return ( sysDev->_name == d ); } );
      if ( i == sysDevs.end() ) {
          ERR << "Device " << d << " from Attach request is not known ignoring!" << std::endl;
          continue;
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

  const auto attachRoot    = zypp::Pathname( attachUrl.getPathName() );
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

    MIL << "Found mounted device: " << dev->_name << " on mountpoint: " << dev->_mountPoint << std::endl;

    auto res = isDesiredMedium( attachUrl, dev->_mountPoint / attachRoot, verifier, attachMediaNr );
    if ( !res )
      continue;

    MIL << "Found requested medium in dev " << dev->_name << std::endl;

    // we found the device we want!
    attachedMedia().insert( std::make_pair( attachId, zyppng::worker::AttachedMedia{ dev, attachRoot } ) );
    return zyppng::worker::AttachResult::success();
  }

  while ( true ) {

    // pick up changes from the test
    detectDevices();

    // remember how many devices we were able to test
    uint devicesTested = 0;

    // none of the already mounted devices matched, lets try what we have left
    for( const auto &dev : possibleDevs ) {
      if ( !dev->_mountPoint.empty() )
        continue;

      MIL << "Trying to mount dev " << dev->_name << std::endl;

      devicesTested++;

      bool mountsucceeded = false;

      // we simply create a symbolic link in the attach dir to simulate mounting
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

        newAp = newAp/"mount";

        // simulate failed mount
        const auto &contDir = std::any_cast<zypp::Pathname>(dev->_properties[std::string(CONTENTDIR_PROP)]);
        if ( contDir.empty() ) {
          ZYPP_THROW( zypp::media::MediaMountException(
            "Failed to mount device",
            dev->_name, newAp.asString()
            ));
        }

        const auto res = zypp::filesystem::symlink( contDir, newAp );
        mountsucceeded = ( res == 0 );

        if( !mountsucceeded) {
          ZYPP_THROW( zypp::media::MediaMountException(
            "Failed to mount device",
            dev->_name, newAp.asString()
            ));
        } else {
          MIL << "Device mounted on " << newAp << std::endl;
          dev->_mountPoint = newAp;
        }
      } catch (const zypp::media::MediaException & excpt_r) {
        removeAttachPoint(newAp.dirname());
        ZYPP_CAUGHT(excpt_r);
      }

      if ( mountsucceeded ) {
        // lets check if we have the correct medium
        auto res = isDesiredMedium( attachUrl, dev->_mountPoint / attachRoot, verifier, attachMediaNr );
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

      // k, we need to ask the user to give us the medium we need
      // first find the devices that are free
      std::vector<std::string> freeDevs;

      for( const auto &dev : possibleDevs ) {
        if ( !dev->_mountPoint.empty() )
          continue;

        MIL << "Adding " << dev->_name << " to list of free devs" << std::endl;
        freeDevs.push_back( dev->_name );
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
      auto parent = parentWorker();
      if ( parent )
        changeRes = parent->requestMediaChange ( id, label, attachMediaNr, freeDevs );
      if ( changeRes != zyppng::worker::ProvideWorker::SUCCESS ) {
        if ( changeRes == zyppng::worker::ProvideWorker::SKIP) {
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MediaChangeSkip
            , "User asked to skip the media change."
            , false
          );
        } else {
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MediaChangeAbort
            , "User asked to abort the media change."
            , false
          );
        }
      }
    }
  }
}

void TestVMProvider::unmountDevice( zyppng::worker::Device &dev )
{
  if ( dev._mountPoint.empty () )
    return;
  try {
    zypp::filesystem::unlink( dev._mountPoint );
    removeAttachPoint( dev._mountPoint.dirname() );
  } catch (const zypp::media::MediaException & excpt_r) {
    ERR << "Failed to unmount device: " << dev._name << std::endl;
    ZYPP_CAUGHT(excpt_r);
  }
  dev._mountPoint = zypp::Pathname();
}

void TestVMProvider::detectDevices( )
{
  std::ifstream in( (_provRoot/"tvm.conf").asString(), std::iostream::binary );
  zypp::proto::test::TVMSettings set;
  if ( !set.ParseFromIstream( &in ) ) {
    MIL << "No devices configured!" << std::endl;
    return;
  }

  auto &sysDevs = knownDevices();

  if ( sysDevs.size() ) {
    for ( int i = 0; i < set.devices_size(); i++ ) {
      const auto &dev = set.devices(i);
      auto iDev = std::find_if( sysDevs.begin(), sysDevs.end(), [&]( const auto &d ){
        return d->_name == dev.name();
      });
      if ( iDev == sysDevs.end() ) {
        MIL << "Previously unknown device detected" << std::endl;
        auto d = std::make_shared<zyppng::worker::Device>( zyppng::worker::Device{
          ._name = dev.name()
        });
        if ( dev.insertedpath().size() ) {
          d->_properties[std::string(CONTENTDIR_PROP)] = zypp::Pathname(dev.insertedpath());
        }
        sysDevs.push_back(d);
        continue;
      }

      // mounted devices are never updated
      if ( !(*iDev)->_mountPoint.empty() ) {
        continue;
      }

      (*iDev)->_properties[std::string(CONTENTDIR_PROP)] = zypp::Pathname(dev.insertedpath());
    }
  } else {
    for ( int i = 0; i < set.devices_size(); i++ ) {
      const auto &dev = set.devices(i);
      MIL << "Found device: " << dev.name() << std::endl;
      auto d = std::make_shared<zyppng::worker::Device>( zyppng::worker::Device{
        ._name = dev.name()
      });
      if ( dev.insertedpath().size() ) {
        d->_properties[std::string(CONTENTDIR_PROP)] = zypp::Pathname(dev.insertedpath());
      }
      sysDevs.push_back(d);
    }
  }
}
