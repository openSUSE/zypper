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
#ifndef ZYPP_NG_BASE_BASE_H_INCLUDED
#define ZYPP_NG_BASE_BASE_H_INCLUDED

#include <zypp/zyppng/base/zyppglobal.h>
#include <zypp/zyppng/base/signals.h>
#include <memory>
#include <unordered_set>
#include <vector>
#include <thread>

namespace zyppng {

  class Base;
  class BasePrivate;

  namespace internal {
    template <typename Func>
    struct MemberFunction;

    template <typename BType, typename Ret, typename ...Args >
    struct MemberFunction<Ret (BType::*)( Args... )> {
      using ClassType = BType;
    };

    template <typename T>
    inline constexpr bool is_base_receiver = std::is_base_of_v<Base, T> || std::is_base_of_v<BasePrivate, T>;

  }

  /*!
   * The Base class is used as a common base class for objects that emit signals,
   * it also supports a parent/child relationship where the parent object keeps
   * a reference for all its children.
   *
   * Generally all objects that want to send or receive signals should derive from Base since it
   * will help on enforcing a correct use of shared_ptr semantics. Generally with signal/slot emission
   * its easy to run into the issue of a object being deleted while it emits a signal. Think about a Socket
   * that emits a \a closed signal. It might be removed from the list of connections and the last reference of the
   * object deleted before the signal returns.
   * In order to prevent this from happening we established a rule of "Always own a reference to the object you use".
   * The connection helpers \ref Base::connect and \ref Base::connectFunc help with enforcing this rule, even asserting
   * when compiled without NDEBUG defined.
   *
   * \sa zypp/zyppng/base/signals.h
   */
  class LIBZYPP_NG_EXPORT Base : public sigc::trackable, public std::enable_shared_from_this<Base>
  {
    NON_COPYABLE(Base);
    ZYPP_DECLARE_PRIVATE(Base)
  public:

    using Ptr = std::shared_ptr<Base>;
    using WeakPtr = std::weak_ptr<Base>;

    Base ();
    virtual ~Base();

    /*!
     * Returns the parent object if there is one, otherwise
     * returns a zero WeakPtr
     */
    WeakPtr parent() const;

    /*!
     * Adds a new object to the child list, the object
     * will keep a reference for its entire lifetime or until the object is removed
     */
    void addChild ( Base::Ptr child );

    /*!
     * Removes a child object from the internal child list
     */
    void removeChild (Ptr child );

    /*!
     * Returns all child objects of this object
     */
    const std::unordered_set<Ptr> &children() const;

    /*!
     * Returns the thread ID this object was created in
     */
    std::thread::id threadId () const;

    /*!
     * Returns all children that can be casted to type T
     */
    template<typename T>
    std::vector< std::weak_ptr<T> > findChildren () const {
      std::vector< std::weak_ptr<T> > result;
      for ( Ptr p : children() ) {
        std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(p);
        if ( casted )
          result.push_back( std::weak_ptr<T>(casted) );
      }
      return result;
    }

    template<typename T>
    inline std::shared_ptr<T> shared_this () const {
      return std::static_pointer_cast<T>( shared_from_this() );
    }

    template<typename T>
    inline std::shared_ptr<T> shared_this () {
      return std::static_pointer_cast<T>( shared_from_this() );
    }

    template<typename T>
    inline std::weak_ptr<T> weak_this () const {
      return std::static_pointer_cast<T>( weak_from_this().lock() );
    }

    template<typename T>
    inline std::weak_ptr<T> weak_this () {
      return std::static_pointer_cast<T>( weak_from_this().lock() );
    }

    template<typename Obj, typename Functor >
    static decltype (auto) make_base_slot( Obj *o, Functor &&f ) {
      //static_assert ( !internal::is_base_receiver<Obj>, "Can not make a slot for a Object that does not derive from Base or BasePrivate.");
      return internal::locking_fun( sigc::mem_fun( o, std::forward<Functor>(f) ), *o );
    }

    /*!
     * Preferred way to connect a signal to a slow, this will automatically take care of tracking the target object in the connection
     */
    template< typename SenderFunc, typename ReceiverFunc >
    static auto connect ( typename internal::MemberFunction<SenderFunc>::ClassType &s, SenderFunc &&sFun, typename internal::MemberFunction<ReceiverFunc>::ClassType &recv, ReceiverFunc &&rFunc ) {
      return std::invoke( std::forward<SenderFunc>(sFun), &s ).connect( make_base_slot( &recv, std::forward<ReceiverFunc>(rFunc) ) );
    }


    /*!
     * Convenience func that uses "this" as the sender object in the signal / slot connection, this allows syntax like:
     * \code
     * object.connect( &Obj::signal, *targetObj, &TargetObj::onSignal );
     * \endcode
     */
    template< typename SenderFunc, typename ReceiverFunc >
    auto connect ( SenderFunc &&sFun, typename internal::MemberFunction<ReceiverFunc>::ClassType &recv, ReceiverFunc &&rFunc ) {
      return connect( static_cast<typename internal::MemberFunction<SenderFunc>::ClassType &>(*this), std::forward<SenderFunc>(sFun), recv, std::forward<ReceiverFunc>(rFunc) );
    }

    /*!
     * This allows to connect a lambda to a signal in a \ref Base derived type. Make sure to track the objects used inside the slot for shared_ptr
     * correctness ( always own a reference to the object you are calling )
     */
    template< typename SenderFunc, typename ReceiverFunc, typename ...Tracker >
    static auto connectFunc ( typename internal::MemberFunction<SenderFunc>::ClassType &s, SenderFunc &&sFun, ReceiverFunc &&rFunc, const Tracker&...trackers ) {
      return std::invoke( std::forward<SenderFunc>(sFun), &s ).connect( internal::locking_fun( std::forward<ReceiverFunc>(rFunc), trackers... ) );
    }

    template< typename SenderFunc, typename ReceiverFunc, typename ...Tracker  >
    std::enable_if_t< std::is_member_function_pointer_v< SenderFunc >,  connection > connectFunc ( SenderFunc &&sFun, ReceiverFunc &&rFunc, const Tracker&...trackers  ) {
      return connectFunc( static_cast<typename internal::MemberFunction<SenderFunc>::ClassType &>(*this), std::forward<SenderFunc>(sFun), std::forward<ReceiverFunc>(rFunc), trackers... );
    }

  protected:
    Base ( BasePrivate &dd );
    std::unique_ptr<BasePrivate> d_ptr;
  };


  template<typename Obj, typename Functor,
    std::enable_if_t< std::is_base_of_v< Base, Obj> || std::is_base_of_v<BasePrivate, Obj>, bool> = true
  >
  inline decltype(auto) base_slot( Obj *o, Functor &&f )
  {
    return Base::make_base_slot(o, std::forward<Functor>(f) );
  }

  template<typename Obj, typename Functor,
    std::enable_if_t< std::is_base_of_v< Base, Obj> || std::is_base_of_v<BasePrivate, Obj>, bool> = true
    >
  inline decltype(auto) base_slot( Obj &o, Functor &&f )
  {
    return Base::make_base_slot(&o, std::forward<Functor>(f) );
  }

} // namespace zyppng

#endif // ZYPP_NG_CORE_BASE_H_INCLUDED
