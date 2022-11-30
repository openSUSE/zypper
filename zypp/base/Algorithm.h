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
  template <class TIterator, class TFilter, class TFunction>
    inline int invokeOnEach( TIterator begin_r, TIterator end_r,
                             TFilter filter_r,
                             TFunction fnc_r )
    {
      int cnt = 0;
      for ( TIterator it = begin_r; it != end_r; ++it )
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
  template <class TIterator, class TFunction>
    inline int invokeOnEach( TIterator begin_r, TIterator end_r,
                             TFunction fnc_r )
    {
      int cnt = 0;
      for ( TIterator it = begin_r; it != end_r; ++it )
        {
          ++cnt;
          if ( ! fnc_r( *it ) )
            return -cnt;
        }
      return cnt;
    }

    template <class Container, class Elem>
    bool contains ( const Container &c, const Elem &elem )
    {
      return ( std::find( c.begin(), c.end(), elem ) != c.end() );
    }

    template <class Container, class Fnc >
    bool any_of ( const Container &c, Fnc &&cb )
    {
      return std::any_of( c.begin(), c.end(), std::forward<Fnc>(cb) );
    }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_ALGORITHM_H
