/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include "mountingworker.h"
#include <zypp-media/ng/private/providedbg_p.h>
#include <zypp-media/ng/MediaVerifier>
#include <zypp-core/fs/PathInfo.h>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "MountingWorker"

namespace zyppng::worker
{

  MountingWorker::MountingWorker( std::string_view workerName, DeviceDriverRef driver )
    : ProvideWorker( workerName )
    , _driver(driver)
  { }

  MountingWorker::~MountingWorker()
  {
    _driver->immediateShutdown();
  }

  zyppng::expected<zyppng::worker::WorkerCaps> MountingWorker::initialize( const zyppng::worker::Configuration &conf )
  {
    return _driver->initialize(conf);
  }

  void MountingWorker::provide()
  {
    _driver->detectDevices();
    auto &queue = requestQueue();

    if ( !queue.size() )
      return;

    auto req = queue.front();
    queue.pop_front();

    MIL_PRV << "Received provide: " << req->_spec.code() << std::endl;

    try {
      switch ( req->_spec.code () ) {
        case zyppng::ProvideMessage::Code::Attach: {

          const auto attachUrl = zypp::Url( req->_spec.value( zyppng::AttachMsgFields::Url ).asString() );
          const auto label     = req->_spec.value( zyppng::AttachMsgFields::Label, "No label" ).asString();
          const auto attachId  = req->_spec.value( zyppng::AttachMsgFields::AttachId ).asString();
          HeaderValueMap vals;
          req ->_spec.forEachVal([&]( const std::string &name, const auto &val ) {
            if ( name == zyppng::AttachMsgFields::Url
              || name == zyppng::AttachMsgFields::Label
              || name == zyppng::AttachMsgFields::AttachId )
              return true;
            vals.add( name, val );
            return true;
          });

          const auto &res = _driver->mountDevice( req->_spec.requestId(),  attachUrl, attachId, label, vals );
          if ( !res ) {
            const auto &err = res.error();
            req->_state = zyppng::worker::ProvideWorkerItem::Finished;
            provideFailed( req->_spec.requestId()
              , err._code
              , err._reason
              , err._transient
              , err._extra );
            return;
          }

          MIL << "Attach of " << attachUrl << " was successfull" << std::endl;
          attachSuccess( req->_spec.requestId() );
          return;
        }
        case zyppng::ProvideMessage::Code::Detach: {

          const auto url = zypp::Url( req->_spec.value( zyppng::DetachMsgFields::Url ).asString() );
          const auto &attachId = url.getAuthority();

          if ( _driver->detachMedia( attachId ) ) {
            detachSuccess ( req->_spec.requestId() );
          } else {
            provideFailed( req->_spec.requestId()
              , zyppng::ProvideMessage::Code::NotFound
              , "Attach ID not known."
              , false
              , {} );
            return;
          }

          _driver->releaseIdleDevices();
          return;
        }

        case zyppng::ProvideMessage::Code::Provide: {

          const auto url = zypp::Url( req->_spec.value( zyppng::DetachMsgFields::Url ).asString() );
          const auto &attachId = url.getAuthority();
          const auto &path = zypp::Pathname(url.getPathName());
          const auto &availMedia = _driver->attachedMedia();

          auto i = availMedia.find( attachId );
          if ( i == availMedia.end() ) {
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

  void MountingWorker::cancel( const std::deque<zyppng::worker::ProvideWorkerItemRef>::iterator &i )
  {
    ERR << "Bug, cancel should never be called for running items" << std::endl;
  }

  void MountingWorker::immediateShutdown()
  {
    _driver->immediateShutdown();
  }
}
