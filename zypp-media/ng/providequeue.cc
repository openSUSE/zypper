/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "private/providequeue_p.h"
#include "private/provideitem_p.h"
#include "private/provide_p.h"
#include "private/providemessage_p.h"
#include "private/providedbg_p.h"

#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/zyppng/rpc/MessageStream>
#include <zypp-core/base/StringV.h>
#include <zypp-media/ng/provide-configvars.h>
#include <zypp-media/MediaException>
#include <zypp-media/auth/CredentialManager>

#include <zypp/APIConfig.h>
#include <variant>
#include <bitset>

namespace zyppng {

  bool ProvideQueue::Item::isAttachRequest() const
  {
    return ( _request->code () == ProvideMessage::Code::Attach );
  }

  bool ProvideQueue::Item::isFileRequest() const
  {
    return ( _request->code () == ProvideMessage::Code::Provide );
  }

  bool ProvideQueue::Item::isDetachRequest() const
  {
    return ( _request->code () == ProvideMessage::Code::Detach );
  }

  ProvideQueue::ProvideQueue(ProvidePrivate &parent) : _parent(parent)
  { }

  ProvideQueue::~ProvideQueue()
  {
    if ( zyppng::provideDebugEnabled() ) {
      if ( this->_activeItems.size() || this->_waitQueue.size() ) {
        DBG << "Queue shutdown with Items still running" << std::endl;
      }
    }
    immediateShutdown(std::make_exception_ptr(zypp::media::MediaException("Cancelled by queue shutdown")));
  }

  bool ProvideQueue::startup(const std::string &workerScheme, const zypp::filesystem::Pathname &workDir, const std::string &hostname ) {

    if ( _workerProc ) {
      ERR << "Queue Worker was already initialized" << std::endl;
      return true;
    }

    _myHostname = hostname;

    const auto &pN = _parent.workerPath() / ( "zypp-media-"+workerScheme ) ;
    MIL << "Trying to start " << pN << std::endl;
    const auto &pi = zypp::PathInfo( pN );
    if ( !pi.isExist() ) {
      ERR << "Failed to find worker for " << workerScheme << std::endl;
      return false;
    }

    if ( !pi.userMayX() ) {
      ERR << "Failed to start worker for " << workerScheme << " binary " << pi.asString() << " is not executable." << std::endl;
      return false;
    }

    if ( zypp::filesystem::assert_dir( workDir ) != 0 ) {
      ERR << "Failed to assert working directory '" << workDir << "' for worker " << workerScheme << std::endl;
      return false;
    }

    _currentExe = pN;
    _workerProc = Process::create();
    _workerProc->setWorkingDirectory ( workDir );
    _messageStream = RpcMessageStream::create( _workerProc );
    return doStartup();
  }


  void ProvideQueue::enqueue( ProvideRequestRef request )
  {
    Item i;
    i._request   = request;
    i._request->provideMessage().setRequestId( nextRequestId() );
    request->setCurrentQueue( shared_this<ProvideQueue>() );
    _waitQueue.push_back( std::move(i) );
    if ( _parent.isRunning() )
      scheduleNext();
  }

  void ProvideQueue::cancel( ProvideRequest *item , std::exception_ptr error )
  {
    const auto &isSameItem = [item]( const Item &i ){
      if ( i.isDetachRequest () )
        return false;
      return i._request.get() == item;
    };

    if ( !item )
      return;

    if ( item->code() != ProvideMessage::Code::Attach
         && item->code() != ProvideMessage::Code::Provide ) {
      ERR << "Can not cancel a " << item->code() << " request!" << std::endl;
      return;
    }

    if ( auto i = std::find_if( _waitQueue.begin(), _waitQueue.end(), isSameItem ); i != _waitQueue.end() ) {
      auto &reqRef = i->_request;
      reqRef->setCurrentQueue(nullptr);
      if ( reqRef->owner() )
        reqRef->owner()->finishReq( this, reqRef, error );
      _waitQueue.erase(i);
      _parent.schedule( ProvidePrivate::FinishReq ); // let the parent scheduler run since we have a open spot now
    } else if ( auto i = std::find_if( _activeItems.begin(), _activeItems.end(), isSameItem ); i != _activeItems.end() ) {
      cancelActiveItem(i, error);
    }
  }

  std::list<ProvideQueue::Item>::iterator ProvideQueue::dequeueActive( std::list<Item>::iterator it )
  {
    if ( it == _activeItems.end() )
      return it;

    if ( it->_request )
      it->_request->setCurrentQueue( nullptr );

    auto i = _activeItems.erase(it);
    _parent.schedule ( ProvidePrivate::FinishReq ); // Trigger the scheduler
    scheduleNext (); // keep the active items full
    return i;
  }

  void ProvideQueue::fatalWorkerError( const std::exception_ptr &reason )
  {
    immediateShutdown( reason ? reason : std::make_exception_ptr( zypp::media::MediaException("Fatal worker error")) );
  }

  void ProvideQueue::immediateShutdown( const std::exception_ptr &reason )
  {
    _queueShuttingDown  = true;

    while ( _waitQueue.size() ) {
      auto &item = _waitQueue.front();
      auto &reqRef = item._request;
      if ( reqRef && reqRef->owner() && !item.isDetachRequest() )
        reqRef->owner()->finishReq( this, reqRef, reason );
      _waitQueue.pop_front();
    }

    for ( auto i = _activeItems.begin(); i != _activeItems.end();  ) {
      auto &reqRef = i->_request;
      if ( reqRef && reqRef->owner() && !i->isDetachRequest() ) {
        i = cancelActiveItem(i, reason );
      } else {
        i++;
      }
    }

    if ( _workerProc && _workerProc->isRunning() ) {
      _workerProc->flush();
      _workerProc->closeWriteChannel();
      _workerProc->waitForExit();
      readAllStderr();
    }
  }

  std::list< ProvideQueue::Item >::iterator ProvideQueue::cancelActiveItem( std::list< Item >::iterator i , const std::__exception_ptr::exception_ptr &error )
  {
    auto &reqRef = i->_request;

    // already in cancelling process or finished
    if ( i->_state == Item::Cancelling || i->_state == Item::Finished )
      return (++i);

    // not possible but lets be safe
    if ( i->_state == Item::Pending ) {
      reqRef->setCurrentQueue(nullptr);
      if ( reqRef->owner() )
        reqRef->owner()->finishReq( this, reqRef, error );
      return dequeueActive(i);
    }

    // we first need to cancel the item
    auto c = ProvideMessage::createCancel ( i->_request->provideMessage().requestId() );
    if( !_messageStream->sendMessage(c.impl()) )
      ERR << "Failed to send cancel message to worker" << std::endl;

    i->_state = Item::Cancelling;
    reqRef->setCurrentQueue(nullptr);
    if ( reqRef->owner() )
      reqRef->owner()->finishReq( this, reqRef, error );
    reqRef.reset();
    return (++i);
  }

  void ProvideQueue::scheduleNext()
  {
    if ( _queueShuttingDown )
      return;

    while ( _waitQueue.size() && canScheduleMore() ) {
      auto item = std::move( _waitQueue.front() );
      _waitQueue.pop_front();

      auto &reqRef = item._request;
      if ( !reqRef->activeUrl() ) {
        ERR << "Item without active URL enqueued, this is a BUG." << std::endl;
        if ( reqRef->owner() )
          reqRef->owner()->finishReq( this, reqRef, ZYPP_EXCPT_PTR (zypp::media::MediaException("Item needs a activeURL to be queued.")) );
        continue;
      }

      if ( !_messageStream->sendMessage( reqRef->provideMessage().impl() ) ) {
        ERR << "Failed to send message to worker process." << std::endl;
        fatalWorkerError( ZYPP_EXCPT_PTR( zypp::media::MediaException("Failed to communicate with worker process.") ) );
        return;
      }

      item._state = Item::Queued;
      _activeItems.push_back( std::move(item) );
      _idleSince.reset();
    }

    if ( _waitQueue.empty() && _activeItems.empty() ) {
      _parent.schedule( ProvidePrivate::QueueIdle );
      if ( !_idleSince )
        _idleSince = std::chrono::steady_clock::now();
      _sigIdle.emit();
    }
  }

  bool ProvideQueue::canScheduleMore() const
  {
    return ( _activeItems.size() == 0 || ( _capabilities.cfg_flags () & zypp::proto::Capabilities::Pipeline ) == zypp::proto::Capabilities::Pipeline );
  }

  bool ProvideQueue::isIdle() const
  {
    return ( empty() );
  }

  std::optional<ProvideQueue::TimePoint> ProvideQueue::idleSince() const
  {
    return _idleSince;
  }

  bool ProvideQueue::empty() const
  {
    return ( _activeItems.empty() && _waitQueue.empty() );
  }

  uint ProvideQueue::requestCount() const
  {
    return _activeItems.size() + _waitQueue.size();
  }

  uint ProvideQueue::activeRequests() const
  {
    return _activeItems.size();
  }

  zypp::ByteCount ProvideQueue::expectedProvideSize() const
  {
    zypp::ByteCount dlSize;
    for ( const auto &i : _waitQueue ) {
      if ( i.isDetachRequest () )
        continue;

      auto &reqRef = i._request;
      if ( reqRef->code() != ProvideMessage::Code::Provide )
        continue;
      dlSize += reqRef->provideMessage().value( ProvideMsgFields::ExpectedFilesize, int64_t(0) ).asInt64();
    }
    for ( const auto &i : _activeItems ) {
      if ( i.isDetachRequest () )
        continue;
      auto &reqRef = i._request;
      if ( reqRef->code() != ProvideMessage::Code::Provide )
        continue;
      dlSize += reqRef->provideMessage().value( ProvideMsgFields::ExpectedFilesize, int64_t(0) ).asInt64();
    }
    return dlSize;
  }

  const std::string &ProvideQueue::hostname() const
  {
    return _myHostname;
  }

  const ProvideQueue::Config &ProvideQueue::workerConfig() const
  {
    return _capabilities;
  }

  SignalProxy<void ()> ProvideQueue::sigIdle()
  {
    return _sigIdle;
  }

  bool ProvideQueue::doStartup()
  {
    if ( _currentExe.empty() )
      return false;

    //const char *argv[] = { "gdbserver", ":10000", _currentExe.c_str(), nullptr };
    const char *argv[] = { _currentExe.c_str(), nullptr };
    if ( !_workerProc->start( argv) ) {
      ERR << "Failed to execute worker" << std::endl;

      _messageStream.reset ();
      _workerProc.reset ();

      return false;
    }

    // make sure the default read channel is StdOut so RpcMessageStream gets all the rpc messages
    _workerProc->setReadChannel ( Process::StdOut );

    // we are ready to send the data

    zypp::proto::Configuration conf;
    // @TODO actually write real config data :D
    conf.mutable_values ()->insert ( { AGENT_STRING_CONF.data (), "ZYpp " LIBZYPP_VERSION_STRING } );
    conf.mutable_values ()->insert ( { ATTACH_POINT.data (), _workerProc->workingDirectory().asString() } );
    conf.mutable_values ()->insert ( { PROVIDER_ROOT.data (), _parent.z_func()->providerWorkdir().asString() } );

    const auto &cleanupOnErr = [&](){
      readAllStderr();
      _messageStream.reset ();
      _workerProc->close();
      _workerProc.reset();
      return false;
    };

    if ( !_messageStream->sendMessage( conf ) ) {
      ERR << "Failed to send initial message to queue worker" << std::endl;
      return cleanupOnErr();
    }

    // wait for the data to be written
    _workerProc->flush ();

    // wait until we receive a message
    const auto &caps = _messageStream->nextMessageWait();
    if ( !caps || caps->messagetypename() != rpc::messageTypeName<zypp::proto::Capabilities>() ) {
      ERR << "Worker did not sent a capabilities message, aborting" << std::endl;
      return cleanupOnErr();
    }

    {
      auto p = _messageStream->parseMessage<zypp::proto::Capabilities>( *caps );
      if ( !p )
        return cleanupOnErr();

      _capabilities = std::move(*p);
    }

    DBG << "Received config for worker: " << this->_currentExe.asString() << " Worker Type: " << this->_capabilities.worker_type() << " Flags: " << std::bitset<32>( _capabilities.cfg_flags() ).to_string() << std::endl;

    // now we can set up signals and start processing messages
    connect( *_messageStream, &RpcMessageStream::sigMessageReceived, *this, &ProvideQueue::processMessage );
    connect( *_workerProc, &IODevice::sigChannelReadyRead, *this, &ProvideQueue::processReadyRead );
    connect( *_workerProc, &Process::sigFinished, *this, &ProvideQueue::procFinished );

    // make sure we do not miss messages
    processMessage();
    return true;
  }

  void ProvideQueue::processMessage() {

    const auto &getRequest = [&]( const auto &exp ) -> decltype(_activeItems)::iterator {
      if ( !exp ) {
        ERR << "Ignoring invalid request!" << std::endl;
        return _activeItems.end();
      }

      auto i = std::find_if( _activeItems.begin(), _activeItems.end(), [&]( const auto &elem ) {
        return exp->requestId() == elem._request->provideMessage().requestId();
      });

      if ( i == _activeItems.end() ) {
        ERR << "Ignoring unknown request ID: " << exp->requestId() << std::endl;
        return _activeItems.end();
      }

      return i;
    };

    const auto &sendErrorToWorker = [&]( const uint32_t reqId, const uint code, const std::string &reason, bool transient = false ) {
      auto r = ProvideMessage::createErrorResponse ( reqId, code, reason, transient );
      if ( !_messageStream->sendMessage( r.impl() ) ) {
        ERR << "Failed to send Error message to worker process." << std::endl;
        fatalWorkerError( ZYPP_EXCPT_PTR( zypp::media::MediaException("Failed to communicate with worker process.") ) );
        return false;
      }
      return true;
    };

    const bool doesDownload = this->_capabilities.worker_type() == Config::Downloading;
    const bool fileNeedsCleanup = doesDownload || ( _capabilities.worker_type() == Config::CPUBound && _capabilities.cfg_flags() & Config::FileArtifacts );

    while ( auto msg = _messageStream->nextMessage () ) {

      if ( msg->messagetypename() == rpc::messageTypeName<zypp::proto::ProvideMessage>() ) {

        const auto &provMsg = ProvideMessage::create(*msg);
        if ( !provMsg ) {
          fatalWorkerError( provMsg.error() );
          return;
        }

        const auto &reqIter = getRequest( provMsg );
        if ( reqIter == _activeItems.end() ) {
          if (  provMsg->code() == ProvideMessage::Code::ProvideFinished && fileNeedsCleanup ) {
            const auto locFName = provMsg->value( ProvideFinishedMsgFields::LocalFilename ).asString();
            if ( !_parent.isInCache(locFName) ) {
              MIL << "Received a ProvideFinished message for a non existant request. Since this worker reported to create file artifacts, the file is cleaned up." << std::endl;
              zypp::filesystem::unlink( locFName );
            }
          }
          continue;
        }

        auto &req = *reqIter;
        auto &reqRef =req._request;

        const auto code = provMsg->code();

        if ( code >= ProvideMessage::Code::FirstInformalCode && code <= ProvideMessage::Code::LastInformalCode ) {

          // send the message to the item but don't dequeue
          if ( reqRef && reqRef->owner() )
            reqRef->owner()->informalMessage ( *this, reqRef, *provMsg );
          continue;

        } else if ( code >= ProvideMessage::Code::FirstSuccessCode && code <= ProvideMessage::Code::LastSuccessCode ) {

          if ( req._state == Item::Cancelling ) {
            req._state = Item::Finished;
            dequeueActive( reqIter );
            continue;
          }

          if ( code == ProvideMessage::Code::ProvideFinished ) {

            // we are going to register the file to the cache if this is a downloading worker, so it can not leak
            // no matter if the item does the correct dance or not, this code is duplicated by all ProvideItems that receive ProvideFinished
            // results that require file cleanups.
            // we keep the ref around until after sending the result to the item. At that point it should take a reference
            std::optional<zypp::ManagedFile> dataRef;

            if ( !reqIter->isFileRequest() ) {
              ERR << "Invalid message for request ID: " << reqIter->_request->provideMessage().requestId() << std::endl;
              fatalWorkerError();
              return;
            }

            // when a worker is downloading we keep a internal book of cache files
            if ( doesDownload ) {
              const auto locFName = provMsg->value( ProvideFinishedMsgFields::LocalFilename ).asString();
              if ( provMsg->value( ProvideFinishedMsgFields::CacheHit, false ).asBool()) {
                dataRef = _parent.addToFileCache ( locFName );
                if ( !dataRef ) {
                  MIL << "CACHE MISS, file " << locFName << " was already removed, queueing again" << std::endl;
                  if ( reqRef->owner() )
                    reqRef->owner()->cacheMiss( reqRef );
                  reqRef->provideMessage().setRequestId( InvalidId );
                  req._state = Item::Pending;
                  _waitQueue.push_front( req );
                  dequeueActive( reqIter );
                  continue;
                }
              } else {
                dataRef = _parent.addToFileCache ( locFName );

                // unlikely this can happen but better be safe than sorry
                if ( !dataRef ) {
                  req._state = Item::Finished;
                  reqRef->setCurrentQueue(nullptr);
                  auto resp = ProvideMessage::createErrorResponse ( provMsg->requestId(), ProvideMessage::Code::InternalError, "File vanished between downloading and adding it to cache." );
                  if ( reqRef->owner() )
                    reqRef->owner()->finishReq( *this, reqRef, resp );
                  dequeueActive( reqIter );
                  continue;
                }
              }
            }
          }

          // send the message to the item and dequeue
          reqRef->setCurrentQueue(nullptr);
          if ( reqRef->owner() )
            reqRef->owner()->finishReq( *this, reqRef, *provMsg );
          req._state = Item::Finished;
          dequeueActive( reqIter );
          continue;

        } else if ( code >= ProvideMessage::Code::FirstClientErrCode && code <= ProvideMessage::Code::LastSrvErrCode ) {

          if ( req._state == Item::Cancelling ) {
            req._state = Item::Finished;
            dequeueActive( reqIter );
            continue;
          }

          // send the message to the item and dequeue
          reqRef->setCurrentQueue(nullptr);

          if ( reqRef->owner() )
            reqRef->owner()->finishReq( *this, reqRef, *provMsg );

          req._state = Item::Finished;
          dequeueActive( reqIter );
          continue;

        } else if ( code >= ProvideMessage::Code::FirstRedirCode && code <= ProvideMessage::Code::LastRedirCode ) {

          // redir is like a finished message, we can simply forgot about a cancelling request
          if ( req._state == Item::Cancelling ) {
            req._state = Item::Finished;
            dequeueActive( reqIter );
            continue;
          }

          // send the message to the item and dequeue
          reqRef->setCurrentQueue(nullptr);
          if ( reqRef->owner() )
            reqRef->owner()->finishReq( *this, reqRef, *provMsg );
          req._state = Item::Finished;
          dequeueActive( reqIter );
          continue;

        } else if ( code >= ProvideMessage::Code::FirstControllerCode && code <= ProvideMessage::Code::LastControllerCode ) {

          ERR << "Received Controller message from worker, this is a fatal error. Cancelling all requests!" << std::endl;
          fatalWorkerError ( ZYPP_EXCPT_PTR( zypp::media::MediaException("Controller message received from worker.") ) );
          return;

        } else if ( code >= ProvideMessage::Code::FirstWorkerCode && code <= ProvideMessage::Code::LastWorkerCode ) {

          if ( code == ProvideMessage::Code::AuthDataRequest ) {
            if ( !reqIter->isFileRequest() && !reqIter->isAttachRequest() ) {
              ERR << "Invalid message for request ID: " << reqRef->provideMessage().requestId() << std::endl;
              fatalWorkerError();
              return;
            }

            // if the file was cancelled we send a failure back
            if( reqIter->_state == Item::Cancelling ) {
              if ( !sendErrorToWorker( reqRef->provideMessage().requestId(), ProvideMessage::Code::NoAuthData, "Item was cancelled") )
                return;
              continue;
            }

            // we need a owner item to fetch the auth data for us
            if ( !reqRef->owner() ) {
              if ( !sendErrorToWorker( reqRef->provideMessage().requestId(), ProvideMessage::Code::NoAuthData, "Request has no owner" ) )
                return;
              continue;
            }

            if ( !reqRef->activeUrl() ) {
              if ( !sendErrorToWorker( reqRef->provideMessage().requestId(), ProvideMessage::Code::NoAuthData, "Item has no active URL, this is a bug." ) )
                return;
              continue;
            }

            try {
              zypp::Url u( provMsg->value( AuthDataRequestMsgFields::EffectiveUrl ).asString() );

              std::map<std::string, std::string> extraVals;
              provMsg->forEachVal( [&]( const std::string &name, const zyppng::ProvideMessage::FieldVal &val ) {

                if ( name == AuthDataRequestMsgFields::EffectiveUrl
                  || name == AuthDataRequestMsgFields::LastAuthTimestamp )
                  return true;

                if ( !val.isString() ) {
                  WAR << "Ignoring non string value for " << name << std::endl;
                  return true;
                }

                extraVals[name] = val.asString();
                return true;
              });

              const auto &authOpt = reqRef->owner()->authenticationRequired( *this, reqRef, u, provMsg->value( AuthDataRequestMsgFields::LastAuthTimestamp ).asInt64(), extraVals );
              if ( !authOpt ) {
                if ( !sendErrorToWorker( reqRef->provideMessage().requestId(), ProvideMessage::Code::NoAuthData, "No auth given by user." ) )
                  return;
                continue;
              }

              auto r = ProvideMessage::createAuthInfo ( reqRef->provideMessage().requestId(), authOpt->username(), authOpt->password(), authOpt->lastDatabaseUpdate(), authOpt->extraValues() );
              if ( !_messageStream->sendMessage( r.impl() ) ) {
                ERR << "Failed to send AuthorizationInfo to worker process." << std::endl;
                fatalWorkerError( ZYPP_EXCPT_PTR( zypp::media::MediaException("Failed to communicate with worker process.") ) );
                return;
              }
              continue;

            } catch ( const zypp::Exception &e ) {
              ZYPP_CAUGHT(e);
              if ( !sendErrorToWorker( reqRef->provideMessage().requestId(), ProvideMessage::Code::NoAuthData, e.asString() ) )
                  return;
              continue;
            }

          } else if ( code == ProvideMessage::Code::MediaChangeRequest ) {

            if ( !reqIter->isAttachRequest() ) {
              ERR << "Invalid message for request ID: " << reqIter->_request->provideMessage().requestId() << std::endl;
              fatalWorkerError();
              return;
            }

            // if the file was cancelled we send a failure back
            if( reqIter->_state == Item::Cancelling ) {
              if ( !sendErrorToWorker( reqRef->provideMessage().requestId(), ProvideMessage::Code::MediaChangeAbort, "Item was cancelled" ) )
                  return;
              continue;
            }

            MIL << "Worker sent a MediaChangeRequest, asking the user to insert the correct medium" << std::endl;

            //const std::string &label, const int32_t mediaNr, const std::vector<std::string> &devices, const std::optional<std::string> &desc
            std::vector<std::string> freeDevs;
            for ( const auto &val : provMsg->values( MediaChangeRequestMsgFields::Device) ) {
              freeDevs.push_back( val.asString() );
            }

            std::optional<std::string> desc;
            const auto &descVal = provMsg->value( MediaChangeRequestMsgFields::Desc );
            if ( descVal.valid () && descVal.isString() )
              desc = descVal.asString();

            auto res = _parent._sigMediaChange.emit(
              _parent.queueName(*this),
              provMsg->value( MediaChangeRequestMsgFields::Label ).asString(),
              provMsg->value( MediaChangeRequestMsgFields::MediaNr ).asInt(),
              freeDevs,
              desc
            );

            auto action = res ? *res : Provide::Action::ABORT;
            switch ( action ) {
              case Provide::Action::RETRY: {
                MIL << "Sending back a MediaChanged message, retrying to find medium " << std::endl;
                auto r = ProvideMessage::createMediaChanged ( reqIter->_request->provideMessage().requestId() );
                if ( !_messageStream->sendMessage( r.impl() ) ){
                  ERR << "Failed to send MediaChanged to worker process." << std::endl;
                  fatalWorkerError( ZYPP_EXCPT_PTR( zypp::media::MediaException("Failed to communicate with worker process.") ) );
                  return;
                }
                continue;
              }
              case Provide::Action::ABORT: {
                MIL << "Sending back a MediaChangeFailure message, request will fail " << std::endl;
                if ( !sendErrorToWorker( reqRef->provideMessage().requestId(), ProvideMessage::Code::MediaChangeAbort, "Cancelled by User" ) )
                  return;
                continue;
              }
              case Provide::Action::SKIP: {
                MIL << "Sending back a MediaChangeFailure message, request will fail " << std::endl;
                if ( !sendErrorToWorker( reqRef->provideMessage().requestId(), ProvideMessage::Code::MediaChangeSkip, "Skipped by User" ) )
                  return;
                continue;
              }
            }
          } else {
            // if there is a unsupported worker request we need to stop immediately because the worker will be blocked until it gets a answer
            ERR << "Unsupported worker request: "<<code<<", this is a fatal error!" << std::endl;
            fatalWorkerError();
            return;
          }

        } else  {
          // unknown code
          ERR << "Received unsupported message " << msg->messagetypename() << " with code " << code << " ignoring! " << std::endl;
        }

      } else {
        ERR << "Received unsupported message " << msg->messagetypename() << "ignoring" << std::endl;
      }
    }
  }

  /*!
   * Reads all of the log lines from stderr, call only when shutting down the queue
   * because this will also read partial lines and forwards them
   */
  void ProvideQueue::readAllStderr()
  {
    // read all stderr data so we get the full logs
    auto ba = _workerProc->channelReadLine(Process::StdErr);
    while ( !ba.empty() ) {
      forwardToLog(std::string( ba.data(), ba.size() ) );
      ba = _workerProc->channelReadLine(Process::StdErr);
    }
  }

  void ProvideQueue::forwardToLog( std::string &&logLine )
  {
    if ( (_capabilities.cfg_flags () & zypp::proto::Capabilities::ZyppLogFormat) == zypp::proto::Capabilities::ZyppLogFormat )
      zypp::base::LogControl::instance ().logRawLine( std::move(logLine) );
    else
      MIL << "Message from worker: " << _capabilities.worker_name() << ":" << logLine << std::endl;
  }

  void ProvideQueue::processReadyRead(int channel) {
    // ignore stdout here
    if ( channel == Process::StdOut )
      return;

    // forward the stderr output to the log bypassing the formatter
    // the worker already formatted the line
    while ( _workerProc->canReadLine(Process::StdErr) ) {
      const auto &data = _workerProc->channelReadLine( Process::StdErr );
      if ( data.empty() )
        return;

      forwardToLog(std::string( data.data(), data.size() ) );
    }
  }

  void ProvideQueue::procFinished(int exitCode)
  {
    // process all pending messages
    processMessage();

    // get all of the log lines
    readAllStderr();

    // shut down
    // @todo implement worker restart in case of a unexpected exit
    if ( !_queueShuttingDown )
      immediateShutdown( ZYPP_EXCPT_PTR( zypp::media::MediaException("Unexpected queue worker exit!") ) );

#if 0
    if ( !_queueShuttingDown ) {

      _crashCounter++;
      if ( _crashCounter > 3 )  {
        immediateShutdown( ZYPP_EXCPT_PTR( zypp::media::MediaException("Unexpected queue worker exit!") ) );
        return;
      }

      MIL << "Unexpected queue worker exit with code: " << exitCode << std::endl;
      // try to spawn the worker again, move active items back to wait list and start over

      if ( !doStartup () ) {

      }
    }
#endif
  }

  uint32_t ProvideQueue::nextRequestId() {
    return _parent.nextRequestId();
  }
}
