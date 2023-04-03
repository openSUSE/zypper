/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include "diskprovider.h"
#include <zypp-media/ng/private/providedbg_p.h>
#include <zypp-media/ng/MediaVerifier>
#include <zypp-media/Mount>

#include <zypp-core/fs/TmpPath.h>
#include <zypp-core/fs/PathInfo.h>

#include <iostream>
#include <fstream>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "DiskProvider"


/*!
 * Check if specified device file name is
 * a disk volume device or throw an error.
 */
bool verifyIfDiskVolume( const zypp::Pathname &dev_name )
{
  if( dev_name.empty() ||
      dev_name.asString().compare(0, sizeof("/dev/")-1, "/dev/"))
  {
    ERR << "Specified device name " << dev_name
        << " is not allowed" << std::endl;
    return false;
  }

  zypp::PathInfo dev_info(dev_name);
  if( !dev_info.isBlk())
  {
    ERR << "Specified device name " << dev_name
        << " is not a block device" << std::endl;
    return false;
  }

  // check if a volume using /dev/disk/by-uuid links first
  {
    zypp::Pathname            dpath("/dev/disk/by-uuid");
    std::list<zypp::Pathname> dlist;
    if( zypp::filesystem::readdir(dlist, dpath) == 0)
    {
      std::list<zypp::Pathname>::const_iterator it;
      for(it = dlist.begin(); it != dlist.end(); ++it)
      {
        zypp::PathInfo vol_info(*it);
        if( vol_info.isBlk() && vol_info.devMajor() == dev_info.devMajor() &&
                                vol_info.devMinor() == dev_info.devMinor())
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
    zypp::Pathname            dpath("/dev/disk/by-label");
    std::list<zypp::Pathname> dlist;
    if( zypp::filesystem::readdir(dlist, dpath) == 0)
    {
      std::list<zypp::Pathname>::const_iterator it;
      for(it = dlist.begin(); it != dlist.end(); ++it)
      {
        zypp::PathInfo vol_info(*it);
        if( vol_info.isBlk() && vol_info.devMajor() == dev_info.devMajor() &&
                                vol_info.devMinor() == dev_info.devMinor())
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

  // check if a filesystem volume using the 'blkid' tool
  // (there is no /dev/disk link for some of them)
  zypp::ExternalProgram::Arguments args;
  args.push_back( "blkid" );
  args.push_back( "-p" );
  args.push_back( dev_name.asString() );

  zypp::ExternalProgram cmd( args, zypp::ExternalProgram::Stderr_To_Stdout );
  cmd >> DBG;
  if ( cmd.close() != 0 )
  {
    ERR << cmd.execError()
        << "\nSpecified device name " << dev_name
        << " is not a usable disk volume"
        << std::endl;
    return false;
  }
  return true;
}



DiskProvider::DiskProvider()
  : DeviceDriver( zyppng::worker::WorkerCaps::SimpleMount )
{ }

DiskProvider::~DiskProvider()
{ }

zyppng::worker::AttachResult DiskProvider::mountDevice ( const uint32_t id, const zypp::Url &attachUrl, const std::string &attachId, const std::string &label, const zyppng::HeaderValueMap &extras )
{
  try
  {
    if ( !attachUrl.getHost().empty() ) {
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , "Host must be empty in dir:// and file:// URLs"
        , false
        );
    }

    const std::string device = zypp::Pathname(attachUrl.getQueryParam("device")).asString();
    if ( device.empty() ) {
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , "Media url does not contain a device specification"
        , false
      );
    }

    std::string filesystem = attachUrl.getQueryParam("filesystem");
    if( filesystem.empty() )
      filesystem="auto";

    zypp::PathInfo dev_info( device );
    if(!dev_info.isBlk()) {
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , "Media url does not specify a valid block device"
        , false
        );
    }

    DBG << "Verifying " << device << " ..." << std::endl;
    if( !verifyIfDiskVolume( device)) {
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , "Could not verify URL points to a disk volume!"
        , false
        );
    }

    // disks can have a attach root ( path relative to the device root )
    const auto relAttachRoot = zypp::Pathname( attachUrl.getPathName() );

    // set up the verifier
    zyppng::MediaDataVerifierRef verifier;
    if ( extras.value( zyppng::AttachMsgFields::VerifyType ).valid() ) {
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
    const auto &devs = knownDevices();

    // first check if we have that device already
    auto i = std::find_if( devs.begin(), devs.end(), [&]( const auto &d ) {
      return d->_maj_nr == dev_info.devMajor()
          && d->_min_nr == dev_info.devMinor();
    });
    if ( i != devs.end() ) {
      auto res = isDesiredMedium( attachUrl, (*i)->_mountPoint / relAttachRoot, verifier, extras.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt() );
      if ( !res ) {
        try {
          std::rethrow_exception( res.error() );
        } catch( const zypp::Exception& e ) {
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MediumNotDesired
            , false
            , e
          );
        } catch ( ... ) {
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MediumNotDesired
            , "Checking the medium failed with an uknown error"
            , false
          );
        }
      } else {
        attachedMedia().insert( std::make_pair( attachId, zyppng::worker::AttachedMedia{ *i, relAttachRoot } ) );
        return zyppng::worker::AttachResult::success();
      }
    }

    // we did not find a existing mount, well lets make a new one ...
    std::optional<zypp::Pathname> bindSource;
    auto devPtr = std::make_shared<zyppng::worker::Device>( zyppng::worker::Device{
      ._name   = dev_info.path().asString(),
      ._maj_nr = dev_info.devMajor(),
      ._min_nr = dev_info.devMinor(),
      ._mountPoint = {},
      ._ephemeral = true, // forget about the device after we are finished with it
      ._properties = {}
      });

    // since the kernel will not let us remount a disc ro if it was already mounted in the system as rw we need to
    // go over the existing mounts and revert to a bind mount if we find that the device has a mountpoint already (#163486).
    zypp::media::MountEntries  entries( zypp::media::Mount::getEntries() );
    for( auto e = entries.cbegin(); e != entries.cend(); ++e)
    {
      bool            is_device = false;
      std::string     dev_path(zypp::Pathname(e->src).asString());
      zypp::PathInfo  dev_info;

      if( dev_path.compare(0, sizeof("/dev/")-1, "/dev/") == 0 &&
          dev_info(e->src) && dev_info.isBlk()) {
        is_device = true;
      }

      if( is_device &&  devPtr->_maj_nr == dev_info.devMajor() &&
                        devPtr->_min_nr == dev_info.devMinor())
      {
        DBG << "Device " << devPtr->_name << " is already mounted, using bind mount!" << std::endl;
        bindSource = e->dir;
        break;
      }
    }

    zypp::media::Mount mount;
    zypp::Pathname newAp;
    try {
      newAp = createAttachPoint( attachRoot() );
      if ( newAp.empty() ) {
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MountFailed
          , "Failed to create mount directory."
          , false
        );
      }

      std::string options = attachUrl.getQueryParam("mountoptions");
      if(options.empty()) {
        options = "ro";
      }

      if( bindSource ) {
        options += ",bind";
        mount.mount( bindSource->asString(), newAp.asString(), "none", options);
      } else {
        mount.mount( dev_info.path().asString(), newAp.asString(), filesystem, options);
      }

      // wait for /etc/mtab update ...
      // (shouldn't be needed)
      int limit = 3;
      bool mountsucceeded = false;

      const auto &checkAttachedDisk = [&]( const zypp::media::MountEntry &e ) {
        if ( bindSource) {
          if ( *bindSource == e.src ) {
            DBG << "Found bound media "
                << devPtr->_name
                << " in the mount table as " << e.src << std::endl;
            return true;
          }
        }
        return DeviceDriver::devicePredicate( devPtr->_maj_nr, devPtr->_min_nr ) ( e );
      };

      // mount command came back OK, lets wait for mtab to update
      while( !(mountsucceeded=checkAttached( newAp, checkAttachedDisk )) && --limit) {
        MIL << "Mount did not appear yet, sleeping for 1s" << std::endl;
        sleep(1);
      }

      // mount didn't work after all, bail out
      if ( !mountsucceeded ) {
        try {
          mount.umount( newAp.asString() );
        } catch (const zypp::media::MediaException & excpt_r) {
          ZYPP_CAUGHT(excpt_r);
        }
        ZYPP_THROW( zypp::media::MediaMountException(
          "Unable to verify that the media was mounted",
          devPtr->_name, newAp.asString()
        ));
      }

      // if we reach this place, mount worked -> YAY, lets see if that is the desired medium!
      auto isDesired = isDesiredMedium( attachUrl, newAp / relAttachRoot, verifier, extras.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt() );
      if ( !isDesired ) {
        try {
          mount.umount( newAp.asString() );
        } catch (const zypp::media::MediaException & excpt_r) {
          ZYPP_CAUGHT(excpt_r);
        }
        ZYPP_THROW( zypp::media::MediaNotDesiredException( attachUrl ) );
      }
    }
    catch ( const zypp::Exception &e ) {
      removeAttachPoint(newAp);
      ZYPP_CAUGHT(e);
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , false
        , e
      );
    }
    catch ( ... ) {
      removeAttachPoint(newAp);
      std::rethrow_exception( std::current_exception() );
    }

    // mount worked ! YAY
    devPtr->_mountPoint = newAp;
    knownDevices().push_back( devPtr );
    attachedMedia().insert( std::make_pair( attachId, zyppng::worker::AttachedMedia{ devPtr, relAttachRoot } ) );
    return zyppng::worker::AttachResult::success();

  }  catch ( const zypp::Exception &e  ) {
    return zyppng::worker::AttachResult::error(
      zyppng::ProvideMessage::Code::BadRequest
      , false
      , e
    );
  }  catch ( const std::exception &e  ) {
    return zyppng::worker::AttachResult::error(
      zyppng::ProvideMessage::Code::BadRequest
      , e.what()
      , false
    );
  }  catch ( ... ) {
    return zyppng::worker::AttachResult::error(
      zyppng::ProvideMessage::Code::BadRequest
      , "Unknown exception"
      , false
    );
  }
}
