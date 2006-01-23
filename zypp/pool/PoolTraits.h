/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/pool/PoolTraits.h
 *
*/
#ifndef ZYPP_POOL_POOLTRAITS_H
#define ZYPP_POOL_POOLTRAITS_H

#include <set>

#include "zypp/pool/PoolItem.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    class PoolImpl;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PoolTraits
    //
    /** */
    struct PoolTraits
    {
    public:
      /** */
      typedef PoolItem                   Item;
      typedef std::set<Item>             ContainerT;
      typedef ContainerT::size_type      size_type;
      typedef ContainerT::iterator       iterator;
      typedef ContainerT::const_iterator const_iterator;

      typedef PoolImpl                   Impl;
      typedef shared_ptr<PoolImpl>       Impl_Ptr;
      typedef shared_ptr<const PoolImpl> Impl_constPtr;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOL_POOLTRAITS_H
