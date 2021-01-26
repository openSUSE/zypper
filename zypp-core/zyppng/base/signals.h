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
#include <zypp-core/base/Exception.h>
#include <zypp-core/base/LogControl.h>

/*!
 * \file signals.h
 *
 * This file implements the signal and slot concept in libzypp.
 *
 * Signals and slots are basically a implementation of the observer pattern which makes it possible
 * for objects to work together without them having to know about each other by connecting the
 * signals of one object to the slots of another. Whenever the object owning the signal
 * emits said signal, all connected slots are executed. This is especially helpful in async environments
 * where we need to react on specific events like timers or filedescriptor events that can
 * happen at any time during execution.
 *
 * Using signals and slots in libzypp requires the developer to make sure
 * that all objects that are used in the signal chain can not be deleted
 * during emission, for example a Socket emitting the closed() signal might result in
 * its deletion. This is usually made sure by taking a reference to the sender object
 * via shared_ptr, e.g. the \ref EventLoop does this for all \ref AbstractEventSource 's that are
 * registered to receive events using shared_from_this.
 *
 * To have signals in a Object its recommended to subclass the Type from \ref zyppng::Base
 * and always have the objects in a \ref std::shared_ptr.
 * \ref zyppng::Base provides the \ref zyppng::Base::connect and \ref zyppng::Base::connectFunc
 * helpers to connect signals and slots. Connections made with those helpers will make sure the
 * receiving objects are properly locked.
 *
 * The \ref zyppng::Base::connect helper function can be used to connect a signal that is hosted by a \ref zyppng::Base
 * derived type to a slot hosted by a \ref zyppng::Base derived type. No special care needs to be taken to make sure
 * the receiver object is locked since the function is handling all the details about that:
 * \code
 * class Receiver : public zyppng::Base
 * {
 * public:
 *   static std::shared_ptr<Receiver> create();
 *   void writeOnTimeout() {
 *    std::cout << "Hello World" << std::endl;
 *   }
 * }
 *
 * auto evLoop   = zyppng::EventLoop::create();
 * auto sender   = zyppng::Timer::create();
 * auto receiver = Receiver::create();
 *
 * // invoking the static Base::connect() function will make sure receiver is locked, the sender will be locked by the
 * // eventloop triggering the timeout
 * sender->connect( *sender, &zyppng::Timer::sigExpired, *receiver, &Receiver::writeOnTimeout );
 * sender->start(1000);
 * evLoop->run();
 * \endcode
 *
 * The \ref zyppng::Base::connect helper function can be used to connect a signal that is hosted by a \ref zyppng::Base
 * derived type to a lambda type slot, commonly used when the developer wants to have the code that is invoked by the signal
 * right where the slot is connected, to make the code easier to read or when the slot code is not reused anywhere else.
 *
 * In this example the receiver object uses a slot to invoke the actual function that handles the event. Since we only connect
 * a lambda that invokes the writeOnTimeout function using a caputured \a this pointer we need to manually take care of locking the receiver object during the
 * slot invocation by passing it as a extra argument after the lambda. While the code would compile without doing that we
 * introduce a bug that is very hard to track down because the subsequent signal emitted from the receiver object causes it to be deleted,
 * making the code fail afterwards.
 * \code
 * class Receiver : public zyppng::Base
 * {
 * public:
 *   static std::shared_ptr<Receiver> create();
 *
 *   void connectToTimer ( std::shared_ptr<Timer> &p ) {
 *     p->connectFunc( *sender, &zyppng::Timer::sigExpired, [ this ](){
 *       onTimeout();
 *     }, *this );
 *   }
 *
 *   SignalProxy<void()> sigGotTimeout() {
 *    return _sigGotTimeout;
 *   }
 *
 * private:
 *   void onTimeout() {
 *    std::cout << "Hello World" << std::endl;
 *    // emits a subsequent signal
 *    _sigGotTimeout.emit();
 *
 *    // calls another function on this after emitting the signal
 *    // in cases where our instance was not locked by the caller and
 *    // a slot that received our signal released the last reference to us
 *    // this will run into a segfault
 *    this->doSomethingElse();
 *   }
 *   void doSomethingElse() {
 *    std::cout << "Something else" << std::endl;
 *   }
 *   Signal<void()> _sigGotTimeout;
 * }
 *
 * auto evLoop   = zyppng::EventLoop::create();
 * auto sender   = zyppng::Timer::create();
 * auto receiver = Receiver::create();
 *
 * // invoking the static Base::connect() function will make sure receiver is locked, the sender will be locked by the
 * // eventloop triggering the timeout
 * receiver->connectToTimer( sender );
 *
 * // release the receiver after the timeout was triggered
 * receiver->connectFunc( *receiver, &Receiver::sigGotTimeout, [ &receiver ](){
 *   // releasing the receiver Object once the timer was triggered once. If the object
 *   // is not properly referenced during signal emission we will run into a SEGFAULT
 *   receiver.reset();
 * });
 *
 * sender->start( 1000 );
 * evLoop->run();
 * \endcode
 */
namespace zyppng {


  /*!
   * \class Signal
   * Simple signal that can be invoked with the given arguments.
   */
  template <class R, class... T>
  class Signal;

  /*!
   * \class MemSignal
   * This is a special type of signal that takes a reference to the sender
   * object during signal emission. This is only required in cases where
   * sending a signal might delete the sender and no external object holds a reference to it.
   */
  template <class SignalHost, typename ReturnType, typename... Arguments>
  class MemSignal;

  class Base;
  class BasePrivate;

#ifdef ENABLE_SYWU
  using connection = sywu::Connection;

  template <class R, class... T>
  using SignalProxyBase = sywu::SignalConceptImpl<R(T...)>;

  template <class R, class... T>
  class Signal<R(T...)> : public sywu::Signal<R(T...)>
  { };

  template <class SignalHost, typename ReturnType, typename... Arguments>
  class MemSignal<SignalHost, ReturnType(Arguments...)> : public sywu::MemberSignal<SignalHost, ReturnType(Arguments...)>
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
      assert(this->impl()->exec_count_ == 0);
      if ( this->impl()->exec_count_ > 0 ) {
        WAR << "Deleting Signal during emission, this is usually a BUG, Slots will be blocked to prevent SIGSEGV." << std::endl;
#ifdef LIBZYPP_USE_SIGC_BLOCK_WORKAROUND
        // older sigc versions will segfault if clear() is called in signal emission
        // we use block instead in those cases which seems to have the same result
        // since we do not use the slot instances explicitely that _should_ not have side effects
        // https://bugzilla.gnome.org/show_bug.cgi?id=784550
        this->block();
        return;
#endif
      }

      this->clear();
    }
  };

  template <class SignalHost, typename ReturnType, typename... Arguments>
  class MemSignal<SignalHost, ReturnType(Arguments...)> : public sigc::signal<ReturnType(Arguments...)>
  {
  public:
    MemSignal ( SignalHost &host ) : _host(host) {}

    ~MemSignal() {

      assert(this->impl()->exec_count_ == 0);
      if ( this->impl()->exec_count_ > 0 ) {
        WAR << "Deleting MemSignal during emission, this is definitely a BUG, Slots will be blocked to prevent SIGSEGV." << std::endl;
#ifdef LIBZYPP_USE_SIGC_BLOCK_WORKAROUND
        // older sigc versions will segfault if clear() is called in signal emission
        // we use block instead in those cases which seems to have the same result
        // since we do not use the slot instances explicitely that _should_ not have side effects
        // https://bugzilla.gnome.org/show_bug.cgi?id=784550
        this->block();
        return;
#endif
      }
      this->clear();
    }

    template<typename... Args>
    auto emit( Args&& ...arg ) const {
      auto ref = _host.shared_from_this();
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

    /*!
     * Helper tool that always locks the public object in case a BasePrivate derived type
     * is passed.
     */
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
     * Adaptor that locks and tracks the given objects, this implements locking
     * of the receiver object in a signal chain.
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
