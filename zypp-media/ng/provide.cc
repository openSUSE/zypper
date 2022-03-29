#include "private/provide_p.h"
#include "private/providedbg_p.h"
#include "private/providequeue_p.h"
#include "private/provideitem_p.h"
#include <zypp-core/zyppng/io/IODevice>
#include <zypp-core/Url.h>
#include <zypp-core/base/DtorReset>
#include <zypp-core/fs/PathInfo.h>
#include <zypp-media/MediaException>
#include <zypp-media/CDTools>

// required to generate uuids
#include <glib.h>


L_ENV_CONSTR_DEFINE_FUNC(ZYPP_MEDIA_PROVIDER_DEBUG)

namespace zyppng {

  ProvidePrivate::ProvidePrivate(zypp::filesystem::Pathname &&workDir, Provide &pub)
    : BasePrivate(pub)
    , _workDir( std::move(workDir) )
    , _workerPath( constants::DEFAULT_PROVIDE_WORKER_PATH.data() )
  {
    if ( _workDir.empty() ) {
      _workDir = zypp::Pathname(".").realpath();
    } else {
      _workDir = _workDir.realpath();
    }

    MIL << "Provider workdir is: " << _workDir << std::endl;

    _scheduleTrigger->setSingleShot(true);
    Base::connect( *_scheduleTrigger, &Timer::sigExpired, *this, &ProvidePrivate::doSchedule );
  }

  void ProvidePrivate::schedule( ScheduleReason reason )
  {
    if ( provideDebugEnabled () ) {
      std::string_view reasonStr;
      switch( reason ) {
        case ProvideStart:
          reasonStr = "ProvideStart";
          break;
        case QueueIdle:
          reasonStr = "QueueIdle";
          break;
        case EnqueueItem:
          reasonStr = "EnqueueItem";
          break;
        case EnqueueReq:
          reasonStr = "EnqueueReq";
          break;
        case FinishReq:
          reasonStr = "FinishReq";
          break;
        case RestartAttach:
          reasonStr = "RestartAttach";
          break;
      }
      DBG << "Triggering the schedule timer (" << reasonStr << ")" << std::endl;
    }

    // we use a single shot timer that instantly times out when the event loop is entered the next time
    // this way we compress many schedule requests that happen during a eventloop run into one
    _scheduleTrigger->start(0);
  }

  void ProvidePrivate::doSchedule ( zyppng::Timer & )
  {
    if ( !_isRunning )
      return;

    if ( _isScheduling ) {
      DBG_PRV << "Scheduling triggered during scheduling, returning immediately." << std::endl;
      return;
    }

    const int cpuLimit =
#ifdef _SC_NPROCESSORS_ONLN
      sysconf(_SC_NPROCESSORS_ONLN) * 2;
#else
      DEFAULT_CPU_WORKERS;
#endif

    // helper lambda to find the worker that is idle for the longest time
    constexpr auto findLaziestWorker = []( const auto &workerQueues, const auto &idleNames  ) {
      auto candidate = workerQueues.end();
      ProvideQueue::TimePoint candidateIdleSince = ProvideQueue::TimePoint::max();

      //find the worker thats idle the longest
      for ( const auto &name : idleNames ) {
        auto thisElem = workerQueues.find(name);
        if ( thisElem == workerQueues.end() )
          continue;

        const auto idleS = thisElem->second->idleSince();
        if (  idleS
             && ( candidate == workerQueues.end() || *idleS < candidateIdleSince ) ) {
          candidateIdleSince = *idleS;
          candidate = thisElem;
        }
      }

      if ( candidate != workerQueues.end() )
        MIL_PRV << "Found idle worker:" << candidate->first << " idle since: " << candidateIdleSince.time_since_epoch().count() << std::endl;

      return candidate;
    };

    // clean up old media

    for ( auto iMedia = _attachedMediaInfos.begin(); iMedia != _attachedMediaInfos.end();  ) {
      if ( iMedia->_refCount > 0 ) {
        MIL_PRV << "Not releasing media " << iMedia->_name << " refcount is not zero" << std::endl;
        ++iMedia;
        continue;
      }
      if ( iMedia->_workerType == ProvideQueue::Config::Downloading ) {
        // we keep the information around for an hour so we do not constantly download the media files for no reason
        if ( std::chrono::system_clock::now() - iMedia->_idleSince >= std::chrono::hours(1) ) {
          MIL << "Detaching medium " << iMedia->_name << " for baseUrl " << iMedia->_attachedUrl << std::endl;
          iMedia = _attachedMediaInfos.erase(iMedia);
          continue;
        } else {
          MIL_PRV << "Not releasing media " << iMedia->_name << " downloading worker and not timed out yet." << std::endl;
        }
      } else {
        // mounting handlers, we need to send a request to the workers
        auto bQueue = iMedia->_backingQueue.lock();
        if ( bQueue ) {
          zypp::Url url = iMedia->_attachedUrl;
          url.setScheme( url.getScheme() + std::string( constants::ATTACHED_MEDIA_SUFFIX) );
          url.setAuthority( iMedia->_name );
          const auto &req = ProvideRequest::createDetach( url );
          if ( req ) {
            MIL << "Detaching medium " << iMedia->_name << " for baseUrl " << iMedia->_attachedUrl << std::endl;
            bQueue->enqueue ( *req );
            iMedia = _attachedMediaInfos.erase(iMedia);
            continue;
          } else {
            ERR << "Could not send detach request, creating the request failed" << std::endl;
          }
        } else {
          ERR << "Could not send detach request since no backing queue was defined" << std::endl;
        }
      }
      ++iMedia;
    }

    zypp::DtorReset schedFlag( _isScheduling, false );
    _isScheduling = true;

    const auto schedStart = std::chrono::system_clock::now();
    MIL_PRV << "Start scheduling" << std::endl;

    zypp::OnScopeExit deferExitMessage( [&](){
      const auto dur = std::chrono::system_clock::now() - schedStart;
      MIL_PRV << "Exit scheduling after:" << std::chrono::duration_cast<std::chrono::milliseconds>( dur ).count () << std::endl;
    });

    // bump inactive items
    for ( auto it = _items.begin (); it != _items.end(); ) {
      // was maybe released during scheduling
      if ( !(*it) )
        it = _items.erase(it);
      else {
        auto &item = *it;
        if ( item->state() == ProvideItem::Uninitialized ) {
          item->initialize();
        }
        it++;
      }
    }

    // we are scheduling now, everything that triggered the timer until now we can forget about
    _scheduleTrigger->stop();

    for( auto queueIter = _queues.begin(); queueIter != _queues.end(); queueIter ++ ) {

      const auto &scheme = queueIter->_schemeName;
      auto &queue = queueIter->_requests;

      if ( !queue.size() )
        continue;

      const auto &configOpt = schemeConfig ( scheme );

      MIL_PRV << "Start scheduling for scheme:" << scheme << " queue size is: " << queue.size() << std::endl;

      if ( !configOpt ) {
        // FAIL all requests in this queue
        ERR << "Scheme: " << scheme << " failed to return a valid configuration." << std::endl;

        while( queue.size() ) {
          auto item = std::move( queue.front() );
          queue.pop_front();
          if ( item->owner() )
            item->owner()->finishReq( nullptr, item, ZYPP_EXCPT_PTR(zypp::media::MediaException("Failed to query scheme config.")) );
        }

        continue;
      }

      // the scheme config that defines how we schedule requests on this set of queues
      const auto &config = configOpt.get();
      const auto isSingleInstance = ( (config.cfg_flags() & ProvideQueue::Config::SingleInstance) == ProvideQueue::Config::SingleInstance );
      if ( config.worker_type() == ProvideQueue::Config::Downloading && !isSingleInstance ) {

        for( auto i = queue.begin (); i != queue.end(); ) {

          // this is the only place where we remove elements from the queue when the scheduling flag is active
          // other code just nulls out requests in the queue if during scheduling items need to be removed
          while ( i != queue.end() && !(*i) ) {
            i = queue.erase(i);
          }

          if ( i == queue.end() )
            break;

          ProvideRequestRef item = *i;

          // Downloading queues do not support attaching via a AttachRequest, this is handled by simply providing the media verification files
          // If we hit this code path, its a bug
          if( item->code() == ProvideMessage::Code::Attach || item->code() == ProvideMessage::Code::Detach ) {
            i = queue.erase(i);
            if ( item->owner() )
              item->owner()->finishReq( nullptr, item, ZYPP_EXCPT_PTR( zypp::Exception("Downloading Queues do not support ProvideMessage::Code::Attach requests") ) );
            continue;
          }

          MIL_PRV << "Trying to schedule request: " << item->urls().front() << std::endl;

          // how many workers for this type do already exist
          int existingTypeWorkers = 0;

          // how many currently active connections are there
          int existingConnections = 0;

          // all currently available possible queues for the request
          std::vector< std::pair<zypp::Url, ProvideQueue*> > possibleHostWorkers;

          // currently idle workers
          std::vector<std::string> idleWorkers;

          // all mirrors without a existing worker
          std::vector<zypp::Url> mirrsWithoutWorker;
          for ( const auto &url : item->urls() ) {

            if ( effectiveScheme( url.getScheme() ) != scheme ) {
              MIL << "Mirror URL " << url << " is incompatible with current scheme: " << scheme << ", ignoring." << std::endl;
              continue;
            }

            if( item->owner()->canRedirectTo( item, url ) )
              mirrsWithoutWorker.push_back( url );
            else {
              MIL_PRV << "URL was rejected" << url << std::endl;
            }
          }

          // at this point the list contains all useable mirrors, if this list is empty the request needs to fail
          if( mirrsWithoutWorker.size() == 0 ) {
            MIL << "Request has NO usable URLs" << std::endl;
            if ( item->owner() )
              item->owner()->finishReq( nullptr, item, ZYPP_EXCPT_PTR(zypp::media::MediaException("No usable URLs in request spec.")) );
            i = queue.erase(i);
            continue;
          }


          for ( auto &[ queueName, workerQueue ] : _workerQueues ) {
            if ( ProvideQueue::Config::Downloading != workerQueue->workerConfig().worker_type() )
              continue;

            existingTypeWorkers ++;
            existingConnections += workerQueue->activeRequests();

            if ( workerQueue->isIdle() )
              idleWorkers.push_back (queueName);

            if ( !zypp::str::startsWith( queueName, scheme ) )
              continue;

            for ( auto i = mirrsWithoutWorker.begin (); i != mirrsWithoutWorker.end(); ) {
              const auto &u = *i;
              if ( u.getHost() == workerQueue->hostname() ) {
                if ( workerQueue->requestCount() < constants::DEFAULT_ACTIVE_CONN_PER_HOST )
                  possibleHostWorkers.push_back( {u, workerQueue.get()} );
                i = mirrsWithoutWorker.erase( i );
                // we can not stop after removing the first hit, since there could be multiple mirrors with the same hostname
              } else {
                ++i;
              }
            }
          }

          if( provideDebugEnabled() ) {
            MIL << "Current stats: " << std::endl;
            MIL << "Existing type workers: " << existingTypeWorkers << std::endl;
            MIL << "Existing active connections: " <<  existingConnections << std::endl;
            MIL << "Possible host workers: "<< possibleHostWorkers.size() << std::endl;
            MIL << "Mirrors without worker: " << mirrsWithoutWorker.size() << std::endl;
          }

          // need to wait for requests to finish in order to schedule more requests
          if ( existingConnections >= constants::DEFAULT_ACTIVE_CONN  ) {
            MIL_PRV << "Reached maximum nr of connections, break" << std::endl;
            break;
          }

          // if no workers are running, take the first mirror and start a worker for it
          // if < nr of workers are running, use a mirror we do not have a conn yet to
          if ( existingTypeWorkers < constants::DEFAULT_MAX_DYNAMIC_WORKERS
               && mirrsWithoutWorker.size() ) {

            MIL_PRV << "Free worker slots and available mirror URLs, starting a new worker" << std::endl;

            //@TODO out of the available mirrors use the best one based on statistics ( if available )
            bool found = false;
            for( const auto &url : mirrsWithoutWorker ) {

              // mark this URL as used now, in case the queue can not be started we won't try it anymore
              if ( !item->owner()->safeRedirectTo ( item, url ) )
                continue;

              ProvideQueueRef q = std::make_shared<ProvideQueue>( *this );
              if ( !q->startup( scheme, _workDir / scheme / url.getHost(), url.getHost() ) ) {
                break;
              } else {

                MIL_PRV << "Started worker for " << url.getHost() << " enqueing request" << std::endl;

                item->setActiveUrl(url);
                found = true;

                std::string str = zypp::str::Format("%1%://%2%") % scheme % url.getHost();
                _workerQueues[str] = q;
                q->enqueue( item );
                break;
              }
            }

            if( found ) {
              i = queue.erase(i);
              continue;
            }
          }

          // if we cannot start a new worker, find the best queue where we can push the item into
          if ( possibleHostWorkers.size() ) {

            MIL_PRV << "No free worker slots, looking for the best existing worker" << std::endl;
            bool found = false;
            while( possibleHostWorkers.size () ) {
              std::vector< std::pair<zypp::Url, ProvideQueue *> >::iterator candidate = possibleHostWorkers.begin();
              for ( auto i = candidate+1; i != possibleHostWorkers.end(); i++ ) {
                if ( i->second->activeRequests () < candidate->second->activeRequests () )
                  candidate = i;
              }

              if ( !item->owner()->safeRedirectTo( item, candidate->first ) ) {
                possibleHostWorkers.erase( candidate );
                continue;
              }

              MIL_PRV << "Using existing worker " << candidate->first.getHost() << " to download request" << std::endl;

              found = true;
              item->setActiveUrl( candidate->first );
              candidate->second->enqueue( item );
              break;
            }

            if( found ) {
              i = queue.erase(i);
              continue;
            }
          }

          // if we reach this place all we can now try is to decomission idle queues and use the new slot to start
          // a new worker
          if ( idleWorkers.size() && mirrsWithoutWorker.size() ) {

            MIL_PRV << "No free worker slots, no slots in existing queues, trying to decomission idle queues." << std::endl;

            auto candidate = findLaziestWorker( _workerQueues, idleWorkers );
            if ( candidate != _workerQueues.end() ) {

              // for now we decomission the worker and start a new one, should we instead introduce a "reset" message
              // that repurposes the worker to another hostname/workdir config?
              _workerQueues.erase(candidate);

              //@TODO out of the available mirrors use the best one based on statistics ( if available )
              bool found = false;
              for( const auto &url : mirrsWithoutWorker ) {

                if ( !item->owner()->safeRedirectTo ( item, url ) )
                  continue;

                ProvideQueueRef q = std::make_shared<ProvideQueue>( *this );
                if ( !q->startup( scheme, _workDir / scheme / url.getHost(), url.getHost() ) ) {
                  break;
                } else {

                  MIL_PRV << "Replaced worker for " << url.getHost() << ", enqueing request" << std::endl;

                  item->setActiveUrl(url);
                  found = true;

                  auto str = zypp::str::Format("%1%://%2%") % scheme % url.getHost();
                  _workerQueues[str] = q;
                  q->enqueue( item );
                }
              }

              if( found ) {
                i = queue.erase(i);
                continue;
              }
            }
          }

          // if we reach here we skip over the item and try to schedule it again later
          MIL_PRV << "End of line, deferring request for next try." << std::endl;
          i++;

        }
      } else if ( config.worker_type() == ProvideQueue::Config::CPUBound && !isSingleInstance ) {

        for( auto i = queue.begin (); i != queue.end(); ) {

          // this is the only place where we remove elements from the queue when the scheduling flag is active
          // other code just nulls out requests in the queue if during scheduling items need to be removed
          while ( i != queue.end() && !(*i) ) {
            i = queue.erase(i);
          }

          if ( i == queue.end() )
            break;

          // make a real reference so it does not dissapear when we remove it from the queue
          ProvideRequestRef item = *i;

          // CPU bound queues do not support attaching via a AttachRequest, this is handled by simply providing the media verification files
          // If we hit this code path, its a bug
          if( item->code() == ProvideMessage::Code::Attach || item->code() == ProvideMessage::Code::Detach ) {
            i = queue.erase(i);
            if ( item->owner () )
              item->owner()->finishReq( nullptr, item, ZYPP_EXCPT_PTR( zypp::Exception("CPU bound Queues do not support ProvideAttachSpecRef requests") ) );
            continue;
          }

          MIL_PRV << "Trying to schedule request: " << item->urls().front() << std::endl;

          // how many workers for this type do already exist
          int existingTypeWorkers = 0;
          int existingSchemeWorkers = 0;

          // all currently available possible queues for the request
          std::vector< ProvideQueue* > possibleWorkers;

          // currently idle workers
          std::vector<std::string> idleWorkers;

          // the URL we are going to use this time
          zypp::Url url;

          //CPU bound queues do not spawn per mirrors, we use the first compatible URL
          for ( const auto &tmpurl : item->urls() ) {
            if ( effectiveScheme( tmpurl.getScheme() ) != scheme ) {
              MIL << "Mirror URL " << tmpurl << " is incompatible with current scheme: " << scheme << ", ignoring." << std::endl;
              continue;
            }
            url = tmpurl;
            break;
          }

          // at this point if the URL is empty the request needs to fail
          if( !url.isValid() ) {
            MIL << "Request has NO usable URLs" << std::endl;
            if ( item->owner() )
              item->owner()->finishReq( nullptr, item, ZYPP_EXCPT_PTR(zypp::media::MediaException("No usable URLs in request spec.")) );
            i = queue.erase(i);
            continue;
          }

          for ( auto &[ queueName, workerQueue ] : _workerQueues ) {

            if ( ProvideQueue::Config::CPUBound != workerQueue->workerConfig().worker_type() )
              continue;

            const bool thisScheme = zypp::str::startsWith( queueName, scheme );

            existingTypeWorkers ++;
            if ( thisScheme ) {
              existingSchemeWorkers++;
              if ( workerQueue->canScheduleMore() )
                possibleWorkers.push_back(workerQueue.get());
            }

            if ( workerQueue->isIdle() )
              idleWorkers.push_back(queueName);
          }

          if( provideDebugEnabled() ) {
            MIL << "Current stats: " << std::endl;
            MIL << "Existing type workers: " << existingTypeWorkers << std::endl;
            MIL << "Possible CPU workers: "<< possibleWorkers.size() << std::endl;
          }

          // first we use existing idle workers of the current type
          if ( possibleWorkers.size() ) {
            bool found = false;
            for ( auto &w : possibleWorkers ) {
              if ( w->isIdle() ) {
                MIL_PRV << "Using existing idle worker to provide request" << std::endl;
                // this is not really required because we are not doing redirect checks
                item->owner()->redirectTo ( item, url );
                item->setActiveUrl( url );
                w->enqueue( item );
                i = queue.erase(i);
                found = true;
                break;
              }
            }
            if ( found )
              continue;
          }

          // we first start as many workers as we need before queueing more request to existing ones
          if ( existingTypeWorkers < cpuLimit  ) {

            MIL_PRV << "Free CPU slots, starting a new worker" << std::endl;

            // this is not really required because we are not doing redirect checks
            item->owner()->redirectTo ( item, url );

            ProvideQueueRef q = std::make_shared<ProvideQueue>( *this );
            if ( q->startup( scheme, _workDir / scheme ) ) {

              item->setActiveUrl(url);

              auto str = zypp::str::Format("%1%#%2%") % scheme % existingSchemeWorkers;
              _workerQueues[str] = q;
              q->enqueue( item );
              i = queue.erase(i);
              continue;
            } else {
              // CPU bound requests can not recover from this error
              i = queue.erase(i);
              if ( item->owner() )
                item->owner()->finishReq( nullptr, item, ZYPP_EXCPT_PTR( zypp::Exception("Unable to start worker for request.") ) );
              continue;
            }
          }

          // we can not start more workers, all we can do now is fill up queues of existing ones
          if ( possibleWorkers.size() ) {
            MIL_PRV << "No free CPU slots, looking for the best existing worker" << std::endl;

            if( possibleWorkers.size () ) {
              std::vector<ProvideQueue *>::iterator candidate = possibleWorkers.begin();
              for ( auto i = candidate+1; i != possibleWorkers.end(); i++ ) {
                if ( (*i)->activeRequests () < (*candidate)->activeRequests () )
                  candidate = i;
              }

              // this is not really required because we are not doing redirect checks
              item->owner()->redirectTo ( item, url );

              MIL_PRV << "Using existing worker to provide request" << std::endl;
              item->setActiveUrl( url );
              (*candidate)->enqueue( item );
              i = queue.erase(i);
              continue;
            }
          }

          // if we reach this place all we can now try is to decomission idle queues and use the new slot to start
          // a new worker
          if ( idleWorkers.size() ) {

            MIL_PRV << "No free CPU slots, no slots in existing queues, trying to decomission idle queues." << std::endl;

            auto candidate = findLaziestWorker( _workerQueues, idleWorkers );
            if ( candidate != _workerQueues.end() ) {

              _workerQueues.erase(candidate);

              // this is not really required because we are not doing redirect checks
              item->owner()->redirectTo ( item, url );

              ProvideQueueRef q = std::make_shared<ProvideQueue>( *this );
              if ( q->startup( scheme, _workDir / scheme ) ) {

                MIL_PRV << "Replaced worker, enqueing request" << std::endl;

                item->setActiveUrl(url);

                auto str = zypp::str::Format("%1%#%2%") % scheme % ( existingSchemeWorkers + 1 );
                _workerQueues[str] = q;
                q->enqueue( item );
                i = queue.erase(i);
                continue;
              } else {
                // CPU bound requests can not recover from this error
                i = queue.erase(i);
                if ( item->owner() )
                  item->owner()->finishReq( nullptr, item, ZYPP_EXCPT_PTR( zypp::Exception("Unable to start worker for request.") ) );
                continue;
              }
            }
          }

          // if we reach here we skip over the item and try to schedule it again later
          MIL_PRV << "End of line, deferring request for next try." << std::endl;
          i++;
        }

      } else {
        // either SingleInstance worker or Mounting/VolatileMounting

        for( auto i = queue.begin (); i != queue.end(); ) {

          // this is the only place where we remove elements from the queue when the scheduling flag is active
          // other code just nulls out requests in the queue if during scheduling items need to be removed
          while ( i != queue.end() && !(*i) ) {
            i = queue.erase(i);
          }

          if ( i == queue.end() )
            break;

          // make a real reference so it does not dissapear when we remove it from the queue
          ProvideRequestRef item = *i;
          MIL_PRV << "Trying to schedule request: " << item->urls().front() << std::endl;

          zypp::Url url;

          //mounting queues do not spawn per mirrors, we use the first compatible URL
          for ( const auto &tmpurl : item->urls() ) {
            if ( effectiveScheme( tmpurl.getScheme() ) != scheme ) {
              MIL << "Mirror URL " << tmpurl << " is incompatible with current scheme: " << scheme << ", ignoring." << std::endl;
              continue;
            }
            url = tmpurl;
            break;
          }

          // at this point if the URL is empty the request needs to fail
          if( !url.isValid() ) {
            MIL << "Request has NO usable URLs" << std::endl;
            if ( item->owner() )
              item->owner()->finishReq( nullptr, item, ZYPP_EXCPT_PTR(zypp::media::MediaException("No usable URLs in request spec.")) );
            i = queue.erase(i);
            continue;
          }


          ProvideQueue *qToUse = nullptr;
          if ( !_workerQueues.count(scheme) ) {
            ProvideQueueRef q = std::make_shared<ProvideQueue>( *this );
            if ( !q->startup( scheme, _workDir / scheme ) ) {
              ERR << "Worker startup failed!" << std::endl;
              // mounting/single instance requests can not recover from this error
              i = queue.erase(i);

              if ( item->owner() )
                item->owner()->finishReq( nullptr, item, ZYPP_EXCPT_PTR( zypp::Exception("Unable to start worker for request.") ) );
              continue;
            }

            MIL_PRV << "Started worker, enqueing request" << std::endl;
            qToUse = q.get();
            _workerQueues[scheme] = q;
          } else {
            MIL_PRV << "Found worker, enqueing request" << std::endl;
            qToUse = _workerQueues.at(scheme).get();
          }

          // this is not really required because we are not doing redirect checks
          item->owner()->redirectTo ( item, url );

          item->setActiveUrl(url);
          qToUse->enqueue( item );
          i = queue.erase(i);
        }
      }
    }
  }

  std::list<ProvideItemRef> &ProvidePrivate::items()
  {
    return _items;
  }

  zypp::media::CredManagerOptions &ProvidePrivate::credManagerOptions ()
  {
    return _credManagerOptions;
  }

  std::vector<AttachedMediaInfo> &ProvidePrivate::attachedMediaInfos()
  {
    return _attachedMediaInfos;
  }

  expected<ProvideQueue::Config> ProvidePrivate::schemeConfig( const std::string &scheme )
  {
    if ( auto i = _schemeConfigs.find( scheme ); i != _schemeConfigs.end() ) {
      return expected<ProvideQueue::Config>::success(i->second);
    } else {
      // we do not have the queue config yet, we need to start a worker to get one
      ProvideQueue q( *this );
      if ( !q.startup( scheme, _workDir / scheme ) ) {
        return expected<ProvideQueue::Config>::error(ZYPP_EXCPT_PTR(zypp::media::MediaException("Failed to start worker to read scheme config.")));
      }
      auto newItem = _schemeConfigs.insert( std::make_pair( scheme, q.workerConfig() ));
      return expected<ProvideQueue::Config>::success(newItem.first->second);
    }
  }

  std::shared_ptr<ProvideResourceData> ProvidePrivate::addToFileCache( const zypp::filesystem::Pathname &downloadedFile )
  {
    const auto &key = downloadedFile.asString();

    if ( !zypp::PathInfo(downloadedFile).isExist() ) {
      _fileCache.erase ( key );
      return nullptr;
    }

    auto i = _fileCache.insert( { key, nullptr } );
    if ( !i.second ) {
      // file did already exist in the cache, return the shared data
      return i.first->second;
    }

    i.first->second = std::make_shared<ProvideResourceData>();
    i.first->second->_myFile = zypp::ManagedFile( downloadedFile, zypp::filesystem::unlink );

    return i.first->second;
  }

  void ProvidePrivate::queueItem  ( ProvideItemRef item )
  {
    _items.push_back( item );
    schedule( ProvidePrivate::EnqueueItem );
  }

  void ProvidePrivate::dequeueItem( ProvideItem *item)
  {
    auto elem = std::find_if( _items.begin(), _items.end(), [item]( const auto &i){ return i.get() == item; } );
    if ( elem != _items.end() ) {
      if ( _isScheduling ) {
        (*elem).reset();
      } else {
        _items.erase(elem);
      }
    }
  }

  std::string ProvidePrivate::nextMediaId() const
  {
    zypp::AutoDispose rawStr( g_uuid_string_random (), g_free );
    return zypp::str::asString ( rawStr.value() );
  }

  AttachedMediaInfo &ProvidePrivate::addMedium(ProvideQueue::Config::WorkerType workerType, const zypp::Url &baseUrl, ProvideMediaSpec &spec )
  {
    auto str = nextMediaId();
    MIL_PRV << "Generated new ID for media attachment: " << str << std::endl;
    _attachedMediaInfos.push_back( AttachedMediaInfo{ std::move(str), {}, workerType, baseUrl, spec } );
    return _attachedMediaInfos.back();
  }

  AttachedMediaInfo &ProvidePrivate::addMedium(zypp::proto::Capabilities::WorkerType workerType, ProvideQueueWeakRef backingQueue, const std::string &id, const zypp::Url &baseUrl, ProvideMediaSpec &spec)
  {
    MIL_PRV << "New media attachment with id: " << id << std::endl;
    _attachedMediaInfos.push_back( AttachedMediaInfo{ id, backingQueue, workerType, baseUrl, spec } );
    return _attachedMediaInfos.back();
  }

  bool ProvidePrivate::queueRequest ( ProvideRequestRef req )
  {
    const auto &schemeName = effectiveScheme( req->url().getScheme() );
    auto existingQ = std::find_if( _queues.begin (), _queues.end(), [&schemeName]( const auto &qItem) {
      return (qItem._schemeName == schemeName);
    });
    if ( existingQ != _queues.end() ) {
      existingQ->_requests.push_back(req);
    } else {
      _queues.push_back( ProvidePrivate::QueueItem{ schemeName, {req} } );
    }

    schedule( ProvidePrivate::EnqueueReq );
    return true;
  }

  bool ProvidePrivate::dequeueRequest(ProvideRequestRef req , std::exception_ptr error)
  {
    auto queue = req->currentQueue ();
    if ( queue ) {
      queue->cancel( req.get(), error );
      return true;
    } else {
      // Request not started yet, search request queues
      for ( auto &q : _queues ) {
        auto elem = std::find( q._requests.begin(), q._requests.end(), req );
        if ( elem != q._requests.end() ) {
          q._requests.erase(elem);

          if ( req->owner() )
            req->owner()->finishReq( nullptr, req, error );
          return true;
        }
      }
    }
    return false;
  }

  const zypp::Pathname &ProvidePrivate::workerPath() const
  {
    return _workerPath;
  }

  const std::string ProvidePrivate::queueName( ProvideQueue &q ) const
  {
    for ( const auto &v : _workerQueues ) {
      if ( v.second.get() == &q )
        return v.first;
    }
    return {};
  }

  bool ProvidePrivate::isRunning() const
  {
    return _isRunning;
  }

  std::string ProvidePrivate::effectiveScheme(const std::string &scheme) const
  {
    const std::string &ss = zypp::str::stripSuffix( scheme, constants::ATTACHED_MEDIA_SUFFIX );
    if ( auto it = _workerAlias.find ( ss ); it != _workerAlias.end () ) {
      return it->second;
    }
    return ss;
  }

  void ProvidePrivate::onPulseTimeout( Timer & )
  {
    DBG_PRV << "Pulse timeout" << std::endl;

    auto now = std::chrono::system_clock::now();

    if ( _log ) _log->pulse();

    // release old cache files
    for ( auto i = _fileCache.begin (); i != _fileCache.end(); ) {
      auto &ptr = i->second;
      if( !ptr.unique() || now - ptr->_lastUnref < std::chrono::seconds(20)  ) {
        ++i;
        continue;
      }

      MIL << "Releasing file " << i->second->_myFile << " from cache, death timeout." << std::endl;
      i = _fileCache.erase(i);
    }
  }

  void ProvidePrivate::onQueueIdle()
  {
    if ( !_items.empty() )
      return;
    for ( auto &[k,q] : _workerQueues ) {
      if ( !q->empty() )
        return;
    }

    // all queues are empty
    _sigIdle.emit();
  }

  void ProvidePrivate::onItemStateChanged( ProvideItem &item )
  {
    if ( item.state() == ProvideItem::Finished ) {
      auto itemRef = item.shared_this<ProvideItem>();
      auto i = std::find( _items.begin(), _items.end(), itemRef );
      if ( i == _items.end() ) {
        ERR << "State of unknown Item changed, ignoring" << std::endl;
        return;
      }
      if ( _isScheduling )
        i->reset();
      else
        _items.erase(i);
    }
    if ( _items.empty() )
      onQueueIdle();
  }

  uint32_t ProvidePrivate::nextRequestId()
  {
    //@TODO is it required to handle overflow?
    return ++_nextRequestId;
  }


  ProvideMediaHandle::ProvideMediaHandle( Provide &parent, const std::string &hdl )
  : _ref( hdl, [ weakP = parent.weak_this<Provide>() ]( const std::string &v ) { auto p = weakP.lock(); if (p) p->releaseMedia(v);})
  {}

  bool ProvideMediaHandle::isValid() const
  {
    return (!_ref->empty());
  }

  std::string ProvideMediaHandle::handle() const
  {
    return _ref.value();
  }

  Provide::Provide( const zypp::Pathname &workDir ) : Base( *new ProvidePrivate( zypp::Pathname(workDir), *this ) )
  {
    Z_D();
    connect( *d->_pulseTimer, &Timer::sigExpired, *d, &ProvidePrivate::onPulseTimeout );
  }

  ProvideRef Provide::create( const zypp::filesystem::Pathname &workDir )
  {
    return ProvideRef( new Provide(workDir) );
  }

  AsyncOpRef<expected<Provide::MediaHandle>> Provide::attachMedia( const zypp::Url &url, const ProvideMediaSpec &request )
  {
    return attachMedia (  std::vector<zypp::Url>{url}, request );
  }

  AsyncOpRef<expected<Provide::MediaHandle>> Provide::attachMedia( const std::vector<zypp::Url> &urls, const ProvideMediaSpec &request )
  {
    Z_D();

    if ( urls.empty() ) {
      return makeReadyResult( expected<Provide::MediaHandle>::error( ZYPP_EXCPT_PTR( zypp::media::MediaException("No usable mirrors in mirrorlist."))) );
    }

    // sanitize the mirrors to contain only URLs that have same worker types
    std::vector<zypp::Url> usableMirrs;
    std::optional<ProvideQueue::Config> scheme;

    for ( auto mirrIt = urls.begin() ; mirrIt != urls.end(); mirrIt++ ) {
      const auto &s = d->schemeConfig( d->effectiveScheme( mirrIt->getScheme() ) );
      if ( !s ) {
        WAR << "URL: " << *mirrIt << " is not supported, ignoring!" << std::endl;
        continue;
      }
      if ( !scheme ) {
        scheme = *s;
        usableMirrs.push_back ( *mirrIt );
      } else {
        if ( scheme->worker_type () == s->worker_type () ) {
          usableMirrs.push_back( *mirrIt );
        } else {
          WAR << "URL: " << *mirrIt << " has different worker type than the primary URL: "<< usableMirrs.front() <<", ignoring!" << std::endl;
        }
      }
    }

    if ( !scheme || usableMirrs.empty() ) {
      return makeReadyResult( expected<Provide::MediaHandle>::error( ZYPP_EXCPT_PTR ( zypp::media::MediaException("No valid mirrors available") )) );
    }

    // first check if there is a already attached medium we can use as well
    auto &attachedMedia = d->attachedMediaInfos ();
    for ( auto &medium : attachedMedia ) {
      if ( medium.isSameMedium ( usableMirrs, request ) ) {
        medium.ref();
        return makeReadyResult( expected<Provide::MediaHandle>::success( Provide::MediaHandle( *this, medium._name)  ) );
      }
    }

    auto op = AttachMediaItem::create( usableMirrs, request, *d_func() );
    d->queueItem (op);
    return op->promise();
  }

  void Provide::releaseMedia( const std::string &mediaRef )
  {
    Z_D();

    if ( mediaRef.empty() )
      return;

    const auto i = std::find_if( d->_attachedMediaInfos.begin(), d->_attachedMediaInfos.end(), [&]( const auto &info ){ return info._name == mediaRef;} );
    if ( i == d->_attachedMediaInfos.end() ) {
      ERR << "Unknown media attach handle" << std::endl;
      return;
    }

    // only unref'ing here, the scheduler will generate a message to the queues if needed
    i->unref();
  }

  AsyncOpRef< expected<ProvideRes> > Provide::provide( const std::vector<zypp::Url> &urls, const ProvideFileSpec &request )
  {
    Z_D();
    auto op = ProvideFileItem::create( urls, request, *d );
    d->queueItem (op);
    return op->promise();
  }

  AsyncOpRef< expected<ProvideRes> > Provide::provide( const zypp::Url &url, const ProvideFileSpec &request )
  {
    return provide( std::vector<zypp::Url>{ url }, request );
  }

  AsyncOpRef< expected<ProvideRes> > Provide::provide( const MediaHandle &attachHandle, const zypp::Pathname &fileName, const ProvideFileSpec &request )
  {
    Z_D();
    const auto i = std::find_if( d->_attachedMediaInfos.begin(), d->_attachedMediaInfos.end(), [&]( const auto &info ){ return info._name == attachHandle.handle();} );
    if ( i == d->_attachedMediaInfos.end() ) {
      return makeReadyResult( expected<ProvideRes>::error( ZYPP_EXCPT_PTR( zypp::media::MediaException("Invalid attach handle")) ) );
    }

    // for downloading items we need to make the baseUrl part of the request URL
    zypp::Url url = i->_attachedUrl;

    // real mount devices use a ID to reference a attached medium, for those we do not need to send the baseUrl as well since its already
    // part of the mount point, so if we mount host:/path/to/repo to the ID 1234 and look for the file /path/to/repo/file1 the request URL will look like:  nfs-media://1234/file1
    if ( i->_workerType == ProvideQueue::Config::SimpleMount || i->_workerType == ProvideQueue::Config::VolatileMount ) {
      url = zypp::Url();
      // work around the zypp::Url requirements for certain Url schemes by attaching a suffix, that way we are always able to have a authority
      url.setScheme( i->_attachedUrl.getScheme() + std::string(constants::ATTACHED_MEDIA_SUFFIX) );
      url.setAuthority( i->_name );
      url.setPathName("/");
    }

    url.appendPathName( fileName );
    auto op = ProvideFileItem::create( {url}, request, *d );

    i->ref();
    op->setMediaRef( MediaHandle( *this, i->_name ));

    d->queueItem (op);
    return op->promise();
  }

  void Provide::start ()
  {
    Z_D();
    d->_isRunning = true;
    d->_pulseTimer->start( 5000 );
    d->schedule( ProvidePrivate::ProvideStart );
    if ( d->_log ) d->_log->provideStart();
  }

  void Provide::setWorkerPath(const zypp::filesystem::Pathname &path)
  {
    d_func()->_workerPath = path;
  }

  bool Provide::ejectDevice(const std::string &queueRef, const std::string &device)
  {
    if ( !queueRef.empty() ) {
      return zypp::media::CDTools::openTray(device);
    }
    return false;
  }

  void Provide::setStatusTracker( ProvideStatusRef tracker )
  {
    d_func()->_log = tracker;
  }

  const zypp::Pathname &Provide::providerWorkdir () const
  {
    return d_func()->_workDir;
  }

  const zypp::media::CredManagerOptions &Provide::credManangerOptions () const
  {
    Z_D();
    return d->_credManagerOptions;
  }

  void Provide::setCredManagerOptions( const zypp::media::CredManagerOptions & opt )
  {
    d_func()->_credManagerOptions = opt;
  }

  SignalProxy<void ()> Provide::sigIdle()
  {
    return d_func()->_sigIdle;
  }

  SignalProxy<Provide::MediaChangeAction ( const std::string &queueRef, const std::string &, const int32_t, const std::vector<std::string> &, const std::optional<std::string> &)> Provide::sigMediaChangeRequested()
  {
    return d_func()->_sigMediaChange;
  }

  SignalProxy< std::optional<zypp::media::AuthData> ( const zypp::Url &reqUrl, const std::string &triedUsername, const std::map<std::string, std::string> &extraValues ) > Provide::sigAuthRequired()
  {
    return d_func()->_sigAuthRequired;
  }

  ZYPP_IMPL_PRIVATE(Provide);

  ProvideStatus::ProvideStatus( ProvideRef parent )
    : _provider( parent )
  { }

  void ProvideStatus::provideStart ()
  {
    _stats = Stats();
    _stats._startTime     = std::chrono::steady_clock::now();
    _stats._lastPulseTime = std::chrono::steady_clock::now();
  }

  void ProvideStatus::itemDone ( ProvideItem &item )
  {
    const auto &sTime = item.startTime();
    const auto &fTime = item.finishedTime();
    if ( sTime > sTime.min() && fTime >= sTime ) {
      auto duration = std::chrono::duration_cast<std::chrono::seconds>( item.finishedTime() - item.startTime() );
      if ( duration.count() )
        MIL << "Item finished after " << duration.count() << " seconds, with " << zypp::ByteCount( item.currentStats()->_bytesProvided.operator zypp::ByteCount::SizeType() / duration.count() ) << "/s" << std::endl;
      MIL << "Item finished after " << (item.finishedTime() - item.startTime()).count() << " ns" << std::endl;
    }
    pulse( );
  }

  void ProvideStatus::itemFailed ( ProvideItem &item )
  {
    MIL << "Item failed" << std::endl;
  }

  const ProvideStatus::Stats& ProvideStatus::stats() const
  {
    return _stats;
  }

  void ProvideStatus::pulse ( )
  {
    auto prov = _provider.lock();
    if ( !prov )
      return;

    const auto lastFinishedBytes = _stats._finishedBytes;
    const auto lastPartialBytes  = _stats._partialBytes;
    _stats._expectedBytes        = _stats._finishedBytes; // finished bytes are expected too!
    zypp::ByteCount tmpPartialBytes (0); // bytes that are finished in staging, but not commited to cache yet

    for ( const auto &i : prov->d_func()->items() ) {

      if ( !i // maybe released during scheduling
         || i->state() == ProvideItem::Cancelling )
        continue;

      if ( i->state() == ProvideItem::Uninitialized
        || i->state() == ProvideItem::Pending ) {
          _stats._expectedBytes += i->bytesExpected();
          continue;
        }

      i->pulse();

      const auto & stats = i->currentStats();
      const auto & prevStats = i->previousStats();
      if ( !stats || !prevStats ) {
        ERR << "Bug! Stats should be initialized by now" << std::endl;
        continue;
      }

      if ( i->state() == ProvideItem::Downloading
        || i->state() == ProvideItem::Processing
        || i->state() == ProvideItem::Finalizing ) {
        _stats._expectedBytes  += stats->_bytesExpected;
        tmpPartialBytes        += stats->_bytesProvided;
      } else if ( i->state() == ProvideItem::Finished ) {
         _stats._finishedBytes += stats->_bytesProvided; // remember those bytes are finished in stats directly
         _stats._expectedBytes += stats->_bytesProvided;
      }
    }

    const auto now = std::chrono::steady_clock::now();
    const auto sinceLast = std::chrono::duration_cast<std::chrono::seconds>( now - _stats._lastPulseTime );
    const auto lastFinB = lastPartialBytes + lastFinishedBytes;
    const auto currFinB = tmpPartialBytes  + _stats._finishedBytes;

    const auto diff = currFinB - lastFinB;
    _stats._lastPulseTime = now;
    _stats._partialBytes  = tmpPartialBytes;

    if ( sinceLast >= std::chrono::seconds(1) )
       _stats._perSecondSinceLastPulse = ( diff / ( sinceLast.count() )  );

    auto sinceStart = std::chrono::duration_cast<std::chrono::seconds>( _stats._lastPulseTime - _stats._startTime );
    if ( sinceStart.count() ) {
      const size_t diff = _stats._finishedBytes + _stats._partialBytes;
      _stats._perSecond = zypp::ByteCount( diff / sinceStart.count() );
    }
  }
}
