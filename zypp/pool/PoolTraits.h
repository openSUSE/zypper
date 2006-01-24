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

    /**  */
    struct PoolImplInserter
    {
      void operator()( ResObject::constPtr ptr_r );

      PoolImplInserter( PoolImpl & poolImpl_r )
      : _poolImpl( poolImpl_r )
      {}
      PoolImpl & _poolImpl;
    };

    /**  */
    struct PoolImplDeleter
    {
      void operator()( ResObject::constPtr ptr_r );

      PoolImplDeleter( PoolImpl & poolImpl_r )
      : _poolImpl( poolImpl_r )
      {}
      PoolImpl & _poolImpl;
    };

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
      typedef PoolImplInserter           Inserter;
      typedef PoolImplDeleter            Deleter;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOL_POOLTRAITS_H
