/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPP_NG_BASE_ABSTRACTEVENTSOURCE_H_INCLUDED
#define ZYPP_NG_BASE_ABSTRACTEVENTSOURCE_H_INCLUDED

#include <zypp-core/zyppng/base/zyppglobal.h>
#include <zypp-core/zyppng/base/Signals>
#include <zypp-core/zyppng/base/Base>

namespace zyppng {

class EventDispatcher;
class AbstractEventSourcePrivate;

/*!
 * The AbstractEventSource class is the base class for all objects that can
 * be tracked using one or more file descriptors
 *
 * A subclass can register multiple file descriptors to be tracked using
 * \a updateFdWatch and remove them with \a removeFdWatch. This is internally
 * forwarded to the \sa EventDispatcher belonging to the \a AbstractEventSource.
 *
 * In case there is activity on the file descriptor the EventDispatcher wakes up
 * and forwards it to \a onFdReady.
 */
class LIBZYPP_NG_EXPORT AbstractEventSource : public Base
{
  ZYPP_DECLARE_PRIVATE(AbstractEventSource)
public:

  /*!
   * The different types of events a AbstractEventSource can
   * listen for.
   */
  enum EventTypes {
    Read  = 0x01,  //< The fd is ready to read data without blocking
    Write = 0x02,  //< The fd has written pending data
    Exception = 0x04, //< A exception has happend on the fd
    Error = 0x08      //< There was a error polling the fd
  };

  AbstractEventSource();
  AbstractEventSource( AbstractEventSourcePrivate &dd );
  virtual ~AbstractEventSource();

  /*!
   * Returns the \sa EventDispatcher that takes care of this event source
   */
  std::weak_ptr<EventDispatcher> eventDispatcher () const;

  /*!
   * Called by the \sa EventDispatcher when activity was detected
   * on a registered \a fd. The \a events are set according to the activity
   * on the file descriptor.
   */
  virtual void onFdReady ( int fd, int events ) = 0;

  /*!
   * Called when a event source has registered to receive unix signals
   * \note this is not supported yet
   */
  virtual void onSignal  ( int signal ) = 0;

protected:
  /*!
   * Register or update a file descriptor poll mode.
   * \param fd The fd to add or update to the watchlist of the \sa EventDispatcher
   * \param mode The watchmode used in the watchlist \sa EventTypes
   */
  void updateFdWatch ( int fd, int mode );

  /*!
   * Removes the given \a fd from the \sa EventDispatcher watchlist.
   * After calling this no more events will be received by the object it was called on.
   */
  void removeFdWatch   ( int fd );
};

}

#endif
