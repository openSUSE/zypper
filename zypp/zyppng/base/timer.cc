#include "timer.h"
#include "private/base_p.h"

#include "eventdispatcher.h"

#include <time.h>
#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>
#include <iostream>
#include <glib.h>

namespace zyppng {

using namespace zypp;


class TimerPrivate : BasePrivate
{
  ZYPP_DECLARE_PUBLIC(Timer)
public:
  TimerPrivate( Timer &p );
  virtual ~TimerPrivate();

  uint64_t _beginMs = 0;
  uint64_t _requestedTimeout = 0;
  std::weak_ptr<EventDispatcher> _ev;
  bool _isRunning = false;

  MemSignal<Timer, void (Timer &t)> _expired;

  bool _singleShot = false;

};

TimerPrivate::TimerPrivate(Timer &p) : BasePrivate(p), _expired(p)
{
  auto ev = EventDispatcher::instance();
  if ( !ev )
    ZYPP_THROW( zypp::Exception( "Creating timers without a EventDispatcher instance is not supported" ) );
  _ev = ev;
}

TimerPrivate::~TimerPrivate()
{}

std::shared_ptr<Timer> Timer::create()
{
  return std::shared_ptr<Timer>( new Timer() );
}

Timer::~Timer()
{ stop(); }

void Timer::setSingleShot(bool singleShot)
{
  d_func()->_singleShot = singleShot;
}

bool Timer::singleShot() const
{
  return d_func()->_singleShot;
}

uint64_t Timer::now()
{
  return static_cast<uint64_t>( g_get_monotonic_time () / 1000 );
#if 0
  timespec now{0 ,0};
  if ( clock_gettime( CLOCK_MONOTONIC_RAW, &now ) ) {
    WAR << "Unable to get current monotonic time, timers will not work" << std::endl;
    return 1;
  }
  return ( uint(now.tv_sec) * 1000 ) + uint( now.tv_nsec * 1e-6 );
#endif
}

uint64_t Timer::elapsedSince( const uint64_t start )
{
  uint64_t nowMs = now();
  return ( nowMs - start );
}

uint64_t Timer::started() const
{
  return d_func()->_beginMs;
}

uint64_t Timer::interval() const
{
  return d_func()->_requestedTimeout;
}

uint64_t Timer::remaining() const
{
  Z_D();

  uint64_t elapsed = this->elapsed();
  if ( elapsed >= d->_requestedTimeout )
    return 0;
  return ( d->_requestedTimeout - elapsed );
}

uint64_t Timer::elapsed() const
{
  Z_D();
  return elapsedSince( d->_beginMs );
}

uint64_t Timer::expires() const
{
  return d_func()->_beginMs + d_func()->_requestedTimeout;
}

SignalProxy<void (Timer &t)> Timer::sigExpired()
{
  return d_func()->_expired;
}

uint64_t Timer::expire()
{
  Z_D();
  //@FIXME, we should not rely on this, maybe a "deleteLater" feature
  //in the MainLoop?
  //make sure timer is not deleted during signal emission
  auto lock = shared_from_this();

  auto exp = remaining();
  if ( exp == 0 ) {
    if ( d->_singleShot )
      stop();
    else
      d->_beginMs = now();
    d->_expired.emit( *this );
  }
  return exp;
}

bool Timer::isRunning() const
{
  return d_func()->_isRunning;
}

void Timer::start()
{
  start ( d_func()->_requestedTimeout );
}

void Timer::start( uint64_t timeout )
{
  Z_D();

  d->_requestedTimeout = timeout;
  d->_beginMs = now();

  if ( !d->_isRunning ) {
    auto ev = d->_ev.lock();
    //if ev is null we are shutting down
    if ( !ev )
      return;
    ev->registerTimer( *this );

    d->_isRunning = true;
  }

}

void Timer::stop()
{
  Z_D();

  if ( !d->_isRunning )
    return;

  auto ev = d->_ev.lock();

  //event loop might be shutting down
  if ( ev )
    ev->removeTimer( *this );

  d->_isRunning = false;
}

Timer::Timer() : Base ( *new TimerPrivate( *this ) )
{ }

}
