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
  template<class _Tp, class _Compare = Compare<_Tp> >
    struct Range
    {
      /** */
      Rel op;
      /** */
      _Tp value;

      /** Default ctor: \ref Rel::ANY. */
      Range()
      : op( Rel::ANY )
      {}

      /** Ctor taking \a _Tp (\ref Rel::EQ). */
      Range( const _Tp & value_r )
      : op( Rel::EQ )
      , value( value_r )
      {}

      /** Ctor taking \ref Rel and \a _Tp. */
      Range( Rel op_r, const _Tp & value_r )
      : op( op_r )
      , value( value_r )
      {}

      /** Return whether two Ranges overlap. */
      bool overlaps( const Range & rhs ) const
      {
        return range_detail::overlaps( op, rhs.op,
                                       _Compare()( value, rhs.value ) );
      }
    };
  ///////////////////////////////////////////////////////////////////

  template<class _Tp, class _Compare>
    inline bool overlaps( const Range<_Tp,_Compare> & lhs,
                          const Range<_Tp,_Compare> & rhs )
    { return lhs.overlaps( rhs ); }

  ///////////////////////////////////////////////////////////////////

  template<class _Tp, class _Compare>
    inline bool operator==( const Range<_Tp,_Compare> & lhs,
                            const Range<_Tp,_Compare> & rhs )
    {
      return( lhs.op == rhs.op
              && (    lhs.op == Rel::ANY
                   || lhs.op == Rel::NONE
                   || relCompare( Rel::EQ, lhs.value, rhs.value,
                                  _Compare() )
                 )
            );
    }

  template<class _Tp, class _Compare>
    inline bool operator!=( const Range<_Tp,_Compare> & lhs,
                            const Range<_Tp,_Compare> & rhs )
    { return ! ( lhs == rhs ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RANGE_H
