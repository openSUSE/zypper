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
*/

#ifndef ZYPP_ZYPPNG_MONADIC_WAIT_H
#define ZYPP_ZYPPNG_MONADIC_WAIT_H

#include <zypp-core/zyppng/pipelines/AsyncResult>

namespace detail {

  template < class AsyncOp,
    class InnerResult = typename AsyncOp::value_type
    >
  struct WaitForImpl : public zyppng::AsyncOp< std::vector<InnerResult> > {

    WaitForImpl () = default;

    WaitForImpl ( const WaitForImpl &other ) = delete;
    WaitForImpl& operator= ( const WaitForImpl &other ) = delete;

    WaitForImpl& operator= ( WaitForImpl &&other ) = default;
    WaitForImpl ( WaitForImpl &&other ) = default;

    void operator()( std::vector< std::unique_ptr< AsyncOp > > &&ops ) {
      assert( _allOps.empty() );

      _allOps = std::move( ops );
      for ( auto &op : _allOps ) {
        op->onReady( [ this ](  typename AsyncOp::value_type &&res  )  {
          this->resultReady( std::move(res));
        });
      }

    }

  private:

    void resultReady ( InnerResult &&res ) {
      _allResults.push_back( std::move( res ) );
      if ( _allOps.size() == _allResults.size() ) {
        //release all ops we waited on
        _allOps.clear();

        this->setReady( std::move(_allResults) );
      }
    }

    std::vector< std::unique_ptr<zyppng::AsyncOp<InnerResult>> > _allOps;
    std::vector< InnerResult > _allResults;
  };

}

/*!
 *  Returns a async operation that waits for all async operations that are passed to it and collects their results,
 *  forwarding them as one
 */
template < class Res  >
auto waitFor () {
  return std::make_unique<detail::WaitForImpl<zyppng::AsyncOp<Res>>>();
}


#endif
