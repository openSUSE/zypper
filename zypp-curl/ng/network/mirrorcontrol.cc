/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#include "private/mirrorcontrol_p.h"
#include "private/mediadebug_p.h"
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-core/zyppng/base/Signals>
#include <zypp-core/base/String.h>
#include <iostream>

namespace zyppng {

  constexpr uint penaltyIncrease = 100;
  constexpr uint defaultSampleTime = 2;
  constexpr uint defaultMaxConnections = 5;

  MirrorControl::Mirror::Mirror( MirrorControl &parent ) : _parent( parent )
  {}

  void MirrorControl::Mirror::startTransfer()
  {
    runningTransfers++;
  }

  void MirrorControl::Mirror::finishTransfer(const bool success)
  {
    if ( success ) {
      if ( penalty >= penaltyIncrease ) penalty -= penaltyIncrease;
      successfulTransfers++;
      failedTransfers = 0;
    } else {
      penalty += penaltyIncrease;
      failedTransfers++;
    }
    transferUnref();
  }

  void MirrorControl::Mirror::cancelTransfer()
  {
    transferUnref();
  }

  uint MirrorControl::Mirror::maxConnections() const
  {
    return ( _maxConnections > 0 ? _maxConnections : defaultMaxConnections ); //max connections per mirror @todo make this configurable
  }

  bool MirrorControl::Mirror::hasFreeConnections() const
  {
    return ( runningTransfers < maxConnections() );
  }

  void MirrorControl::Mirror::transferUnref()
  {
    bool stillLoaded = ( runningTransfers - 1 ) >= maxConnections();
    runningTransfers--;
    if ( !stillLoaded )
      _parent._sigNewMirrorsReady.emit();
  }

  MirrorControl::MirrorControl()
  {
    // set up the single shot timer, that way we can emit a signal after we finished processing all
    // events that have queued up in the event loop instead of constantly firing the signal
    _newMirrSigDelay = Timer::create();
    _newMirrSigDelay->setSingleShot( true );
    _newMirrSigDelay->connectFunc( &Timer::sigExpired, [this]( const auto & ){
      _sigNewMirrorsReady.emit();
    });

    _dispatcher = std::make_shared<NetworkRequestDispatcher>();
    _queueEmptyConn = _dispatcher->connectFunc( &NetworkRequestDispatcher::sigQueueFinished, [ this ]( NetworkRequestDispatcher& ) {
      //tell the world the queue is empty

      std::vector< std::unordered_map<std::string, MirrorHandle>::const_iterator > allOfEm;
      for ( auto i = _handles.begin(); i != _handles.end(); i++ ) {
        allOfEm.push_back( i );
      }

      std::sort( allOfEm.begin(), allOfEm.end(), []( const auto &a, const auto &b ){
        return ( zypp::str::compareCI( a->second->mirrorUrl.asString().c_str(), b->second->mirrorUrl.asString().c_str() ) < 0 );
      });

      DBG_MEDIA << "Finished probing mirrors, these are the results: \n";
      for ( const auto &iter : allOfEm ) {
        DBG_MEDIA << "Mirror: " << iter->second->mirrorUrl << ", rating is: " << iter->second->rating << "\n";
      }
      DBG_MEDIA << "End Mirror probing results." << std::endl;

      _sigAllMirrorsReady.emit();
    }, *this );
    _dispatcher->run();
  }

  MirrorControl::Ptr MirrorControl::create()
  {
    return std::shared_ptr<MirrorControl>( new MirrorControl );
  }

  MirrorControl::~MirrorControl()
  {
    // do not send signals to us while we are destructing
    _queueEmptyConn.disconnect();

    if ( _dispatcher->count() > 0 ) {
      MIL << "Destroying MirrorControl while measurements are still running, aborting" << std::endl;
      for ( auto &mirr : _handles )  {
        if ( mirr.second->_request ) {
          mirr.second->_finishedConn.disconnect();
          _dispatcher->cancel( *mirr.second->_request );
        }
      }
    }

  }

  void MirrorControl::registerMirrors( const std::vector<zypp::media::MetalinkMirror> &urls )
  {
    bool doesKnowSomeMirrors = false;
    for ( const auto &mirror : urls ) {

      const auto scheme = mirror.url.getScheme();
      if ( scheme == "http" || scheme == "https" || scheme == "ftp" || scheme == "tftp" ) {

        const std::string urlKey = makeKey( mirror.url );

        // already there
        const auto hndlIt = _handles.find( urlKey );
        if ( hndlIt != _handles.end() ) {
          doesKnowSomeMirrors = true;
          continue;
        }

        auto mirrorHandle = std::shared_ptr<Mirror>( new Mirror(*this) );
        mirrorHandle->rating          = mirror.priority;
        mirrorHandle->_maxConnections = mirror.maxConnections;
        mirrorHandle->mirrorUrl       = mirror.url;
        mirrorHandle->mirrorUrl.setPathName("/");

        mirrorHandle->_request = std::make_shared<NetworkRequest>( mirrorHandle->mirrorUrl, "/dev/null", NetworkRequest::WriteShared );
        mirrorHandle->_request->setOptions( NetworkRequest::ConnectionTest );
        mirrorHandle->_request->transferSettings().setTimeout( defaultSampleTime );
        mirrorHandle->_request->transferSettings().setConnectTimeout( defaultSampleTime );
        mirrorHandle->_finishedConn = mirrorHandle->_request->connectFunc( &NetworkRequest::sigFinished, [ mirrorHandle, &someReadyDelay = _newMirrSigDelay ](  NetworkRequest &req, const NetworkRequestError & ){

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

          DBG_MEDIA << "Got rating for mirror: " <<  mirrorHandle->mirrorUrl << ", rating was " << mirrorHandle->rating;
          mirrorHandle->rating += connTime.count();
          DBG_MEDIA << " rating is now " << mirrorHandle->rating << " conn time was " << connTime.count() << std::endl;

          // clean the request up
          mirrorHandle->_finishedConn.disconnect();
          mirrorHandle->_request.reset();

          // start the timer to emit someMirrorsReady
          someReadyDelay->start( 0 );
        });

        _dispatcher->enqueue( mirrorHandle->_request );
        _handles.insert( std::make_pair(urlKey, mirrorHandle ) );
      }
    }

    if ( doesKnowSomeMirrors )
      _sigNewMirrorsReady.emit();

    if ( _dispatcher->count() == 0 ) {
      // we did know all Mirrors before, notify the outside world we are ready
      _sigAllMirrorsReady.emit();
    }
  }

  MirrorControl::PickResult MirrorControl::pickBestMirror( const std::vector<Url> &mirrors )
  {
    bool hasPendingRating = false;
    std::vector< MirrorPick > possibleMirrs;
    for ( auto i = mirrors.begin(); i != mirrors.end(); i++ ) {
      const auto key = makeKey( *i );
      const auto hdlIt = this->_handles.find( key );
      if ( hdlIt == _handles.end( ) )
        continue;
      // still waiting for the request to finish
      if ( hdlIt->second->_request ) {
        hasPendingRating = true;
        continue;
      }
      possibleMirrs.push_back( std::make_pair( i, hdlIt->second ) );
    }

    if ( possibleMirrs.empty() && hasPendingRating ) {
      // still waiting return , tell the caller to try again later
      return PickResult{ PickResult::Again, std::make_pair( mirrors.end(), MirrorHandle() ) };
    }

    std::stable_sort( possibleMirrs.begin(), possibleMirrs.end(), []( const auto &a, const auto &b ) {
      return a.second->rating < b.second->rating;
    });

    bool hasLoadedOne = false; // do we have a mirror that will be ready again later?
    for ( const auto &mirr : possibleMirrs ) {
      if ( !mirr.second->hasFreeConnections() ) {
        hasLoadedOne = true;
        continue;
      }
      if ( mirr.second->failedTransfers >= 10 )
        continue;
      return PickResult{ PickResult::Ok, mirr };
    }

    if ( hasLoadedOne ){
      // we have mirrors, but they have reached maximum capacity, tell the caller to delay the call
      return PickResult{ PickResult::Again, std::make_pair( mirrors.end(), MirrorHandle() ) };
    }

    return PickResult{ PickResult::Unknown, std::make_pair( mirrors.end(), MirrorHandle() ) };
  }

  SignalProxy<void ()> MirrorControl::sigNewMirrorsReady()
  {
    return _sigNewMirrorsReady;
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

#if 0

  MirrorRef::MirrorRef( MirrorControl::MirrorHandle handle )
  {
    _data = std::make_shared<Helper>( handle, false );
  }

  MirrorRef::~MirrorRef()
  { }

  void MirrorRef::startTransfer()
  {
    _data->_myHandle->startTransfer();
    _data->_cancelOnDestruct = true;
  }

  void MirrorRef::finishTransfer(const bool success)
  {
    _data->_cancelOnDestruct = false;
    _data->_myHandle->finishTransfer( success );
  }

  void MirrorRef::cancelTransfer()
  {
    _data->_cancelOnDestruct = false;
    _data->_myHandle->cancelTransfer();
  }

  MirrorRef::operator bool() const
  {
    return _data->_myHandle.operator bool();
  }

  MirrorControl::MirrorHandle MirrorRef::get()
  {
    return _data->_myHandle;
  }

  MirrorRef::Helper::~Helper()
  {
    if ( _cancelOnDestruct )
      _myHandle->cancelTransfer();
  }
#endif

}

