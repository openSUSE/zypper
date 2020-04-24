#include "private/threaddata_p.h"
#include "private/eventdispatcher_glib_p.h"
#include <zypp/base/Logger.h>
#include <ostream> //for std::endl

namespace zyppng {

  ThreadData::ThreadData()
  {

  }

  ThreadData &ThreadData::current()
  {
    static thread_local ThreadData data;
    return data;
  }

  std::shared_ptr<EventDispatcher> ThreadData::ensureDispatcher()
  {
    auto sp = dispatcher.lock();
    if (!sp) {
      dispatcher = sp = EventDispatcherPrivate::create();
    }
    return sp;
  }

  void ThreadData::setDispatcher( const std::shared_ptr<EventDispatcher> &disp )
  {
    if ( dispatcher.lock() ) {
      WAR << "Dispatcher was already created for the current thread" << std::endl;
      return;
    }
    dispatcher = disp;
  }
}
