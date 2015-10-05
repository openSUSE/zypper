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

#include "zypp/APIConfig.h"
#include "zypp/base/Iterator.h"

#include "zypp/pool/PoolTraits.h"
#include "zypp/PoolItem.h"
#include "zypp/Filter.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class SerialNumber;
  class ResPoolProxy;
  class Resolver;

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
   * For most (*Begin,*End) iterator-pairs there's also an \ref Iterable
   * provided, so you can use then in range-based for loops:
   * \code
   *   // classic:
   *   for_( it, pool.filterBegin(myfilter), pool.filterEnd(myfilter) )
   *   { ... }
   *
   *   // range based:
   *   for ( const PoolItem & pi : pool.filter(myfilter) )
   *   { ... }
   * \endcode
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
      typedef pool::PoolTraits::repository_iterator      repository_iterator;

    public:
      /** Singleton ctor. */
      static ResPool instance();

      /** preliminary */
      ResPoolProxy proxy() const;

      /** The Resolver */
      Resolver & resolver() const;

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

      /** \name Iterate over all PoolItems (all kinds). */
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
      /** \overload */
      PoolItem find( const ResObject::constPtr & resolvable_r ) const
      { return( resolvable_r ? find( resolvable_r->satSolvable() ) : PoolItem() ); }

    public:
      /** \name Iterate over all PoolItems matching a \c TFilter. */
      //@{
      template<class TFilter>
      filter_iterator<TFilter,const_iterator> filterBegin( const TFilter & filter_r ) const
      { return make_filter_begin( filter_r, *this ); }

      template<class TFilter>
      filter_iterator<TFilter,const_iterator> filterEnd( const TFilter & filter_r ) const
      { return make_filter_end( filter_r, *this ); }

      template<class TFilter>
      Iterable<filter_iterator<TFilter,const_iterator> > filter( const TFilter & filter_r ) const
      { return makeIterable( filterBegin( filter_r ), filterEnd( filter_r ) ); }
      //@}

      /** \name Iterate over all PoolItems by status.
       *
       * Simply pass the \ref ResStatus predicate you want to use as filter:
       * \code
       *   // iterate over all orphaned items:
       *   for_( it, pool.byStatusBegin(&ResStatus::isOrphaned), pool.byStatusEnd(&ResStatus::isOrphaned) )
       *   {...}
       * \endcode
       *
       * Or use \ref filter::ByStatus in more complex queries:
       * \code
       *   // iterate over all (orphaned and recommended) items:
       *   functor::Chain<filter::ByStatus,filter::ByStatus> myfilter( filter::ByStatus(&ResStatus::isOrphaned),
       *                                                               filter::ByStatus(&ResStatus::isRecommended) );
       *   for_( it, pool.filterBegin(myfilter), pool.filterEnd(myfilter) )
       *   { ... }
       * \endcode
       */
      //@{
      filter_iterator<filter::ByStatus,const_iterator> byStatusBegin( const filter::ByStatus & filter_r ) const
      { return make_filter_begin( filter_r, *this ); }

      filter_iterator<filter::ByStatus,const_iterator> byStatusEnd( const filter::ByStatus & filter_r ) const
      { return make_filter_end( filter_r, *this ); }

      Iterable<filter_iterator<filter::ByStatus,const_iterator> > byStatus( const filter::ByStatus & filter_r ) const
      { return makeIterable( byStatusBegin( filter_r ), byStatusEnd( filter_r ) ); }
      //@}

    public:
      /** \name Iterate over all PoolItems of a certain name and kind. */
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

      template<class TRes>
      byIdent_iterator byIdentBegin( IdString name_r ) const
      { return byIdentBegin( ByIdent(ResTraits<TRes>::kind,name_r) ); }

      template<class TRes>
      byIdent_iterator byIdentBegin( const C_Str & name_r ) const
      { return byIdentBegin( ByIdent(ResTraits<TRes>::kind,name_r) ); }

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

      template<class TRes>
      byIdent_iterator byIdentEnd( IdString name_r ) const
      { return byIdentEnd( ByIdent(ResTraits<TRes>::kind,name_r) ); }

      template<class TRes>
      byIdent_iterator byIdentEnd( const C_Str & name_r ) const
      { return byIdentEnd( ByIdent(ResTraits<TRes>::kind,name_r) ); }

      /** Derive name and kind from \ref PoolItem. */
      byIdent_iterator byIdentEnd( const PoolItem & pi_r ) const
      { return byIdentEnd( ByIdent(pi_r.satSolvable()) ); }
      /** Derive name and kind from \ref sat::Solvable. */
      byIdent_iterator byIdentEnd( sat::Solvable slv_r ) const
      { return byIdentEnd( ByIdent(slv_r) ); }
      /** Takes a \ref sat::Solvable::ident string. */
      byIdent_iterator byIdentEnd( IdString ident_r ) const
      { return byIdentEnd( ByIdent(ident_r) ); }


      Iterable<byIdent_iterator> byIdent( const ByIdent & ident_r ) const
      { return makeIterable( byIdentBegin( ident_r ), byIdentEnd( ident_r ) ); }

      Iterable<byIdent_iterator> byIdent( ResKind kind_r, IdString name_r ) const
      { return makeIterable( byIdentBegin( kind_r, name_r ), byIdentEnd(  kind_r, name_r ) ); }

      Iterable<byIdent_iterator> byIdent( ResKind kind_r, const C_Str & name_r ) const
      { return makeIterable( byIdentBegin(  kind_r, name_r ), byIdentEnd(  kind_r, name_r ) ); }

      template<class TRes>
      Iterable<byIdent_iterator> byIdent( IdString name_r ) const
      { return makeIterable( byIdentBegin<TRes>( name_r ), byIdentEnd<TRes>( name_r ) ); }

      template<class TRes>
      Iterable<byIdent_iterator> byIdent( const C_Str & name_r ) const
      { return makeIterable( byIdentBegin<TRes>( name_r ), byIdentEnd<TRes>( name_r ) ); }

      Iterable<byIdent_iterator> byIdent( const PoolItem & pi_r ) const
      { return makeIterable( byIdentBegin( pi_r ), byIdentEnd( pi_r ) ); }

      Iterable<byIdent_iterator> byIdent(sat::Solvable slv_r ) const
      { return makeIterable( byIdentBegin( slv_r ), byIdentEnd( slv_r ) ); }

      Iterable<byIdent_iterator> byIdent( IdString ident_r ) const
      { return makeIterable( byIdentBegin( ident_r ), byIdentEnd( ident_r ) ); }
     //@}

    public:
      /** \name Iterate over all ResObjects of a certain kind. */
      //@{
      typedef filter::ByKind ByKind;
      typedef filter_iterator<ByKind,const_iterator> byKind_iterator;

      byKind_iterator byKindBegin( const ResKind & kind_r ) const
      { return make_filter_begin( ByKind(kind_r), *this ); }

      template<class TRes>
          byKind_iterator byKindBegin() const
      { return make_filter_begin( resfilter::byKind<TRes>(), *this ); }

      byKind_iterator byKindEnd( const ResKind & kind_r ) const
      { return make_filter_end( ByKind(kind_r), *this ); }

      template<class TRes>
          byKind_iterator byKindEnd() const
      { return make_filter_end( resfilter::byKind<TRes>(), *this ); }

      Iterable<byKind_iterator> byKind( const ResKind & kind_r ) const
      { return makeIterable( byKindBegin( kind_r ), byKindEnd( kind_r ) ); }

      template<class TRes>
      Iterable<byKind_iterator> byKind() const
      { return makeIterable( byKindBegin<TRes>(), byKindEnd<TRes>() ); }
      //@}

    public:
      /** \name Iterate over all ResObjects with a certain name (all kinds). */
      //@{
      typedef zypp::resfilter::ByName ByName;
      typedef filter_iterator<ByName,const_iterator> byName_iterator;

      byName_iterator byNameBegin( const std::string & name_r ) const
      { return make_filter_begin( ByName(name_r), *this ); }

      byName_iterator byNameEnd( const std::string & name_r ) const
      { return make_filter_end( ByName(name_r), *this ); }

      Iterable<byName_iterator> byName( const std::string & name_r ) const
      { return makeIterable( byNameBegin( name_r ), byNameEnd( name_r ) ); }
      //@}

    public:
      /** \name Special iterators. */
      //@{

      //@}
   public:
      /** \name Iterate over all Repositories that contribute ResObjects.
       */
      //@{
      size_type knownRepositoriesSize() const;

      repository_iterator knownRepositoriesBegin() const;

      repository_iterator knownRepositoriesEnd() const;

      /** Find a \ref Repository named \c alias_r.
       * Returns \ref Repository::norepository if there is no such \ref Repository.
       */
      Repository reposFind( const std::string & alias_r ) const;

      Iterable<repository_iterator> knownRepositories() const
      { return makeIterable( knownRepositoriesBegin(), knownRepositoriesEnd() ); }
      //@}

    public:
      /** \name Handle locale support.
       *
       * A \ref filter::ByLocaleSupport is provided to iterate over
       * all items supporting a specific locale.
       *
       * \see \ref sat::LocaleSupport for a more convenient interface.
       *
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

      /** Whether this \ref Locale is in the set of requested locales. */
      bool isRequestedLocale( const Locale & locale_r ) const;

      /** Get the set of available locales.
       * This is computed from the package data so it actually
       * represents all locales packages claim to support.
       */
      const LocaleSet & getAvailableLocales() const;

      /** Whether this \ref Locale is in the set of available locales. */
      bool isAvailableLocale( const Locale & locale_r ) const;
      //@}

    public:
      /** \name Handle hard locks (e.g set from /etc/zypp/locks).
       *
       * As this kind of lock is query based, it's quite expensive.
       *
       * These queries are re-evaluated when adding new repos to the pool.
       */
      //@{
      typedef pool::PoolTraits::HardLockQueries           HardLockQueries;
      typedef pool::PoolTraits::hardLockQueries_iterator  hardLockQueries_iterator;

      bool hardLockQueriesEmpty() const;
      size_type hardLockQueriesSize() const;
      hardLockQueries_iterator hardLockQueriesBegin() const;
      hardLockQueries_iterator hardLockQueriesEnd() const;

      Iterable<hardLockQueries_iterator> hardLockQueries() const
      { return makeIterable( hardLockQueriesBegin(), hardLockQueriesEnd() ); }

      /** Set a new set of queries.
       * The hard-locks of existing PoolItems are adjusted according
       * to the queries. (usually called on target load)
       */
      void setHardLockQueries( const HardLockQueries & newLocks_r );

      /** Suggest a new set of queries based on the current selection.
       * (usually remembered on commit).
       */
      void getHardLockQueries( HardLockQueries & activeLocks_r );
      //@}

    private:
      const pool::PoolTraits::ItemContainerT & store() const;
      const pool::PoolTraits::Id2ItemT & id2item() const;

    private:
      /** Ctor */
      ResPool( pool::PoolTraits::Impl_Ptr impl_r );
      /** Access to implementation. */
      RW_pointer<pool::PoolTraits::Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResPool Stream output */
  std::ostream & operator<<( std::ostream & str, const ResPool & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#include "zypp/ResPoolProxy.h"

#endif // ZYPP_RESPOOL_H
