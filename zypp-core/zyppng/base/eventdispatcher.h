/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPP_BASE_EVENTDISPATCHER_DEFINED
#define ZYPP_BASE_EVENTDISPATCHER_DEFINED

#include <zypp-core/zyppng/base/zyppglobal.h>
#include <zypp-core/zyppng/base/Signals>
#include <zypp-core/zyppng/base/Base>
#include <zypp-core/zyppng/base/AbstractEventSource>
#include <memory>
#include <functional>

namespace zyppng {

class SocketNotifier;
class Timer;
class EventDispatcher;


class EventDispatcherPrivate;

/*!
 * The EventDispatcher class implements the libzypp event loop native backend.
 *
 * Libzypp is using a thread local dispatcher, means each thread has its own unique disptacher. Only special case is when
 * we need to work together with a already exisiting event loop, for example in a Qt application. The default implementation however
 * uses the glib eventloop, just like Qt and GTK, so integrating libzypp here is just a matter of passing the default main context
 * to the constructor of \ref EventDispatcher.
 */
class LIBZYPP_NG_EXPORT EventDispatcher : public Base
{
  ZYPP_DECLARE_PRIVATE(EventDispatcher)
  friend class AbstractEventSource;
  friend class Timer;

public:

  using Ptr = std::shared_ptr<EventDispatcher>;
  using WeakPtr = std::shared_ptr<EventDispatcher>;
  using IdleFunction = std::function<bool ()>;


  virtual ~EventDispatcher();

  /*!
   * Enters the eventloop once and dequeues all pending events, once the event queue is empty the
   * function returns
   */
  virtual bool run_once();

  /*!
   * \brief Convenience function to schedule a callback to be called later.
   * \param callback a std::function that is called after all other events have been processed
   */
  template< typename T = IdleFunction >
  static void invokeOnIdle ( T &&callback )
  {
    auto ev = instance();
    if ( ev )
      ev->invokeOnIdleImpl( std::forward<T>(callback) );
  }

  /*!
   * Schedules a \sa std::shared_ptr to be unreferenced in the next idle phase of the
   * event loop.
   *
   * In some cases it might be required to delay the cleanup of a ressource until the current
   * event loop iteration was finished, in case there are more pending events for the ressource.
   *
   * \note Normally this should be handled by using correct shared_ptr semantics, and always owning a reference
   *       to a object were functions are called on. Only if that is not possible unrefLater should be used.
   */
  template< typename T >
  static void unrefLater ( T &&ptr ) {
    auto ev = instance();
    if ( ev )
      ev->unrefLaterImpl( std::static_pointer_cast<void>( std::forward<T>(ptr) ) );
  }

  /*!
   * Immediately clears the list of all shared_ptr's that were registered
   * to be unreferenced later. Mainly used to be called when a event loop exits, otherwise we
   * might have weird side effects when the main loop instance preserves pointers that should be cleared
   * at that point already. Like child items still trying to access the parent object that was already cleaned
   * up before the MainLoop instance.
   */
  void clearUnrefLaterList ();

  /*!
   * Returns the number of the currently active timers
   */
  ulong runningTimers() const;

  /*!
   * Returns the EventDispatcher instance for the current thread.
   */
  static std::shared_ptr<EventDispatcher> instance();

  /*!
   * Registers the given event dispatcher as the default for the current thread. The reference count for the
   * shared pointer will not be increased, so the application is responsible to keep it until the application exits.
   * \note This must be called before the default dispatcher is registered, otherwise its ignored.
   */
  static void setThreadDispatcher ( const std::shared_ptr<EventDispatcher> &disp );

  /**
   * Returns the native dispatcher handle if the used implementation supports it
   * \note the glib backend will return the used glib \a GMainContext
   */
  void *nativeDispatcherHandle () const;

  /*!
   * Waits until one of the requested events in \a events happens on the file descriptor.
   * Use \ref AbstractEventSource::EventTypes to define for which events should be polled.
   * Returns true on success, \a revents will contain the bitwise combination of \ref AbstractEventSource::EventTypes
   * that triggered the wakeup.
   */
  static bool waitForFdEvent ( const int fd, int events, int &revents, int &timeout );

  using WaitPidCallback = std::function<void(int, int)>;

  /*!
   * Tracks a child process until its execution did end.
   * Callback is called when the child exits
   */
  void trackChildProcess ( int pid, std::function<void(int, int)> callback  );

protected:

  /*!
   * Create a new instance of the EventDispatcher, if \a ctx is given it is used as the new
   * context for the eventloop
   */
  EventDispatcher( void *ctx = nullptr );

  /*!
   * \see unrefLater
   */
  void unrefLaterImpl ( std::shared_ptr<void> &&ptr );

  /*!
   * \see invokeOnIdle
   */
  void invokeOnIdleImpl ( IdleFunction &&callback );

  /*!
   * Updates or registeres a event source in the event loop
   * \param notifier The event source implementation that should receive the event notification
   * \param fd The file descriptor that is added to the internal watchlist
   * \param mode The watch mode for the given file desriptor \sa zyppng::AbstractEventSource::EventTypes
   */
  virtual void updateEventSource ( AbstractEventSource &notifier, int fd, int mode );

  /*!
   * Removes a file descriptor from the internal watchlist, if \a fd is set to -1 all file descriptors
   * associated with the \sa AbstractEventSource are removed
   * \param notifier The \sa AbstractEventSource parent of the file descriptor
   * \param fd The file descriptor to be removed, set to -1 to remove all descriptors for a \sa AbstractEventSource
   */
  virtual void removeEventSource   ( AbstractEventSource &notifier, int fd = -1 );

  /*!
   * Adds a new Timer instance to the internal timer list
   */
  virtual void registerTimer ( Timer &timer );

  /*!
   * Removes a timer from the internal timer list, once a Timer is removed it does not fire anymore
   */
  virtual void removeTimer ( Timer &timer );

};

}

#endif
