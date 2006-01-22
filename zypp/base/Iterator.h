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

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

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
  */
  using boost::filter_iterator;

  /** \class transform_iterator
  */
  using boost::transform_iterator;

  //@}
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_ITERATOR_H
