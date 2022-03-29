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
#ifndef ZYPPNG_META_FUNCTIONAL_H_INCLUDED
#define ZYPPNG_META_FUNCTIONAL_H_INCLUDED

#include <functional>

#if __cplusplus <= 201402L || !defined ( __cpp_lib_invoke )

#include <type_traits>

// this is a workaround for std::invoke not being available in C++14
// and the proposed minimal implementation in
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4169.html

namespace std {
  template<typename Functor, typename... Args>
  typename std::enable_if<
    std::is_member_pointer<typename std::decay<Functor>::type>::value,
    typename std::result_of<Functor&&(Args&&...)>::type
  >::type invoke(Functor&& f, Args&&... args)
  {
    return std::mem_fn(f)(std::forward<Args>(args)...);
  }

  template<typename Functor, typename... Args>
  typename std::enable_if<
    !std::is_member_pointer<typename std::decay<Functor>::type>::value,
    typename std::result_of<Functor&&(Args&&...)>::type
  >::type invoke(Functor&& f, Args&&... args)
  {
    return std::forward<Functor>(f)(std::forward<Args>(args)...);
  }
}

#endif

/*!
 * Simple helper template to make a callback that binds the "this" pointer, to be used in
 * a pipeline.
 */
template<typename Obj, typename Ret, typename Arg>
auto mem_fn_cb( Obj& o, Ret (Obj::*objMemFunc)( Arg&& ) ) {
  return [tPtr = &o, fun = objMemFunc ]( Arg &&r ){
    return std::invoke(fun, tPtr, std::move(r) );
  };
}

template<typename Obj, typename Ret, typename Arg>
auto mem_fn_cb( Obj& o, Ret (Obj::*objMemFunc)( const Arg& ) ) {
  return [tPtr = &o, fun = objMemFunc ]( const Arg &r ){
    return std::invoke(fun, tPtr, r );
  };
}

template<typename Obj, typename Ret, typename Arg>
auto mem_fn_cb( Obj& o, Ret (Obj::*objMemFunc)( Arg ) ) {
  return [tPtr = &o, fun = objMemFunc ]( Arg r ){
    if constexpr ( std::is_invocable_v< decltype (fun), Obj*, Arg&&> ) {
      return std::invoke(fun, tPtr, std::move(r) );
    } else {
      return std::invoke(fun, tPtr, r );
    }
  };
}


#endif
