#include "private/mirrorcontrol_p.h"
#include <zypp/zyppng/base/EventDispatcher>
#include <zypp/base/Logger.h>
#include <zypp/zyppng/base/Signals>
#include <iostream>

namespace zyppng {

  static const uint penaltyIncrease = 100;
  static const uint defaultSampleTime = 2;

  void MirrorControl::Mirror::startTransfer()
  {
    runningTransfers++;
  }

  void MirrorControl::Mirror::finishTransfer(const bool success)
  {
    runningTransfers--;
    if ( success ) {
      if ( penalty >= penaltyIncrease ) penalty -= penaltyIncrease;
      successfulTransfers++;
      failedTransfers = 0;
    } else {
      penalty += penaltyIncrease;
      failedTransfers++;
    }
  }

  void MirrorControl::Mirror::cancelTransfer()
  {
    runningTransfers--;
  }

  MirrorControl::MirrorControl()
  {
    _dispatcher = std::make_shared<NetworkRequestDispatcher>();
    _dispatcher->connectFunc( &NetworkRequestDispatcher::sigQueueFinished, [ this ]( NetworkRequestDispatcher& ) {
      //tell the world the queue is empty
      _sigAllMirrorsReady.emit();
    }, *this );
    _dispatcher->run();
  }

  MirrorControl::Ptr MirrorControl::create()
  {
    return std::shared_ptr<MirrorControl>( new MirrorControl );
  }

  MirrorControl::~MirrorControl()
  { }

  void MirrorControl::registerMirrors( const std::vector<zypp::media::MetalinkMirror> &urls )
  {
    for ( const auto &mirror : urls ) {

      const auto scheme = mirror.url.getScheme();
      if ( scheme == "http" || scheme == "https" || scheme == "ftp" || scheme == "tftp" ) {

        const std::string urlKey = makeKey( mirror.url );

        // already there
        const auto hndlIt = _handles.find( urlKey );
        if ( hndlIt != _handles.end() ) {
          continue;
        }

        auto mirrorHandle = std::make_shared<Mirror>();
        mirrorHandle->rating         = mirror.priority;
        mirrorHandle->maxConnections = mirror.maxConnections;
        mirrorHandle->mirrorUrl      = mirror.url;
        mirrorHandle->mirrorUrl.setPathName("/");

        mirrorHandle->_request = std::make_shared<NetworkRequest>( mirrorHandle->mirrorUrl, "/dev/null", NetworkRequest::WriteShared );
        mirrorHandle->_request->setOptions( NetworkRequest::ConnectionTest );
        mirrorHandle->_request->transferSettings().setTimeout( defaultSampleTime );
        mirrorHandle->_request->transferSettings().setConnectTimeout( defaultSampleTime );
        mirrorHandle->_finishedConn = mirrorHandle->_request->connectFunc( &NetworkRequest::sigFinished, [ mirrorHandle ](  NetworkRequest &req, const NetworkRequestError & ){

          if ( req.hasError() )
            ERR << "Mirror request failed: " << req.error().toString() << " ; " << req.extendedErrorString() << "; for url: "<<req.url()<<std::endl;

          const auto timings = req.timings();
          std::chrono::milliseconds connTime;
          if ( timings ) {
            connTime = std::chrono::duration_cast<std::chrono::milliseconds>(timings->connect - timings->namelookup);
          } else {
            // we can not get any measurements, maximum penalty
            connTime = std::chrono::seconds( defaultSampleTime );
          }

          DBG << "Got rating for mirror: " <<  mirrorHandle->mirrorUrl << ", rating was " << mirrorHandle->rating;
          mirrorHandle->rating += connTime.count();
          DBG << " rating is now " << mirrorHandle->rating << " conn time was " << connTime.count() << std::endl;

          // clean the request up
          mirrorHandle->_finishedConn.disconnect();
          mirrorHandle->_request.reset();
        });

        _dispatcher->enqueue( mirrorHandle->_request );
        _handles.insert( std::make_pair(urlKey, mirrorHandle ) );
      }
    }

    if ( _dispatcher->count() == 0 ) {
      // we did know all Mirrors before, notify the outside world we are ready
      _sigAllMirrorsReady.emit();
    }
  }

  std::pair< std::vector<Url>::const_iterator, MirrorControl::MirrorHandle> MirrorControl::pickBestMirror( const std::vector<Url> &mirrors )
  {
    std::vector< std::pair< std::vector<Url>::const_iterator, MirrorHandle > > possibleMirrs;
    for ( auto i = mirrors.begin(); i != mirrors.end(); i++ ) {
      const auto key = makeKey( *i );
      const auto hdlIt = this->_handles.find( key );
      if ( hdlIt == _handles.end( ) )
        continue;
      possibleMirrs.push_back( std::make_pair( i, hdlIt->second ) );
    }
    std::stable_sort( possibleMirrs.begin(), possibleMirrs.end(), []( const auto &a, const auto &b ) {
      return a.second->rating < b.second->rating;
    });

    for ( const auto &mirr : possibleMirrs ) {
      const auto maxConn = mirr.second->maxConnections > 0 ? mirr.second->maxConnections : 5; //max connections per mirror @todo make this configurable
      if ( mirr.second->runningTransfers >= maxConn )
        continue;
      if ( mirr.second->failedTransfers >= 10 )
        continue;
      return mirr;
    }
    return std::make_pair( mirrors.end(), MirrorHandle() );
  }

  SignalProxy<void ()> MirrorControl::sigAllMirrorsReady()
  {
    return _sigAllMirrorsReady;
  }

  std::string MirrorControl::makeKey(const zypp::Url &url) const
  {
    return url.asString( zypp::Url::ViewOptions::WITH_SCHEME +
                         zypp::Url::ViewOptions::WITH_HOST +
                         zypp::Url::ViewOptions::WITH_PORT +
                         zypp::Url::ViewOptions::EMPTY_AUTHORITY
    );
  }

}

