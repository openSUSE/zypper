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
#ifndef ZYPPNG_MONADIC_LIFT_H_INCLUDED
#define ZYPPNG_MONADIC_LIFT_H_INCLUDED

#include <utility>
#include <memory>
#include <iostream>

#include <zypp-core/zyppng/meta/Functional>
#include <zypp-core/zyppng/pipelines/AsyncResult>

namespace zyppng {

  namespace detail {
  template <typename LiftedFun, typename extra = void >
  struct lifter {

    lifter( LiftedFun &&fun ) : _fun(std::move(fun)) {}
    lifter( lifter && ) = default;
    ~lifter(){}

    template< typename T1
      , typename T2
      , typename Ret = std::pair<std::result_of_t<LiftedFun(T1)>, T2>
      >
    Ret operator()( std::pair<T1, T2> &&data ) {
      return std::make_pair( std::invoke( _fun, std::move(data.first) ), std::move(data.second) );
    }
  private:
    LiftedFun _fun;
  };

  template < typename AsyncOp >
  struct lifter< std::shared_ptr<AsyncOp>, std::void_t< std::enable_if_t< zyppng::detail::is_async_op<AsyncOp>::value > > > {

    using LiftedFun = std::shared_ptr<AsyncOp>;

    lifter( LiftedFun &&fun ) : _fun(std::move(fun)) {}
    lifter( lifter && ) = default;
    ~lifter(){}

    template< typename T1
      , typename T2
      >
    auto operator()( std::pair<T1, T2> &&data ) {

      using namespace zyppng;
      using namespace zyppng::operators;

      return std::move(data.first)
             | ( std::move(_fun) )
             | [ other = std::move(data.second)]( auto && res ) mutable {
                 return std::make_pair( std::forward<decltype (res)>(res), std::move(other) );
               };
    }
  private:
    LiftedFun _fun;
  };
  }

  template< typename Fun >
  auto lift ( Fun && func ) {
    return detail::lifter<Fun>( std::forward<Fun>(func) );
  }

}

#endif
