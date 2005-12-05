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

#include "zypp/Rel.h"

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
   * version, let the container use Edition::Less to compare.
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
    /** Compare two Editions using relational operator \a op.
     * \return Result of expression \c ( \a lhs \a op \a rhs \c ).<BR>
     * If \a op is Rel::ANY, the expression is always \c true.<BR>
     * If \a op is Rel::NONE, the expression is always \c false.
     *
     * \todo optimize impementation. currently a full compare( lhs, rhs )
     * is done and the result evaluated. But step by step would be faster.
    */
    static bool compare( Rel op, const Edition & lhs, const Edition & rhs );

    /** Compare two Editions returning <tt>-1,0,1</tt>.
     * \return <tt>-1,0,1</tt> if editions are <tt>\<,==,\></tt>
    */
    static int compare( const Edition & lhs, const Edition & rhs );

    /* Binary operator functor comparing Edition. */
    struct Less;

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
  { return Edition::compare( Rel::EQ, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator!=( const Edition & lhs, const Edition & rhs )
  { return Edition::compare( Rel::NE, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator<( const Edition & lhs, const Edition & rhs )
  { return Edition::compare( Rel::LT, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator<=( const Edition & lhs, const Edition & rhs )
  { return Edition::compare( Rel::LE, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator>( const Edition & lhs, const Edition & rhs )
  { return Edition::compare( Rel::GT, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator>=( const Edition & lhs, const Edition & rhs )
  { return Edition::compare( Rel::GE, lhs, rhs ); }
  //@}

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Edition::Less
  //
  /** Binary operator functor comparing Edition.
   * Default order for std::container using Edition as key is based
   * on <em>plain string comparison</em>. Use Less to compare, if
   * real version comparison is desired.
   * \code
   * set<Edition> eset;
   * eset.insert( Edition("1.0","") );
   * eset.insert( Edition("1_0","") );
   * // Now: eset.size() == 2
   * \endcode
   * \code
   * set<Edition,Edition::Less> eset;
   * eset.insert( Edition("1.0","") );
   * eset.insert( Edition("1_0","") );
   * // Now: eset.size() == 1
   * \endcode
  */
  struct Edition::Less : public std::binary_function<Edition,Edition,bool>
  {
    /** \return <tt>lhs < rhs</tt> */
    bool operator()(const Edition & lhs, const Edition & rhs ) const
    { return lhs < rhs; }
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace std
{ /////////////////////////////////////////////////////////////////

  /** \relates Edition Default to lexicographical order in std::container.*/
  template<>
    inline bool less<zypp::Edition>::operator()( const zypp::Edition & lhs, const zypp::Edition & rhs ) const
    { return lhs.asString() < rhs.asString(); }

  /** \relates Edition Lexicographical equal for std::container. */
  template<>
    inline bool equal_to<zypp::Edition>::operator()( const zypp::Edition & lhs, const zypp::Edition & rhs ) const
    { return lhs.asString() == rhs.asString(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_EDITION_H
