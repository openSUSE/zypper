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

    /** Pool internal filter skiping invalid/unwanted PoolItems. */
    struct ByPoolItem
    {
      bool operator()( const PoolItem & pi ) const
      { return pi; }
    };

    /** Main filter selecting PoolItems bu \c name and \c kind.
     *
    */
    class ByIdent
    {
      public:
        ByIdent( sat::Solvable slv_r )
        : _id( makeIdent( slv_r ) )
        {}

        ByIdent( IdString ident_r )
        : _id( ident_r.id() )
        {}

        ByIdent( ResKind kind_r, IdString name_r )
        : _id( makeIdent( kind_r, name_r ) )
        {}

        ByIdent( ResKind kind_r, const C_Str & name_r )
        : _id( makeIdent( kind_r, name_r ) )
        {}

      public:
        bool operator()( sat::Solvable slv_r ) const
        {
          return _id >= 0 ? ( slv_r.ident().id() == _id && ! slv_r.isKind( ResKind::srcpackage ) )
                          : ( slv_r.ident().id() == -_id && slv_r.isKind( ResKind::srcpackage ) );
        }

        bool operator()( const PoolItem & pi_r ) const
        { return operator()( pi_r.satSolvable() ); }

        bool operator()( ResObject::constPtr p_r ) const
        { return p_r ? operator()( p_r->satSolvable() ) : !_id; }

      private:
        sat::detail::IdType makeIdent( sat::Solvable slv_r )
        {
          return slv_r.isKind( ResKind::srcpackage ) ? -slv_r.ident().id()
                                                     : slv_r.ident().id();
        }

        sat::detail::IdType makeIdent( ResKind kind_r, IdString name_r )
        {
          if ( kind_r == ResKind::package )
            return name_r.id();
          else if ( kind_r == ResKind::srcpackage )
            return -name_r.id();
          return IdString( str::form( "%s:%s", kind_r.c_str(), name_r.c_str() ) ).id();
        }

        sat::detail::IdType makeIdent( ResKind kind_r, const C_Str & name_r )
        {
          if ( kind_r == ResKind::package )
            return IdString( name_r ).id();
          else if ( kind_r == ResKind::srcpackage )
            return -(IdString( name_r ).id());
          return IdString( str::form( "%s:%s", kind_r.c_str(), name_r.c_str() ) ).id();
        }

      public:
        sat::detail::IdType get() const { return _id; }

      private:
        /** negative \c _id for \c srcpackage, as they use the same \c ident
         * as \c package.
         */
        sat::detail::IdType _id;
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
#if 0
       typedef std::map<sat::Solvable,PoolItem>		ItemContainerT;
       typedef MapKVIteratorTraits<ItemContainerT>::Value_const_iterator
       							const_iterator;
#endif
      typedef std::vector<PoolItem>			ItemContainerT;
      typedef ItemContainerT::const_iterator            item_iterator;
      typedef filter_iterator<ByPoolItem,ItemContainerT::const_iterator>
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
      typedef std::list<Repository>                     RepoContainerT;
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
