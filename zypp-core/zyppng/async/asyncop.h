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


#include <zypp-core/base/Exception.h>
#include <zypp-core/zyppng/base/Signals>
#include <boost/optional.hpp>
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

  AsyncOpNotReadyException::~AsyncOpNotReadyException()
  { }

  /*!
   *\class AsyncOp
   * The \a AsyncOp template class is the basic building block for the asynchronous pipelines.
   * Its the base for all async callbacks as well as the async result type. That basically means
   * every pipeline is just a AsyncOp that contains all previous operations that were defined in the pipeline.
   *
   * When implementing a async operation it is required to add a operator() to the class taking the
   * input parameter. After the operation is finished the implementation must call setReady(). Calling
   * setReady() must be treated like calling return in a normal function and not execute anymore code on the
   * AsyncOp instance afterwards, since the next operation in the pipeline is allowed to free the previous operation
   * as soon as it gets ready.
   *
   * In case no next operation is defined on the AsyncOp ( when the instance is used as result object ) the return value
   * is cached internally and can be retrieved with \sa get().
   *
   * A async operation can be cancelled by releasing the result object ( the resulting combinatory object ), this will
   * destroy all previous operations that are either running or pending as well.
   */
  template <typename Result>
  struct AsyncOp {

    using value_type = Result;

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
      if ( _readyCb )
        _readyCb( std::move( val ) );
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
      return _maybeValue.is_initialized();
    }

    /*!
     * Registeres a callback that is immediately called
     * when the object gets into ready state. In case the
     * object is in ready state when registering the callback
     * it is called right away.
     */
    template< typename Fun >
    void onReady ( Fun cb ) {
      this->_readyCb =  std::forward<Fun>(cb);

      if ( isReady() ) {
        _readyCb( std::move( _maybeValue.get()) );
        _maybeValue = boost::optional<value_type>();
      }
    }

    /*!
     * Returns the internally cached value if there is one.
     * \throws AsyncOpNotReadyException if called before the operation is ready
     */
    value_type &get (){
      if ( !isReady() )
        ZYPP_THROW(AsyncOpNotReadyException());
      return _maybeValue.get();
    }

    /*!
     * Signal that is emitted once the AsyncOp is ready and no
     * callback was registered with \ref onReady
     */
    SignalProxy<void()> sigReady () {
      return _sigReady;
    }

  private:
    Signal<void()> _sigReady;
    std::function<void(value_type &&)> _readyCb;
    boost::optional<value_type> _maybeValue;
  };

  template <typename T>
  using AsyncOpPtr = std::unique_ptr<AsyncOp<T>>;

  namespace detail {

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

}



#endif
