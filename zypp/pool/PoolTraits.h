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

#include "zypp/base/Iterator.h"

#include "zypp/PoolItem.h"
#include "zypp/sat/Pool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class CapAndItem;
  class Repository;

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
      typedef sat::detail::SolvableIdType		SolvableIdType;
      /** pure items  */
      typedef std::map<sat::Solvable,PoolItem>		ItemContainerT;
      typedef MapKVIteratorTraits<ItemContainerT>::Value_const_iterator
          						const_iterator;
      typedef ItemContainerT::size_type			size_type;

      // internal organization
      typedef std::list<zypp::CapAndItem>		CapItemContainerT;	// (why,who) pairs
      typedef std::map<std::string,CapItemContainerT>	CapItemStoreT;		// capability.index -> (why,who) pairs
      typedef std::map<Dep,CapItemStoreT>		DepCapItemContainerT;	// Dep -> (capability.index -> (why,who) pairs)

      typedef CapItemContainerT::iterator		capitemiterator;
      typedef CapItemContainerT::const_iterator		const_capitemiterator;
      typedef CapItemContainerT::size_type		capitemsize_type;
      /** hashed by capability index */
      typedef const_capitemiterator                     byCapabilityIndex_iterator;

      /* list of known Repositories */
      typedef std::set<Repository>                      RepoContainerT;
      typedef RepoContainerT::const_iterator		repository_iterator;

      typedef PoolImpl                   Impl;
      typedef shared_ptr<PoolImpl>       Impl_Ptr;
      typedef shared_ptr<const PoolImpl> Impl_constPtr;

      /** Map of Capabilities and "who" has set it*/
      typedef std::map<ResStatus::TransactByValue,Capabilities>		AdditionalCapabilities;

    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOL_POOLTRAITS_H
