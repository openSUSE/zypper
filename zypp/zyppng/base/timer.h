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
#ifndef ZYPP_NG_BASE_TIMER_H_INCLUDED
#define ZYPP_NG_BASE_TIMER_H_INCLUDED

#include <zypp/zyppng/base/zyppglobal.h>
#include <zypp/zyppng/base/Base>
#include <zypp/zyppng/base/Signals>
#include <functional>

namespace zyppng {
class TimerPrivate;
class EventDispatcher;

/*!
 * \brief The Timer class provides repetitive and single-shot timers.
 *
 * Provides a high level interface for timers. To use it, create a Timer and
 * connect a slot to its \sa sigExpired signal.
 *
 * \code
 * zyppng::Timer::Ptr t1 = zyppng::Timer::create();
 * t1->sigExpired().connect( sigc::mem_fun(this, &HandlerClass::timeout) );
 * t1->start(1000);
 * \endcode
 *
 * The timeout slot will now be called every second.
 *
 * \note The accuracy of the timer should be around 1ms , but also depends on the underlying hardware.
 */
class Timer : public Base
{
  ZYPP_DECLARE_PRIVATE(Timer)
  friend class EventDispatcher;

public:

  using Ptr = std::shared_ptr<Timer>;
  using WeakPtr = std::shared_ptr<Timer>;

  /*!
   * \brief Creates a new Timer object, the timer is not started at this point
   */
  static std::shared_ptr<Timer> create ();
  virtual ~Timer ();

  /*!
   * \brief Sets the timer to trigger only once, after it has expired once
   *        \sa start needs to be called again
   */
  void setSingleShot ( bool singleShot = true );

  /*!
   * \returns true if the timer is a single shot timer
   */
  bool singleShot () const;

  /*!
   * \returns The current monotonic system time in milliseconds
   */
  static uint64_t now ();

  /*!
   * \return the time that has elapsed since the timepoint given in \a start
   */
  static uint64_t elapsedSince ( const uint64_t start );

  /*!
   * \returns the monotonic system time when the timer started
   */
  uint64_t started  () const;

  /*!
   * \returns the requested interval in milliseconds
   */
  uint64_t interval () const;

  /*!
   * \returns the remaining time until the timer expires in milliseconds
   */
  uint64_t remaining  () const;

  /*!
   * \return the time that has elapsed since the last call to \a start in milliseconds
   */
  uint64_t elapsed  () const;

  /*!
   * \returns the monotonic system time in ms when the timer is about to expire
   */
  uint64_t expires () const;

  /*!
   * \brief Advances the internal clock of the timer, if the timer expires the \a sigExpired signal is emitted
   *
   * \returns the monotonic system time in ms when the timer is about to expire
   * \note There should not be any reason to call this manually, the \sa EventDispatcher is taking care of that
   */
  uint64_t expire ();

  /*!
   * \returns if the timer is currently active
   */
  bool isRunning ( ) const;

  /*!
   * Starts the timer, if the timer is already running this will restart the currently running timer
   */
  void start ( );

  /*!
   * \brief Starts the timer, if the timer is already running this will restart the currently running timer
   * \param timeout the new timeout in ms
   */
  void start ( uint64_t timeout );

  /*!
   * \brief Stops the timer if its running. The \sa sigExpired signal will not emit until \sa start was called again
   */
  void stop  ();

  /*!
   * \brief This signal is always emitted when the timer expires.
   */
  SignalProxy<void (Timer &t)> sigExpired ();

protected:
  Timer ();
};

}

#endif
