/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Range.h
 *
*/
#ifndef ZYPP_RANGE_H
#define ZYPP_RANGE_H

#include "zypp/RelCompare.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace range_detail
  {
    bool overlaps( Rel lhs, Rel rhs, int cmp );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Range
  //
  /**
   *
  */
  template<class Tp, class TCompare = Compare<Tp> >
    struct Range
    {
      /** */
      Rel op;
      /** */
      Tp value;

      /** Default ctor: \ref Rel::ANY. */
      Range()
      : op( Rel::ANY )
      {}

      /** Ctor taking \a Tp (\ref Rel::EQ). */
      Range( const Tp & value_r )
      : op( Rel::EQ )
      , value( value_r )
      {}

      /** Ctor taking \ref Rel and \a Tp. */
      Range( Rel op_r, const Tp & value_r )
      : op( op_r )
      , value( value_r )
      {}

      /** Return whether two Ranges overlap. */
      bool overlaps( const Range & rhs ) const
      { return range_detail::overlaps( op, rhs.op, TCompare()( value, rhs.value ) ); }
    };
  ///////////////////////////////////////////////////////////////////

  template<class Tp, class TCompare>
    inline bool overlaps( const Range<Tp,TCompare> & lhs,
                          const Range<Tp,TCompare> & rhs )
    { return lhs.overlaps( rhs ); }

  ///////////////////////////////////////////////////////////////////

  template<class Tp, class TCompare>
    inline bool operator==( const Range<Tp,TCompare> & lhs,
                            const Range<Tp,TCompare> & rhs )
    {
      return( lhs.op == rhs.op
              && (    lhs.op == Rel::ANY
                   || lhs.op == Rel::NONE
                   || relCompare( Rel::EQ, lhs.value, rhs.value, TCompare() )
                 )
            );
    }

  template<class Tp, class TCompare>
    inline bool operator!=( const Range<Tp,TCompare> & lhs,
                            const Range<Tp,TCompare> & rhs )
    { return ! ( lhs == rhs ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RANGE_H
