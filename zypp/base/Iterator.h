/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Iterator.h
 *
*/
#ifndef ZYPP_BASE_ITERATOR_H
#define ZYPP_BASE_ITERATOR_H

#include <iterator>
#include <utility>

#include <boost/functional.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/function_output_iterator.hpp>

#include "zypp/base/Iterable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** \defgroup ITERATOR Boost.Iterator Library
   *
   * \see http://www.boost.org/libs/iterator/doc/index.html
   *
   * \li \b counting_iterator: an iterator over a sequence of
   *        consecutive values. Implements a "lazy sequence"
   * \li \b filter_iterator: an iterator over the subset of elements
   *        of some sequence which satisfy a given predicate
   * \li \b function_output_iterator: an output iterator wrapping a
   *        unary function object; each time an element is written into
   *        the dereferenced iterator, it is passed as a parameter to
   *        the function object.
   * \li \b indirect_iterator: an iterator over the objects pointed-to
   *        by the elements of some sequence.
   * \li \b permutation_iterator: an iterator over the elements of
   *        some random-access sequence, rearranged according to some
   *        sequence of integer indices.
   * \li \b reverse_iterator: an iterator which traverses the elements
   *        of some bidirectional sequence in reverse. Corrects many of the shortcomings of C++98's std::reverse_iterator.
   * \li \b shared_container_iterator: an iterator over elements of
   *        a container whose lifetime is maintained by a shared_ptr
   *        stored in the iterator.
   * \li \b transform_iterator: an iterator over elements which are
   *        the result of applying some functional transformation to
   *        the elements of an underlying sequence. This component
   *        also replaces the old projection_iterator_adaptor.
   * \li \b zip_iterator: an iterator over tuples of the elements
   *        at corresponding positions of heterogeneous underlying
   *        iterators.
   *
   * There are in fact more interesting iterator concepts
   * available than the ones listed above. Have a look at them.
   *
   * Some of the iterator types are already dragged into namespace
   * zypp. Feel free to add what's missing.
   *
   * \todo Separate them into individual zypp header files.
  */
  //@{

  /** \class filter_iterator
   * An iterator over the subset of elements of some sequence
   * which satisfy a given predicate.
   *
   * Provides boost::filter_iterator and boost::make_filter_iterator
   * convenience function.
   * \see http://www.boost.org/libs/iterator/doc/filter_iterator.html
   * \code
   * template <class Predicate, class Iterator>
   *   filter_iterator<Predicate,Iterator>
   *   make_filter_iterator(Predicate f, Iterator x, Iterator end = Iterator());
   *
   * template <class Predicate, class Iterator>
   *   filter_iterator<Predicate,Iterator>
   *   make_filter_iterator(Iterator x, Iterator end = Iterator());
   * \endcode
   * Remember the deduction rules for template arguments.
   * \code
   * struct MyDefaultConstructibleFilter;
   * make_filter_iterator<MyDefaultConstructibleFilter>( c.begin(), c.end() );
   * make_filter_iterator( MyDefaultConstructibleFilter(), c.begin(), c.end() );
   * ...
   * make_filter_iterator( resfilter::ByName("foo"), c.begin(), c.end() );
   *
   * \endcode
  */
  using boost::filter_iterator;
  using boost::make_filter_iterator;

  /** Convenience to create filter_iterator from container::begin(). */
  template<class TFilter, class TContainer>
    filter_iterator<TFilter, typename TContainer::const_iterator>
    make_filter_begin( TFilter f, const TContainer & c )
    {
      return make_filter_iterator( f, c.begin(), c.end() );
    }

  /** Convenience to create filter_iterator from container::begin(). */
  template<class TFilter, class TContainer>
    filter_iterator<TFilter, typename TContainer::const_iterator>
    make_filter_begin( const TContainer & c )
    {
      return make_filter_iterator( TFilter(), c.begin(), c.end() );
    }

  /** Convenience to create filter_iterator from container::end(). */
  template<class TFilter, class TContainer>
    filter_iterator<TFilter, typename TContainer::const_iterator>
    make_filter_end( TFilter f, const TContainer & c )
    {
      return make_filter_iterator( f, c.end(), c.end() );
    }

  /** Convenience to create filter_iterator from container::end(). */
  template<class TFilter, class TContainer>
    filter_iterator<TFilter, typename TContainer::const_iterator>
    make_filter_end( const TContainer & c )
    {
      return make_filter_iterator( TFilter(), c.end(), c.end() );
    }

  /** \class transform_iterator
   * An iterator over elements which are the result of applying
   * some functional transformation to the elements of an underlying
   * sequence.
   *
   * Provides boost::transform_iterator and boost::make_transform_iterator
   * convenience function.
   * \see http://www.boost.org/libs/iterator/doc/transform_iterator.html
   * \code
   * template <class UnaryFunction, class Iterator>
   *   transform_iterator<UnaryFunction, Iterator>
   *   make_transform_iterator(Iterator it, UnaryFunction fun);
   *
   * template <class UnaryFunction, class Iterator>
   *   transform_iterator<UnaryFunction, Iterator>
   *   make_transform_iterator(Iterator it);
   * \endcode
  */
  using boost::transform_iterator;
  using boost::make_transform_iterator;

  /** Functor taking a \c std::pair returning \c std::pair.first.
   * \see MapKVIteratorTraits
  */
  template<class TPair>
    struct GetPairFirst : public std::unary_function<TPair, const typename TPair::first_type &>
    {
      const typename TPair::first_type & operator()( const TPair & pair_r ) const
      { return pair_r.first; }
    };

  /** Functor taking a \c std::pair returning \c std::pair.second .
   * \see MapKVIteratorTraits
  */
  template<class TPair>
    struct GetPairSecond : public std::unary_function<TPair, const typename TPair::second_type &>
    {
      const typename TPair::second_type & operator()( const TPair & pair_r ) const
      { return pair_r.second; }
    };

  /** Traits for std::map key and value iterators.
   *
   * \ref GetPairFirst and \ref GetPairSecond help building a transform_iterator
   * that iterates over keys or values of a std::map. Class MapKVIteratorTraits
   * provides some typedefs, you usg. do not want to write explicitly.
   *
   * \code
   * // typedefs
   * typedef std::map<K,V> MapType;
   *
   * // transform_iterator<GetPairFirst<MapType::value_type>,  MapType::const_iterator>
   * typedef MapKVIteratorTraits<MapType>::Key_const_iterator MapTypeKey_iterator;
   * // transform_iterator<GetPairSecond<MapType::value_type>, MapType::const_iterator>
   * typedef MapKVIteratorTraits<MapType>::Value_const_iterator  MapTypeValue_iterator;
   *
   * // usage
   * MapType mymap;
   *
   * MapTypeKey_const_iterator keyBegin( make_map_key_begin( mymap ) );
   * MapTypeKey_const_iterator keyEnd  ( make_map_key_end( mymap ) );
   *
   * MapTypeValue_const_iterator valBegin( make_map_value_begin( mymap ) );
   * MapTypeValue_const_iterator valEnd  ( make_map_value_end( mymap ) );
   *
   * std::for_each( keyBegin, keyEnd, DoSomething() );
   * std::for_each( valBegin, valEnd, DoSomething() );
   * \endcode
   *
   * Or short:
   *
   * \code
   * typedef std::map<K,V> MapType;
   * MapType mymap;
   *
   * std::for_each( make_map_key_begin( mymap ),   make_map_key_end( mymap ),   DoSomething() );
   * std::for_each( make_map_value_begin( mymap ), make_map_value_end( mymap ), DoSomething() );
   * \endcode
   */
  template<class TMap>
    struct MapKVIteratorTraits
    {
      /** The map type */
      typedef TMap                       MapType;
      /** The maps key type */
      typedef typename TMap::key_type    KeyType;
      /** The key iterator type */
      typedef transform_iterator<GetPairFirst<typename MapType::value_type>,
                                 typename MapType::const_iterator> Key_const_iterator;
      /** The maps value (mapped) type */
      typedef typename TMap::mapped_type ValueType;
      /** The value iterator type */
      typedef transform_iterator<GetPairSecond<typename MapType::value_type>,
                                 typename MapType::const_iterator> Value_const_iterator;
    };

  /** Convenience to create the key iterator from container::begin() */
  template<class TMap>
    inline typename MapKVIteratorTraits<TMap>::Key_const_iterator make_map_key_begin( const TMap & map_r )
    { return make_transform_iterator( map_r.begin(), GetPairFirst<typename TMap::value_type>() ); }

  /** Convenience to create the key iterator from container::end() */
  template<class TMap>
    inline typename MapKVIteratorTraits<TMap>::Key_const_iterator make_map_key_end( const TMap & map_r )
    { return make_transform_iterator( map_r.end(), GetPairFirst<typename TMap::value_type>() ); }

  /** Convenience to create the value iterator from container::begin() */
  template<class TMap>
    inline typename MapKVIteratorTraits<TMap>::Value_const_iterator make_map_value_begin( const TMap & map_r )
    { return make_transform_iterator( map_r.begin(), GetPairSecond<typename TMap::value_type>() ); }

  /** Convenience to create the value iterator from container::end() */
  template<class TMap>
    inline typename MapKVIteratorTraits<TMap>::Value_const_iterator make_map_value_end( const TMap & map_r )
    { return make_transform_iterator( map_r.end(), GetPairSecond<typename TMap::value_type>() ); }

  /** Convenience to create the key iterator from container::lower_bound() */
  template<class TMap>
    inline typename MapKVIteratorTraits<TMap>::Key_const_iterator make_map_key_lower_bound( const TMap & map_r, const typename TMap::key_type & key_r )
    { return make_transform_iterator( map_r.lower_bound( key_r ), GetPairFirst<typename TMap::value_type>() ); }

  /** Convenience to create the key iterator from container::upper_bound() */
  template<class TMap>
    inline typename MapKVIteratorTraits<TMap>::Key_const_iterator make_map_key_upper_bound( const TMap & map_r, const typename TMap::key_type & key_r )
    { return make_transform_iterator( map_r.upper_bound( key_r ), GetPairFirst<typename TMap::value_type>() ); }

  /** Convenience to create the value iterator from container::lower_bound() */
  template<class TMap>
    inline typename MapKVIteratorTraits<TMap>::Value_const_iterator make_map_value_lower_bound( const TMap & map_r, const typename TMap::key_type & key_r )
    { return make_transform_iterator( map_r.lower_bound( key_r ), GetPairSecond<typename TMap::value_type>() ); }

  /** Convenience to create the value iterator from container::upper_bound() */
  template<class TMap>
    inline typename MapKVIteratorTraits<TMap>::Value_const_iterator make_map_value_upper_bound( const TMap & map_r, const typename TMap::key_type & key_r )
    { return make_transform_iterator( map_r.upper_bound( key_r ), GetPairSecond<typename TMap::value_type>() ); }

  /** \class function_output_iterator
   * An output iterator wrapping a unary function object; each time an
   * element is written into the dereferenced iterator, it is passed as
   * a parameter to the function object.
   *
   * Provides boost::function_output_iterator and boost::make_function_output_iterator
   * convenience function.
   * \see http://www.boost.org/libs/iterator/doc/function_output_iterator.html
   * \code
   * template <class UnaryFunction>
   *   function_output_iterator<UnaryFunction>
   *   make_function_output_iterator(const UnaryFunction& f = UnaryFunction());
   * \endcode
  */
  using boost::function_output_iterator;
  using boost::make_function_output_iterator;

  //@}
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_ITERATOR_H
