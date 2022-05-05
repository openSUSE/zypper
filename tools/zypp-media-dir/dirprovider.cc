/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include "dirprovider.h"
#include <zypp-media/ng/private/providedbg_p.h>
#include <zypp-media/ng/private/providemessage_p.h>
#include <zypp-media/ng/MediaVerifier>

#include <zypp-core/Url.h>
#include <zypp-core/fs/TmpPath.h>
#include <zypp-core/fs/PathInfo.h>

#include <iostream>
#include <fstream>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "DirProvider"


DirProvider::DirProvider()
  : DeviceDriver( zyppng::worker::WorkerCaps::SimpleMount )
{ }

DirProvider::~DirProvider()
{ }

zyppng::worker::AttachResult DirProvider::mountDevice ( const uint32_t id, const zypp::Url &attachUrl, const std::string &attachId, const std::string &label, const zyppng::HeaderValueMap &extras )
{
  try
  {
    if ( !attachUrl.getHost().empty() ) {
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , "Host must be empty in dir:// and file:// URLs"
        , false );
    }

    // set up the verifier
    zyppng::MediaDataVerifierRef verifier;
    if ( extras.contains(zyppng::AttachMsgFields::VerifyType) ) {
      verifier = zyppng::MediaDataVerifier::createVerifier( extras[zyppng::AttachMsgFields::VerifyType].asString() );
      if ( !verifier ) {
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MountFailed
          , "Invalid verifier type"
          , false );
      }

      if ( !verifier->load( extras[zyppng::AttachMsgFields::VerifyData].asString() ) ) {
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MountFailed
          , "Failed to create verifier from file"
          , false );
      }
    }
    const auto &devs = knownDevices();

    // we simulate a device by simply using the pathname
    zypp::Pathname path = zypp::Pathname( attachUrl.getPathName() ).realpath();
    const auto &pathStr = path.asString();

    zypp::PathInfo adir( path );
    if( !adir.isDir()) {
      // URl did not point to a directory
      return zyppng::worker::AttachResult::error(
        zyppng::ProvideMessage::Code::MountFailed
        , zypp::str::Str()<< "Specified path '" << attachUrl << "' is not a directory"
        , false
      );
    }

    // lets check if the path is what we want
    auto res = isDesiredMedium( attachUrl, path, verifier, extras.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt() );
    if ( !res ) {
      try {
        std::rethrow_exception( res.error() );
      } catch( const zypp::Exception& e ) {
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MediumNotDesired
          , false
          , e );
      } catch ( ... ) {
        return zyppng::worker::AttachResult::error(
          zyppng::ProvideMessage::Code::MediumNotDesired
          , "Checking the medium failed with an uknown error"
          , false );
      }
    }

    // first check if we have that device already
    auto i = std::find_if( devs.begin(), devs.end(), [&]( const auto &d ) { return d->_name == pathStr; } );
    if ( i != devs.end() ) {
      attachedMedia().insert( { attachId, zyppng::worker::AttachedMedia{ ._dev = *i, ._attachRoot = "/" } } );
      return zyppng::worker::AttachResult::success();
    }

    // we did not find a existing device, well lets make a new one
    MIL << "New device " << path << " mounted on " << path << std::endl;
    auto newDev = std::make_shared<zyppng::worker::Device>( zyppng::worker::Device{
      ._name = pathStr,
      ._maj_nr = 0,
      ._min_nr = 0,
      ._mountPoint = path,
      ._ephemeral = true, // device should be removed after the last attachment was released
      ._properties = {}
      });
    attachedMedia().insert( { attachId, zyppng::worker::AttachedMedia{ ._dev = newDev, ._attachRoot = "/" } } );
    return zyppng::worker::AttachResult::success();

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

void DirProvider::unmountDevice ( zyppng::worker::Device &dev ) {
  // do nothing , this is just a local dir
}
