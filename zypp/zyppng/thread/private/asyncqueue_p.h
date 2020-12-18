#ifndef ZYPP_NG_THREAD_PRIVATE_ASYNCQUEUE_P_H
#define ZYPP_NG_THREAD_PRIVATE_ASYNCQUEUE_P_H

#include <zypp/zyppng/base/private/abstracteventsource_p.h>
#include <zypp/zyppng/thread/asyncqueue.h>
#include <zypp/zyppng/base/Signals>

#include <glib.h>

namespace zyppng {

  class AsyncQueueWatchPrivate : public AbstractEventSourcePrivate
  {
    ZYPP_DECLARE_PUBLIC(AsyncQueueWatch)
    public:
      AsyncQueueWatchPrivate( std::shared_ptr<AsyncQueueBase> &&q, AsyncQueueWatch &p );
      virtual ~AsyncQueueWatchPrivate();

      std::shared_ptr<AsyncQueueBase> _queue;
      gint fds[2] = { -1, -1 };
      MemSignal<AsyncQueueWatch, void()> _sigMessageAvailable;
  };

}

#endif // ZYPP_NG_THREAD_PRIVATE_ASYNCQUEUE_P_H
