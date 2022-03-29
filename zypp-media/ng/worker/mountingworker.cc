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

  MountingWorker::MountingWorker( zyppng::worker::WorkerCaps::WorkerType wType, std::string_view workerName )
    : ProvideWorker( workerName )
    , _wtype( wType )
  {

  }

  MountingWorker::~MountingWorker()
  { }

  zyppng::expected<zyppng::worker::WorkerCaps> MountingWorker::initialize( const zyppng::worker::Configuration &conf )
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
    caps.set_worker_type ( _wtype );
    caps.set_cfg_flags(
      zyppng::worker::WorkerCaps::Flags (
        zyppng::worker::WorkerCaps::Pipeline
        | zyppng::worker::WorkerCaps::ZyppLogFormat
        | zyppng::worker::WorkerCaps::SingleInstance
        )
      );

    return zyppng::expected<zyppng::worker::WorkerCaps>::success(caps);
  }

  void MountingWorker::detectDevices()
  {
    return;
  }

  std::vector<std::shared_ptr<Device>> &MountingWorker::knownDevices()
  {
    return _sysDevs;
  }

  const std::vector<std::shared_ptr<Device>> &MountingWorker::knownDevices() const
  {
    return _sysDevs;
  }

  std::unordered_map<std::string, AttachedMedia> &MountingWorker::attachedMedia()
  {
    return _attachedMedia;
  }

  void MountingWorker::provide()
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
          return handleMountRequest( *req );
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
          for ( auto i = _sysDevs.begin (); i != _sysDevs.end(); ) {
            if ( i->use_count() == 1 && !(*i)->_mountPoint.empty() ) {
              MIL << "Unmounting device " << (*i)->_name << " since its not used anymore" << std::endl;
              unmountDevice(*(*i));
              if ( (*i)->_ephemeral ) {
                i = _sysDevs.erase(i);
                continue;
              }
            }
            ++i;
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

  void MountingWorker::cancel( const std::deque<zyppng::worker::ProvideWorkerItemRef>::iterator &i )
  {
    ERR << "Bug, cancel should never be called for running items" << std::endl;
  }

  void MountingWorker::immediateShutdown()
  {
    // here we need to unmount everything
    for ( auto &dev : _sysDevs )
      unmountDevice(*dev);
    _attachedMedia.clear();
  }

  void MountingWorker::unmountDevice ( Device &dev )
  {
    if ( dev._mountPoint.empty () )
      return;
    try {
      zypp::media::Mount mount;
      mount.umount( dev._mountPoint.asString() );
      removeAttachPoint( dev._mountPoint );
    } catch (const zypp::media::MediaException & excpt_r) {
      ERR << "Failed to unmount device: " << dev._name << std::endl;
      ZYPP_CAUGHT(excpt_r);
    }
    dev._mountPoint = zypp::Pathname();
  }

  bool MountingWorker::isVolatile () const
  {
    return false;
  }

  const zypp::Pathname &MountingWorker::attachRoot () const
  {
    return _attachRoot;
  }

  zyppng::expected<void> MountingWorker::isDesiredMedium ( const zypp::Url &deviceUrl, const zypp::Pathname &mountPoint, const zyppng::MediaDataVerifierRef &verifier, uint mediaNr )
  {
    if ( !verifier ) {
      return zyppng::expected<void>::success();	// we have no valid data
    }

    auto devVerifier = verifier->clone();
    if ( !devVerifier ) {
      // unlikely to happen
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR( zypp::Exception("Failed to clone verifier") ) );
    }

    // bsc#1180851: If there is just one not-volatile medium in the set
    // tolerate a missing (vanished) media identifier and let the URL rule.
    bool relaxed = verifier->totalMedia() == 1 && !isVolatile();

    const auto &relMediaPath = devVerifier->mediaFilePath( mediaNr );
    zypp::Pathname mediaFile { mountPoint / relMediaPath };
    zypp::PathInfo pi( mediaFile );
    if ( !pi.isExist() ) {
      if ( relaxed )
        return zyppng::expected<void>::success();
      auto excpt =  zypp::media::MediaFileNotFoundException( deviceUrl, relMediaPath ) ;
      excpt.addHistory( verifier->expectedAsUserString( mediaNr ) );
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR( std::move(excpt) ) );
    }
    if ( !pi.isFile() ) {
      if ( relaxed )
        return zyppng::expected<void>::success();
      auto excpt =  zypp::media::MediaNotAFileException( deviceUrl, relMediaPath ) ;
      excpt.addHistory( verifier->expectedAsUserString( mediaNr ) );
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR( std::move(excpt) ) );
    }

    if ( !devVerifier->load( mediaFile ) ) {
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR( zypp::Exception("Failed to load media information from medium") ) );
    }
    if ( !verifier->matches( devVerifier ) ) {
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR( zypp::media::MediaNotDesiredException( deviceUrl ) ) );
    }
    return zyppng::expected<void>::success();
  }
}
