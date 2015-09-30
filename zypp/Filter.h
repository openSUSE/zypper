/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Filter.h
 *
*/
#ifndef ZYPP_FILTER_H
#define ZYPP_FILTER_H

#include <iosfwd>

#include "zypp/base/Functional.h"
#include "zypp/base/Function.h"
// #include "zypp/ResFilters.h"  included at the end!
#include "zypp/sat/Pool.h"
#include "zypp/PoolItem.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace filter
  { /////////////////////////////////////////////////////////////////
    /** \defgroup POOLFILTER Collection solvable filter functors.
     *
     * All functors should be able to process \ref Solvable as well
     * as \ref PoolItem.
     *
     * \code
     *   // The same filter...
     *   filter::ByLocaleSupport f( Locale("de") );
     *
     *   // ...can be used to iterate the sat::Pool...
     *   sat::Pool satpool( sat::Pool::instance() );
     *   for_( it, satpool.filterBegin(f), satpool.filterEnd(f) )
     *   {
     *     MIL << *it << endl; // prints sat::Solvable
     *   }
     *
     *   // ...as well as the ResPool.
     *   ResPool   pool   ( ResPool::instance() );
     *   for_( it, pool.filterBegin(f), pool.filterEnd(f) )
     *   {
     *     MIL << *it << endl; // prints PoolItem
     *   }
     * \endcode
     * \ingroup g_Functor
     */
    //@{

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ByLocaleSupport
    //
    /** Filter solvables according to their locale support.
    */
    class ByLocaleSupport
    {
      private:
        typedef bool (sat::Solvable::*LS1) (const Locale &) const;
        typedef bool (sat::Solvable::*LS2) (const LocaleSet &) const;

      public:
        /** Solvables with locale support. */
        ByLocaleSupport()
        : _sel( boost::mem_fun_ref( &sat::Solvable::supportsLocales ) )
        {}

        /** Solvables supporting \c locale_r. */
        explicit ByLocaleSupport( const Locale & locale_r )
        : _sel( boost::bind( boost::mem_fun_ref( (LS1)&sat::Solvable::supportsLocale ), _1, locale_r ) )
        {}

        /** Solvables supporting at least one locale in \c locales_r. */
        explicit ByLocaleSupport( const LocaleSet & locales_r )
        : _sel( boost::bind( boost::mem_fun_ref( (LS2)&sat::Solvable::supportsLocale ), _1, locales_r ) )
        {}

      public:
        /** Filter on \ref Solvable. */
        bool operator()( const sat::Solvable & solv_r ) const
        { return _sel && _sel( solv_r ); }

        /** Filter fitting PoolItem/ResObject. */
        template<class TSolv>
        bool operator()( const TSolv & solv_r ) const
        { return operator()( solv_r.satSolvable() ); }

      private:
        function<bool(const sat::Solvable &)> _sel;
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ByKind
    //
    /** Filter solvables according to their kind.
    */
    class ByKind
    {
      public:
        ByKind()
        {}

        ByKind( const ResKind & kind_r )
        : _kind( kind_r )
        {}

      public:
        /** Filter on \ref Solvable. */
        bool operator()( const sat::Solvable & solv_r ) const
        { return solv_r.isKind( _kind ); }

        /** Filter fitting PoolItem/ResObject. */
        template<class TSolv>
        bool operator()( const TSolv & solv_r ) const
        { return operator()( solv_r.satSolvable() ); }

      private:
        ResKind _kind;
    };

    /** \relates ByKind templated convenience ctor. */
    template<class TRes>
    inline ByKind byKind()
    { return ByKind( ResTraits<TRes>::kind ); }
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ByStatus
    //
    /** Filter solvables according to their status.
    */
    class ByStatus
    {
      public:
        typedef bool (ResStatus::*Predicate)() const;

      public:
        ByStatus( Predicate pred_r = 0 )
        : _pred( pred_r )
        {}

      public:
        /** Filter on \ref PoolItem. */
        bool operator()( const PoolItem & pi_r ) const
        { return _pred && (pi_r.status().*_pred)(); }

        /** Filter fitting sat::Solvable/ResObject. */
        template<class TSolv>
        bool operator()( const TSolv & solv_r ) const
        { return operator()( PoolItem(solv_r) ); }

      private:
        Predicate _pred;
    };
    ///////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SameItemAs
    //
    /** Filter items with at least same NVRA, vendor.
     * This is usually used to find available packages
     * that matche an insytalled one.
    */
    class SameItemAs
    {
      public:
        SameItemAs( const sat::Solvable & solv_r )
        : _item( solv_r )
        {}

        /** Fitting PoolItem/ResObject. */
        template<class TSolv>
        SameItemAs( const TSolv & solv_r )
        : _item( solv_r.satSolvable() )
        {}

      public:
        /** Filter on \ref Solvable. */
        bool operator()( const sat::Solvable & solv_r ) const
        {
          return solv_r.name()    == _item.name()
              && solv_r.edition() == _item.edition()
              && solv_r.arch()    == _item.arch()
              && solv_r.vendor()  == _item.vendor();
        }

        /** Filter fitting PoolItem/ResObject. */
        template<class TSolv>
        bool operator()( const TSolv & solv_r ) const
        { return operator()( solv_r.satSolvable() ); }

      private:
        sat::Solvable _item;
    };
    ///////////////////////////////////////////////////////////////////
    //@}

  /////////////////////////////////////////////////////////////////
  } // namespace filter
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#include "zypp/ResFilters.h"

#endif // ZYPP_FILTER_H
