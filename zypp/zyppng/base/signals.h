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

#ifdef ENABLE_SYWU
#include <sywu/signal.hpp>
#else
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include <sigc++/visit_each.h>
#include <sigc++/adaptors/adaptors.h>
#endif
#include <memory>
#include <utility>
#include <cassert>
#include <zypp/base/Exception.h>
#include <zypp/base/LogControl.h>

namespace zyppng {

  template <class R, class... T>
  class Signal;

  template <class SignalHost, typename ReturnType, typename... Arguments>
  class MemSignal;

  class Base;
  class BasePrivate;

#ifdef ENABLE_SYWU
  using connection = sywu::Connection;

  template <class R, class... T>
  using SignalProxyBase = sywu::SignalConceptImpl<R(T...)>;

  template <class R, class... T>
  class signal<R(T...)> : public sywu::Signal<R(T...)>
  { };

  template <class SignalHost, typename ReturnType, typename... Arguments>
  class memsignal<SignalHost, ReturnType(Arguments...)> : public sywu::MemberSignal<SignalHost, ReturnType(Arguments...)>
  { };

#else
  using connection = sigc::connection;
  using trackable  = sigc::trackable;
  using sigc::track_obj;

  template <class R, class... T>
  using SignalProxyBase = sigc::signal<R(T...)>;


  template <class R, class... T>
  class Signal<R(T...)> : public sigc::signal<R(T...)>
  {
  public:
    ~Signal() {
      this->clear();
    }
  };

  template <class SignalHost, typename ReturnType, typename... Arguments>
  class MemSignal<SignalHost, ReturnType(Arguments...)> : public sigc::signal<ReturnType(Arguments...)>
  {
  public:
    MemSignal ( SignalHost &host ) : _host(host) {}

    ~MemSignal() {
      this->clear();
    }

    template<typename... Args>
    auto emit( Args&& ...arg ) const {
      //auto ref = _host.shared_from_this();
      return sigc::signal<ReturnType(Arguments...)>::emit( std::forward<Args>(arg)...);
    }

    template<typename... Args>
    auto operator()( Args&& ...arg ) const {
      //auto ref = _host.shared_from_this();
      return sigc::signal<ReturnType(Arguments...)>::operator()( std::forward<Args>(arg)...);
    }

  private:
    SignalHost &_host;
  };

  namespace internal {

    template <typename T>
    inline auto lock_shared_makeLock ( const T& locker ) {
      try {
        if constexpr ( std::is_base_of_v<BasePrivate, T> ) {
          return locker.z_func()->shared_from_this();
        } else {
          return locker.shared_from_this();
        }
      }  catch (  const std::bad_weak_ptr &e ) {
        ZYPP_CAUGHT( e );
        ZYPP_THROW( e );
      }
    }

    /*!
     * Adaptor that locks and tracks the given objects
     */
    template <typename T_functor, typename ...Lockers>
    struct lock_shared : public sigc::adapts<T_functor>
    {
      template <typename... Args>
      decltype(auto) operator()( Args&&... args ) const {

        try {
          auto __attribute__ ((__unused__)) lck = std::apply( []( auto&... lockers ) {
            return std::make_tuple( lock_shared_makeLock( lockers.invoke() )... );
          }, _lcks );

          // seems the overloaded function templates in sigc++2 force us to fully specify the overload
          // we want to use. std::invoke() fails in that case.
          //return std::invoke( this->functor_, std::forward<Args>(args)... );
          if constexpr ( sizeof... (Args) == 0 ) {
            return this->functor_();
          } else {
            return this->functor_.template operator()<decltype ( std::forward<Args>(args) )...> ( std::forward<Args>(args)... );
          }
        }  catch ( const std::bad_weak_ptr &e ) {
          ZYPP_CAUGHT( e );
          ERR << "Ignoring signal emit due to a bad_weak_ptr exception during object locking. Maybe the signal was sent to a object that is currently destructing?" << std::endl;
#ifndef ZYPP_NDEBUG
          assert( false );
#endif
          if constexpr ( !std::is_same_v<void, typename sigc::adapts<T_functor>::result_type> )
            return typename sigc::adapts<T_functor>::result_type{};
          else
            return;
        }
      }

      // Constructs a my_adaptor object that wraps the passed functor.
      // Initializes adapts<T_functor>::functor_, which is invoked from operator()().
      explicit lock_shared( const T_functor& functor, const Lockers&... lcks ) :
        sigc::adapts<T_functor>(functor),
        _lcks( lcks... )
      {}

      std::tuple<sigc::const_limit_reference<Lockers>...> _lcks;
    };

    template< typename Functor, typename ...Obj >
    inline decltype(auto) locking_fun( const Functor &f, const Obj&... o )
    {
      return lock_shared<Functor, Obj...>( f, o...);
    }

  }
#endif

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
    using SignalType = SignalProxyBase<R,T...>;

    SignalProxy ( SignalType &sig ) : _sig ( sig ) {}

    /**
     * Forwards the arguments to the internal connect function of the signal type
     */
    template <typename... Args>
    connection connect( Args&&... slot )
    {
      return _sig.connect( std::forward<Args>(slot)... );
    }

  private:
    SignalType &_sig;
  };

}

#ifndef ENABLE_SYWU
//
// Specialization of sigc::visitor for lock_shared.
namespace sigc
{
  template <typename T_functor, typename ...Lockers>
  struct visitor<zyppng::internal::lock_shared<T_functor, Lockers...> >
  {
    template <typename T_action>
    static void do_visit_each(const T_action& action,
      const zyppng::internal::lock_shared<T_functor, Lockers...>& target)
    {
      sigc::visit_each(action, target.functor_);
      std::apply( [&]( auto&... a) {
        ((void)sigc::visit_each(action, a),...);
      }, target._lcks );

    }
  };
} // end namespace sigc
#endif

#endif // ZYPP_NG_CORE_SIGNALS_H_INCLUDED
