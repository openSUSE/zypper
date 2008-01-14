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
#include <string>
#include <functional>

#include "zypp/sat/IdStrType.h"
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
   *
   * \attention Edition::match compares two editions, treating empty
   * version or release strings as wildcard. Thus match is not transitive,
   * and you don't want to use it to order keys in a a std::container.
   *
   * \ingroup g_BackendSpecific
  */
  class Edition : public sat::IdStrType<Edition>
  {
    public:
      /** Type of an epoch. */
      typedef unsigned epoch_t;

      /** Value representing \c noepoch. */
      static const epoch_t noepoch = 0;

    /** Value representing \c noedition (<tt>""</tt>)
     * This is in fact a valid Edition. It's what the default ctor
     * creates or will be parsed from an empty string.
     */
      static const Edition noedition;

    public:
      /** Default ctor: \ref noedition. */
      Edition() {}

      /** Ctor taking edition as string. */
      explicit Edition( sat::detail::IdType id_r )   : _str( sat::IdStr(id_r).c_str() ) {}
      explicit Edition( const sat::IdStr & idstr_r ) : _str( idstr_r.c_str() ) {}
      explicit Edition( const char * cstr_r )        : _str( cstr_r ) {}
      explicit Edition( const std::string & str_r )  : _str( str_r ) {}

      /** Ctor taking \a version_r, \a release_r and optional \a epoch_r */
      Edition( const std::string & version_r,
               const std::string & release_r,
               epoch_t epoch_r = noepoch );

      /** Ctor taking \a version_r, \a release_r and optional \a epoch_r as string. */
      Edition( const std::string & version_r,
               const std::string & release_r,
               const std::string & epoch_r );

    public:
      /** Epoch */
      epoch_t epoch() const;

      /** Version */
      const std::string & version() const;

      /** Release */
      const std::string & release() const;

    public:
      /** */
      using sat::IdStrType<Edition>::compare;

      /** Compare two Editions returning <tt>-1,0,1</tt>.
       * \return <tt>-1,0,1</tt> if editions are <tt>\<,==,\></tt>.
       *
       * \attention An empty version or release string is not treated
       * specialy. It's the least possible value. If you want an empty
       * string treated as \c ANY, use \ref match.
       */
      static int compare( const Edition & lhs, const Edition & rhs )
      { return lhs.compare( rhs ); }

      /** \ref compare functor.
       * \see \ref RelCompare.
       */
      typedef zypp::Compare<Edition> Compare;

      /** \ref Edition \ref Range based on \ref Compare.
       * \see \ref RelCompare.
       */
      typedef Range<Edition> CompareRange;

    public:
      /** Match two Editions returning <tt>-1,0,1</tt>, treating empty
       *  version/release strings as \c ANY.
       */
      int match( const Edition & rhs )     const { return _doMatchI( rhs.idStr() ); }
      int match( const sat::IdStr & rhs )  const { return _doMatchI( rhs ); }
      int match( const char * rhs )        const { return _doMatchC( rhs ); }
      int match( const std::string & rhs ) const { return _doMatchC( rhs.c_str() ); }

      /** Match two Editions returning <tt>-1,0,1</tt>, treating empty
       *  version/release strings as \c ANY.
       * \return <tt>-1,0,1</tt> if editions match <tt>\<,==,\></tt>.
       */
      static int match( const Edition & lhs, const Edition & rhs )
      { return lhs.match( rhs ); }

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
      int _doCompareC( const char * rhs )     const;
      int _doMatchI( const sat::IdStr & rhs ) const { return idStr().compareEQ( rhs ) ? 0 : _doMatchC( rhs.c_str() ); }
      int _doMatchC( const char * rhs )       const;

    private:
      friend class sat::IdStrType<Edition>;
      sat::IdStr _str;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_EDITION_H
