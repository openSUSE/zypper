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
#ifndef ZYPPNG_MONADIC_AWAIT_H_INCLUDED
#define ZYPPNG_MONADIC_AWAIT_H_INCLUDED


#include <zypp-core/zyppng/pipelines/AsyncResult>
#include <zypp-core/zyppng/base/Signals>

namespace zyppng {

  namespace detail
  {
  template <typename T, typename SigGetter> struct AwaitImpl;

  template <typename ArgType, typename SigR, typename ...SigT >
  struct AwaitImpl<std::shared_ptr<ArgType>, SignalProxy<SigR(SigT...)> (ArgType::*)()> : public zyppng::AsyncOp< std::shared_ptr<ArgType> > {

    using SigGetter = SignalProxy<SigR(SigT...)> (ArgType::*)();

    template<typename S>
    AwaitImpl ( S&& sigGet ) : _sigGet( std::forward<S>(sigGet ) ) {}

    virtual ~AwaitImpl(){ }

    void operator() ( std::shared_ptr<ArgType> &&req ) {
      _req = std::move(req);
      std::invoke( _sigGet, _req ).connect( sigc::mem_fun(this, &AwaitImpl::sigHandler<SigR>) );
    }

  private:
    template < typename RetType = SigR >
    std::enable_if_t< std::is_same<void,RetType>::value, RetType >
    sigHandler ( SigT... ) {
      this->setReady( std::move(_req) );
    }

    template < typename RetType = SigR >
    std::enable_if_t< !std::is_same<void,RetType>::value, RetType >
    sigHandler ( SigT... ) {
      this->setReady( std::move(_req) );
      return {};
    }

    std::shared_ptr<ArgType> _req;
    SigGetter _sigGet;
  };
  }

  //return a async op that waits for a signal to emitted by a object
  template <typename T,
            typename SignalGetter >
  auto await ( SignalGetter &&sigGet )
  {
    return  std::make_shared<detail::AwaitImpl<T, SignalGetter>>( std::forward<SignalGetter>(sigGet) );
  }

}

#endif
