/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include "isoprovider.h"
#include <zypp-media/ng/private/providedbg_p.h>
#include <zypp-media/ng/MediaVerifier>

#include <zypp-core/fs/TmpPath.h>
#include <zypp-core/fs/PathInfo.h>

#include <iostream>
#include <fstream>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "IsoProvider"

RaiiHelper::~RaiiHelper()
{
  _backingDriver->detachMedia( id );
  _backingDriver->releaseIdleDevices();
}

IsoProvider::IsoProvider( )
  : DeviceDriver( zyppng::worker::WorkerCaps::SimpleMount )
{ }

IsoProvider::~IsoProvider()
{ }

zyppng::worker::AttachResult IsoProvider::mountDevice ( const uint32_t id, const zypp::Url &attachUrl, const std::string &attachId, const std::string &label, const zyppng::HeaderValueMap &extras )
{
  try
  {
    zypp::Pathname isofile = attachUrl.getQueryParam("iso");
    if( isofile.empty()) {
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , zypp::str::Str()<<"Media url: "<<attachUrl<<" does not contain iso filename"
        , false );
    }

    std::string filesystem = attachUrl.getQueryParam("filesystem");
    if( filesystem.empty())
      filesystem = "auto";

    zypp::Url src;
    {
      const std::string & arg { attachUrl.getQueryParam("url") };
      if ( arg.empty() ) {
        src = "dir:/";
        src.setPathName( isofile.dirname() );
        isofile = isofile.basename();
      }
      else try {
        src = arg;
      }
      catch( const zypp::url::UrlException & e )
      {
        ZYPP_CAUGHT(e);
        const std::string &err = zypp::str::Str()<<"Unable to parse iso filename source media url: "<<attachUrl;
        ERR << err << std::endl;
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MountFailed
          , err
          , false );
      }
    }

    // the full URL to the ISO file, we will use that to identify the device
    auto fullIsoUrl = src;
    fullIsoUrl.appendPathName( isofile );

    // the root of the medium inside the ISO file
    zypp::Pathname relAttachRoot = attachUrl.getPathName();
    if ( relAttachRoot.empty() )
      relAttachRoot = "/";

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

    // first check if we have that device already
    const auto &devs = knownDevices();
    auto i = std::find_if( devs.begin(), devs.end(), [&]( const auto &d ) {
      return d->_name == fullIsoUrl.asString();
    });
    if ( i != devs.end() )  {
      // if we can find the device, isDesiredMedium needs to return true,
      // otherwise URL and verifier do not match and its not the desired medium
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

    // helper lambda to invoke the deviceDriver
    const auto &attachViaHelper = [&]( auto &driver, const std::string &type ) {

      auto mountRes = driver->mountDevice( id, src, attachId, "", {} );
      if ( !mountRes ) {
        return mountRes;
      }

      // make sure the media is released as soon as its not needed anymore
      auto hlpr = std::make_shared<RaiiHelper>();
      hlpr->_backingDriver = driver;
      hlpr->id = attachId;

      // k, get the mountpoint from the backing driver
      const auto &attMedias = driver->attachedMedia();
      auto i = attMedias.find( attachId );
      if ( i == attMedias.end() ) {
        const std::string &err = zypp::str::Str()<< "Failed to query mounted supporting device: " << src.asString();
        ERR << err << std::endl;
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MountFailed
          , err
          , false );
      }

      // full path to the ISO in the filesystem
      auto isopath = zypp::filesystem::expandlink( i->second._dev->_mountPoint / isofile );
      if( isopath.empty() || !zypp::PathInfo(isopath).isFile()) {
        const std::string &err = zypp::str::Str()<< "No ISO file found on given source location: " << src.asString();
        ERR << err << std::endl;
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MountFailed
          , err
          , false );
      }

      // we have everything we need, let's try to mount it
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

        std::string mountpoint( newAp.asString() );
        std::string mountopts("ro,loop");

        mount.mount( isopath.asString(), mountpoint,
                     filesystem, mountopts );

        // wait for /etc/mtab update ...
        // (shouldn't be needed)
        int limit = 3;
        bool mountsucceeded = false;

        // Type ISO: Since 11.1 mtab might contain the name of
        // the loop device instead of the iso file:
        const auto &checkAttachedISO = [&]( const zypp::media::MountEntry &e ) {
          if ( zypp::str::hasPrefix( zypp::Pathname(e.src).asString(), "/dev/loop" )
            && mountpoint == zypp::Pathname(e.dir) ) {
              DBG << "Found bound media "
                  << isopath
                  << " in the mount table as " << e.src << std::endl;
              return true;
          }
          return false;
        };

        // mount command came back OK, lets wait for mtab to update
        while( !(mountsucceeded=checkAttached( newAp, checkAttachedISO )) && --limit) {
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
            isopath.asString(), newAp.asString()
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

      } catch ( const zypp::Exception &e ) {
        removeAttachPoint(newAp);
        ZYPP_CAUGHT(e);
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MountFailed
          , false
          , e
        );
      }

      // here we have our device AND it is verified , lets register it and return it to the user
      auto devPtr    = std::make_shared<zyppng::worker::Device>( zyppng::worker::Device{
        ._name       = fullIsoUrl.asString(),
        ._maj_nr = 0,
        ._min_nr = 0,
        ._mountPoint = newAp,
        ._ephemeral  = true, // forget about the device after we are finished with it
        ._properties = {}
      });
      devPtr->_properties["raiiHelper"] = hlpr;


      knownDevices().push_back( devPtr );
      attachedMedia().insert( std::make_pair( attachId, zyppng::worker::AttachedMedia{ devPtr, relAttachRoot } ) );
      return zyppng::worker::AttachResult::success();

    };

    if( !src.isValid()) {
      const std::string &err = zypp::str::Str()<<"Invalid iso filename source media url: "<<attachUrl;
      ERR << err << std::endl;
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , err
        , false );
    }
    if( src.getScheme() == "iso") {
      const std::string &err = zypp::str::Str()<< "ISO filename source media url with iso scheme (nested iso): " << src.asString();
      ERR << err << std::endl;
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , err
        , false );

    } else if ( src.getScheme() == "hd" ) {

      if ( !_diskWorker ) {
        _diskWorker = std::make_shared<DiskProvider>();

        auto subAttachRoot =  ( attachRoot().realpath()/"disk" );
        zypp::filesystem::assert_dir( subAttachRoot );

        auto confCopy = config();
        (*confCopy.mutable_values())[std::string(zyppng::ATTACH_POINT)] = subAttachRoot.asString();
        const auto &res = _diskWorker->initialize( confCopy );
        if ( !res ) {
          const std::string &err = zypp::str::Str()<< "Failed to initialize worker for: " << src.asString();
          ERR << err << std::endl;
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MountFailed
            , err
            , false );
        }
      }

      return attachViaHelper( _diskWorker, "disk" );

    } else if ( src.getScheme() == "dir" || src.getScheme() == "file" ) {

      if ( !_dirWorker ) {
        _dirWorker = std::make_shared<DirProvider>();

        auto subAttachRoot =  ( attachRoot().realpath()/"dir" );
        zypp::filesystem::assert_dir( subAttachRoot );

        auto confCopy = config();
        (*confCopy.mutable_values())[std::string(zyppng::ATTACH_POINT)] = subAttachRoot.asString();
        const auto &res = _dirWorker->initialize( confCopy );
        if ( !res ) {
          const std::string &err = zypp::str::Str()<< "Failed to initialize worker for: " << src.asString();
          ERR << err << std::endl;
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MountFailed
            , err
            , false );
        }
      }

      return attachViaHelper( _dirWorker, "dir" );

    } else if ( src.getScheme() == "nfs" || src.getScheme() == "nfs4" ) {

      if ( !_nfsWorker ) {
        _nfsWorker = std::make_shared<NfsProvider>();

        auto subAttachRoot =  ( attachRoot().realpath()/"nfs" );
        zypp::filesystem::assert_dir( subAttachRoot );

        auto confCopy = config();
        (*confCopy.mutable_values())[std::string(zyppng::ATTACH_POINT)] = subAttachRoot.asString();
        const auto &res = _nfsWorker->initialize( confCopy );
        MIL << "AFTER INIT " << _nfsWorker->attachRoot() << std::endl;
        if ( !res ) {
          const std::string &err = zypp::str::Str()<< "Failed to initialize worker for: " << src.asString();
          ERR << err << std::endl;
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MountFailed
            , err
            , false );
        }
      }

      return attachViaHelper( _nfsWorker, "nfs" );

    } else if ( src.getScheme() == "smb" || src.getScheme() == "cifs" ) {

      if ( !_smbWorker ) {
        _smbWorker = std::make_shared<SmbProvider>();

        auto subAttachRoot =  ( attachRoot().realpath()/"smb" );
        zypp::filesystem::assert_dir( subAttachRoot );

        auto confCopy = config();
        (*confCopy.mutable_values())[std::string(zyppng::ATTACH_POINT)] = subAttachRoot.asString();
        const auto &res = _smbWorker->initialize( confCopy );
        if ( !res ) {
          const std::string &err = zypp::str::Str()<< "Failed to initialize worker for: " << src.asString();
          ERR << err << std::endl;
          return zyppng::worker::AttachResult::error(
            zyppng::ProvideMessage::Code::MountFailed
            , err
            , false );
        }
      }

      return attachViaHelper( _smbWorker, "smb" );

    } else {
      const std::string &err = zypp::str::Str()<< "ISO filename source media url scheme is not supported: " << src.asString();
      ERR << err << std::endl;
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , err
        , false );
    }
  }  catch ( const zypp::Exception &e  ) {
    return zyppng::worker::AttachResult::error (
      zyppng::ProvideMessage::Code::BadRequest
      , false
      , e );
  }  catch ( const std::exception &e  ) {
      return zyppng::worker::AttachResult::error (
        zyppng::ProvideMessage::Code::BadRequest
      , e.what()
      , false );
  }  catch ( ... ) {
    return zyppng::worker::AttachResult::error(
      zyppng::ProvideMessage::Code::BadRequest
      , "Unknown exception"
      , false);
  }
}
