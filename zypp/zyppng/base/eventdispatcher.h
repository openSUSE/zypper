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

#include <zypp/zyppng/base/zyppglobal.h>
#include <zypp/zyppng/base/Signals>
#include <zypp/zyppng/base/Base>
#include <zypp/zyppng/base/AbstractEventSource>
#include <memory>
#include <functional>

namespace zyppng {

class SocketNotifier;
class Timer;
class EventDispatcher;


class EventDispatcherPrivate;

/*!
 * The EventDispatcher class implements the libzypp event loop.
 *
 * A event loop is used to execute multiple tasks concurrently. This is not implemented using threads but usually by either
 * using Timers to poll a ressource or by reacting on events from a file descriptor or socket.
 * In a application like zypper where we heavily make use of I/O heavy tasks like downloading packages, rebuilding the repo metadata
 * or generating a checksum over a file the application needs to wait more for those tasks to finish than actually doing anything.
 * By using a event loop we can start one of those tasks and let the OS handle the execution and subscribe to certain events that can happen,
 * utilizing more of the CPU compared to starting all the tasks serially.
 *
 * Libzypp is using a thread local eventloop, means each thread needs to start its own loop. Only special case is when
 * we need to work together with a already exisiting event loop, for example in a Qt application. The default implementation however
 * uses the glib eventloop, just like Qt and GTK, so integrating libzypp here is just a matter of passing the default main context
 * to the constructor of \a EventDispatcher.
 *
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

  /*!
   * Creates a new EventDispatcher, use this function to create a Dispatcher
   * running on the default thread
   *
   * \note the glib implementation will use the g_default_context(), this means
   * it will attach to any running main loop
   */
  static std::shared_ptr<EventDispatcher> createMain ( );

  /*!
   * Creates a new EventDispatcher, use this function to create a Dispatcher
   * running on a threads aside the main thread
   *
   * \note the glib implementation will use the g_main_context_get_thread_default(), this means
   * it will attach to any loop that was set as the default for the current thread, if there is no
   * default context a new one will be created
   */
  static std::shared_ptr<EventDispatcher> createForThread ( );

  virtual ~EventDispatcher();

  /*!
   * Enters the eventloop once and dequeues all pending events, once the event queue is empty the
   * function returns
   */
  virtual bool run_once();

  /*!
   * Start dispatching events, this function will block until \sa quit was called for the EventDispatcher instance
   */
  virtual void run ();

  /*!
   * Stop dispatching events and return from the main loop.
   */
  virtual void quit ();

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
   */
  template< typename T >
  static void unrefLater ( T &&ptr ) {
    auto ev = instance();
    if ( ev )
      ev->unrefLaterImpl( std::static_pointer_cast<void>( std::forward<T>(ptr) ) );
  }

  /*!
   * Returns the number of the currently active timers
   */
  ulong runningTimers() const;

  /*!
   * Returns the EventDispatcher instance for the current thread.
   */
  static std::shared_ptr<EventDispatcher> instance();

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
  virtual void updateEventSource ( AbstractEventSource *notifier, int fd, int mode );

  /*!
   * Removes a file descriptor from the internal watchlist, if \a fd is set to -1 all file descriptors
   * associated with the \sa AbstractEventSource are removed
   * \param notifier The \sa AbstractEventSource parent of the file descriptor
   * \param fd The file descriptor to be removed, set to -1 to remove all descriptors for a \sa AbstractEventSource
   */
  virtual void removeEventSource   ( AbstractEventSource *notifier, int fd = -1 );

  /*!
   * Adds a new Timer instance to the internal timer list
   */
  virtual void registerTimer ( Timer *timer );

  /*!
   * Removes a timer from the internal timer list, once a Timer is removed it does not fire anymore
   */
  virtual void removeTimer ( Timer *timer );
};

}

#endif
