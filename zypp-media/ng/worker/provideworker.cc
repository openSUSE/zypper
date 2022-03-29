/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "provideworker.h"
#include <zypp-core/base/DtorReset>
#include <zypp-core/AutoDispose.h>
#include <zypp-core/Url.h>
#include <zypp-core/Date.h>
#include <zypp-core/zyppng/pipelines/AsyncResult>
#include <zypp-core/base/LogControl.h>
#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/fs/TmpPath.h>
#include <zypp-core/zyppng/base/private/threaddata_p.h>
#include <zypp-core/zyppng/base/AutoDisconnect>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-media/MediaConfig>
#include <ostream>
#include <fstream>

#include <zypp-media/ng/private/providedbg_p.h>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "ProvideWorker"

namespace zyppng::worker {

  using namespace zyppng::operators;

  RequestCancelException::RequestCancelException() : zypp::media::MediaException ("Request was cancelled")
  { }

  ProvideWorker::ProvideWorker(std::string_view workerName) : _workerName(workerName)
  {
    // do not change the order of these calls, otherwise showing the threadname does not work
    // enableLogForwardingMode will initialize the log which would override the current thread name
    zypp::base::LogControl::instance().enableLogForwardingMode( true );
    ThreadData::current().setName( workerName );

    // we use a singleshot timer that triggers message handling
    connect( *_msgAvail, &Timer::sigExpired, *this, &ProvideWorker::messageLoop );
    _msgAvail->setSingleShot(true);

    // another timer to trigger a delayed shutdown
    connectFunc( *_delayedShutdown, &Timer::sigExpired, [this]( zyppng::Timer & ) {
      maybeDelayedShutdown();
    }, *this );
    _delayedShutdown->setSingleShot(true);
  }

  ProvideWorker::~ProvideWorker()
  { }

  RpcMessageStream::Ptr ProvideWorker::messageStream() const
  {
    return _stream;
  }

  expected<void> ProvideWorker::run( int recv, int send )
  {
    // reentry not supported
    assert ( !_isRunning );

    zypp::DtorReset res( _isRunning );
    _isRunning = true;

    initLog();

    zypp::OnScopeExit cleanup([&](){
      _stream.reset();
      _controlIO.reset();
      _loop.reset();
    });

    _controlIO = AsyncDataSource::create();
    if ( !_controlIO->openFds( { recv }, send ) ) {
      return expected<void>::error( ZYPP_EXCPT_PTR(zypp::Exception("Failed to open control FDs")) );
    }

    connect( *_controlIO, &AsyncDataSource::sigReadFdClosed, *this, &ProvideWorker::readFdClosed );
    connect( *_controlIO, &AsyncDataSource::sigWriteFdClosed, *this, &ProvideWorker::writeFdClosed );

    _stream = RpcMessageStream::create( _controlIO );

    return executeHandshake () | mbind( [&]() {
      AutoDisconnect disC[] = {
        connect( *_stream, &RpcMessageStream::sigMessageReceived, *this, &ProvideWorker::messageReceived ),
        connect( *_stream, &RpcMessageStream::sigInvalidMessageReceived, *this, &ProvideWorker::onInvalidMessageReceived )
      };
      _loop->run();
      if ( _fatalError )
        return expected<void>::error( _fatalError );
      return expected<void>::success();
    });
  }

  std::deque<ProvideWorkerItemRef> &ProvideWorker::requestQueue()
  {
    return _pendingProvides;
  }

  ProvideWorker::ProvideNotificatioMode ProvideWorker::provNotificationMode() const {
    return _provNotificationMode;
  }

  void ProvideWorker::setProvNotificationMode( const ProvideNotificatioMode &provNotificationMode ) {
    _provNotificationMode = provNotificationMode;
  }

  void ProvideWorker::initLog()
  {
    // by default we log to strErr, if user code wants to change that it can overload this function
    zypp::base::LogControl::instance().logToStdErr();
  }

  ProvideWorkerItemRef ProvideWorker::makeItem( ProvideMessage &&spec )
  {
    return std::make_shared<ProvideWorkerItem>( std::move(spec) );
  }

  void ProvideWorker::provideStart(const uint32_t id, const zypp::Url &url, const zypp::filesystem::Pathname &localFile, const zypp::Pathname &stagingFile )
  {
    if ( !_stream->sendMessage( ProvideMessage::createProvideStarted ( id
          , url
          , localFile.empty () ? std::optional<std::string>() : localFile.asString ()
          , stagingFile.empty () ? std::optional<std::string>() : stagingFile.asString ()
        ).impl() ) ) {
      ERR << "Failed to send ProvideStart message" << std::endl;
    }
  }

  void ProvideWorker::provideSuccess(const uint32_t id, bool cacheHit, const zypp::filesystem::Pathname &localFile)
  {
    MIL_PRV << "Sending provideSuccess for id " << id  << " file " << localFile << std::endl;
    if ( !_stream->sendMessage( ProvideMessage::createProvideFinished( id ,localFile.asString() ,cacheHit).impl() ) ) {
      ERR << "Failed to send ProvideSuccess message" << std::endl;
    }
  }

  void ProvideWorker::provideFailed(const uint32_t id, const uint code, const std::string &reason, const bool transient, const HeaderValueMap extra )
  {
    MIL_PRV << "Sending provideFailed for request " << id << " err: " << reason << std::endl;
    auto msg = ProvideMessage::createErrorResponse ( id, code, reason, transient );
    for ( auto i = extra.beginList (); i != extra.endList(); i++ ) {
      for ( const auto &val : i->second )
        msg.addValue( i->first, val );
    }
    if ( !_stream->sendMessage( msg.impl() ) ) {
      ERR << "Failed to send ProvideFailed message" << std::endl;
    }
  }


  void ProvideWorker::provideFailed  ( const uint32_t id, const uint code, const bool transient, const zypp::Exception &e )
  {
    zyppng::HeaderValueMap extra;
    if ( !e.historyEmpty() ) {
      extra = { { std::string(zyppng::ErrMsgFields::History),  { zyppng::HeaderValueMap::Value(e.historyAsString()) }} };
    }
    provideFailed( id
      , code
      , e.asUserString()
      , transient
      , extra );
  }


  void ProvideWorker::attachSuccess(const uint32_t id)
  {
    MIL_PRV << "Sending attachSuccess for request " << id << std::endl;
    if ( !_stream->sendMessage( ProvideMessage::createAttachFinished ( id ).impl() ) ) {
      ERR << "Failed to send AttachFinished message" << std::endl;
    } else {
      MIL << "Sent back attach success" << std::endl;
    }
  }

  void ProvideWorker::detachSuccess(const uint32_t id)
  {
    MIL_PRV << "Sending detachSuccess for request " << id << std::endl;
    if ( !_stream->sendMessage( ProvideMessage::createDetachFinished ( id ).impl() ) ) {
      ERR << "Failed to send DetachFinished message" << std::endl;
    }
  }

  expected<ProvideMessage> ProvideWorker::sendAndWaitForResponse( const ProvideMessage &request , const std::vector<uint> &responseCodes )
  {
    // make sure immediateShutdown is not called while we are blocking here
    zypp::DtorReset delayedReset( _inControllerRequest );
    _inControllerRequest = true;

    if ( !_stream->sendMessage( request.impl() ) )
      return expected<ProvideMessage>::error( ZYPP_EXCPT_PTR(zypp::Exception("Failed to send message")) );

    // flush the io device, this will block until all bytes are written
    _controlIO->flush();

    while ( !_fatalError ) {

      const auto &msg = _stream->nextMessageWait() | [&]( auto &&nextMessage ) {
        if ( !nextMessage ) {
          if ( _fatalError )
            return expected<RpcMessage>::error( _fatalError );
          else
            return expected<RpcMessage>::error( ZYPP_EXCPT_PTR(zypp::Exception("Failed to wait for response")) );
        }
        return expected<RpcMessage>::success( std::move(*nextMessage) );
      } | mbind ( [&]( auto && m) {
        return parseReceivedMessage(m);
      } );

      if ( !msg ) {
        ERR << "Failed to receive message" << std::endl;
        return msg;
      }

      if ( std::find( responseCodes.begin (), responseCodes.end(), msg->code() ) != responseCodes.end() ) {
        return msg;
      }

      // remember other messages for later
      MIL << "Remembering message for later: " << msg->code () << std::endl;
      _pendingMessages.push_back(*msg);
      _msgAvail->start(0);
    }
    return expected<ProvideMessage>::error( _fatalError );
  }

  ProvideWorker::MediaChangeRes ProvideWorker::requestMediaChange(const uint32_t id, const std::string &label, const int32_t mediaNr, const std::vector<std::string> &devices, const std::optional<std::string> &desc )
  {
    return sendAndWaitForResponse( ProvideMessage::createMediaChangeRequest ( id, label, mediaNr, devices, desc  ), { ProvideMessage::Code::MediaChanged, ProvideMessage::Code::MediaChangeAbort, ProvideMessage::Code::MediaChangeSkip } )
      | [&]( expected<ProvideMessage> &&m ) {
          if ( !m ) {
            MIL << "Failed to wait for message, aborting the request " << std::endl;
            return ProvideWorker::MediaChangeRes::ABORT;
          }
          MIL << "Wait finished, with messages still pending: " << this->_pendingMessages.size() << " and provs still pending: " << this->_pendingProvides.size() << std::endl;
          if ( m->code() == ProvideMessage::Code::MediaChanged )
            return ProvideWorker::MediaChangeRes::SUCCESS;
          else if ( m->code() == ProvideMessage::Code::MediaChangeSkip )
            return ProvideWorker::MediaChangeRes::SKIP;
          else
            return ProvideWorker::MediaChangeRes::ABORT;
        };
  }

  expected<AuthInfo> ProvideWorker::requireAuthorization( const uint32_t id, const zypp::Url &url, const std::string &lastTriedUsername, const int64_t lastTimestamp, const std::map<std::string, std::string> &extraFields )
  {
    return sendAndWaitForResponse( ProvideMessage::createAuthDataRequest( id, url, lastTriedUsername, lastTimestamp, extraFields ), { ProvideMessage::Code::AuthInfo, ProvideMessage::Code::NoAuthData } )
           | mbind( [&]( ProvideMessage &&m ) {
               if ( m.code() == ProvideMessage::Code::AuthInfo ) {

                AuthInfo inf;
                m.forEachVal( [&]( const std::string &name, const ProvideMessage::FieldVal &val ) {
                  if ( name == AuthInfoMsgFields::Username ) {
                    inf.username = val.asString();
                  } else if ( name == AuthInfoMsgFields::Password ) {
                    inf.password = val.asString();
                  } else if ( name == AuthInfoMsgFields::AuthTimestamp ) {
                    inf.last_auth_timestamp = val.asInt64();
                  } else {
                    if ( !val.isString() ) {
                      ERR << "Ignoring invalid extra value, " << name << " is not of type string" << std::endl;
                    }
                    inf.extraKeys[name] = val.asString();
                  }
                  return true;
                });
                return expected<AuthInfo>::success(inf);

               }
               return expected<AuthInfo>::error( ZYPP_EXCPT_PTR( zypp::media::MediaException("No Auth data")) );
             });
  }

  AsyncDataSource &ProvideWorker::controlIO()
  {
    return *_controlIO.get();
  }

  expected<void> ProvideWorker::executeHandshake()
  {
    const auto &helo = _stream->nextMessageWait();
    if ( !helo ) {
      ERR << "Could not receive a handshake message, aborting" << std::endl;
      return expected<void>::error( ZYPP_EXCPT_PTR(zypp::Exception("Failed to receive handshake message")) );;
    }

    auto exp = _stream->parseMessage<zypp::proto::Configuration>( *helo );
    if ( !exp ) {
      invalidMessageReceived( exp.error() );
      return expected<void>::error(exp.error());
    }

    return std::move(*exp) | [&]( auto &&conf ) {

      _workerConf = std::move(conf);

      auto &mediaConf = zypp::MediaConfig::instance();
      for( const auto &[key,value] : _workerConf.values() ) {
        zypp::Url keyUrl( key );
        if ( keyUrl.getScheme() == "zconfig" && keyUrl.getAuthority() == "main" ) {
          mediaConf.setConfigValue( keyUrl.getAuthority(), zypp::Pathname(keyUrl.getPathName()).basename(), value );
        }
      }

      return initialize( _workerConf ) | mbind([&]( WorkerCaps &&caps ){

        caps.set_worker_name( _workerName.data() );

        caps.set_cfg_flags ( WorkerCaps::Flags(caps.cfg_flags() | WorkerCaps::ZyppLogFormat) );
        if ( !_stream->sendMessage ( caps ) ) {
          return expected<void>::error( ZYPP_EXCPT_PTR(zypp::Exception("Failed to send capabilities")) );
        }
        return expected<void>::success ();
      });
    };
  }

  void ProvideWorker::messageLoop( Timer & )
  {
    if ( _fatalError )
      return;

    while ( _pendingMessages.size () ) {
      auto m = _pendingMessages.front ();
      _pendingMessages.pop_front ();
      handleSingleMessage(m);
    }

    if ( !_fatalError && _pendingProvides.size() ) {
      provide();
    }

    // keep poking until there are no provides anymore
    if ( !_fatalError && ( _pendingMessages.size() || ( _pendingProvides.size () && _provNotificationMode == QUEUE_NOT_EMTPY ) ) ) {
      _msgAvail->start(0);
    }

  }

  void ProvideWorker::maybeDelayedShutdown()
  {
    if ( _inControllerRequest ) {
      _delayedShutdown->start(0);
      return;
    }

    immediateShutdown();
    _loop->quit ();
  }

  void ProvideWorker::readFdClosed( uint, AsyncDataSource::ChannelCloseReason )
  {
    MIL << "Read FD closed, exiting." << std::endl;
    maybeDelayedShutdown();
  }

  void ProvideWorker::writeFdClosed( AsyncDataSource::ChannelCloseReason )
  {
    MIL << "Write FD closed, exiting." << std::endl;
    maybeDelayedShutdown();
  }

  void ProvideWorker::messageReceived()
  {
    while ( auto message = _stream->nextMessage() ) {
      if ( _fatalError )
        break;
      pushSingleMessage(*message);
    }
  }

  void ProvideWorker::onInvalidMessageReceived()
  {
    invalidMessageReceived( std::exception_ptr() );
  }

  void ProvideWorker::invalidMessageReceived( std::exception_ptr p )
  {
    ERR << "Received a invalid message on the input stream, aborting" << std::endl;
    if ( p )
      _fatalError = p;
    else
      _fatalError = ZYPP_EXCPT_PTR( InvalidMessageReceivedException() );
    immediateShutdown();
    _loop->quit();
  }

  void ProvideWorker::handleSingleMessage( const ProvideMessage &provide )
  {
    const auto code = provide.code();
    // we only accept requests here
    if ( code >= ProvideMessage::Code::FirstControllerCode && code <= ProvideMessage::Code::LastControllerCode ) {

      MIL_PRV << "Received request: " << code << std::endl;

      if ( code == ProvideMessage::Code::Cancel ) {
        const auto &i = std::find_if( _pendingProvides.begin (), _pendingProvides.end(), [ id = provide.requestId() ]( const auto &it ){ return it->_spec.requestId() == id; } );
        if ( i != _pendingProvides.end() ) {
          switch ( (*i)->_state ) {
            case ProvideWorkerItem::Pending:
              _stream->sendMessage ( ProvideMessage::createErrorResponse ( provide.requestId (), ProvideMessage::Code::Cancelled, "Cancelled by user." ).impl() );
              _pendingProvides.erase(i);
              break;
            case ProvideWorkerItem::Running:
              cancel(i);
              break;
            case ProvideWorkerItem::Finished:
              break;
          }
          MIL << "Received Cancel for unknown request: " << provide.requestId() << ", ignoring!" << std::endl;
        }
        return;
      }

      _pendingProvides.push_back( makeItem (ProvideMessage(provide)) );
      return;
    }
    ERR << "Unsupported request with code: " << code << " received!" << std::endl;
  }

  void ProvideWorker::pushSingleMessage( const RpcMessage &message )
  {
    const auto &handle = [&]( const RpcMessage &message ){
      const auto &msgTypeName = message.messagetypename();
      if ( msgTypeName == rpc::messageTypeName<zypp::proto::ProvideMessage>() ) {
        return parseReceivedMessage( message )
               | mbind( [&]( ProvideMessage &&provide ){
                   _pendingMessages.push_back(provide);
                   _msgAvail->start(0);
                   return expected<void>::success();
                });
      }
      return expected<void>::error( ZYPP_EXCPT_PTR( std::invalid_argument(zypp::str::Str()<<"Unknown message received: " << message.messagetypename())) );
    };

    const auto &exp = handle( message );
    if ( !exp ) {
      try {
        std::rethrow_exception ( exp.error () );
      } catch ( const zypp::Exception &e ) {
        ERR << "Catched exception during message handling: " << e << std::endl;
      } catch ( const std::exception &e ) {
        ERR << "Catched exception during message handling: " << e.what()<< std::endl;
      } catch ( ... ) {
        ERR << "Unknown Exception during message handling" << std::endl;
      }
    }
  }

  expected<ProvideMessage> ProvideWorker::parseReceivedMessage(const RpcMessage &m)
  {
    auto exp = ProvideMessage::create(m);
    if ( !exp )
      invalidMessageReceived( exp.error() );
    return exp;
  }

  zypp::Pathname ProvideWorker::createAttachPoint( const zypp::Pathname &attach_root ) const
  {
    zypp::Pathname apoint;

    if( attach_root.empty() || !attach_root.absolute()) {
      ERR << "Create attach point: invalid attach root: '"
          << attach_root << "'" << std::endl;
      return apoint;
    }

    zypp::PathInfo adir( attach_root );
    if( !adir.isDir() || (geteuid() != 0 && !adir.userMayRWX())) {
      DBG << "Create attach point: attach root is not a writable directory: '"
          << attach_root << "'" << std::endl;
      return apoint;
    }

    static bool cleanup_once( true );
    if ( cleanup_once )
    {
      cleanup_once = false;
      DBG << "Look for orphaned attach points in " << adir << std::endl;
      std::list<std::string> entries;
      zypp::filesystem::readdir( entries, attach_root, false );
      for ( const std::string & entry : entries )
      {
        if ( ! zypp::str::hasPrefix( entry, "AP_0x" ) )
          continue;
        zypp::PathInfo sdir( attach_root + entry );
        if ( sdir.isDir()
             && sdir.dev() == adir.dev()
             && ( zypp::Date::now()-sdir.mtime() > zypp::Date::month ) )
        {
          DBG << "Remove orphaned attach point " << sdir << std::endl;
          zypp::filesystem::recursive_rmdir( sdir.path() );
        }
      }
    }

    zypp::filesystem::TmpDir tmpdir( attach_root, "AP_0x" );
    if ( tmpdir )
    {
      apoint = tmpdir.path().asString();
      if ( ! apoint.empty() )
      {
        tmpdir.autoCleanup( false );	// Take responsibility for cleanup.
      }
      else
      {
        ERR << "Unable to resolve real path for attach point " << tmpdir << std::endl;
      }
    }
    else
    {
      ERR << "Unable to create attach point below " << attach_root << std::endl;
    }
    return apoint;
  }

  void ProvideWorker::removeAttachPoint( const zypp::filesystem::Pathname &attachRoot ) const
  {
    if( !attachRoot.empty() &&
         zypp::PathInfo(attachRoot).isDir() &&
         attachRoot != "/" ) {
      int res = recursive_rmdir( attachRoot );
      if ( res == 0 ) {
        MIL << "Deleted default attach point " << attachRoot << std::endl;
      } else {
        ERR << "Failed to Delete default attach point " << attachRoot
          << " errno(" << res << ")" << std::endl;
      }
    }
  }

  bool ProvideWorker::checkAttached ( const zypp::filesystem::Pathname &mountPoint, const std::function<bool (const zypp::media::MountEntry &)> predicate )
  {
    bool isAttached = false;
    time_t old_mtime = _attach_mtime;
    _attach_mtime = zypp::media::Mount::getMTime();
    if( !(old_mtime <= 0 || _attach_mtime != old_mtime) ) {
      // OK, skip the check (we've seen it at least once)
      isAttached = true;
    } else {
      if( old_mtime > 0)
        DBG << "Mount table changed - rereading it" << std::endl;
      else
        DBG << "Forced check of the mount table" << std::endl;

      for( const auto &entry : zypp::media::Mount::getEntries() ) {

        if ( mountPoint != zypp::Pathname(entry.dir) )
          continue;	// at least the mount points must match
        if ( predicate(entry) ) {
          isAttached = true;
          break;
        }
      }
    }

    // force recheck
    if ( !isAttached )
      _attach_mtime = 0;

    return isAttached;
  }

  const std::function<bool (const zypp::media::MountEntry &)> ProvideWorker::devicePredicate( unsigned int majNr, unsigned int minNr )
  {
    return [ majNr, minNr ]( const zypp::media::MountEntry &entry ) -> bool {
      if( entry.isBlockDevice() ) {
        zypp::PathInfo dev_info( entry.src );
        if ( dev_info.devMajor () == majNr && dev_info.devMinor () == minNr ) {
          DBG << "Found device "
              << majNr << ":" << minNr
              << " in the mount table as " << entry.src << std::endl;
          return true;
        }
      }
      return false;
    };
  }

  const std::function<bool (const zypp::media::MountEntry &)> ProvideWorker::fstypePredicate( const std::string &src, const std::vector<std::string> &fstypes )
  {
    return [ srcdev=src, fst=fstypes ]( const zypp::media::MountEntry &entry ) -> bool {
      if( !entry.isBlockDevice() ) {
        if ( std::find( fst.begin(), fst.end(), entry.type ) != fst.end() ) {
          if ( srcdev == entry.src ) {
            DBG << "Found media mount"
                << " in the mount table as " << entry.src << std::endl;
            return true;
          }
        }
      }
      return false;
    };
  }

  const std::function<bool (const zypp::media::MountEntry &)> ProvideWorker::bindMountPredicate( const std::string &src )
  {
    return [ srcdev=src ]( const zypp::media::MountEntry &entry ) -> bool {
      if( !entry.isBlockDevice() ) {
        if ( srcdev == entry.src ) {
          DBG << "Found bound media "
              << " in the mount table as " << entry.src << std::endl;
          return true;
        }
      }
      return false;
    };
  }
}
