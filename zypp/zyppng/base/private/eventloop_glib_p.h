#ifndef ZYPP_BASE_EVENTLOOP_GLIB_P_DEFINED
#define ZYPP_BASE_EVENTLOOP_GLIB_P_DEFINED

#include "base_p.h"
#include "threaddata_p.h"
#include <zypp/zyppng/base/eventloop.h>
#include <glib.h>

namespace zyppng {

  class EventLoopPrivate : public BasePrivate
  {
    ZYPP_DECLARE_PUBLIC(EventLoop)
  public:
    EventLoopPrivate ( EventLoop &p );

    std::shared_ptr<EventDispatcher> _dispatcher;
    GMainLoop *_loop = nullptr;

  };

}


#endif
