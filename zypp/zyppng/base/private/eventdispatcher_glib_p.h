#ifndef ZYPP_BASE_EVENTDISPATCHER_GLIB_P_DEFINED
#define ZYPP_BASE_EVENTDISPATCHER_GLIB_P_DEFINED

#include "base_p.h"
#include <zypp/zyppng/base/eventdispatcher.h>
#include <glib.h>
#include <thread>
#include <unordered_map>
#include <queue>

namespace zyppng {

struct GUnixPollFD
{
  GIOCondition reqEvents;
  int pollfd   = -1;
  gpointer tag = nullptr;
};

/*!
 * \internal GSource for all AbstractEventSources
 * \attention this struct is initialized with malloc, make sure
 *            to manually call all constructurs and destructors in the
 *            \a create and \a destruct functions
 */
struct GAbstractEventSource
{
  GSource source;
  EventDispatcherPrivate *_ev;
  AbstractEventSource *eventSource;
  std::vector<GUnixPollFD> pollfds;

  static gboolean prepare(GSource *, gint *timeout);
  static gboolean check(GSource *source);
  static gboolean dispatch(GSource *source, GSourceFunc, gpointer);

  static GAbstractEventSource *create (EventDispatcherPrivate *ev);
  static void destruct ( GAbstractEventSource *src );
};

/*!
 * \internal GSource for all Timers
 * \attention this struct is initialized with malloc, make sure
 *            to manually call all constructurs and destructors in the
 *            \a create and \a destruct functions
 */
struct GLibTimerSource
{
  GSource source;
  Timer   *_t = nullptr;

  static gboolean prepare(GSource *src, gint *timeout);
  static gboolean check(GSource *source);
  static gboolean dispatch(GSource *source, GSourceFunc, gpointer);

  static GLibTimerSource *create ();
  static void destruct ( GLibTimerSource *src );
};

class EventDispatcherPrivate : public BasePrivate
{
  ZYPP_DECLARE_PUBLIC(EventDispatcher)
public:
  EventDispatcherPrivate( GMainContext *ctx, EventDispatcher &p );
  virtual ~EventDispatcherPrivate();

  bool runIdleTasks();
  void enableIdleSource ();

  static std::shared_ptr<EventDispatcher> create ( );

  std::thread::id _myThreadId;
  GMainContext *_ctx = nullptr;

  GSource *_idleSource  = nullptr;

  std::vector<GLibTimerSource *> _runningTimers;
  std::vector<GAbstractEventSource *> _eventSources;
  std::vector< std::shared_ptr<void> > _unrefLater;
  std::queue< EventDispatcher::IdleFunction > _idleFuncs;
};

}


#endif
