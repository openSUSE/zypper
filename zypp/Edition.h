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

#include "zypp/IdStringType.h"
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
  class Edition : public IdStringType<Edition>
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
      explicit Edition( IdString::IdType id_r )     : _str( id_r ) {}
      explicit Edition( const IdString & idstr_r )  : _str( idstr_r ) {}
      explicit Edition( const std::string & str_r ) : _str( str_r ) {}
      explicit Edition( const char * cstr_r )       : _str( cstr_r ) {}

      /** Ctor taking \a version_r, \a release_r and optional \a epoch_r */
      Edition( const std::string & version_r,
               const std::string & release_r,
               epoch_t epoch_r = noepoch );
      /** \overload */
      Edition( const char * version_r,
               const char * release_r,
               epoch_t epoch_r = noepoch );

      /** Ctor taking \a version_r, \a release_r and optional \a epoch_r as string. */
      Edition( const std::string & version_r,
               const std::string & release_r,
               const std::string & epoch_r );
      /** \overload */
      Edition( const char * version_r,
               const char * release_r,
               const char * epoch_r );

    public:
      /** Epoch */
      epoch_t epoch() const;

      /** Version */
      std::string version() const;

      /** Release */
      std::string release() const;

    public:
      /** \ref compare functor.
       * \see \ref RelCompare.
       */
      typedef zypp::Compare<Edition> Compare;

      /** \ref Edition \ref Range based on \ref Compare.
       * \see \ref RelCompare.
       */
      typedef Range<Edition> CompareRange;

    public:
      /** \name Match two Editions
       *  Match two Editions returning <tt>-1,0,1</tt>, treating empty
       *  version/release strings as \c ANY.
       */
      //@{
      static int match( const Edition & lhs,     const Edition & rhs )     { return match( lhs.idStr(), rhs.idStr() ); }
      static int match( const Edition & lhs,     const IdString & rhs )    { return match( lhs.idStr(), rhs ); }
      static int match( const Edition & lhs,     const std::string & rhs ) { return _doMatch( lhs.c_str(), rhs.c_str() ); }
      static int match( const Edition & lhs,     const char * rhs )        { return _doMatch( lhs.c_str(), rhs );}

      static int match( const IdString & lhs,    const Edition & rhs )     { return match( lhs, rhs.idStr() ); }
      static int match( const IdString & lhs,    const IdString & rhs )    { return lhs.compareEQ( rhs ) ? 0 :
                                                                                    _doMatch( lhs.c_str(), rhs.c_str() ); }
      static int match( const IdString & lhs,    const std::string & rhs ) { return _doMatch( lhs.c_str(), rhs.c_str() ); }
      static int match( const IdString & lhs,    const char * rhs )        { return _doMatch( lhs.c_str(), rhs ); }

      static int match( const std::string & lhs, const Edition & rhs )     { return _doMatch( lhs.c_str(), rhs.c_str() );}
      static int match( const std::string & lhs, const IdString & rhs )    { return _doMatch( lhs.c_str(), rhs.c_str() ); }
      static int match( const std::string & lhs, const std::string & rhs ) { return _doMatch( lhs.c_str(), rhs.c_str() ); }
      static int match( const std::string & lhs, const char * rhs )        { return _doMatch( lhs.c_str(), rhs ); }

      static int match( const char * lhs,        const Edition & rhs )     { return _doMatch( lhs, rhs.c_str() );}
      static int match( const char * lhs,        const IdString & rhs )    { return _doMatch( lhs, rhs.c_str() ); }
      static int match( const char * lhs,        const std::string & rhs ) { return _doMatch( lhs, rhs.c_str() ); }
      static int match( const char * lhs,        const char * rhs )        { return _doMatch( lhs, rhs ); }

      int match( const Edition & rhs )     const { return match( idStr(), rhs.idStr() ); }
      int match( const IdString & rhs )    const { return match( idStr(), rhs ); }
      int match( const std::string & rhs ) const { return _doMatch( c_str(), rhs.c_str() ); }
      int match( const char * rhs )        const { return _doMatch( c_str(), rhs ); }
      //@}

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
      static int _doCompare( const char * lhs,  const char * rhs );
      static int _doMatch( const char * lhs,  const char * rhs );

    private:
      friend class IdStringType<Edition>;
      IdString _str;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Edition XML output. */
  inline std::ostream & dumpAsXmlOn( std::ostream & str, const Edition & obj )
  { return str << "<edition"
	       << " epoch=\"" << obj.epoch() << "\""
	       << " version=\"" << obj.version() << "\""
	       << " release=\"" << obj.release() << "\""
	       << "/>";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_EDITION_H
