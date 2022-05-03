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
#include <zypp-media/ng/MediaVerifier>

#include <zypp-core/fs/TmpPath.h>
#include <zypp-core/fs/PathInfo.h>

#include <iostream>
#include <fstream>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "DirProvider"


DirProvider::DirProvider( std::string_view workerName )
  : MountingWorker( zyppng::worker::WorkerCaps::SimpleMount, workerName )
{ }

DirProvider::~DirProvider()
{ }

void DirProvider::handleMountRequest ( zyppng::worker::ProvideWorkerItem &req )
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

    // we simulate a device by simply using the pathname
    zypp::Pathname path = zypp::Pathname( attachUrl.getPathName() ).realpath();
    const auto &pathStr = path.asString();

    // first check if we have that device already
    auto i = std::find_if( devs.begin(), devs.end(), [&]( const auto &d ) { return d->_name == pathStr; } );
    if ( i != devs.end() ) {
      auto res = isDesiredMedium( attachUrl, (*i)->_mountPoint, verifier, req._spec.value( zyppng::AttachMsgFields::MediaNr, 1 ).asInt() );
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
        attachedMedia().insert( std::make_pair( req._spec.value(zyppng::AttachMsgFields::AttachId ).asString(), zyppng::worker::AttachedMedia{ *i, "/" } ) );
        attachSuccess( req._spec.requestId() );
        return;
      }
    }

    // we did not find a existing mount, well lets make a new one
    zypp::PathInfo adir( path );
    if( adir.isDir()) {
          MIL << "New device " << path << " mounted on " << path << std::endl;
          auto newDev = std::shared_ptr<zyppng::worker::Device>( new zyppng::worker::Device{
            ._name = pathStr,
            ._mountPoint = path,
            ._ephemeral = true // device should be removed after the last attachment was released
            });
          knownDevices().push_back( newDev );
          attachedMedia().insert( std::make_pair( req._spec.value(zyppng::AttachMsgFields::AttachId ).asString(), zyppng::worker::AttachedMedia{ newDev, "/" } ) );
          attachSuccess( req._spec.requestId() );
          return;
    }

    // URl did not point to a directory
    provideFailed( req._spec.requestId()
      , zyppng::ProvideMessage::Code::MountFailed
      , zypp::str::Str()<< "Specified path '" << attachUrl << "' is not a directory"
      , false
    );
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

void DirProvider::unmountDevice ( zyppng::worker::Device &dev ) {
  // do nothing , this is just a local dir
}
