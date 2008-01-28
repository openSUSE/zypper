/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/NVRAD.cc
 *
*/

#include "zypp/NVRAD.h"
#include "zypp/Resolvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
    void copycaps( CapabilitySet & lhs, const Capabilities & rhs )
    {
      if ( rhs.empty() )
        return;
      CapabilitySet( rhs.begin(), rhs.end() ).swap( lhs );
    }
  }

  NVRAD::NVRAD( Resolvable::constPtr res_r )
  {
    if ( res_r )
    {
      *this = NVRAD( res_r->name(), res_r->edition(), res_r->arch() );
#define OUTS(X) copycaps( operator[](X), res_r->dep(X) )
      OUTS( Dep::PROVIDES );
      OUTS( Dep::PREREQUIRES );
      OUTS( Dep::CONFLICTS );
      OUTS( Dep::OBSOLETES );
      OUTS( Dep::FRESHENS );
      OUTS( Dep::REQUIRES );
      OUTS( Dep::RECOMMENDS );
      OUTS( Dep::ENHANCES );
      OUTS( Dep::SUPPLEMENTS );
      OUTS( Dep::SUGGESTS );
#undef OUTS
    }
  }

  std::ostream & operator<<( std::ostream & str, const NVRAD & obj )
  {
    return str << obj.name << '-' << obj.edition << '.' << obj.arch;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
