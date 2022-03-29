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
#ifndef ZYPPNG_MONADIC_ASYNCRESULT_H_INCLUDED
#define ZYPPNG_MONADIC_ASYNCRESULT_H_INCLUDED

#include <zypp-core/zyppng/meta/TypeTraits>
#include <zypp-core/zyppng/meta/FunctionTraits>
#include <zypp-core/zyppng/meta/Functional>
#include <zypp-core/zyppng/async/AsyncOp>


namespace zyppng {


  namespace detail {

    /*!
     * Checks if a Callable object is of type async_op and if
     * it accepts the given MsgType and returns the expected type
     */
    template< typename Callback, typename Ret, typename MsgType, typename = std::void_t<> >
    struct is_future_monad_cb : public std::false_type{};

    template< typename Callback, typename Ret, typename MsgType >
    struct is_future_monad_cb<Callback, Ret, MsgType,
      std::void_t<
        typename Callback::value_type,
        std::enable_if_t< std::is_same< typename Callback::value_type, Ret >::value >,
        std::enable_if_t< is_async_op<Callback>::value >,
        decltype ( std::declval<Callback>()( std::declval<MsgType>()) )//check if the callback has a operator() member with the correct signature
        >
      > : public std::true_type{};

    /*!
     * Checks if a Callable object is a syncronous callback type with the expected signature
     */
    template< typename Callback, typename MsgType, typename = std::void_t<> >
    struct is_sync_monad_cb : public std::false_type{};

    template< typename Callback, typename MsgType >
    struct is_sync_monad_cb<Callback, MsgType
      , std::void_t<
        std::enable_if_t< !is_async_op<Callback>::value >,
        std::enable_if_t< !std::is_same< void, decltype ( std::declval<Callback>()(std::declval<MsgType>())) >::value > > //check if the callback has the correct signature:  cb( MsgType )
      > : public std::true_type{};


    /*!
     * The AsyncResult class is used to bind previous operations to a new operation and to carry
     * the result of the full pipeline. It has a pointer to the _prevTask, which is usually
     * a AsyncResult too, and to the task it should execute once the previous task enters ready state.
     *
     * In theory this is a single linked list, but each node owns all the previous nodes, means that once
     * a node is destroyed all previous ones are destroyed as well. Basically the async results are nested objects, where
     * the outermost object is the last to be executed. While executing the nodes they are cleaned up right away
     * after they enter finished or ready state.
     *
     * When returned to the code the AsyncResult is casted into the AsyncOp base class, otherwise the type
     * information gets too complex and matching the pipe operator functions is a nightmare. This could be revisited
     * with newer C++ versions.
     *
     */
    template <typename Prev, typename AOp, typename = void>
    struct AsyncResult;

    template <typename Prev, typename AOp >
    struct AsyncResult<Prev,AOp> : public zyppng::AsyncOp< typename AOp::value_type > {

      AsyncResult ( std::shared_ptr<Prev> && prevTask, std::shared_ptr<AOp> &&cb )
        : _prevTask( std::move(prevTask) )
        , _myTask( std::move(cb) ) {
        connect();
      }

      AsyncResult ( const AsyncResult &other ) = delete;
      AsyncResult& operator= ( const AsyncResult &other ) = delete;

      AsyncResult ( AsyncResult &&other ) = delete;
      AsyncResult& operator= ( AsyncResult &&other ) = delete;

      virtual ~AsyncResult() {}

      void connect () {
        //not using a lambda here on purpose, binding this into a lambda that is stored in the _prev
        //object causes segfaults on gcc when the lambda is cleaned up with the _prev objects signal instance
        _prevTask->onReady( std::bind( &AsyncResult::readyWasCalled, this, std::placeholders::_1) );
        _myTask->onReady( [this] ( typename AOp::value_type && res ){
          this->setReady( std::move( res ) );
        });
      }

    private:
      void readyWasCalled ( typename Prev::value_type && res ) {
        //we need to force the passed argument into our stack, otherwise we
        //run into memory issues if the argument is moved out of the _prevTask object
        typename Prev::value_type resStore = std::move(res);

        if ( _prevTask ) {
          //dumpInfo();
          _prevTask.reset();
        }

        _myTask->operator()(std::move(resStore));
      }

      std::shared_ptr<Prev> _prevTask;
      std::shared_ptr<AOp> _myTask;
    };

    template<typename AOp, typename In>
    struct AsyncResult<void, AOp, In> : public zyppng::AsyncOp< typename AOp::value_type > {

      AsyncResult ( std::shared_ptr<AOp> &&cb )
        : _myTask( std::move(cb) ) {
        connect();
      }

      virtual ~AsyncResult() { }

      void run ( In &&val ) {
        _myTask->operator()( std::move(val) );
      }

      AsyncResult ( const AsyncResult &other ) = delete;
      AsyncResult& operator= ( const AsyncResult &other ) = delete;

      AsyncResult ( AsyncResult &&other ) = delete;
      AsyncResult& operator= ( AsyncResult &&other ) = delete;

    private:
      void connect () {
        _myTask->onReady( [this] ( typename AOp::value_type && in ){
          this->setReady( std::move( in ) );
        });
      }
      std::shared_ptr<AOp> _myTask;
    };

    //need a wrapper to connect a sync callback to a async one
    template < typename Callback, typename In, typename Out >
    struct SyncCallbackWrapper : public AsyncOp<Out>
    {
      using value_type = Out;

      template< typename C = Callback >
      SyncCallbackWrapper( C &&c ) : _c( std::forward<C>(c) ){}

      virtual ~SyncCallbackWrapper(){ }

      void operator() ( In &&value ) {
        this->setReady( std::invoke( _c, std::move(value)) );
      }

    private:
      Callback _c;
    };

    /*!
     * Simple AsyncOperator wrapper that accepts a Async result
     * forwarding the actual value when it gets ready. This is used to
     * simplify nested asyncronous results.
     */
    template< typename AOp, typename Ret = typename AOp::value_type >
    struct SimplifyHelper : public AsyncOp<Ret>
    {

      virtual ~SimplifyHelper(){}

      void operator() ( std::shared_ptr<AOp> &&op ) {
        assert( !_task );
        _task = std::move(op);
        _task->onReady( [this]( Ret &&val ){
          this->setReady( std::move(val) );
        });
      }
    private:
      std::shared_ptr<AOp> _task;
    };

    /*!
     * Takes a possibly nested future and simplifies it,
     * so instead of AsyncResult<AsyncResult<int>> we get AsyncResult<int>.
     * Usually we do not want to wait on the future of a future but get the nested result immediately
     */
    template < typename Res >
    inline std::shared_ptr<AsyncOp<Res>> simplify ( std::shared_ptr< AsyncOp<Res> > &&res ) {
      return std::move(res);
    }

    template < typename Res,
      typename AOp =  AsyncOp< std::shared_ptr< AsyncOp<Res>> > >
    inline std::shared_ptr<AsyncOp<Res>> simplify ( std::shared_ptr< AsyncOp< std::shared_ptr< AsyncOp<Res>> > > &&res ) {
      std::shared_ptr<AsyncOp<Res>> op = std::make_shared< detail::AsyncResult< AOp, SimplifyHelper< AsyncOp<Res>>> >( std::move(res), std::make_shared<SimplifyHelper< AsyncOp<Res>>>() );
      return detail::simplify( std::move(op) );
    }
  }

  namespace operators {

    //case 1 : binding a async message to a async callback
    template< typename PrevOp
      , typename Callback
      , typename Ret =  typename Callback::value_type
      , std::enable_if_t< detail::is_async_op<PrevOp>::value, int> = 0
      //is the callback signature what we want?
      , std::enable_if_t< detail::is_future_monad_cb< Callback, Ret, typename PrevOp::value_type >::value, int> = 0
      >
    auto operator| ( std::shared_ptr<PrevOp> &&future, std::shared_ptr<Callback> &&c )
    {
      std::shared_ptr<AsyncOp<Ret>> op = std::make_shared<detail::AsyncResult< PrevOp, Callback>>( std::move(future), std::move(c) );
      return detail::simplify( std::move(op) );
    }

    //case 2: binding a async message to a sync callback
    template< typename PrevOp
      , typename Callback
      , typename Ret = std::result_of_t< Callback( typename PrevOp::value_type&& ) >
      , std::enable_if_t< detail::is_async_op<PrevOp>::value, int> = 0
      , std::enable_if_t< detail::is_sync_monad_cb< Callback, typename PrevOp::value_type >::value, int> = 0
      >
    auto operator| ( std::shared_ptr<PrevOp> &&future, Callback &&c )
    {
      std::shared_ptr<AsyncOp<Ret>> op(std::make_shared<detail::AsyncResult< PrevOp, detail::SyncCallbackWrapper<Callback, typename PrevOp::value_type, Ret> >>(
        std::move(future)
          ,  std::make_shared<detail::SyncCallbackWrapper<Callback, typename PrevOp::value_type, Ret>>( std::forward<Callback>(c) ) ));

      return detail::simplify( std::move(op) );
    }

    //case 3: binding a sync message to a async callback
    template< typename SyncRes
      , typename Callback
      , typename Ret = typename Callback::value_type
      , std::enable_if_t< !detail::is_async_op< remove_smart_ptr_t<SyncRes> >::value, int> = 0
      , std::enable_if_t< detail::is_future_monad_cb< Callback, Ret, SyncRes >::value, int> = 0
      >
    auto  operator| ( SyncRes &&in, std::shared_ptr<Callback> &&c )
    {
      AsyncOpRef<Ret> op( std::make_shared<detail::AsyncResult<void, Callback, SyncRes>>( std::move(c) ) );
      static_cast< detail::AsyncResult<void, Callback, SyncRes>* >(op.get())->run( std::move(in) );
      return detail::simplify( std::move(op) );
    }

    //case 4: binding a sync message to a sync callback
    template< typename SyncRes
      , typename Callback
      , std::enable_if_t< !detail::is_async_op< remove_smart_ptr_t<SyncRes> >::value, int > = 0
      , std::enable_if_t< detail::is_sync_monad_cb< Callback, SyncRes >::value, int > = 0
      >
    auto operator| ( SyncRes &&in, Callback &&c )
    {
      return std::forward<Callback>(c)(std::forward<SyncRes>(in));
    }

  }
}

#endif
