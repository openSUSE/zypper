/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/pool/GetResolvablesToInsDel.h
 *
*/
#ifndef ZYPP_POOL_GETRESOLVABLESTOINSDEL_H
#define ZYPP_POOL_GETRESOLVABLESTOINSDEL_H

#include <iosfwd>
#include <list>

#include "zypp/ResPool.h"
#include "zypp/APIConfig.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : GetResolvablesToInsDel
    //
    /** Collect transacting items and sort according to prereqs and
     *  media access.
     *
     * \deprecated Use class \ref sat::Transaction which does a better job
     *             esp. when packages are to be deleted.
     */
    struct ZYPP_DEPRECATED GetResolvablesToInsDel
    {
      typedef std::list<PoolItem> PoolItemList;

      /** Influences the sequence of sources and media proscessed.
       * If true prefer a better source, otherwise a better media.
       * \code
       * ORDER_BY_SOURCE:  [S1:1], [S1:2], ... , [S2:1], [S2:2], ...
       * ORDER_BY_MEDIANR: [S1:1], [S2:1], ... , [S1:2], [S2:2], ...
       * \endcode
       */
      enum Order { ORDER_BY_SOURCE, ORDER_BY_MEDIANR };

      /** */
      GetResolvablesToInsDel( ResPool pool_r,
                              Order order_r = ORDER_BY_SOURCE );

      /** Diff with new style \ref Transacrion and write result to log. */
      void debugDiffTransaction() const;

      PoolItemList _toDelete;
      PoolItemList _toInstall;
      PoolItemList _toSrcinstall;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates GetResolvablesToInsDel Stream output */
    std::ostream & operator<<( std::ostream & str, const GetResolvablesToInsDel & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOL_GETRESOLVABLESTOINSDEL_H

