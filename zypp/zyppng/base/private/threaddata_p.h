#ifndef ZYPP_BASE_THREADDATA_P_DEFINED
#define ZYPP_BASE_THREADDATA_P_DEFINED

#include <memory>

namespace zyppng {

  class EventDispatcher;

  struct ThreadData {
    static ThreadData &current ();

    std::shared_ptr<EventDispatcher> ensureDispatcher ();
    void setDispatcher (const std::shared_ptr<EventDispatcher> &disp );


    std::weak_ptr<EventDispatcher> dispatcher;

  private:
    ThreadData();

  };
  ThreadData& threadData ();
}


#endif
