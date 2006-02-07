/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ResolvableImpl.cc
 *
*/
#include <iostream>

#include "zypp/base/Algorithm.h"
#include "zypp/detail/ResolvableImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
    struct FilterUnwantedReq
    {
      bool operator()( const Capability & cap_r ) const
      {
        return cap_r.index().substr( 0, 7 ) == "rpmlib(";
      }
    };

    void filterUnwantedReq( const CapSet & from, CapSet & to )
    {
      to.clear();
      std::remove_copy_if( from.begin(), from.end(),
                           std::inserter( to, to.end() ),
                           FilterUnwantedReq() );
    }
  }

  Resolvable::Impl::Impl( const Kind & kind_r,
                          const NVRAD & nvrad_r )
  : _kind( kind_r )
  , _name( nvrad_r.name )
  , _edition( nvrad_r.edition )
  , _arch( nvrad_r.arch )
  , _deps( nvrad_r )
  {
    if (_arch != Arch_src) {
    // assert self provides
    _deps[Dep::PROVIDES].insert( CapFactory()
                                 .parse( _kind, _name, Rel::EQ, _edition ) );
    }
    // Filter 'rpmlib(...)' requirements (refill from nvrad_r)
    filterUnwantedReq( nvrad_r[Dep::PREREQUIRES], _deps[Dep::PREREQUIRES] );
    filterUnwantedReq( nvrad_r[Dep::REQUIRES], _deps[Dep::REQUIRES] );

    // assert all prerequires are in requires too
    _deps[Dep::REQUIRES].insert( _deps[Dep::PREREQUIRES].begin(),
                                 _deps[Dep::PREREQUIRES].end() );

  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
