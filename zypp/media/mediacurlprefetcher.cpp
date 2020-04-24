#include "mediacurlprefetcher.h"

#include "zypp/zyppng/base/EventLoop"
#include "zypp/zyppng/base/SocketNotifier"
#include "zypp/zyppng/media/network/downloader.h"
#include "zypp/zyppng/media/network/networkrequestdispatcher.h"
#include "zypp/media/CurlHelper.h"
#include "zypp/PathInfo.h"

#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::prefetcher"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"

#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <iostream>

namespace zypp
{
  namespace media
  {

    template<typename T, typename Callback>
    typename std::vector<T>::size_type indexOf ( const std::vector<T> &vec, Callback &&predicate ) {
      typename std::vector<T>::size_type i = 0;
      for ( ; i < vec.size(); i++ ) {
        if( std::forward<Callback>(predicate)(vec[i]) )
          break;
      }
      return i;
    }

    struct MediaCurlPrefetcher::RunningRequest {
      RunningRequest ( const zypp::filesystem::Pathname & path ) : f(path){ }
      ~RunningRequest() {
        //zyppng::EventDispatcher::instance()->unrefLater( dl );
      }
      Request r;
      int requestCount = 1; //how many times was that file requested, we keep it around until it was required the same times
      std::atomic<off_t> _dlNow;
      std::atomic<off_t> _dlTotal;
      std::shared_ptr<zyppng::Download> dl;
      zypp::filesystem::TmpFile f;
      std::promise<zyppng::Download::State> result;
    };

    MediaCurlPrefetcher::MediaCurlPrefetcher()
    {
      _stop = false;
      _workingDir.autoCleanup( true );

      _lastFinishedIndex = -1;
      _firstWaitingIndex = -1;

      //start up thread
      _fetcherThread = std::thread( [this](){ workerMain(); });

      // wake up pipe to interrupt the event main loop
      pipe ( _wakeupPipe );
      fcntl( _wakeupPipe[0], F_SETFL, O_NONBLOCK );
    }

    MediaCurlPrefetcher &MediaCurlPrefetcher::instance()
    {
      static MediaCurlPrefetcher prefetch;
      return prefetch;
    }

    MediaCurlPrefetcher::~MediaCurlPrefetcher()
    {
      //tell the thread to stop
      _stop = true;

      //signal the thread to wake up
      write( _wakeupPipe[1], "\n", 1);

      _fetcherThread.join();

      close (_wakeupPipe[0]);
      close (_wakeupPipe[1]);
    }

    MediaCurlPrefetcher::CacheId MediaCurlPrefetcher::createCache()
    {
      MIL << "CREATING CACHE " << _nextCacheId <<std::endl;
      return _nextCacheId++;
    }

    void MediaCurlPrefetcher::closeCache( const MediaCurlPrefetcher::CacheId id )
    {
      std::unique_lock<std::recursive_mutex> guard( _lock );
      _cachesToClose.push_back( id );

      //signal the thread to wake up
      write( _wakeupPipe[1], "\n", 1);
    }

    void MediaCurlPrefetcher::precacheFiles( std::vector<Request> &&files )
    {
      std::unique_lock<std::recursive_mutex> guard( _lock );
      for ( Request &req : files ) {

        MIL << "PRECACHE FILE " << req.url << " to CACHE " << req.cache << std::endl;

        bool firstInWaiting = _firstWaitingIndex == -1;

        //check if we have a download for that file already
        if ( _requests.size() ) {
          auto it = std::find_if( _requests.begin(), _requests.end(), [ &req ]( const auto &elem ) { return ( elem->r.url == req.url && elem->r.cache == req.cache); });
          if ( it != _requests.end() ) {
            (*it)->requestCount++;
            continue;
          }
        }

        auto reqPtr = std::make_unique<RunningRequest>( _workingDir.path() );
        reqPtr->r = std::move( req );
        reqPtr->_dlNow = 0;
        reqPtr->_dlTotal = 0;
        _requests.push_back( std::move(reqPtr) );

        if ( firstInWaiting ) _firstWaitingIndex = _requests.size() - 1;
      }

      //signal the thread to wake up
      write( _wakeupPipe[1], "\n", 1);
    }

    bool MediaCurlPrefetcher::requireFile(const CacheId id, const Url &url, const zypp::filesystem::Pathname &targetPath , zypp::callback::SendReport<zypp::media::DownloadProgressReport> &report)
    {

      MIL << "Require file " << url << " from Precache " << id << std::endl;
      std::unique_lock<std::recursive_mutex> guard( _lock );

      const auto predicate = [ &id, &url ]( const auto &elem ) { return ( elem->r.url == url && elem->r.cache == id); };
      ReqQueue::size_type i = indexOf( _requests, predicate );
      if ( i >= 0 && i < _requests.size() ) {

        const auto request = _requests[i].get();
        report->start( url, targetPath );

        if ( static_cast<off_t>(i) > _lastFinishedIndex ) {
          // the request is still in the running part, lets wait for it
          std::future<zyppng::Download::State>  fut;
          try {
            fut = request->result.get_future();
          } catch ( const std::future_error &e ) {
            ZYPP_CAUGHT( e );
            WAR << "Future error while requiring file from precache." << std::endl;
            return false;
          }

          guard.unlock();

          if ( fut.valid() ) {
            internal::ProgressData prog( nullptr, 0, url, 0, &report );
            while( fut.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready ) {
              prog.updateStats( request->_dlNow.load(), request->_dlTotal.load() );
              prog.reportProgress();
            }
          } else {
            report->finish( url, zypp::media::DownloadProgressReport::ERROR, "Failed to precache." );
            WAR << "Got a invalid future, can't continue" << std::endl;
            return false;
          }

          guard.lock();
        }

        // at this point the download is either done or in error state
        if ( request->dl->state() < zyppng::Download::Success ) {
          WAR << "Request in invalid state, can't continue" << std::endl;
          return false;
        }

        if ( request->dl->state() == zyppng::Download::Success ) {
          bool success = true;
          MIL << "Download finished by MediaCurlPrefetcher: " << url << std::endl;
          if(  zypp::filesystem::assert_dir( targetPath.dirname() ) ) {
            DBG << "assert_dir " << targetPath.dirname() << " failed" << std::endl;
            success = false;
          }
          if( success && zypp::filesystem::hardlinkCopy( request->f.path(), targetPath ) != 0 ) {
            DBG << "Failed to hardlinkCopy the requested file <<"<<request->dl->targetPath()<<" to the targetPath " << targetPath;
            success = false;
          }
          request->requestCount--;
          if ( request->requestCount <= 0 ) {
            //need to recalculate the index in case the thread shuffled the elements around while we were waiting
            ReqQueue::size_type i = indexOf( _requests, predicate );
            markRequestForCleanup( _requests.begin() + i );
          }

          guard.unlock();

          if ( success )
            report->finish( url, zypp::media::DownloadProgressReport::NO_ERROR, std::string() );
          else
            report->finish( url, zypp::media::DownloadProgressReport::ERROR, "Failed to precache." );
          return success;
        } else {

          //need to recalculate the index in case the thread shuffled the elements around while we were waiting
          ReqQueue::size_type i = indexOf( _requests, predicate );
          markRequestForCleanup( _requests.begin() + i );

          guard.unlock();

          MIL << "Request for "<< url <<" fails for reason: " << request->dl->errorString() << std::endl;

          report->finish( url, zypp::media::DownloadProgressReport::ERROR, "Failed to precache." );
        }
      }

      //never heard of that file, the MediaHandler needs to take care of it
      MIL << "Request for "<< url <<" was never precached." << std::endl;
      return false;
    }

    void MediaCurlPrefetcher::workerMain()
    {
      if ( getenv("ZYPP_PREFETCHER_LOGFILE") )
        zypp::base::LogControl::instance().logfile( getenv("ZYPP_PREFETCHER_LOGFILE") );

      auto dispatch = zyppng::EventLoop::create();

      zyppng::Downloader downloader;
      downloader.requestDispatcher()->setMaximumConcurrentConnections( 30 );

      auto dlProgress = [ this ]( zyppng::Download &req, off_t dltotal, off_t dlnow ){
        std::lock_guard<std::recursive_mutex> guard( _lock );
        auto i = std::find_if( _requests.begin(), _requests.end(), [ &req ]( const auto &elem ) { return ( elem->dl.get() == &req ); });
        if ( i == _requests.end() ) {
          DBG << "Received a progress signal for a unknown request " << req.url() << std::endl;
          req.cancel();
          return;
        }

        (*i)->_dlNow.store( dlnow );
        (*i)->_dlTotal.store( dltotal );
      };

      auto dlAlive = [ &dlProgress ]( zyppng::Download &req, off_t dlnow ){
        dlProgress( req, 0, dlnow );
      };

      auto dlFinished = [ this ]( zyppng::Download &req ) {
        std::lock_guard<std::recursive_mutex> guard( _lock );
        auto i = std::find_if( _requests.begin(), _requests.end(), [ &req ]( const auto &elem ) {
          return ( elem->dl.get() == &req ); }
        );
        if ( i == _requests.end() ) {
          WAR << "Received a finished signal for a unknown request ignoring " << req.url() << std::endl;
          return;
        }

        MIL << "Finished download of " << req.url() << " with state " << req.state() << "(" << req.errorString() << ")" <<std::endl;

        auto index = std::distance( _requests.begin(), i );

        _lastFinishedIndex++;
        if ( _lastFinishedIndex != index ) {
          _requests[index].swap(_requests[_lastFinishedIndex]);
        }

        const auto &reqObj = _requests[_lastFinishedIndex];
        reqObj->result.set_value( req.state() );
      };

      auto wakeupReceived = [ &, this]( ){
        //clear pipe
        char dummy;
        while ( read( _wakeupPipe[0], &dummy, 1 ) > 0 ) { continue; }

        MIL << "Thread wakeup " << std::endl;

        //we were asked to stop the event loop
        if ( _stop.load() ) {
          std::unique_lock<std::recursive_mutex> guard( _lock );
          _requests.clear();
          _requestsToCleanup.clear();
          _cachesToClose.clear();
          _lastFinishedIndex = -1;
          _firstWaitingIndex = -1;
          dispatch->quit();
          guard.unlock();
          _waitCond.notify_all();
          MIL << "Thread shutting down " << std::endl;
          return;
        }

        std::lock_guard<std::recursive_mutex> guard( _lock );

        //close caches we do not need anymore
        for ( const auto id : _cachesToClose ) {
          DBG << "Destroying prefetch cache" << id << std::endl;
          for ( auto i = _requests.begin(); i != _requests.end(); ) {
            if ( (*i)->r.cache == id ) {
              (*i)->dl->cancel();
              i = markRequestForCleanup(i);
            } else {
              ++i;
            }
          }
        }
        _cachesToClose.clear();

        //first clean up old requests
        _requestsToCleanup.clear();

        while ( _firstWaitingIndex >= 0 && _firstWaitingIndex < static_cast<int>(_requests.size()) ) {
          auto &req = _requests[_firstWaitingIndex];
          _firstWaitingIndex++;

          req->_dlNow = 0;
          req->_dlTotal = 0;

          MIL << "Starting to prefetch file " << req->r.url << std::endl;

          req->dl = downloader.downloadFile( req->r.url, req->f.path(), req->r.expectedFileSize );
          req->dl->sigFinished().connect( dlFinished );
          req->dl->sigAlive().connect( dlAlive );
          req->dl->sigProgress().connect( dlProgress );
          req->dl->settings() = req->r.settings;
          req->dl->start();
        }
      };

      // we are using a pipe to wake up from the event loop, the SocketNotifier will throw a signal every
      // time there is data available
      auto sNotify = zyppng::SocketNotifier::create( _wakeupPipe[0], zyppng::SocketNotifier::Read, false );
      sNotify->sigActivated().connect( [&wakeupReceived]( const zyppng::SocketNotifier &, int ) { wakeupReceived(); } );
      sNotify->setEnabled( true );

      MIL << "Starting event loop " << std::endl;

      wakeupReceived();
      dispatch->run();

      MIL << "Thread exit " << std::endl;
    }

    MediaCurlPrefetcher::ReqQueue::iterator MediaCurlPrefetcher::markRequestForCleanup(const MediaCurlPrefetcher::ReqQueue::iterator position)
    {
      MediaCurlPrefetcher::ReqQueue::iterator iter = position;
      ReqQueue::size_type reqIndex = std::distance( _requests.begin(), position );
      if ( reqIndex >= 0 && reqIndex < _requests.size() ) {
        _requestsToCleanup.push_back( std::move( *position ) );
        iter = _requests.erase( position );
        if ( static_cast<off_t>(reqIndex) <= _lastFinishedIndex )
          _lastFinishedIndex--;
        if ( static_cast<off_t>(reqIndex) <= _firstWaitingIndex )
          _firstWaitingIndex--;
      }
      return iter;
    }
  }
}
