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



DiskProvider::DiskProvider( std::string_view workerName )
  : MountingWorker( zyppng::worker::WorkerCaps::SimpleMount, workerName )
{ }

DiskProvider::~DiskProvider()
{ }

void DiskProvider::handleMountRequest ( zyppng::worker::ProvideWorkerItem &req )
{
  if ( req._spec.code() != zyppng::ProvideMessage::Code::Attach ) {
    req._state = zyppng::worker::ProvideWorkerItem::Finished;
    provideFailed( req._spec.requestId()
      , zyppng::ProvideMessage::Code::MountFailed
      , "Unsupported message for handleMountRequest"
      , false
      , {} );
    return;
  }

  try
  {
    const auto attachUrl = zypp::Url( req._spec.value( zyppng::AttachMsgFields::Url ).asString() );
    if ( !attachUrl.getHost().empty() ) {
      req._state = zyppng::worker::ProvideWorkerItem::Finished;
      provideFailed( req._spec.requestId()
        , zyppng::ProvideMessage::Code::MountFailed
        , "Host must be empty in dir:// and file:// URLs"
        , false
        , {} );
      return;
    }

    const std::string &device = zypp::Pathname(attachUrl.getQueryParam("device")).asString();
    if ( device.empty() ) {
      req._state = zyppng::worker::ProvideWorkerItem::Finished;
      provideFailed( req._spec.requestId()
        , zyppng::ProvideMessage::Code::MountFailed
        , "Media url does not contain a device specification"
        , false
        , {} );
      return;
    }

    std::string filesystem = attachUrl.getQueryParam("filesystem");
    if( filesystem.empty() )
      filesystem="auto";

    zypp::PathInfo dev_info( device );
    if(!dev_info.isBlk()) {
      req._state = zyppng::worker::ProvideWorkerItem::Finished;
      provideFailed( req._spec.requestId()
        , zyppng::ProvideMessage::Code::MountFailed
        , "Media url does not specify a valid block device"
        , false
        , {} );
      return;
    }

    DBG << "Verifying " << device << " ..." << std::endl;
    if( !verifyIfDiskVolume( device)) {
      req._state = zyppng::worker::ProvideWorkerItem::Finished;
      provideFailed( req._spec.requestId()
        , zyppng::ProvideMessage::Code::MountFailed
        , "Could not verify URL points to a disk volume!"
        , false
        , {} );
      return;
    }

    // disks can have a attach root ( path relative to the device root )
    const auto relAttachRoot = zypp::Pathname( attachUrl.getPathName() );

    // set up the verifier
    zyppng::MediaDataVerifierRef verifier;
    zyppng::MediaDataVerifierRef devVerifier;
    if ( req._spec.value( zyppng::AttachMsgFields::VerifyType ).valid() ) {
      verifier = zyppng::MediaDataVerifier::createVerifier( req._spec.value(zyppng::AttachMsgFields::VerifyType).asString() );
      devVerifier = zyppng::MediaDataVerifier::createVerifier( req._spec.value(zyppng::AttachMsgFields::VerifyType).asString() );
      if ( !verifier || !devVerifier ) {
        provideFailed( req._spec.requestId()
          , zyppng::ProvideMessage::Code::MountFailed
          , "Invalid verifier type"
          , false
          , {} );
        return;
      }

      if ( !verifier->load( req._spec.value(zyppng::AttachMsgFields::VerifyData).asString() ) ) {
        provideFailed( req._spec.requestId()
          , zyppng::ProvideMessage::Code::MountFailed
          , "Failed to create verifier from file"
          , false
          , {} );
        return;
      }
    }
    const auto &devs = knownDevices();

    // first check if we have that device already
    auto i = std::find_if( devs.begin(), devs.end(), [&]( const auto &d ) {
      return d->_maj_nr == dev_info.devMajor()
          && d->_min_nr == dev_info.devMinor();
    });
    if ( i != devs.end() ) {
      auto res = isDesiredMedium( attachUrl, (*i)->_mountPoint / relAttachRoot, verifier, req._spec.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt() );
      if ( !res ) {
        try {
          std::rethrow_exception( res.error() );
        } catch( const zypp::Exception& e ) {
          provideFailed( req._spec.requestId()
            , zyppng::ProvideMessage::Code::MediumNotDesired
            , false
            , e );
        } catch ( ... ) {
          provideFailed( req._spec.requestId()
            , zyppng::ProvideMessage::Code::MediumNotDesired
            , "Checking the medium failed with an uknown error"
            , false
            , {} );
        }
        return;
      } else {
        attachedMedia().insert( std::make_pair( req._spec.value(zyppng::AttachMsgFields::AttachId ).asString(), zyppng::worker::AttachedMedia{ *i, relAttachRoot } ) );
        attachSuccess( req._spec.requestId() );
        return;
      }
    }

    // we did not find a existing mount, well lets make a new one ...
    std::optional<zypp::Pathname> bindSource;
    auto devPtr = std::make_shared<zyppng::worker::Device>( zyppng::worker::Device{
      ._name   = dev_info.path().asString(),
      ._maj_nr = dev_info.devMajor(),
      ._min_nr = dev_info.devMinor(),
      ._ephemeral = true // forget about the device after we are finished with it
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
        provideFailed( req._spec.requestId()
          , zyppng::ProvideMessage::Code::MountFailed
          , "Failed to create mount directory."
          , false
          , {} );
        return;
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

    }
    catch ( const zypp::Exception &e ) {
      removeAttachPoint(newAp);
      ZYPP_CAUGHT(e);
      provideFailed( req._spec.requestId()
        , zyppng::ProvideMessage::Code::MountFailed
        , false
        , e );
      return;
    }
    catch ( ... ) {
      removeAttachPoint(newAp);
      std::rethrow_exception( std::current_exception() );
    }

    // mount worked ! YAY
    devPtr->_mountPoint = newAp;
    knownDevices().push_back( devPtr );
    attachedMedia().insert( std::make_pair( req._spec.value(zyppng::AttachMsgFields::AttachId ).asString(), zyppng::worker::AttachedMedia{ devPtr, relAttachRoot } ) );
    attachSuccess( req._spec.requestId() );
    return;

  }  catch ( const zypp::Exception &e  ) {
    req._state = zyppng::worker::ProvideWorkerItem::Finished;
    provideFailed( req._spec.requestId()
      , zyppng::ProvideMessage::Code::BadRequest
      , false
      , e );
    return;
  }  catch ( const std::exception &e  ) {
    req._state = zyppng::worker::ProvideWorkerItem::Finished;
    provideFailed( req._spec.requestId()
      , zyppng::ProvideMessage::Code::BadRequest
      , e.what()
      , false
      , {} );
    return;
  }  catch ( ... ) {
    req._state = zyppng::worker::ProvideWorkerItem::Finished;
    provideFailed( req._spec.requestId()
      , zyppng::ProvideMessage::Code::BadRequest
      , "Unknown exception"
      , false
      , {} );
    return;
  }
}
