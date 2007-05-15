/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/String.h
 *
*/
#ifndef ZYPP_BASE_STRING_H
#define ZYPP_BASE_STRING_H

#include <iosfwd>
#include <string>
#include <boost/regex.hpp>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  /** String related utilities and \ref ZYPP_STR_REGEX.
   \see \ref ZYPP_STR_REGEX
  */
  namespace str
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /** Printf style construction of std::string. */
    std::string form( const char * format, ... )
    __attribute__ ((format (printf, 1, 2)));

    ///////////////////////////////////////////////////////////////////
    /** Return string describing the \a error_r code.
     * Like ::strerror, but the numerical value is included in
     * the string as well.
    */
    std::string strerror( int errno_r );

    ///////////////////////////////////////////////////////////////////
    /** Assert \c free called for allocated <tt>char *</tt>.
     * \code
     * ...
     * SafeBuf safe;
     * vasprintf( &safe._buf, format, ap );
     * return safe.asString();
     * \endcode
     *
     * \ingroup g_RAII
    */
    struct SafeBuf
    {
      char * _buf;
      SafeBuf() : _buf( 0 ) {}
      ~SafeBuf() { if ( _buf ) free( _buf ); }
      std::string asString() const
      { return _buf ? std::string(_buf) : std::string(); }
    };

    ///////////////////////////////////////////////////////////////////
    /** \defgroup ZYPP_STR_REGEX Regular expressions
     *
     * Namespace zypp::str regular expressions \b using the
     * boost regex library
     * \url http://www.boost.org/libs/regex/doc/index.html.
     *
     * \li \c regex
     * \li \c regex_match
     * \li \c regex_search
     * \li \c regex_replace
     * \li \c match_results
     * \li \c cmatch
     * \li \c wcmatch
     * \li \c smatch
     * \li \c wsmatch
    */

    //@{
    /** regex */
    using boost::regex;
    using boost::regex_match;
    using boost::regex_search;
    using boost::regex_replace;
    using boost::match_results;
    using boost::match_extra;
    using boost::cmatch;
    using boost::wcmatch;
    using boost::smatch;
    using boost::wsmatch;
    //@}

    /**
     * helper to debug regular expressions matches
     */
    std::ostream & dumpRegexpResult( const boost::smatch &what, std::ostream & str );

    ///////////////////////////////////////////////////////////////////
    /** \name String representation of number.
     *
     * Optional second argument sets the minimal string width (' ' padded).
     * Negative values will cause the number to be left adjusted within the string.
     *
     * Default width is 0.
     * \code
     * numstring(42)           -> "42"
     * numstring(42, 4)        -> "  42"
     * numstring(42,-4)        -> "42  "
     * \endcode
     **/
    //@{
    inline std::string numstring( char n,               int w = 0 ) { return form( "%*hhd",  w, n ); }
    inline std::string numstring( unsigned char n,      int w = 0 ) { return form( "%*hhu",  w, n ); }
    inline std::string numstring( short n,              int w = 0 ) { return form( "%*hd",   w, n ); }
    inline std::string numstring( unsigned short n,     int w = 0 ) { return form( "%*hu",   w, n ); }
    inline std::string numstring( int n,                int w = 0 ) { return form( "%*d",    w, n ); }
    inline std::string numstring( unsigned n,           int w = 0 ) { return form( "%*u",    w, n ); }
    inline std::string numstring( long n,               int w = 0 ) { return form( "%*ld",   w, n ); }
    inline std::string numstring( unsigned long n,      int w = 0 ) { return form( "%*lu",   w, n ); }
    inline std::string numstring( long long n,          int w = 0 ) { return form( "%*lld",  w, n ); }
    inline std::string numstring( unsigned long long n, int w = 0 ) { return form( "%*llu",  w, n ); }
    //@}

    ///////////////////////////////////////////////////////////////////
    /** \name String representation of number as hex value with leading '0x'.
     * Optional second argument sets the minimal
     * string width (0 padded). Negative values will cause the number to be left adjusted
     * within the string. Default width is 10 (4 for char).
     * <PRE>
     * hexstring(42)           -> "0x0000002a"
     * hexstring(42, 4)        -> "0x2a"
     * hexstring(42,-4)        -> "0x2a"
     * </PRE>
     **/
    //@{
    inline std::string hexstring( char n,               int w = 4 ) { return form( "%#0*hhx", w, n ); }
    inline std::string hexstring( unsigned char n,      int w = 4 ) { return form( "%#0*hhx", w, n ); }
    inline std::string hexstring( short n,              int w = 10 ){ return form( "%#0*hx",  w, n ); }
    inline std::string hexstring( unsigned short n,     int w = 10 ){ return form( "%#0*hx",  w, n ); }
    inline std::string hexstring( int n,                int w = 10 ){ return form( "%#0*x",   w, n ); }
    inline std::string hexstring( unsigned n,           int w = 10 ){ return form( "%#0*x",   w, n ); }
    inline std::string hexstring( long n,               int w = 10 ){ return form( "%#0*lx",  w, n ); }
    inline std::string hexstring( unsigned long n,      int w = 10 ){ return form( "%#0*lx",  w, n ); }
    inline std::string hexstring( long long n,          int w = 0 ) { return form( "%#0*llx", w, n ); }
    inline std::string hexstring( unsigned long long n, int w = 0 ) { return form( "%#0*llx", w, n ); }
    //@}

    ///////////////////////////////////////////////////////////////////
    /** \name String representation of number as octal value with leading '0'.
     * Optional second argument sets the minimal
     * string width (0 padded). Negative values will cause the number to be left adjusted
     * within the string. Default width is 5 (4 for char).
     * <PRE>
     * octstring(42)           -> "00052"
     * octstring(42, 4)        -> "0052"
     * octstring(42,-4)        -> "052 "
     * </PRE>
     **/
    //@{
    inline std::string octstring( char n,               int w = 4 ) { return form( "%#0*hho",  w, n ); }
    inline std::string octstring( unsigned char n,      int w = 4 ) { return form( "%#0*hho",  w, n ); }
    inline std::string octstring( short n,              int w = 5 ) { return form( "%#0*ho",   w, n ); }
    inline std::string octstring( unsigned short n,     int w = 5 ) { return form( "%#0*ho",   w, n ); }
    inline std::string octstring( int n,                int w = 5 ) { return form( "%#0*o",    w, n ); }
    inline std::string octstring( unsigned n,           int w = 5 ) { return form( "%#0*o",    w, n ); }
    inline std::string octstring( long n,               int w = 5 ) { return form( "%#0*lo",   w, n ); }
    inline std::string octstring( unsigned long n,      int w = 5 ) { return form( "%#0*lo",   w, n ); }
    inline std::string octstring( long long n,          int w = 0 ) { return form( "%#0*llo",  w, n ); }
    inline std::string octstring( unsigned long long n, int w = 0 ) { return form( "%#0*llo",  w, n ); }
    //@}

    ///////////////////////////////////////////////////////////////////
    /** Parsing numbers from string.
    */
    //@{
    /** String to integer type determined by template arg.
     * \note Only specializations are defined.
     * \code
     * time_t t = strtonum<time_t>( "42" );
     * \endcode
    */
    template<typename _It>
      _It strtonum( const std::string & str );

    template<>
      inline short              strtonum( const std::string & str ) { return ::strtol  ( str.c_str(), NULL, 0 ); }
    template<>
      inline int                strtonum( const std::string & str ) { return ::strtol  ( str.c_str(), NULL, 0 ); }
    template<>
      inline long               strtonum( const std::string & str ) { return ::strtol  ( str.c_str(), NULL, 0 ); }
    template<>
      inline long long          strtonum( const std::string & str ) { return ::strtoll ( str.c_str(), NULL, 0 ); }

    template<>
      inline unsigned short     strtonum( const std::string & str ) { return ::strtoul ( str.c_str(), NULL, 0 ); }
    template<>
      inline unsigned           strtonum( const std::string & str ) { return ::strtoul ( str.c_str(), NULL, 0 ); }
    template<>
      inline unsigned long      strtonum( const std::string & str ) { return ::strtoul ( str.c_str(), NULL, 0 ); }
    template<>
      inline unsigned long long strtonum( const std::string & str ) { return ::strtoull( str.c_str(), NULL, 0 ); }

    /** String to integer type detemined 2nd function arg \a i.
     * \code
     * time_t t; strtonum( "42", t );
     * \endcode
    */
    template<typename _It>
      inline _It strtonum( const std::string & str, _It & i )
      { return i = strtonum<_It>( str ); }
    //@}

    ///////////////////////////////////////////////////////////////////
    /** \name Split. */
    //@{
    /** Split \a line_r into words.
     * Any sequence of characters in \a sepchars_r is treated as
     * delimiter. The words are passed to OutputIterator \a result_r.
     * \code
     * std::vector<std::string> words;
     * str::split( "some line", std::back_inserter(words) )
     * \endcode
     *
    */
    template<class _OutputIterator>
      unsigned split( const std::string & line_r,
                      _OutputIterator     result_r,
                      const std::string & sepchars_r = " \t" )
      {
        const char * beg = line_r.c_str();
        const char * cur = beg;
        // skip leading sepchars
        while ( sepchars_r.find( *cur ) != std::string::npos )
          ++cur;
        unsigned ret = 0;
        for ( beg = cur; *beg; beg = cur, ++result_r, ++ret )
          {
            // skip non sepchars
            while( *cur && sepchars_r.find( *cur ) == std::string::npos )
              ++cur;
            // build string
            *result_r = std::string( beg, cur-beg );
            // skip sepchars
            while ( sepchars_r.find( *cur ) != std::string::npos )
              ++cur;
          }
        return ret;
      }
    //@}

    ///////////////////////////////////////////////////////////////////
    /** \name Join. */
    //@{
    /** Join strings using separator \a sep_r (defaults to BLANK). */
    template <class _Iterator>
      std::string join( _Iterator begin, _Iterator end,
                        const std::string & sep_r = " " )
      {
        std::string res;
        for ( _Iterator iter = begin; iter != end; ++ iter )
          {
            if ( iter != begin )
              res += sep_r;
            res += *iter;
          }
        return res;
      }

    /** Join strings using separator \a sep_r (defaults to BLANK). */
    template <class _Container>
      std::string join( const _Container & cont_r,
                        const std::string & sep_r = " " )
      { return join( cont_r.begin(), cont_r.end(), sep_r ); }
    //@}


    ///////////////////////////////////////////////////////////////////
    /** \name Case conversion. */
    //@{
    /** Return lowercase version of \a s
     * \todo improve
    */
    std::string toLower( const std::string & s );

    /** Return uppercase version of \a s
     * \todo improve
    */
    std::string toUpper( const std::string & s );
    //@}

    ///////////////////////////////////////////////////////////////////
    /** \name Trimming whitepace.
     * \todo optimize l/r trim.
    */
    //@{
    /** To define how to trim. */
    enum Trim {
      NO_TRIM = 0x00,
      L_TRIM  = 0x01,
      R_TRIM  = 0x02,
      TRIM    = (L_TRIM|R_TRIM)
    };

    std::string  trim( const std::string & s, const Trim trim_r = TRIM );

    inline std::string ltrim( const std::string & s )
    { return trim( s, L_TRIM ); }

    inline std::string rtrim( const std::string & s )
    { return trim( s, R_TRIM ); }
    //@}

    std::string stripFirstWord( std::string & line, const bool ltrim_first );

    std::string getline( std::istream & str, bool trim = false );

    std::string getline( std::istream & str, const Trim trim_r );

    ///////////////////////////////////////////////////////////////////

    /** \name String prefix handling.
     */
    //@{
    /** Return whether \a str_r has prefix \a prefix_r. */
    inline bool hasPrefix( const std::string & str_r, const std::string & prefix_r )
    { return( str_r.substr( 0, prefix_r.size() ) == prefix_r ); }

    /** Strip a \a prefix_r from \a str_r and return the resulting string. */
    inline std::string stripPrefix( const std::string & str_r, const std::string & prefix_r )
    { return( hasPrefix( str_r, prefix_r ) ? str_r.substr( prefix_r.size() ) : str_r ); }
    //@}

    /////////////////////////////////////////////////////////////////
  } // namespace str
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_STRING_H
