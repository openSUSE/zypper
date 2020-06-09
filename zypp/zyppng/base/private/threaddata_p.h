#ifndef ZYPP_BASE_THREADDATA_P_DEFINED
#define ZYPP_BASE_THREADDATA_P_DEFINED

#include <memory>
#include <thread>

namespace zyppng {

  class EventDispatcher;

  struct ThreadData {
    static ThreadData &current ();

    template<typename T>
    void setName ( T &&name ) {
      threadName = std::forward<T>( name );
      syncNativeName();
    }

    std::string name () const;

    std::shared_ptr<EventDispatcher> ensureDispatcher ();
    void setDispatcher (const std::shared_ptr<EventDispatcher> &disp );


    std::weak_ptr<EventDispatcher> dispatcher;
    std::thread::id threadId;
    std::string threadName;

  private:
    void syncNativeName ();
    ThreadData();

  private:
    std::thread::native_handle_type _nativeHandle;

  };
  ThreadData& threadData ();
}


#endif
