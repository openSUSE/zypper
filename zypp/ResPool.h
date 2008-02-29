/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResPool.h
 *
*/
#ifndef ZYPP_RESPOOL_H
#define ZYPP_RESPOOL_H

#include <iosfwd>

#include "zypp/base/Deprecated.h"
#include "zypp/base/Iterator.h"

#include "zypp/pool/PoolTraits.h"
#include "zypp/PoolItem.h"
#include "zypp/Filter.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class SerialNumber;
  class ResPoolProxy;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResPool
  //
  /** Global ResObject pool.
   *
   * Explicitly shared singleton.
   *
   * \note Filter iterators provided by ResPool are intended to
   * operate on internal index tables for faster access. If the
   * the index is not yet implemented, they are realized as
   * an ordinary filter iterator. Do not provide filter iterators
   * here, if there is no index table for it.
   *
   * \include n_ResPool_nomorenameiter
  */
  class ResPool
  {
    friend std::ostream & operator<<( std::ostream & str, const ResPool & obj );

    public:
      /** \ref PoolItem */
      typedef PoolItem				         value_type;
      typedef pool::PoolTraits::size_type		 size_type;
      typedef pool::PoolTraits::const_iterator	         const_iterator;

      typedef pool::PoolTraits::byCapabilityIndex_iterator byCapabilityIndex_iterator;
      typedef pool::PoolTraits::AdditionalCapabilities	   AdditionalCapabilities;
      typedef pool::PoolTraits::repository_iterator              repository_iterator;

    public:
      /** Singleton ctor. */
      static ResPool instance();

      /** preliminary */
      ResPoolProxy proxy() const;

    public:
      /** The pools serial number. Changing whenever the
       * whenever the content changes. (Resolvables or
       * Dependencies).
       */
      const SerialNumber & serial() const;

    public:
      /**  */
      bool empty() const;
      /**  */
      size_type size() const;

      /** \name Iterate through all PoolItems (all kinds). */
      //@{
      /** */
      const_iterator begin() const
      { return make_filter_begin( pool::ByPoolItem(), store() ); }
      /** */
      const_iterator end() const
      { return make_filter_end( pool::ByPoolItem(), store() ); }
      //@}

    public:
      /** Return the corresponding \ref PoolItem.
       * Pool and sat pool should be in sync. Returns an empty
       * \ref PoolItem if there is no corresponding \ref PoolItem.
       * \see \ref PoolItem::satSolvable.
       */
      PoolItem find( const sat::Solvable & slv_r ) const;

    public:
      /** \name Iterate through all PoolItems matching a \c _Filter. */
      //@{
      template<class _Filter>
      filter_iterator<_Filter,const_iterator> filterBegin( const _Filter & filter_r ) const
      { return make_filter_begin( filter_r, *this ); }

      template<class _Filter>
      filter_iterator<_Filter,const_iterator> filterEnd( const _Filter & filter_r ) const
      { return make_filter_end( filter_r, *this ); }
      //@}

    public:
      /** \name Iterate through all PoolItems of a certain name and kind. */
      //@{
      typedef pool::ByIdent                       ByIdent;
      typedef pool::PoolTraits::byIdent_iterator  byIdent_iterator;

      byIdent_iterator byIdentBegin( const ByIdent & ident_r ) const
      {
	return make_transform_iterator( id2item().equal_range( ident_r.get() ).first,
                                        pool::PoolTraits::Id2ItemValueSelector() );
      }

      byIdent_iterator byIdentBegin( ResKind kind_r, IdString name_r ) const
      { return byIdentBegin( ByIdent(kind_r,name_r) ); }

      byIdent_iterator byIdentBegin( ResKind kind_r, const C_Str & name_r ) const
      { return byIdentBegin( ByIdent(kind_r,name_r) ); }

      template<class _Res>
      byIdent_iterator byIdentBegin( IdString name_r ) const
      { return byIdentBegin( ByIdent(ResTraits<_Res>::kind,name_r) ); }

      template<class _Res>
      byIdent_iterator byIdentBegin( const C_Str & name_r ) const
      { return byIdentBegin( ByIdent(ResTraits<_Res>::kind,name_r) ); }

      /** Derive name and kind from \ref PoolItem. */
      byIdent_iterator byIdentBegin( const PoolItem & pi_r ) const
      { return byIdentBegin( ByIdent(pi_r.satSolvable()) ); }
      /** Derive name and kind from \ref sat::Solvable. */
      byIdent_iterator byIdentBegin( sat::Solvable slv_r ) const
      { return byIdentBegin( ByIdent(slv_r) ); }
      /** Takes a \ref sat::Solvable::ident string. */
      byIdent_iterator byIdentBegin( IdString ident_r ) const
      { return byIdentBegin( ByIdent(ident_r) ); }


      byIdent_iterator byIdentEnd( const ByIdent & ident_r ) const
      {
	return make_transform_iterator( id2item().equal_range( ident_r.get() ).second,
                                        pool::PoolTraits::Id2ItemValueSelector() );
      }

      byIdent_iterator byIdentEnd( ResKind kind_r, IdString name_r ) const
      { return byIdentEnd( ByIdent(kind_r,name_r) ); }

      byIdent_iterator byIdentEnd( ResKind kind_r, const C_Str & name_r ) const
      { return byIdentEnd( ByIdent(kind_r,name_r) ); }

      template<class _Res>
      byIdent_iterator byIdentEnd( IdString name_r ) const
      { return byIdentEnd( ByIdent(ResTraits<_Res>::kind,name_r) ); }

      template<class _Res>
      byIdent_iterator byIdentEnd( const C_Str & name_r ) const
      { return byIdentEnd( ByIdent(ResTraits<_Res>::kind,name_r) ); }

      /** Derive name and kind from \ref PoolItem. */
      byIdent_iterator byIdentEnd( const PoolItem & pi_r ) const
      { return byIdentEnd( ByIdent(pi_r.satSolvable()) ); }
      /** Derive name and kind from \ref sat::Solvable. */
      byIdent_iterator byIdentEnd( sat::Solvable slv_r ) const
      { return byIdentEnd( ByIdent(slv_r) ); }
      /** Takes a \ref sat::Solvable::ident string. */
      byIdent_iterator byIdentEnd( IdString ident_r ) const
      { return byIdentEnd( ByIdent(ident_r) ); }
     //@}

    public:
      /** \name Iterate through all ResObjects of a certain kind. */
      //@{
      typedef zypp::resfilter::ByKind ByKind;
      typedef filter_iterator<ByKind,const_iterator> byKind_iterator;

      byKind_iterator byKindBegin( const ResKind & kind_r ) const
      { return make_filter_begin( ByKind(kind_r), *this ); }

      template<class _Res>
          byKind_iterator byKindBegin() const
      { return make_filter_begin( resfilter::byKind<_Res>(), *this ); }

      byKind_iterator byKindEnd( const ResKind & kind_r ) const
      { return make_filter_end( ByKind(kind_r), *this ); }

      template<class _Res>
          byKind_iterator byKindEnd() const
      { return make_filter_end( resfilter::byKind<_Res>(), *this ); }
      //@}

    public:
      /** \name Iterate through all ResObjects with a certain name (all kinds).
       * \deprecated Instead of iterating byName and filter byKind use ByIdent iterator.
      */
      //@{
      typedef zypp::resfilter::ByName ByName;
      typedef filter_iterator<ByName,const_iterator> byName_iterator;

      byName_iterator ZYPP_DEPRECATED byNameBegin( const std::string & name_r ) const
      { return make_filter_begin( ByName(name_r), *this ); }

      byName_iterator ZYPP_DEPRECATED byNameEnd( const std::string & name_r ) const
      { return make_filter_end( ByName(name_r), *this ); }
      //@}

    public:
      /** \name Iterate through all ResObjects which have at least
       *  one Capability with index \a index_r in dependency \a depType_r.
       *
       * \deprecated If you're looking for providers of a certain capability
       * use \ref sat::WhatProvides. That's currently the only index provided.
       */
      //@{
      byCapabilityIndex_iterator ZYPP_DEPRECATED byCapabilityIndexBegin( const std::string & index_r, Dep depType_r ) const;

      byCapabilityIndex_iterator ZYPP_DEPRECATED byCapabilityIndexEnd( const std::string & index_r, Dep depType_r ) const;
      //@}

    public:
      /** \name Iterate through all Repositories that contribute ResObjects.
       */
      //@{
      size_type knownRepositoriesSize() const;

      repository_iterator knownRepositoriesBegin() const;

      repository_iterator knownRepositoriesEnd() const;
      //@}

     public:
      /** \name Handle locale support.
       *
       * A \ref filter::ByLocaleSupportort is provided to iterate over
       * all items supporting a specific locale.
       * \code
       * ResPool pool( ResPool::instance() );
       *
       * filter::ByLocaleSupport f( Locale("de") );
       * for_( it, pool.filterBegin(f), pool.filterEnd(f) )
       * {
       *   MIL << *it << endl; // supporting "de"
       * }
       *
       * f = filter::ByLocaleSupport( pool.getRequestedLocales() );
       * for_( it, pool.filterBegin(f), pool.filterEnd(f) )
       * {
       *   MIL << *it << endl; // supporting any requested locale
       * }
       *
       * \endcode
       */
      //@{
      /** Set the requested locales.
       * Languages to be supported by the system, e.g. language specific
       * packages to be installed.
       */
      void setRequestedLocales( const LocaleSet & locales_r );

      /** Add one \ref Locale to the set of requested locales.
       * Return \c true if \c locale_r was newly added to the set.
      */
      bool addRequestedLocale( const Locale & locale_r );

      /** Erase one \ref Locale from the set of requested locales.
      * Return \c false if \c locale_r was not found in the set.
       */
      bool eraseRequestedLocale( const Locale & locale_r );

      /** Return the requested locales.
       * \see \ref setRequestedLocales
      */
      const LocaleSet & getRequestedLocales() const;

      /** Wheter this \ref Locale is in the set of requested locales. */
      bool isRequestedLocale( const Locale & locale_r ) const;

      /** Get the set of available locales.
       * This is computed from the package data so it actually
       * represents all locales packages claim to support.
       */
      const LocaleSet & getAvailableLocales() const;

      /** Wheter this \ref Locale is in the set of available locales. */
      bool isAvailableLocale( const Locale & locale_r ) const;
      //@}

   public:
      /** \name Handling addition capabilities in the pool in order for solving it in
       *  a solver run. This is used for tasks like needing a package with the name "foo".
       *  The solver has to evaluate a proper package by his own.
       *
       *  CAUTION: This has another semantic in the solver. The required resolvable has
       *  been set for installation (in the pool) only AFTER a solver run.
       */

      /**
       *  Handling additional requirement. E.G. need package "foo" and package
       *  "foo1" which has a greater version than 1.0:
       *
       *  \code
       *  Capabilities capset;
       *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo"));
       *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo1 > 1.0"));
       *
       *  // The user is setting this capablility
       *  ResPool::AdditionalCapabilities aCapabilities;
       *  aCapabilities[ResStatus::USER] = capset;
       *
       *  setAdditionalRequire( aCapabilities );
       *  \endcode
       */
      void setAdditionalRequire( const AdditionalCapabilities & capset ) const;
      AdditionalCapabilities & additionalRequire() const;

     /**
       *  Handling additional conflicts. E.G. do not install anything which provides "foo":
       *
       *  \code75
       *  Capabilities capset;
       *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo"));
       *
       *  // The user is setting this capablility
       *  ResPool::AdditionalCapabilities aCapabilities;
       *  aCapabilities[ResStatus::USER] = capset;
       *
       *  setAdditionalConflict( aCapabilities );
       *  \endcode
       */
      void setAdditionalConflict( const AdditionalCapabilities & capset ) const;
      AdditionalCapabilities & additionaConflict() const;

     /**
       *  Handling additional provides. This is used for ignoring a requirement.
       *  e.G. Do ignore the requirement "foo":
       *
       *  \code
       *  Capabilities capset;
       *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo"));
       *
       *  // The user is setting this capablility
       *  ResPool::AdditionalCapabilities aCapabilities;
       *  aCapabilities[ResStatus::USER] = capset;
       *
       *  setAdditionalProvide( aCapabilities );
       *  \endcode
       */
      void setAdditionalProvide( const AdditionalCapabilities & capset ) const;
      AdditionalCapabilities & additionaProvide() const;

    private:
      const pool::PoolTraits::ItemContainerT & store() const;
      const pool::PoolTraits::Id2ItemT & id2item() const;

    private:
      /** Ctor */
      ResPool( pool::PoolTraits::Impl_constPtr impl_r );
      /** Const access to implementation. */
      pool::PoolTraits::Impl_constPtr _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResPool Stream output */
  std::ostream & operator<<( std::ostream & str, const ResPool & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#include "zypp/ResPoolProxy.h"

#endif // ZYPP_RESPOOL_H
