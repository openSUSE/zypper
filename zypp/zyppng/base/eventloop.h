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
#ifndef ZYPP_BASE_EVENTLOOP_DEFINED
#define ZYPP_BASE_EVENTLOOP_DEFINED

#include <zypp/zyppng/base/zyppglobal.h>
#include <zypp/zyppng/base/Signals>
#include <zypp/zyppng/base/Base>

// CONTINUE WITH THREAD DATA AND PUT THE DISPATCHER INSTANCE THERE!

namespace zyppng {

  class EventDispatcher;
  class EventLoopPrivate;

  /*!
   * The EventDispatcher class implements the libzypp event loop.
   *
   * A event loop is used to execute multiple tasks concurrently. This is not implemented using threads but usually by either
   * using Timers to poll a ressource or by reacting on events from a file descriptor or socket.
   * In a application like zypper where we make use of I/O heavy tasks like downloading packages, rebuilding the repo metadata
   * or generating a checksum over a file the application needs to wait more for those tasks to finish than actually doing anything.
   * By using a event loop we can start one of those tasks and let the OS handle the execution and subscribe to certain events that can happen,
   * utilizing more of the CPU compared to starting all the tasks serially.
   *
   */
  class LIBZYPP_NG_EXPORT EventLoop : public Base
  {
    ZYPP_DECLARE_PRIVATE(EventLoop)

  public:
    using Ptr = std::shared_ptr<EventLoop>;
    using WeakPtr = std::shared_ptr<EventLoop>;

    static Ptr create ();
    virtual ~EventLoop();

    /*!
     * Start dispatching events, this function will block until \sa quit was called for the EventLoop instance
     */
    void run ();

    /*!
     * Stop dispatching events and return control from the run function
     */
    void quit ();


    /*!
     * Returns the event dispatcher used by the MainLoop instance
     */
    std::shared_ptr<EventDispatcher> eventDispatcher () const;

  private:
    EventLoop();

  };

}

#endif
