#ifndef ZYPP_BASE_THREADDATA_P_DEFINED
#define ZYPP_BASE_THREADDATA_P_DEFINED

#include <memory>
#include <thread>
#include <string>

namespace zyppng
{
  class EventDispatcher;

  struct ThreadData
  {
    static ThreadData &current();

    template<typename T>
    void setName( T &&name ) {
      _threadName = std::forward<T>( name );
      syncNativeName();
    }

    const std::string &name() const;

    std::shared_ptr<EventDispatcher> dispatcher()
    { return _dispatcher.lock(); }
    std::shared_ptr<EventDispatcher> ensureDispatcher();
    void setDispatcher( const std::shared_ptr<EventDispatcher> &disp );


  private:
    void syncNativeName();
    ThreadData();

  private:
    std::thread::id _threadId;
    mutable std::string _threadName;	///< lazy initialized to _threadId if unset
    std::thread::native_handle_type _nativeHandle;
    std::weak_ptr<EventDispatcher> _dispatcher;
  };

  ThreadData& threadData();
}


#endif
