#include "abstracteventsource.h"
#include "private/abstracteventsource_p.h"
#include "eventdispatcher.h"
#include "private/base_p.h"

#include <zypp/base/Exception.h>

namespace zyppng {

AbstractEventSourcePrivate::AbstractEventSourcePrivate( AbstractEventSource &p ) : BasePrivate( p )
{
  auto ev = EventDispatcher::instance();
  if ( !ev )
    ZYPP_THROW( zypp::Exception( "Creating event sources without a EventDispatcher instance is not supported" ) );
  _ev = ev;
}

AbstractEventSource::AbstractEventSource()
  : Base ( * new AbstractEventSourcePrivate( *this ) )
{ }

AbstractEventSource::AbstractEventSource( AbstractEventSourcePrivate &dd )
  : Base ( dd )
{ }

AbstractEventSource::~AbstractEventSource()
{
  Z_D();
  auto ev = d->_ev.lock();
  //if ev is null , eventloop is shutting down
  if ( ev )
    ev->removeEventSource( *this );
}

std::weak_ptr<EventDispatcher> AbstractEventSource::eventDispatcher() const
{
  return d_func()->_ev;
}

void AbstractEventSource::updateFdWatch(int fd, int mode)
{
  auto ev = d_func()->_ev.lock();

  //if ev is null we are shutting down
  if ( !ev )
    return;
  ev->updateEventSource( *this, fd, mode );
}

void AbstractEventSource::removeFdWatch(int fd)
{
  auto ev = d_func()->_ev.lock();

  //if ev is null we are shutting down
  if ( !ev )
    return;
  ev->removeEventSource( *this , fd );
}

}
