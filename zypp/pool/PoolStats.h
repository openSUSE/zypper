/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/pool/PoolStats.h
 *
*/
#ifndef ZYPP_POOL_POOLSTATS_H
#define ZYPP_POOL_POOLSTATS_H

#include <iosfwd>

#include "zypp/base/Functional.h"
#include "zypp/base/Counter.h"
#include "zypp/ResObject.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PoolStats
    //
    /** */
    struct PoolStats : public std::unary_function<ResObject::constPtr, void>
    {
      void operator()( ResObject::constPtr ptr )
      {
        ++_total;
        ++_perKind[ptr->kind()];
      }
    public:
      typedef std::map<ResolvableTraits::KindType,Counter<unsigned> > KindMap;
      Counter<unsigned> _total;
      KindMap           _perKind;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates PoolStats Stream output */
    std::ostream & operator<<( std::ostream & str, const PoolStats & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOL_POOLSTATS_H
