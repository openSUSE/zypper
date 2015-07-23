/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PoolQueryResult.h
 *
*/
#ifndef ZYPP_POOLQUERYRESULT_H
#define ZYPP_POOLQUERYRESULT_H

#include <iosfwd>

#include "zypp/base/Hash.h"
#include "zypp/base/Exception.h"
#include "zypp/sat/SolvIterMixin.h"

#include "zypp/PoolItem.h"
#include "zypp/PoolQuery.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PoolQueryResult
  //
  /** Helper class to collect (not only) \ref PoolQuery results.
   *
   * \note Unfortunately \ref PoolQuery::begin might throw. Exceptions
   * are caught and the query is treated as empty.
   *
   * \ref PoolQueryResult maintains a set of \ref sat::Solvable. You can
   * add/remove solvables to/from the set defined by:
   *
   * \li a single \ref sat::Solvable
   * \li a single \ref PoolItem
   * \li a \ref PoolQuery
   * \li an other \ref PoolQueryResult
   * \li any iterator pair with \c value_type \ref sat::Solvable
   *     or \ref PoolItem or \ref PoolQuery or any type that fits
   *     \c operator+=.
   *
   * The class is a \ref sat::SolvIterMixin, so you can iterate the result
   * not just as \ref sat::Solvable, but also as \ref PoolItem or
   * \ref ui::Selectable.
   *
   * \code
   *   // Constructed from PoolItem iterator pair
   *   PoolQueryResult result( pool.byKindBegin<Package>(), pool.byKindEnd<Package>() );
   *   MIL << result.size() << endl;
   *
   *   {
   *     // Removing a PoolQuery result
   *     PoolQuery q;
   *     q.addAttribute( sat::SolvAttr::name, "[a-zA-Z]*" );
   *     q.setMatchGlob();
   *     result -= q;
   *     MIL << result.size() << endl;
   *   }
   *   MIL << result << endl;
   *
   *   // Removing a range of sat::Solvables
   *   sat::WhatProvides poviders( Capability("3ddiag") );
   *   result -= PoolQueryResult( poviders.begin(), poviders.end() );
   *
   *   // packages not starting with a letter, except 3ddiag
   *   MIL << result << endl;
   * \endcode
   */
  class PoolQueryResult : public sat::SolvIterMixin<PoolQueryResult,std::unordered_set<sat::Solvable>::const_iterator>
  {
    public:
      typedef std::unordered_set<sat::Solvable>	ResultSet;
      typedef ResultSet::size_type                      size_type;
      typedef ResultSet::const_iterator                 const_iterator;

    public:
      /** Default ctor (empty result) */
      PoolQueryResult()
      {}

      /** Ctor adding one \ref sat::Solvable. */
      explicit PoolQueryResult( sat::Solvable result_r )
      { operator+=( result_r ); }

      /** Ctor adding one \ref PoolItem. */
      explicit PoolQueryResult( const PoolItem & result_r )
      { operator+=( result_r ); }

      /** Ctor adding one \ref PoolQuery result. */
      explicit PoolQueryResult( const PoolQuery & query_r )
      { operator+=( query_r ); }

      /** Ctor adding a range of items for which \ref operator+= is defined. */
      template<class _QueryResultIter>
      PoolQueryResult( _QueryResultIter begin_r, _QueryResultIter end_r )
      {
        for_( it, begin_r, end_r )
        {
          operator+=( *it );
        }
      }

    public:
      /** Whether the result is empty. */
      bool empty() const
      { return _result.empty(); }
      /** The number of \ref sat::Solvables. */
      size_type size() const
      { return _result.size(); }
      /** */
      const_iterator begin() const
      { return _result.begin(); }
      /** */
      const_iterator end() const
      { return _result.end(); }

      /** Test whether some item is in the result set. */
      bool contains(sat::Solvable result_r ) const
      { return( _result.find( result_r ) != _result.end() ); }
      /** \overload */
      bool contains( const PoolItem & result_r ) const
      { return contains( result_r.satSolvable() ); }

    public:
      /** Clear the result. */
      void clear()
      { _result.clear(); }

      /** Add items to the result. */
      PoolQueryResult & operator+=( const PoolQueryResult & query_r )
      {
        if ( ! query_r.empty() )
          _result.insert( query_r.begin(), query_r.end() );
        return *this;
      }
      /** \overload */
      PoolQueryResult & operator+=( const PoolQuery & query_r )
      {
        try
        {
          for_( it, query_r.begin(), query_r.end() )
            _result.insert( *it );
        }
        catch ( const Exception & )
        {}
        return *this;
      }
      /** \overload */
      PoolQueryResult & operator+=( sat::Solvable result_r )
      {
        _result.insert( result_r );
        return *this;
      }
      /** \overload */
      PoolQueryResult & operator+=( const PoolItem & result_r )
      {
        _result.insert( result_r.satSolvable() );
        return *this;
      }

      /** Remove Items from the result. */
      PoolQueryResult & operator-=( const PoolQueryResult & query_r )
      {
        if ( &query_r == this ) // catch self removal!
          clear();
        else
          for_( it, query_r.begin(), query_r.end() )
            _result.erase( *it );
        return *this;
      }
      /** \overload */
      PoolQueryResult & operator-=( const PoolQuery & query_r )
      {
        try
        {
          for_( it, query_r.begin(), query_r.end() )
            _result.erase( *it );
        }
        catch ( const Exception & )
        {}
        return *this;
      }
      /** \overload */
      PoolQueryResult & operator-=( sat::Solvable result_r )
      {
        _result.erase( result_r );
        return *this;
      }
      /** \overload */
      PoolQueryResult & operator-=( const PoolItem & result_r )
      {
        _result.erase( result_r.satSolvable() );
        return *this;
      }

    public:
      /** Combine results. */
      PoolQueryResult operator+( const PoolQueryResult & query_r ) const
      { return PoolQueryResult(*this) += query_r; }
      /** \overload */
      PoolQueryResult operator+( const PoolQuery & query_r ) const
      { return PoolQueryResult(*this) += query_r; }
      /** \overload */
      PoolQueryResult operator+( sat::Solvable result_r ) const
      { return PoolQueryResult(*this) += result_r; }

      /** Intersect results. */
      PoolQueryResult operator-( const PoolQueryResult & query_r ) const
      { return PoolQueryResult(*this) -= query_r; }
      /** \overload */
      PoolQueryResult operator-( const PoolQuery & query_r ) const
      { return PoolQueryResult(*this) -= query_r; }
      /** \overload */
      PoolQueryResult operator-( sat::Solvable result_r ) const
      { return PoolQueryResult(*this) -= result_r; }

    private:
      ResultSet _result;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PoolQueryResult Stream output */
  std::ostream & operator<<( std::ostream & str, const PoolQueryResult & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOLQUERYRESULT_H
