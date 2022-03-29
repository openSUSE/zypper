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
#ifndef ZYPPNG_MONADIC_REDO_H_INCLUDED
#define ZYPPNG_MONADIC_REDO_H_INCLUDED

#include <zypp-core/zyppng/pipelines/AsyncResult>
#include <zypp-core/zyppng/meta/FunctionTraits>
#include <zypp-core/zyppng/meta/TypeTraits>
#include <zypp-core/zyppng/meta/Functional>

namespace zyppng {

  namespace detail {


    template< typename Task, typename Pred, typename = void >
  struct RedoWhileImpl
  {

      static_assert ( !is_async_op< zyppng::remove_smart_ptr<Pred> >::value, "" );

      template <typename T, typename P>
      RedoWhileImpl( T &&t, P &&p ) :
        _task( std::forward<T>(t) )
        , _pred( std::forward<P>(p) ) {}

      template <typename Arg>
      std::enable_if_t< is_async_op< remove_smart_ptr_t<std::result_of_t<Task(Arg)>> >::value == false, Arg > operator()( Arg &&arg ) {
        Arg store = std::forward<Arg>(arg);
        do {
          auto res = _task ( Arg(store) );
          if ( !_pred( res ) )
            return std::move(res);
        } while( true );
      }

      template <typename T, typename P>
      static auto create ( T &&t, P &&p ) {
        return RedoWhileImpl( std::forward<T>(t), std::forward<P>(p));
      }

    private:
      Task _task;
      Pred _pred;
    };

    template< typename MyAsyncOp, typename Pred >
    struct RedoWhileImpl< std::shared_ptr<MyAsyncOp>,Pred, std::enable_if_t< is_async_op< MyAsyncOp >::value > > : public AsyncOp<typename MyAsyncOp::value_type> {

      using Task = std::shared_ptr<MyAsyncOp>;
      using OutType  = typename MyAsyncOp::value_type;

      template <typename T, typename P>
      RedoWhileImpl( T &&t, P &&p ) :
        _task( std::forward<T>(t) )
        , _pred( std::forward<P>(p) ) {}

      static_assert ( !is_async_op< remove_smart_ptr<Pred> >::value, "" );

      template<typename InType>
      void operator()( InType &&arg ) {
        _task->onReady(
          [this, inArg = arg]( OutType &&a) mutable {
            if ( _pred(a) )
              this->operator()(std::move(inArg));
            else
              this->setReady( std::move(a) );
          }
        );
        _task->operator()(  InType(arg) );
      }

      template <typename T, typename P>
      static auto create ( T &&t, P &&p ) {
        return std::make_shared<RedoWhileImpl>( std::forward<T>(t), std::forward<P>(p));
      }

    private:

      Task _task;
      Pred _pred;
      std::shared_ptr<AsyncOp<OutType>> _pipeline;
    };

    //implementation for a function returning a asynchronous result
    template< typename Task, typename Pred >
    struct RedoWhileImpl< Task,Pred, std::enable_if_t< is_async_op< remove_smart_ptr_t<typename function_traits<Task>::return_type> >::value > > : public AsyncOp< typename remove_smart_ptr_t<typename function_traits<Task>::return_type>::value_type> {

      using FunRet = remove_smart_ptr_t<typename function_traits<Task>::return_type>;

      //the task function needs to return the same type it takes
      using OutType = typename FunRet::value_type;

      template <typename T, typename P>
      RedoWhileImpl( T &&t, P &&p ) :
        _task( std::forward<T>(t) )
        , _pred( std::forward<P>(p) ) {}

      template<typename InType>
      void operator() ( InType &&arg ) {
        _asyncRes.reset();
        _asyncRes = _task( InType( arg ) );
        _asyncRes->onReady(
          [this, inArg = arg ]( OutType &&arg ) mutable {
            if ( _pred(arg) )
              this->operator()( std::move(inArg) );
            else
              this->setReady( std::move(arg) );
          });
      }

      template <typename T, typename P>
      static auto create ( T &&t, P &&p ) {
        return std::make_shared<RedoWhileImpl>( std::forward<T>(t), std::forward<P>(p));
      }

    private:
      std::shared_ptr<AsyncOp<OutType>> _asyncRes;

      Task _task;
      Pred _pred;
    };

    //check if it is possible to query the given type for function_traits
    template <typename T>
    using has_func_trait = typename function_traits<T>::return_type;

  }

  template <typename Task, typename Pred>
  auto redo_while ( Task &&todo, Pred &&until )
  {
    static_assert ( std::is_detected_v< detail::has_func_trait, Task >, "Not possible to deduce the function_traits for Task, maybe a generic lambda?" );
    return detail::RedoWhileImpl<Task,Pred>::create( std::forward<Task>(todo), std::forward<Pred>(until) );
  }

}

#endif
