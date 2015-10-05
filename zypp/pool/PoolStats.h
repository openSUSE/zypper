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

#include "zypp/base/Iterator.h"
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
    /** Functor counting ResObjects per Kind.
     * \see dumpPoolStats
     * \code
     * Total: 2830
     *   language:     81
     *   package:      2710
     *   product:      2
     *   selection:    36
     *   system:       1
     * \endcode
    */
    struct PoolStats : public std::unary_function<ResObject::constPtr, void>
    {
      void operator()( ResObject::constPtr ptr )
      {
        ++_total;
        ++_perKind[ptr->kind()];
      }
    public:
      typedef std::map<ResKind,Counter<unsigned> > KindMap;
      Counter<unsigned> _total;
      KindMap           _perKind;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates PoolStats Stream output */
    std::ostream & operator<<( std::ostream & str, const PoolStats & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////

  /** \relates pool::PoolStats Convenience to count and print out the
   *  number of ResObjects per Kind in a container.
   * Fits container of ResObject::Ptr or PoolItem.
  */
  template <class TIterator>
    std::ostream & dumpPoolStats( std::ostream & str,
                                  TIterator begin_r, TIterator end_r )
    {
      pool::PoolStats stats;
      std::for_each( begin_r, end_r,
                     functor::functorRef<void,ResObject::constPtr>(stats) );
      return str << stats;
    }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOL_POOLSTATS_H
