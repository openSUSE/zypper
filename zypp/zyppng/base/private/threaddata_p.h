#ifndef ZYPP_BASE_THREADDATA_P_DEFINED
#define ZYPP_BASE_THREADDATA_P_DEFINED

#include <memory>
#include <thread>

namespace zyppng {

  class EventDispatcher;

  struct ThreadData {
    static ThreadData &current ();

    std::shared_ptr<EventDispatcher> ensureDispatcher ();
    void setDispatcher (const std::shared_ptr<EventDispatcher> &disp );


    std::weak_ptr<EventDispatcher> dispatcher;
    std::thread::id threadId;

  private:
    ThreadData();

  };
  ThreadData& threadData ();
}


#endif
