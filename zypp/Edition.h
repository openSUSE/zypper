/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Edition.h
 *
*/
#ifndef ZYPP_EDITION_H
#define ZYPP_EDITION_H

#include <iosfwd>
#include <functional>
#include <string>

#include "zypp/base/PtrTypes.h"

#include "zypp/RelCompare.h"
#include "zypp/Range.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Edition
  //
  /** Edition represents <code>[epoch:]version[-release]</code>
   *
   * \li \c epoch   (optional) number, Edition::noepoch if not supplied
   * \li \c version (required) string, may not contain '-'
   * \li \c release (optional) string, may not contain '-'
   *
   * Comparison is actually \reg g_BackendSpecific.
   *
   * \li \b RPM: Edition are ordered according to \c epoch, then \c version,
   * then \c release. Version and release strings are compared by splitting
   * them into segments of alpha or digit sequences. Segments are compared
   * according to their type. On mixed types a string compares less than a
   * number.
   * \code
   *   compare( 1.a, 1.0 ) == -1 (<)
   *   compare( 1.0, 1.a ) ==  1 (>)
   *   compare( 1.0, 1_0 ) ==  0 (==)
   * \endcode
   *
   * \attention operator< defines equivalence classes of version strings, as non
   * alphanumeric chars are ignored. That' why \c 1.0 and \c 1_0 compare equal
   * in the example.<BR>
   * If Edition is used as key in a std::container, per default
   * <em>plain string comparison</em> is used. If you want to compare by
   * version, let the container use \ref CompareByLT<Edition> to compare.
   *
   * \attention Edition::match compares two editions, treating empty
   * version or release strings as wildcard. Thus match is not transitive,
   * and you don't want to use it to order keys in a a std::container.
   *
   * \ingroup g_BackendSpecific
   * \todo Define exceptions.
   * \todo optimize implementation(e.g don't store epoch if noepoch)
   * \todo implement debian comparison and make choice backend specific
   * \todo optimize noedition. unified Impl and quick check in compare.
  */
  class Edition
  {
  public:
    /** Type of an epoch. */
    typedef unsigned epoch_t;

    /** Value representing \c noepoch. */
    static const epoch_t noepoch = 0;

    /** Value representing \c noedition.
     * This is in fact a valid Edition. It's what the default ctor
     * creates or will be parsed from an empty string.
    */
    static const Edition noedition;

  public:
    /** Default ctor: \ref noedition. */
    Edition();

    /** Ctor taking edition as string.
     * \throw INTERNAL if \a edition_r does not make a valid Edition.
    */
    Edition( const std::string & edition_r );

    /** Ctor taking \a version_r, \a release_r and optional \a epoch_r
     * \throw INTERNAL if \a version_r or \a release_r are not valid.
    */
    Edition( const std::string & version_r,
             const std::string & release_r,
             epoch_t epoch_r = noepoch );

    /** Ctor taking \a version_r, \a release_r and optional \a epoch_r as string.
     * \throw INTERNAL if \a version_r or \a release_r \a epoch_r are
     * not valid.
    */
    Edition( const std::string & version_r,
             const std::string & release_r,
             const std::string & epoch_r );

    /** Dtor */
    ~Edition();

  public:
    /** Epoch */
    epoch_t epoch() const;

    /** Version */
    const std::string & version() const;

    /** Release */
    const std::string & release() const;

    /** String representation of Edition. */
    std::string asString() const;

  public:
    /** Compare two Editions returning <tt>-1,0,1</tt>.
     * \return <tt>-1,0,1</tt> if editions are <tt>\<,==,\></tt>.
     *
     * \attention An empty version or release string is not treated
     * specialy. It's the least possible value. If you want an empty
     * string treated as \c ANY, use \ref match.
    */
    static int compare( const Edition & lhs, const Edition & rhs );

    /** */
    int compare( const Edition & rhs ) const
    { return compare( *this, rhs ); }

    /** \ref compare functor.
     * \see \ref RelCompare.
    */
    typedef Compare<Edition> CompareEd;

    /** \ref Edition \ref Range based on \ref Compare.
     * \see \ref RelCompare.
    */
    typedef Range<Edition> CompareRange;

  public:
    /** Match two Editions returning <tt>-1,0,1</tt>, treating empty
     *  strings as \c ANY.
     * \return <tt>-1,0,1</tt> if editions match <tt>\<,==,\></tt>.
    */
    static int match( const Edition & lhs, const Edition & rhs );

    /** */
    int match( const Edition & rhs ) const
    { return match( *this, rhs ); }

    /** \ref match functor.
     * \see \ref RelCompare.
    */
    struct Match: public std::binary_function<Edition,Edition,int>
    {
      int operator()( const Edition & lhs, const Edition & rhs ) const
      { return Edition::match( lhs, rhs ); }
    };

    /** \ref Edition \ref Range based on \ref Match.
     * \see \ref RelCompare.
    */
    typedef Range<Edition, Match> MatchRange;

  private:
    /** Hides implementation */
    struct Impl;
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Edition Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const Edition & obj )
  { return str << obj.asString(); }

  /** \name Comaprison based on epoch, version, and release. */
  //@{
  /** \relates Edition */
  inline bool operator==( const Edition & lhs, const Edition & rhs )
  { return compareByRel( Rel::EQ, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator!=( const Edition & lhs, const Edition & rhs )
  { return compareByRel( Rel::NE, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator<( const Edition & lhs, const Edition & rhs )
  { return compareByRel( Rel::LT, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator<=( const Edition & lhs, const Edition & rhs )
  { return compareByRel( Rel::LE, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator>( const Edition & lhs, const Edition & rhs )
  { return compareByRel( Rel::GT, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator>=( const Edition & lhs, const Edition & rhs )
  { return compareByRel( Rel::GE, lhs, rhs ); }
  //@}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace std
{ /////////////////////////////////////////////////////////////////

  /** \relates zypp::Edition Default to lexicographical order in std::container.*/
  template<>
    inline bool less<zypp::Edition>::operator()( const zypp::Edition & lhs, const zypp::Edition & rhs ) const
    { return lhs.asString() < rhs.asString(); }

  /** \relates zypp::Edition Lexicographical equal for std::container. */
  template<>
    inline bool equal_to<zypp::Edition>::operator()( const zypp::Edition & lhs, const zypp::Edition & rhs ) const
    { return lhs.asString() == rhs.asString(); }

  /////////////////////////////////////////////////////////////////
} // namespace std
///////////////////////////////////////////////////////////////////

#endif // ZYPP_EDITION_H
