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
#ifndef ZYPP_NG_BASE_SIGNALS_H_INCLUDED
#define ZYPP_NG_BASE_SIGNALS_H_INCLUDED

#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include <sigc++/adaptors/adaptors.h>

namespace zyppng {

using sigc::signal;
using sigc::connection;
using sigc::trackable;
using sigc::track_obj;

template <class R, class... T>
class SignalProxy;

/**
     * Hides the signals emit function from external code.
     *
     * \note based on Glibmms SignalProxy code
     */
template <class R, class... T>
class SignalProxy<R(T...)>
{
public:
  using SlotType = sigc::slot<R(T...)>;
  using SignalType = sigc::signal<R(T...)>;

  SignalProxy ( SignalType &sig ) : _sig ( sig ) {}

  /** Connects a signal handler to a signal.
         *
         * For instance, connect(sigc::mem_fun(*this, &TheClass::on_something));
         *
         * @param slot The signal handler, usually created with sigc::mem_fun() or sigc::ptr_fun().
         * @return A sigc::connection.
         */
  connection connect( const SlotType& slot )
  {
    return _sig.connect( slot );
  }

  /** Connects a signal handler to a signal.
         * @see connect(const SlotType& slot).
         */
  connection connect( SlotType&& slot )
  {
    return _sig.connect( std::move( slot ) );
  }

private:
  SignalType &_sig;
};


}

#endif // ZYPP_NG_CORE_SIGNALS_H_INCLUDED
