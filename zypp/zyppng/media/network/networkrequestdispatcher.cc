#include <zypp/zyppng/media/network/private/networkrequestdispatcher_p.h>
#include <zypp/zyppng/media/network/private/networkrequesterror_p.h>
#include <zypp/zyppng/media/network/private/request_p.h>
#include <zypp/zyppng/media/network/private/mediadebug_p.h>
#include <zypp-core/zyppng/base/Timer>
#include <zypp-core/zyppng/base/SocketNotifier>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp/media/CurlHelper.h>
#include <zypp/media/MediaUserAuth.h>
#include <assert.h>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>

using namespace boost;

namespace zypp {
  L_ENV_CONSTR_DEFINE_FUNC(ZYPP_MEDIA_CURL_DEBUG)
}


namespace zyppng {

NetworkRequestDispatcherPrivate::NetworkRequestDispatcherPrivate(  NetworkRequestDispatcher &p  )
    : BasePrivate( p )
    , _timer( Timer::create() )
    , _multi ( curl_multi_init() )
{
  ::internal::globalInitCurlOnce();

  curl_multi_setopt( _multi, CURLMOPT_TIMERFUNCTION, NetworkRequestDispatcherPrivate::multi_timer_cb );
  curl_multi_setopt( _multi, CURLMOPT_TIMERDATA, reinterpret_cast<void *>( this ) );
  curl_multi_setopt( _multi, CURLMOPT_SOCKETFUNCTION, NetworkRequestDispatcherPrivate::static_socket_callback );
  curl_multi_setopt( _multi, CURLMOPT_SOCKETDATA, reinterpret_cast<void *>( this ) );

  _timer->setSingleShot( true );
  _timer->connect( &Timer::sigExpired, *this, &NetworkRequestDispatcherPrivate::multiTimerTimout );
}

NetworkRequestDispatcherPrivate::~NetworkRequestDispatcherPrivate()
{
  cancelAll( NetworkRequestErrorPrivate::customError( NetworkRequestError::Cancelled, "Dispatcher shutdown" ) );
  curl_multi_cleanup( _multi );
}

//called by curl to setup a timer
int NetworkRequestDispatcherPrivate::multi_timer_cb( CURLM *, long timeout_ms, void *thatPtr )
{
  NetworkRequestDispatcherPrivate *that = reinterpret_cast<NetworkRequestDispatcherPrivate *>( thatPtr );
  assert( that != nullptr );

  if ( timeout_ms >= 0 ) {
    that->_timer->start( static_cast<uint64_t>(timeout_ms) );
  } else {
    //cancel the timer
    that->_timer->stop();
  }
  return 0;
}

void NetworkRequestDispatcherPrivate::multiTimerTimout(const Timer &)
{
  handleMultiSocketAction( CURL_SOCKET_TIMEOUT, 0 );
}

int NetworkRequestDispatcherPrivate::static_socket_callback(CURL * easy, curl_socket_t s, int what, void *userp, SocketNotifier *socketp )
{
  NetworkRequestDispatcherPrivate *that = reinterpret_cast<NetworkRequestDispatcherPrivate *>( userp );
  assert( that != nullptr );
  return that->socketCallback( easy, s, what, socketp );
}

int NetworkRequestDispatcherPrivate::socketCallback(CURL *easy, curl_socket_t s, int what, void * )
{
  std::shared_ptr<SocketNotifier> socketp;

  if ( _socketHandler.count( s ) == 0 ) {
    if ( what == CURL_POLL_REMOVE || what == CURL_POLL_NONE )
      return 0;

    socketp = SocketNotifier::create( s, SocketNotifier::Read, false );
    _socketHandler.insert( std::make_pair( s, socketp ) );

    socketp->connect( &SocketNotifier::sigActivated, *this, &NetworkRequestDispatcherPrivate::onSocketActivated );
  } else {
    socketp = _socketHandler[s];
  }

  //should never happen
  if ( !socketp ) {
    if ( what == CURL_POLL_REMOVE || what == CURL_POLL_NONE )
      return 0;

    if ( _socketHandler.count( s ) > 0 )
      _socketHandler.erase( s );

    void *privatePtr = nullptr;
    if ( curl_easy_getinfo( easy, CURLINFO_PRIVATE, &privatePtr ) != CURLE_OK ) {
      privatePtr = nullptr; //make sure this was not filled with bad info
    }

    if ( privatePtr ) {
      NetworkRequestPrivate *request = reinterpret_cast<NetworkRequestPrivate *>( privatePtr );
      //we stop the download, if we can not listen for socket changes we can not correctly do anything
      setFinished( *request->z_func(), NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, "Unable to assign socket listener." ) );
      return 0;
    } else {
      //a broken handle without anything assigned, also should never happen but make sure and clean it up
      WAR_MEDIA << "Cleaning up unassigned  easy handle" << std::endl;
      curl_multi_remove_handle( _multi, easy );
      curl_easy_cleanup( easy );
      return 0;
    }
  }

  //remove the socket
  if ( what == CURL_POLL_REMOVE ) {
    socketp->setEnabled( false );
    _socketHandler.erase( s );
    return 0;
  }

  if ( what == CURL_POLL_IN ) {
    socketp->setMode( SocketNotifier::Read );
  } else if ( what == CURL_POLL_OUT ) {
    socketp->setMode( SocketNotifier::Write );
  } else if ( what == CURL_POLL_INOUT ) {
    socketp->setMode( SocketNotifier::Read | SocketNotifier::Write );
  }

  socketp->setEnabled();
  return 0;
}

void NetworkRequestDispatcherPrivate::onSocketActivated( const SocketNotifier &listener, int events )
{
  int evBitmask = 0;
  if ( (events & SocketNotifier::Read) == SocketNotifier::Read )
    evBitmask |= CURL_CSELECT_IN;
  if ( (events & SocketNotifier::Write) == SocketNotifier::Write )
    evBitmask |= CURL_CSELECT_OUT;
  if ( (events & SocketNotifier::Error) == SocketNotifier::Error )
    evBitmask |= CURL_CSELECT_ERR;

  handleMultiSocketAction( listener.socket(), evBitmask );
}

void NetworkRequestDispatcherPrivate::handleMultiSocketAction(curl_socket_t nativeSocket, int evBitmask)
{
  int running = 0;
  CURLMcode rc = curl_multi_socket_action( _multi, nativeSocket, evBitmask, &running );
  if (rc != 0) {
    //we can not recover from a error like that, cancel all and stop
    NetworkRequestError err = NetworkRequestErrorPrivate::fromCurlMError( rc );
    cancelAll( err );
    //emit error
    _lastError = err;
    _sigError.emit( *z_func() );
    return;
  }

  int msgs_left = 0;
  CURLMsg *msg = nullptr;
  while( (msg = curl_multi_info_read( _multi, &msgs_left )) ) {
    if(msg->msg == CURLMSG_DONE) {
      CURL *easy = msg->easy_handle;
      CURLcode res = msg->data.result;

      void *privatePtr = nullptr;
      if ( curl_easy_getinfo( easy, CURLINFO_PRIVATE, &privatePtr ) != CURLE_OK ) {
        WAR_MEDIA << "Unable to get CURLINFO_PRIVATE" << std::endl;
        continue;
      }

      if ( !privatePtr ) {
        //broken easy handle not associated, should never happen but clean it up
        WAR_MEDIA << "Cleaning up unassigned  easy handle" << std::endl;
        curl_multi_remove_handle( _multi, easy );
        curl_easy_cleanup( easy );
        continue;
      }

      NetworkRequestPrivate *request = reinterpret_cast<NetworkRequestPrivate *>( privatePtr );

      //trigger notification about file downloaded
      NetworkRequestError e = NetworkRequestErrorPrivate::fromCurlError( *request->z_func(), res, request->errorMessage() );
      setFinished( *request->z_func(), e );

      //attention request could be deleted from here on
    }
  }
}

void NetworkRequestDispatcherPrivate::cancelAll( NetworkRequestError result )
{
  //prevent dequeuePending from filling up the runningDownloads again
  _locked = true;

  while ( _runningDownloads.size() ) {
    std::shared_ptr<NetworkRequest> &req = _runningDownloads.back();
    setFinished(*req, result );
  }
  while ( _pendingDownloads.size() ) {
    std::shared_ptr<NetworkRequest> &req = _pendingDownloads.back();
    setFinished(*req, result );
  }

  _locked = false;
}

void NetworkRequestDispatcherPrivate::setFinished( NetworkRequest &req, NetworkRequestError result )
{
  auto delReq = []( auto &list, NetworkRequest &req ) -> std::shared_ptr<NetworkRequest> {
    auto it = std::find_if( list.begin(), list.end(), [ &req ]( const std::shared_ptr<NetworkRequest> &r ) {
      return req.d_func() == r->d_func();
    } );
    if ( it != list.end() ) {
      auto ptr = *it;
      list.erase( it );
      return ptr;
    }
    return nullptr;
  };

  // We have a tricky situation if a network request is called when inside a callback, in those cases its
  // not allowed to call curl_multi_remove_handle, we need to tell the callback to fail, so the download
  // is cancelled by curl itself. We also need to store the current result for later
  auto rmode = std::get_if<NetworkRequestPrivate::running_t>( &req.d_func()->_runningMode );
  if ( rmode ) {
    if ( rmode->_isInCallback ) {
      // the first cached result wins)
      if  ( !rmode->_cachedResult )
        rmode->_cachedResult = result;
      return;
    } else if ( rmode->_cachedResult ) {
      result = rmode->_cachedResult.value();
    }
  }

  auto rLocked = delReq( _runningDownloads, req );
  if ( !rLocked )
    rLocked = delReq( _pendingDownloads, req );

  void *easyHandle = req.d_func()->_easyHandle;
  if ( easyHandle )
    curl_multi_remove_handle( _multi, easyHandle );

  req.d_func()->_dispatcher = nullptr;

  //first set the result, the Request might have a checksum to check as well so a currently
  //successful request could fail later on
  req.d_func()->setResult( std::move(result) );
  _sigDownloadFinished.emit( *z_func(), req );

  //we got a open slot, try to dequeue or send the finished signals if all queues are empty
  dequeuePending();
}

void NetworkRequestDispatcherPrivate::dequeuePending()
{
  if ( !_isRunning || _locked )
    return;

  while ( _maxConnections == -1 || ( (std::size_t)_maxConnections > _runningDownloads.size() ) ) {
    if ( !_pendingDownloads.size() )
      break;

    std::shared_ptr<NetworkRequest> req = std::move( _pendingDownloads.front() );
    _pendingDownloads.pop_front();

    std::string errBuf = "Failed to initialize easy handle";
    if ( !req->d_func()->initialize( errBuf ) ) {
      //@TODO store the CURL error in the errors extra info
      setFinished( *req, NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, std::move(errBuf) ) );
      continue;
    }

    CURLMcode rc = curl_multi_add_handle( _multi, req->d_func()->_easyHandle );
    if ( rc != 0 ) {
      setFinished( *req, NetworkRequestErrorPrivate::fromCurlMError( rc ) );
      continue;
    }

    req->d_func()->aboutToStart();
    _sigDownloadStarted.emit( *z_func(), *req );

    _runningDownloads.push_back( std::move(req) );
  }

  //check for empty queues
  if ( _pendingDownloads.size() == 0 && _runningDownloads.size() == 0 ) {
    //once we finished all requests, cancel the timer too, so curl is not called without requests
    _timer->stop();
    _sigQueueFinished.emit( *z_func() );
  }
}

NetworkRequestDispatcher::NetworkRequestDispatcher( )
  : Base( * new NetworkRequestDispatcherPrivate ( *this ) )
{

}

bool NetworkRequestDispatcher::supportsProtocol( const Url &url )
{
  curl_version_info_data *curl_info = nullptr;
  curl_info = curl_version_info(CURLVERSION_NOW);
  // curl_info does not need any free (is static)
  if (curl_info->protocols)
  {
    const char * const *proto;
    std::string        scheme( url.getScheme() );
    bool               found = false;
    for(proto=curl_info->protocols; !found && *proto; ++proto) {
      if( scheme == std::string((const char *)*proto))
        found = true;
    }
    return found;
  }
  return true;
}

void NetworkRequestDispatcher::setMaximumConcurrentConnections( const int maxConn )
{
  d_func()->_maxConnections = maxConn;
}

void NetworkRequestDispatcher::enqueue(const std::shared_ptr<NetworkRequest> &req )
{
  if ( !req )
    return;
  Z_D();

  if ( std::find( d->_runningDownloads.begin(), d->_runningDownloads.end(), req ) != d->_runningDownloads.end() )  {
    WAR_MEDIA << "Ignoring request to enqueue download " << req->url().asString() << " request is already running " << std::endl;
    return;
  }

  if ( std::find( d->_pendingDownloads.begin(), d->_pendingDownloads.end(), req ) != d->_pendingDownloads.end() ) {
    WAR_MEDIA << "Ignoring request to enqueue download " << req->url().asString() << " request is already enqueued " << std::endl;
    return;
  }

  req->d_func()->_dispatcher = this;
  if ( req->priority() == NetworkRequest::Normal )
    d->_pendingDownloads.push_back( req );
  else {
    auto it = std::find_if( d->_pendingDownloads.begin(), d->_pendingDownloads.end(), [ prio = req->priority() ]( const auto &pendingReq ){
      return pendingReq->priority() < prio;
    });

    //if we have a valid iterator, decrement we found a pending download request with lower prio, insert before that
    if ( it != d->_pendingDownloads.end() && it != d->_pendingDownloads.begin() )
      it--;
    d->_pendingDownloads.insert( it, req );
  }

  //dequeue if running and we have capacity
  d->dequeuePending();
}

void NetworkRequestDispatcher::cancel( NetworkRequest &req, std::string reason )
{
  cancel( req, NetworkRequestErrorPrivate::customError( NetworkRequestError::Cancelled, reason.size() ? std::move(reason) : "Request explicitely cancelled" ) );
}

void NetworkRequestDispatcher::cancel(NetworkRequest &req, const NetworkRequestError &err)
{
  Z_D();

  if ( req.d_func()->_dispatcher != this ) {
    //TODO throw exception
    return;
  }

  d->setFinished( req, err );
}

void NetworkRequestDispatcher::run()
{
  Z_D();
  d->_isRunning = true;

  if ( d->_pendingDownloads.size() )
    d->dequeuePending();
}

void NetworkRequestDispatcher::reschedule()
{
  Z_D();
  if ( !d->_pendingDownloads.size() )
    return;

  std::stable_sort( d->_pendingDownloads.begin(), d->_pendingDownloads.end(), []( const auto &a, const auto &b ){
    return a->priority() < b->priority();
  });

  d->dequeuePending();
}

size_t NetworkRequestDispatcher::count()
{
  Z_D();
  return d->_pendingDownloads.size() + d->_runningDownloads.size();
}

const zyppng::NetworkRequestError &NetworkRequestDispatcher::lastError() const
{
  return d_func()->_lastError;
}

SignalProxy<void (NetworkRequestDispatcher &, NetworkRequest &)> NetworkRequestDispatcher::sigDownloadStarted()
{
  return d_func()->_sigDownloadStarted;
}

SignalProxy<void (NetworkRequestDispatcher &, NetworkRequest &)> NetworkRequestDispatcher::sigDownloadFinished()
{
  return d_func()->_sigDownloadFinished;
}

SignalProxy<void ( NetworkRequestDispatcher &)> NetworkRequestDispatcher::sigQueueFinished()
{
  return d_func()->_sigQueueFinished;
}

SignalProxy<void ( NetworkRequestDispatcher &)> NetworkRequestDispatcher::sigError()
{
  return d_func()->_sigError;
}

}
