/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#ifndef ZYPP_NG_THREAD_ASYNCQUEUE_H_INCLUDED
#define ZYPP_NG_THREAD_ASYNCQUEUE_H_INCLUDED

#include <zypp/zyppng/base/AbstractEventSource>

#include <queue>
#include <set>
#include <mutex>
#include <memory>
#include <optional>
#include <condition_variable>

namespace zyppng {

  class AsyncQueueWatch;

  class AsyncQueueBase {
    public:
      virtual ~AsyncQueueBase();

      void addWatch ( AsyncQueueWatch &watch );
      void removeWatch (  AsyncQueueWatch &watch );
      void notifyWatches ( );

    private:
      std::set<AsyncQueueWatch *> _watches;
      std::recursive_mutex _watchLock;
  };

  /*!
   * AsyncQueue provides a thread safe way to send messages between threads.
   * Using this class makes only sense for multithreaded applications.
   */
  template< class Message >
  class AsyncQueue : public AsyncQueueBase {

  public:

    using Ptr = std::shared_ptr<AsyncQueue>;

    static Ptr create () {
      return Ptr( new AsyncQueue() );
    }

    AsyncQueue(const AsyncQueue&) = delete;
    AsyncQueue& operator=(const AsyncQueue&) = delete;

    /*!
     * Pushes a new message into the queue, make sure to manually
     * acqiure the lock before calling this function and \ref notify after
     * unlocking it again. This function should be used when multiple messages should be pushed
     * into the queue before notifying the threads.
     * \sa push
     */
    template< typename T = Message >
    void pushUnlocked ( T &&value ) {
      _messages.push( std::forward<T>(value) );
    }

    /*!
     * Pushes a new message into the queue and notify all waiting
     * threads that there is new data available.
     */
    template< typename T = Message >
    void push ( T &&value ) {
      {
        std::lock_guard lk( _mut );
        pushUnlocked( std::forward<T>(value) );
      }
      notify();
    }

    /*!
     * Pops the first message from the queue, blocking until there is data available.
     */
    Message pop () {
      std::unique_lock<std::mutex> lk( _mut );
      _cv.wait( lk, [this](){ return _messages.size() > 0; } );
      Message msg = std::move( _messages.front() );
      _messages.pop();
      return msg;
    }

    /*!
     * Tries to pop the first element from the queue, if no data is available returns a empty
     * \ref std::optional instead of blocking.
     */
    std::optional<Message> tryPop () {
      std::lock_guard lk( _mut );
      return tryPopUnlocked();
    }

    /*!
     * Same as \ref tryPop but does not lock the mutex, make sure to acquire the lock before
     * calling this function.
     */
    std::optional<Message> tryPopUnlocked () {
      if ( _messages.size() ) {
        Message msg = std::move( _messages.front() );
        _messages.pop();
        return msg;
      }
      return {};
    }

    /*!
     * Locks the internal mutex, use \ref std::lock_guard instead of manually
     * calling lock and unlock.
     */
    void lock   () {
      _mut.lock();
    }

    /*!
     * Unlocks the internal mutex, use \ref std::lock_guard instead of manually
     * calling lock and unlock.
     */
    void unlock () {
      _mut.unlock();
    }

    /*!
     * Notifies all waiting threads and registered \ref AsyncQueueWatch instances that
     * new data is available.
     */
    void notify () {
      _cv.notify_all();
      notifyWatches();
    }

  private:
    AsyncQueue() = default;
    std::queue<Message> _messages;
    std::mutex _mut;
    std::condition_variable _cv;
  };

  class AsyncQueueWatchPrivate;
  class LIBZYPP_NG_EXPORT AsyncQueueWatch : public AbstractEventSource
  {
    ZYPP_DECLARE_PRIVATE(AsyncQueueWatch)
  public:

    static std::shared_ptr<AsyncQueueWatch> create ( std::shared_ptr<AsyncQueueBase> queue );
    virtual ~AsyncQueueWatch();

    void postNotifyEvent ();

    SignalProxy<void()> sigMessageAvailable();

    // AbstractEventSource interface
    void onFdReady(int fd, int events) override;
    void onSignal(int signal) override;

  protected:
    AsyncQueueWatch( std::shared_ptr<AsyncQueueBase> &&queue );
    AsyncQueueWatch( AsyncQueueWatchPrivate &dd );
  };

}


#endif
