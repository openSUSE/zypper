/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Iterable.h
 */
#ifndef ZYPP_BASE_ITERABLE_H
#define ZYPP_BASE_ITERABLE_H

#include <iterator>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class Iterable
  /// \brief
  /// \code
  ///   struct Foo
  ///   {
  ///     class Iterator;
  ///     typedef Iterable<Iterator> IterableType;
  ///
  ///     Iterator myBegin();
  ///     Iterator myEnd();
  ///
  ///     IterableType iterate() { return makeIterable( myBegin(), myEnd() ); }
  ///   };
  /// \endcode
  ///////////////////////////////////////////////////////////////////
  template <class TIterator>
  class Iterable
  {
  public:
    typedef size_t size_type;
    typedef TIterator iterator_type;
    typedef typename std::iterator_traits<iterator_type>::value_type		value_type;
    typedef typename std::iterator_traits<iterator_type>::difference_type	difference_type;
    typedef typename std::iterator_traits<iterator_type>::pointer		pointer;
    typedef typename std::iterator_traits<iterator_type>::reference		reference;
    typedef typename std::iterator_traits<iterator_type>::iterator_category	iterator_category;

    /** Ctor taking the iterator pair */
    Iterable()
    {}

    /** Ctor taking the iterator pair */
    Iterable( iterator_type begin_r, iterator_type end_r )
    : _begin( std::move(begin_r) )
    , _end( std::move(end_r) )
    {}

    /** Ctor taking the iterator pair */
    Iterable( std::pair<iterator_type,iterator_type> range_r )
    : _begin( std::move(range_r.first) )
    , _end( std::move(range_r.second) )
    {}

    iterator_type begin() const
    { return _begin; }

    iterator_type end() const
    { return _end; }

    bool empty() const
    { return( _begin == _end ); }

    size_type size() const
    { size_type ret = 0; for ( iterator_type i = _begin; i != _end; ++i ) ++ret; return ret; }

    bool contains( const value_type & val_r ) const
    { return( find( val_r ) != _end ); }

    iterator_type find( const value_type & val_r ) const
    { iterator_type ret = _begin; for ( ; ret != _end; ++ret ) if ( *ret == val_r ) break; return ret; }

  private:
    iterator_type _begin;
    iterator_type _end;
  };

  /** \relates Iterable convenient construction. */
  template <class TIterator>
  Iterable<TIterator> makeIterable( TIterator && begin_r, TIterator && end_r )
  { return Iterable<TIterator>( std::forward<TIterator>(begin_r), std::forward<TIterator>(end_r) ); }

  /** \relates Iterable convenient construction. */
  template <class TIterator>
  Iterable<TIterator> makeIterable( std::pair<TIterator,TIterator> && range_r )
  { return Iterable<TIterator>( std::forward<std::pair<TIterator,TIterator>>(range_r) ); }
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_ITERABLE_H
