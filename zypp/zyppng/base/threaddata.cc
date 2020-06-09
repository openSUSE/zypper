#include "private/threaddata_p.h"
#include "private/eventdispatcher_glib_p.h"
#include <zypp/base/Logger.h>
#include <ostream> //for std::endl
#include <sstream>
#include <pthread.h>

namespace zyppng {

  ThreadData::ThreadData()
  : threadId( std::this_thread::get_id() ),
    _nativeHandle( pthread_self() )
  {
  }

  ThreadData &ThreadData::current()
  {
    static thread_local ThreadData data;
    return data;
  }

  std::string ThreadData::name() const
  {
    if ( threadName.empty() ) {
      std::stringstream strStr;
      strStr << threadId;
      return strStr.str();
    }
    return threadName;
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

  void ThreadData::syncNativeName()
  {
    pthread_setname_np( _nativeHandle, threadName.c_str() );
  }
}
