#include "private/asyncqueue_p.h"
#include <glib-unix.h>
#include <ostream>

#include <zypp/base/Logger.h>

namespace zyppng {

  AsyncQueueBase::~AsyncQueueBase()
  { }

  void AsyncQueueBase::addWatch(AsyncQueueWatch &watch)
  {
    std::lock_guard lock(_watchLock);
    _watches.insert( &watch );
  }

  void AsyncQueueBase::removeWatch(AsyncQueueWatch &watch)
  {
    std::lock_guard lock(_watchLock);
    _watches.erase( &watch );
  }

  void AsyncQueueBase::notifyWatches()
  {
    std::lock_guard lock(_watchLock);
    std::for_each( _watches.begin(), _watches.end(), []( AsyncQueueWatch *w ){
      w->postNotifyEvent();
    });
  }

  AsyncQueueWatchPrivate::AsyncQueueWatchPrivate(  std::shared_ptr<AsyncQueueBase> &&q  ) :
    _queue( std::move(q) )
  {
        GError *error = NULL;

        if (!g_unix_open_pipe (fds, FD_CLOEXEC, &error))
          ERR << "Creating pipes for AsyncQueueWatch: " << error->message << std::endl;

        if (!g_unix_set_fd_nonblocking (fds[0], TRUE, &error) ||
             !g_unix_set_fd_nonblocking (fds[1], TRUE, &error))
          ERR << "Set pipes non-blocking for AsyncQueueWatch: "<< error->message << std::endl;
  }

  AsyncQueueWatchPrivate::~AsyncQueueWatchPrivate()
  {
    close (fds[0]);
    close (fds[1]);
  }

  AsyncQueueWatch::AsyncQueueWatch( std::shared_ptr<AsyncQueueBase> queue )
    : AsyncQueueWatch( *( new AsyncQueueWatchPrivate( std::move(queue) ) ) )
  {  }

  AsyncQueueWatch::AsyncQueueWatch(AsyncQueueWatchPrivate &dd)
    : AbstractEventSource( dd )
  {
    Z_D();
    this->updateFdWatch( d->fds[0], AbstractEventSource::Read );
    d->_queue->addWatch( *this );
  }

  AsyncQueueWatch::~AsyncQueueWatch()
  {
    // sync point, since the queue locks all its watches before changing or notifying them
    // we should never run into a bad situation where the AsyncQueueWatch is deleted while notified from a different thread.
    // In case watches are notified this will block and the other way round
    d_func()->_queue->removeWatch( *this );
  }

  void AsyncQueueWatch::postNotifyEvent()
  {
    Z_D();
    int res = -1;
    guint8 one = 1;

    do
      res = write (d->fds[1], &one, sizeof one);
    while (G_UNLIKELY (res == -1 && errno == EINTR));
  }

  SignalProxy<void ()> AsyncQueueWatch::sigMessageAvailable()
  {
    return d_func()->_sigMessageAvailable;
  }

  void AsyncQueueWatch::onFdReady( int , int )
  {
    Z_D();
    char buffer[16];

    /* read until it is empty */
    while (read (d->fds[0], buffer, sizeof buffer) == sizeof buffer);
    d->_sigMessageAvailable.emit();
  }

  void AsyncQueueWatch::onSignal(int)
  {
  }


}

