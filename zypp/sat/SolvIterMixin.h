/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/SolvIterMixin.h
 *
*/
#ifndef ZYPP_SAT_SOLVITERMIXIN_H
#define ZYPP_SAT_SOLVITERMIXIN_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/Hash.h"

#include "zypp/sat/Solvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class PoolItem;
  class asPoolItem; // transform functor

  namespace ui
  {
    class asSelectable; // transform functor
  }

  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    class Solvable;
    class asSolvable; // transform functor

    namespace solvitermixin_detail
    {
      /** Unify by \c ident \c (kind:name).
       * Return true on the 1st appearance of a new \c ident. This is
       * used in \ref SolvIterMixin when mapping a  Solvable iterator
       * to a Selectable iterator.
      */
      struct UnifyByIdent
      {
        bool operator()( const Solvable & solv_r ) const;

        typedef std::unordered_set<unsigned> Uset;
        UnifyByIdent()
          : _uset( new Uset )
        {}
        shared_ptr<Uset> _uset;
      };


    } // namespace solvitermixin_detail


    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SolvIterMixin<Derived,DerivedSolvable_iterator>
    //
    /** Base class providing common iterator types based on a \ref Solvable iterator.
     *
     * A class deriving from \ref SolvIterMixin must provide two methods
     * \c begin and \c end returning iterator over \ref sat::Solvable.
     *
     * \ref SolvIterMixin will then provide iterators over the corresponding
     * \ref PoolItem and \ref ui::Selectable_Ptr.
     *
     * \ref SolvIterMixin will also provide default implementations for \ref empty
     * and \ref size by iterating from \c begin to \c end. In case \c Derived is
     * able to provide a more efficient implementation, the methods should be overloaded.
     *
     * \note You will sometimes face the problem, that when using the \ref PoolItem
     * iterator you hit multiple version of the same package, while when using the
     * \ref ui::Selectable iterator the information which of the available candidates
     * actually matched got lost. In this case class \ref PoolItemBest may help you.
     * Use it to pick the best version only.
     *
     * \code
     *     namespace detail
     *     {
     *       class WhatProvidesIterator;
     *     }
     *
     *     class WhatProvides : public SolvIterMixin<WhatProvides,detail::WhatProvidesIterator>
     *     {
     *       public:
     *         typedef detail::WhatProvidesIterator const_iterator;
     *
     *         // Iterator pointing to the first Solvable.
     *         const_iterator begin() const;
     *
     *         // Iterator pointing behind the last Solvable.
     *         const_iterator end() const;
     *
     *     };
     *
     *     namespace detail
     *     {
     *       class WhatProvidesIterator : public boost::iterator_adaptor<
     *           WhatProvidesIterator          // Derived
     *          , const detail::IdType *       // Base
     *          , const Solvable               // Value
     *          , boost::forward_traversal_tag // CategoryOrTraversal
     *          , const Solvable               // Reference
     *       >
     *       {
     *          ...
     *       };
     *     }
     * \endcode
     * \ingroup g_CRTP
     */
    template <class Derived,class DerivedSolvable_iterator>
    class SolvIterMixin
    {
      public:
	typedef size_t size_type;

      public:
        /** \name Convenience methods.
         * In case \c Derived is able to provide a more efficient implementation,
	 * the methods should be overloaded.
	 */
        //@{
	/** Whether the collection is epmty. */
        bool empty() const
        { return( self().begin() == self().end() ); }

        /** Size of the collection. */
        size_type size() const
        { size_type s = 0; for_( it, self().begin(), self().end() ) ++s; return s;}

	/** Whether collection contains a specific \ref Solvable. */
	template<class TSolv>
	bool contains( const TSolv & solv_r ) const
	{
	  Solvable solv( asSolvable()( solv_r ) );
	  for_( it, self().begin(), self().end() )
	    if ( *it == solv )
	      return true;
	  return false;
	}
	//@}

      public:
        /** \name Iterate as Solvable */
        //@{
        typedef  DerivedSolvable_iterator Solvable_iterator;
        Solvable_iterator solvableBegin() const
        { return self().begin(); }
        Solvable_iterator solvableEnd() const
        { return self().end(); }
	Iterable<Solvable_iterator> solvable() const
	{ return makeIterable( solvableBegin(), solvableEnd() ); }
        //@}

        /** \name Iterate as PoolItem */
        //@{
        typedef transform_iterator<asPoolItem,Solvable_iterator> PoolItem_iterator;
        PoolItem_iterator poolItemBegin() const
        { return make_transform_iterator( solvableBegin(), asPoolItem() ); }
        PoolItem_iterator poolItemEnd() const
        { return make_transform_iterator( solvableEnd(), asPoolItem() ); }
	Iterable<PoolItem_iterator> poolItem() const
	{ return makeIterable( poolItemBegin(), poolItemEnd() ); }
        //@}

      private:
        typedef filter_iterator<solvitermixin_detail::UnifyByIdent,Solvable_iterator> UnifiedSolvable_iterator;
      public:
        /** \name Iterate ui::Selectable::Ptr */
        //@{
        typedef transform_iterator<ui::asSelectable,UnifiedSolvable_iterator> Selectable_iterator;
        Selectable_iterator selectableBegin() const
        { return make_transform_iterator( unifiedSolvableBegin(), ui::asSelectable() ); }
        Selectable_iterator selectableEnd() const
        { return make_transform_iterator( unifiedSolvableEnd(), ui::asSelectable() ); }
	Iterable<Selectable_iterator> selectable() const
	{ return makeIterable( selectableBegin(), selectableEnd() ); }
        //@}

      private:
        /** \name Iterate unified Solbvables to be transformed into Selectable. */
        //@{
        UnifiedSolvable_iterator unifiedSolvableBegin() const
        { return make_filter_iterator( solvitermixin_detail::UnifyByIdent(), solvableBegin(), solvableEnd() ); }
        UnifiedSolvable_iterator unifiedSolvableEnd() const
        { return make_filter_iterator( solvitermixin_detail::UnifyByIdent(), solvableEnd(), solvableEnd() ); }
	Iterable<UnifiedSolvable_iterator> unifiedSolvable() const
	{ return makeIterable( unifiedSolvableBegin(), unifiedSolvableEnd() ); }
        //@}
      private:
        const Derived & self() const
        { return *static_cast<const Derived*>( this ); }
      protected:
        SolvIterMixin() {}
        ~SolvIterMixin() {}
        SolvIterMixin(const SolvIterMixin &) {}
        void operator=(const SolvIterMixin &) {}
     };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_SOLVITERMIXIN_H
