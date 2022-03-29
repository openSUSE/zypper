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
#ifndef ZYPPNG_ASYNC_ASYNCOP_H_INCLUDED
#define ZYPPNG_ASYNC_ASYNCOP_H_INCLUDED

#include <zypp-core/zyppng/base/Base>
#include <zypp-core/base/Exception.h>
#include <zypp-core/zyppng/base/Signals>
#include <optional>
#include <memory>

namespace zyppng {

  /*!
   * Thrown if code tries to access the result of a async operation
   * that is not ready.
   */
  class AsyncOpNotReadyException : public zypp::Exception
  {
  public:
    AsyncOpNotReadyException()
      : Exception( "AsyncOp instance not ready" )
    {}
    virtual ~AsyncOpNotReadyException();
  };

  class CancelNotImplementedException : public zypp::Exception
  {
  public:
    CancelNotImplementedException()
      : Exception ("AsyncOp does not support cancelling the operation")
    {}
    virtual ~CancelNotImplementedException();


  };

  inline AsyncOpNotReadyException::~AsyncOpNotReadyException()
  { }


  inline CancelNotImplementedException::~CancelNotImplementedException()
  { }


  struct AsyncOpBase : public Base {

    /*!
     * Should return true if the async op supports cancelling, otherwise false
     */
    virtual bool canCancel () {
      return false;
    }

    /*!
     * Explicitely cancels the async operation if it supports that,
     * otherwise throws a \ref CancelNotImplementedException
     *
     * Generally every AsyncOp HAS TO support cancelling by releasing the last reference
     * to it, however in some special cases we need to be able to cancel programatically
     * from the backends and notify the frontends by returning a error from a AsyncOp.
     */
    virtual void cancel () {
      throw CancelNotImplementedException();
    }

    /*!
     * Signal that is emitted once the AsyncOp is started
     */
    SignalProxy<void()> sigStarted () {
      return _sigStarted;
    }

    /*!
     * Signal that might be emitted during operation of the AsyncOp,
     * not every AsyncOp will use this
     */
    SignalProxy<void( const std::string & /*text*/, int /*current*/, int /*max*/ )> sigProgress () {
      return _sigProgress;
    }

    /*!
     * Signal that is emitted once the AsyncOp is ready and no
     * callback was registered with \ref onReady
     */
    SignalProxy<void()> sigReady () {
      return _sigReady;
    }

  protected:
    Signal<void()> _sigStarted;
    Signal<void()> _sigReady;
    Signal<void( const std::string & /*text*/, int /*current*/, int /*max*/ )> _sigProgress;
  };



  /*!
   *\class AsyncOp
   * The \a AsyncOp template class is the basic building block for the asynchronous pipelines.
   * Its the base for all async callbacks as well as the async result type. That basically means
   * every pipeline is just a AsyncOp that contains all previous operations that were defined in the pipeline.
   *
   * When implementing a async operation that is to be used in a pipeline it is required to add a operator() to the class taking the
   * input parameter. After the operation is finished the implementation must call setReady(). Calling
   * setReady() must be treated like calling return in a normal function and not execute anymore code on the
   * AsyncOp instance afterwards, since the next operation in the pipeline is allowed to free the previous operation
   * as soon as it gets ready.
   *
   * In case no next operation is defined on the AsyncOp ( when the instance is used as result object ) the return value
   * is cached internally and can be retrieved with \sa get().
   *
   * A async operation can be cancelled by releasing the result object ( the resulting combinatory object ), this will
   * destroy all previous operations that are either running or pending as well. The destructors MUST NOT call setReady() but
   * only release currently running operations.
   */
  template <typename Result>
  struct AsyncOp : public AsyncOpBase {

    using value_type = Result;
    using Ptr = std::shared_ptr<AsyncOp<Result>>;

    AsyncOp () = default;

    AsyncOp ( const AsyncOp &other ) = delete;
    AsyncOp& operator= ( const AsyncOp &other ) = delete;

    AsyncOp& operator= ( AsyncOp &&other ) = default;
    AsyncOp ( AsyncOp &&other ) = default;

    virtual ~AsyncOp(){}

    /*!
     * Sets the async operation ready, in case a callback
     * is registered the \a val is directly forwarded without
     * storing it.
     */
    void setReady ( value_type && val ) {
      if ( _readyCb ) {
        // use a weak reference to know if `this` was deleted by the callback()
        auto weak = weak_from_this();
        _readyCb( std::move( val ) );
        if ( !weak.expired() )
          _readyCb = {};
      }
      else { //we need to cache the value because no callback is available
        _maybeValue = std::move(val);
        _sigReady.emit();
      }
    }

    /*!
     * Checks if the async operation already has finished.
     *
     * \note This can only be used when no callback is registered.
     */
    bool isReady () const {
      return _maybeValue.has_value();
    }

    /*!
     * Registeres a callback that is immediately called
     * when the object gets into ready state. In case the
     * object is in ready state when registering the callback
     * it is called right away.
     * \note this will disable the emitting of sigReady() so it should
     *       be only used by the pipeline implementation
     *
     * \note the callback is removed from the AsyncOp after it was executed
     *       when the AsyncOp becomes ready
     */
    template< typename Fun >
    void onReady ( Fun cb ) {
      this->_readyCb =  std::forward<Fun>(cb);

      if ( isReady() ) {
        // use a weak reference to know if `this` was deleted by the callback()
        auto weak = weak_from_this();

        _readyCb( std::move( _maybeValue.value()) );
        _maybeValue.reset();

        // reset the callback, it might be a lambda with captured ressources
        // that need to be released after returning from the func
        if ( !weak.expired() )
          _readyCb = {};
      }
    }

    /*!
     * Returns the internally cached value if there is one.
     * \throws AsyncOpNotReadyException if called before the operation is ready
     */
    value_type &get (){
      if ( !isReady() )
        ZYPP_THROW(AsyncOpNotReadyException());
      return _maybeValue.value();
    }

  private:
    std::function<void(value_type &&)> _readyCb;
    std::optional<value_type> _maybeValue;
  };

  template <typename T>
  using AsyncOpRef = std::shared_ptr<AsyncOp<T>>;

  namespace detail {

    //A async result that is ready right away
    template <typename T>
    struct ReadyResult : public zyppng::AsyncOp< T >
    {
      ReadyResult( T &&val ) {
        this->setReady( std::move(val) );
      }
    };

#if 0
    template <typename T>
    using has_value_type_t = typename T::value_type;

    template <typename T, typename AsyncRet>
    using is_asyncop_type = std::is_convertible<T*, AsyncOp<AsyncRet>*>;


    template <typename T>
    using is_async_op = typename std::conjunction<
        std::is_detected< has_value_type_t, T >,
        std::is_detected< is_asyncop_type, T, typename T::value_type >
      >;

#else

    /*!
     * Checks if a type T is a asynchronous operation type
     */
    template < typename T, typename = std::void_t<> >
    struct is_async_op : public std::false_type{};

    template < typename T >
    struct is_async_op<T,
      std::void_t< typename T::value_type, //needs to have a value_type
        std::enable_if_t<std::is_convertible< T*, AsyncOp<typename T::value_type>* >::value>>> //needs to be convertible to AsyncOp<T::value_type>
      : public std::true_type{};

#endif

  }

  template <typename T>
  AsyncOpRef<T> makeReadyResult ( T && result ) {
    return std::make_shared<detail::ReadyResult<T>>( std::move(result) );
  }

}



#endif
