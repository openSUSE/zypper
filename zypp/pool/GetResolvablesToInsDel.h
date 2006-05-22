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
     */
    struct GetResolvablesToInsDel
    {
      typedef std::list<PoolItem_Ref> PoolItemList;

      /** */
      GetResolvablesToInsDel( ResPool pool_r );

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
