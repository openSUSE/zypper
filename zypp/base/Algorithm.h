/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Algorithm.h
 *
*/
#ifndef ZYPP_BASE_ALGORITHM_H
#define ZYPP_BASE_ALGORITHM_H

#include <algorithm>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** Iterate through <tt>[begin_r,end_r)</tt> and invoke \a fnc_r
   *  on each item that passes \a filter_r.
   *
   * Iteration aborts if \a fnc_r returns \c false.
   *
   * \return Number of invokations of \a fnc_r, negative if
   * loop was aborted by \a fnc_.
  */
  template <class _Iterator, class _Filter, class _Function>
    inline int invokeOnEach( _Iterator begin_r, _Iterator end_r,
                             _Filter filter_r,
                             _Function fnc_r )
    {
      int cnt = 0;
      for ( _Iterator it = begin_r; it != end_r; ++it )
        {
          if ( filter_r( *it ) )
            {
              ++cnt;
              if ( ! fnc_r( *it ) )
                  return -cnt;
            }
        }
      return cnt;
    }

  /** Iterate through <tt>[begin_r,end_r)</tt> and invoke \a fnc_r
   *  on each item.
   *
   * Iteration aborts if \a fnc_r returns \c false.
   *
   * \return Number of invokations of \a fnc_r, negative if
   * loop was aborted by \a fnc_.
  */
  template <class _Iterator, class _Function>
    inline int invokeOnEach( _Iterator begin_r, _Iterator end_r,
                             _Function fnc_r )
    {
      int cnt = 0;
      for ( _Iterator it = begin_r; it != end_r; ++it )
        {
          ++cnt;
          if ( ! fnc_r( *it ) )
            return -cnt;
        }
      return cnt;
    }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_ALGORITHM_H
