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

TestVMProvider::TestVMProvider(std::string_view workerName) : zyppng::worker::ProvideWorker( workerName )
{ }

TestVMProvider::~TestVMProvider()
{ }

zyppng::expected<zyppng::worker::WorkerCaps> TestVMProvider::initialize(const zyppng::worker::Configuration &conf)
{
  const auto &values = conf.values();
  if ( const auto &i = values.find( std::string(zyppng::ATTACH_POINT) ); i != values.end() ) {
    const auto &val = i->second;
    MIL << "Got attachpoint from controller: " << val << std::endl;
    _attachRoot = val;
  } else {
    return zyppng::expected<zyppng::worker::WorkerCaps>::error(ZYPP_EXCPT_PTR( zypp::Exception("Attach point required to work.") ));
  }

  if ( const auto &i = values.find( std::string(zyppng::PROVIDER_ROOT) ); i != values.end() ) {
    const auto &val = i->second;
    MIL << "Got provider root from controller: " << val << std::endl;
    _provRoot = val;
  } else {
    return zyppng::expected<zyppng::worker::WorkerCaps>::error(ZYPP_EXCPT_PTR( zypp::Exception("Provider root required to work.") ));
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

void TestVMProvider::provide()
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
            std::shared_ptr<Device> device;
            auto i = std::find_if( _sysDevs.begin(), _sysDevs.end(), [&]( const auto &sysDev ){ return ( sysDev->_name == d ); } );
            if ( i == _sysDevs.end() ) {
                ERR << "Device " << d << " from Attach request is not known ignoring!" << std::endl;
                continue;
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

        const auto attachUrl     = zypp::Url( req->_spec.value( zyppng::AttachMsgFields::Url ).asString() );
        const auto attachRoot    = zypp::Pathname( attachUrl.getPathName() );
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

          MIL << "Found mounted device: " << dev->_name << " on mountpoint: " << dev->_mountPoint << std::endl;

          if ( verifier ) {
            if ( !devVerifier->loadFromMedium( dev->_mountPoint / attachRoot , req->_spec.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt() ) ) {
              continue;
            }

            if ( !verifier->matches(devVerifier) )
              continue;

            MIL << "Found requested medium in dev " << dev->_name << std::endl;

            // we found the device we want!
            _attachedMedia.insert( std::make_pair( req->_spec.value(zyppng::AttachMsgFields::AttachId ).asString(), AttachedMedia{ dev, attachRoot } ) );
            attachSuccess( req->_spec.requestId() );
            return;
          }
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
              newAp = createAttachPoint( _attachRoot );
              if ( newAp.empty() ) {
                provideFailed( req->_spec.requestId()
                  , zyppng::ProvideMessage::Code::MountFailed
                  , "Failed to create mount directory."
                  , false
                  , {} );
                return;
              }

              newAp = newAp/"mount";

              // simulate failed mount
              if ( dev->_contentDir.empty() ) {
                ZYPP_THROW( zypp::media::MediaMountException(
                  "Failed to mount device",
                  dev->_name, newAp.asString()
                  ));
              }

              const auto res = zypp::filesystem::symlink( dev->_contentDir, newAp );
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

                MIL << "Found requested medium in dev " << dev->_name << std::endl;

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
            MIL << "Unmounting device " << (*i)->_name << " since its not used anymore" << std::endl;
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

void TestVMProvider::cancel( const std::deque<zyppng::worker::ProvideWorkerItemRef>::iterator & )
{
  ERR << "Bug, cancel should never be called for running items" << std::endl;
}

void TestVMProvider::immediateShutdown()
{
  // here we need to unmount everything
  for ( auto &dev : _sysDevs )
    unmountDevice(*dev);
  _attachedMedia.clear();
}

void TestVMProvider::unmountDevice( Device &dev )
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

  if ( _sysDevs.size() ) {
    for ( int i = 0; i < set.devices_size(); i++ ) {
      const auto &dev = set.devices(i);
      auto iDev = std::find_if( _sysDevs.begin(), _sysDevs.end(), [&]( const auto &d ){
        return d->_name == dev.name();
      });
      if ( iDev == _sysDevs.end() ) {
        MIL << "Previously unknown device detected" << std::endl;
        auto d = std::make_shared<Device>( Device{ dev.name() } );
        if ( dev.insertedpath().size() ) {
          d->_contentDir = zypp::Pathname(dev.insertedpath());
        }
        _sysDevs.push_back(d);
        continue;
      }

      // mounted devices are never updated
      if ( !(*iDev)->_mountPoint.empty() ) {
        continue;
      }

      (*iDev)->_contentDir = zypp::Pathname(dev.insertedpath());
    }
  } else {
    for ( int i = 0; i < set.devices_size(); i++ ) {
      const auto &dev = set.devices(i);
      MIL << "Found device: " << dev.name() << std::endl;
      auto d = std::make_shared<Device>( Device{ dev.name() } );
      if ( dev.insertedpath().size() ) {
        d->_contentDir = zypp::Pathname(dev.insertedpath());
      }
      _sysDevs.push_back(d);
    }
  }
}
