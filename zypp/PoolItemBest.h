/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PoolItemBest.h
 *
*/
#ifndef ZYPP_POOLITEMBEST_H
#define ZYPP_POOLITEMBEST_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Function.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/Hash.h"

#include "zypp/PoolItem.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PoolItemBest
  //
  /** Find the best candidates e.g. in a \ref PoolQuery result.
   *
   * The class basically maintains a \c map<IdString,PoolItem> and remembers
   * for each \c ident (\ref sat::Solvable::ident) the best \ref PoolItem that
   * was added.
   *
   * The default \ref Predicate to determine the best choice is the same that
   * sorts the \ref ui::Selectable list of available objects, thus follows the
   * same rules the \ref resolver will apply.
   *
   * \code
   *   PoolQuery q;
   *   q.addAttribute(sat::SolvAttr::name, "lib*");
   *   q.setMatchGlob();
   *
   *  // get the best matches and tag them for installation:
   *  PoolItemBest bestMatches( q.begin(), q.end() );
   *  if ( ! bestMatches.empty() )
   *  {
   *    for_( it, bestMatches.begin(), bestMatches.end() )
   *    {
   *      ui::asSelectable()( *it )->setOnSystem( *it, ResStatus::USER );
   *    }
   *  }
   * \endcode
   *
   * \todo Support arbitrary Predicates.
   */
  class PoolItemBest
  {
      typedef std::unordered_map<IdString,PoolItem> Container;
    public:
      /** Predicate returning \c True if \a lhs is a better choice. */
      typedef boost::function<bool ( const PoolItem & lhs, const PoolItem & rhs )> Predicate;

      typedef Container::size_type	size_type;
      typedef Container::value_type	value_type;
      typedef MapKVIteratorTraits<Container>::Value_const_iterator	iterator;
      typedef MapKVIteratorTraits<Container>::Key_const_iterator	ident_iterator;

    public:
      /** Default ctor. */
      PoolItemBest()
      {}

      /** Ctor feeding a \ref sat::Solvable. */
      PoolItemBest( sat::Solvable slv_r )
      { _ctor_init(); add( slv_r ); }

      /** Ctor feeding a \ref PoolItem. */
      PoolItemBest( const PoolItem & pi_r )
      { _ctor_init(); add( pi_r ); }

      /** Ctor feeding a range of  \ref sat::Solvable or \ref PoolItem. */
      template<class _Iterator>
      PoolItemBest( _Iterator begin_r, _Iterator end_r )
      { _ctor_init(); add( begin_r, end_r ); }

    public:
      /** Feed one \ref sat::Solvable. */
      void add( sat::Solvable slv_r )
      { add( PoolItem( slv_r ) ); }

      /** Feed one \ref PoolItem. */
      void add( const PoolItem & pi_r );

      /** Feed a range of  \ref sat::Solvable or \ref PoolItem. */
      template<class _Iterator>
      void add( _Iterator begin_r, _Iterator end_r )
      {
        for_( it, begin_r, end_r )
          add( *it );
      }

    public:
      /** \name Iterate the collected PoolItems. */
      //@{
      /** Whether PoolItems were collected. */
      bool empty() const	{ return container().empty(); }
      /** Number of PoolItems collected. */
      size_type size() const	{ return container().size(); }
      /** Pointer to the first PoolItem. */
      iterator begin() const	{ return make_map_value_begin( container() ); }
      /** Pointer behind the last PoolItem. */
      iterator end() const	{ return make_map_value_end( container() ); }

      /** Return the collected \ref PoolItem with \ref sat::Solvable::ident \a ident_r. */
      PoolItem find( IdString ident_r ) const;
      /** \overload Use Solvables ident string. */
      PoolItem find( sat::Solvable slv_r ) const	{ return find( slv_r.ident() ); }
      /** \overload Use PoolItems ident string. */
      PoolItem find( const PoolItem & pi_r ) const	{ return find( pi_r.satSolvable().ident() ); }
      //@}

      /** \name Iterate the collected PoolItems ident strings. */
      //@{
      /** Pointer to the first item. */
      ident_iterator identBegin() const	{ return make_map_key_begin( container() ); }
      /** Pointer behind the last item. */
      ident_iterator identEnd() const	{ return make_map_key_end( container() ); }
      //@}

    private:
      void _ctor_init();
      const Container & container() const;
    private:
      /** Implementation  */
      class Impl;
      /** Pointer to implementation */
      RWCOW_pointer<Impl> & pimpl()             { return *(reinterpret_cast<RWCOW_pointer<Impl>*>( _dont_use_this_use_pimpl.get() )); }
      /** Pointer to implementation */
      const RWCOW_pointer<Impl> & pimpl() const { return *(reinterpret_cast<RWCOW_pointer<Impl>*>( _dont_use_this_use_pimpl.get() )); }
      /** Avoid need to include Impl definition when inlined ctors (due to tepmlate) are provided. */
      shared_ptr<void> _dont_use_this_use_pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PoolItemBest Stream output */
  std::ostream & operator<<( std::ostream & str, const PoolItemBest & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOLITEMBEST_H
