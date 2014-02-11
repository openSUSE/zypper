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
#include <map>
#include <list>
#include <vector>

#include "zypp/base/Iterator.h"
#include "zypp/base/Tr1hash.h"

#include "zypp/PoolItem.h"
#include "zypp/pool/ByIdent.h"
#include "zypp/sat/Pool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class PoolQuery;

  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    class PoolImpl;

    /** Pool internal filter skiping invalid/unwanted PoolItems. */
    struct ByPoolItem
    {
      bool operator()( const PoolItem & pi ) const
      { return bool(pi); }
    };

    /** In CXX0X std::_Select2nd does no longer derive from std::unary_function
     */
    template<typename _Pair>
    struct P_Select2nd : public std::unary_function<_Pair, typename _Pair::second_type>
    {
      typename _Pair::second_type&
      operator()(_Pair& __x) const
      { return __x.second; }

      const typename _Pair::second_type&
      operator()(const _Pair& __x) const
      { return __x.second; }
    };

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PoolTraits
    //
    /** */
    struct PoolTraits
    {
    public:
      typedef sat::detail::SolvableIdType		SolvableIdType;

      /** pure items  */
      typedef std::vector<PoolItem>			ItemContainerT;
      typedef ItemContainerT::const_iterator            item_iterator;
      typedef filter_iterator<ByPoolItem,ItemContainerT::const_iterator>
      							const_iterator;
      typedef ItemContainerT::size_type			size_type;

      /** ident index */
      typedef std::tr1::unordered_multimap<sat::detail::IdType, PoolItem>
                                                        Id2ItemT;
      typedef P_Select2nd<Id2ItemT::value_type>         Id2ItemValueSelector;
      typedef transform_iterator<Id2ItemValueSelector, Id2ItemT::const_iterator>
                                                        byIdent_iterator;

      /** list of known Repositories */
      typedef sat::Pool::RepositoryIterator	        repository_iterator;

#ifdef WITHSOFTLOCKS
      /** soft locks */
      typedef std::tr1::unordered_set<IdString>		AutoSoftLocks;
      typedef AutoSoftLocks::const_iterator             autoSoftLocks_iterator;
#endif

      /** hard locks from etc/zypp/locks */
      typedef std::list<PoolQuery>			HardLockQueries;
      typedef HardLockQueries::const_iterator		hardLockQueries_iterator;

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
