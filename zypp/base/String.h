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

#include <cstring>

#include <iosfwd>
#include <vector>
#include <string>
#include <sstream>
#include <boost/format.hpp>
#include <boost/utility/string_ref.hpp>

#include "zypp/base/Easy.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Function.h"

///////////////////////////////////////////////////////////////////
namespace boost { namespace logic { class tribool; } }
namespace zypp { typedef boost::logic::tribool TriBool; }
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace boost
{
  /** A formater with (N)o (A)rgument (C)heck.
   * It won't complain about missing or excess arguments. Sometimes
   * usefull when dealing with translations or classes providing a
   * default formater.
   */
  inline format formatNAC( const std::string & string_r ) {
    using namespace boost::io;
    format fmter( string_r );
    fmter.exceptions( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) );
    return fmter;
  }
} // namespace boost
namespace zypp { using boost::formatNAC; }
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{
  /** Request a human readable (translated) string representation of Tp [Tp.asUserString()]
   * Classes may implement a default as member function.
   */
  template <class Tp>
  std::string asUserString( const Tp & val_r )
  { return val_r.asUserString(); }

}// namespace zypp
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  struct MessageString : public std::string
  {
    MessageString() {}
    MessageString( const char * str_r ) 		: std::string( str_r ? str_r : "" ) {}
    MessageString( const std::string & str_r )		: std::string( str_r ) {}
    // boost::format, std::ostringstream, str::Str ...
    template<class TStr>
    MessageString( const TStr & str_r )	: std::string( str_r.str() ) {}
  };

  /** Convenience \c char* constructible from \c std::string and \c char*,
   *  it maps \c (char*)0 to an empty string.
   *
   * \code
   * bool hasPrefix( const std::string & str_r, const std::string & prefix_r )
   * { return( ::strncmp( str_r.c_str(), prefix_r.c_str(), prefix_r.size() ) == 0 ); }
   * \endcode
   *
   * Called with a plain \c char* as argument, the \c std::string is created form
   * for nothing. The implementation actually does not use the \c std::string.
   *
   * Best would be to implement \c hasPrefix for each combination of \c char*
   * and \c std::string arguments:
   *
   * \code
   * bool hasPrefix( const std::string & str_r, const std::string & prefix_r )
   * { return( ::strncmp( str_r.c_str(), prefix_r.c_str(), prefix_r.size() ) == 0 ); }
   *
   * bool hasPrefix( const std::string & str_r, const char * prefix_r )
   * { return( !prefix_r || ::strncmp( str_r.c_str(), prefix_r, ::strlen(prefix_r) ) == 0 ); }
   *
   * bool hasPrefix( const char * str_r, const std::string & prefix_r )
   * { return( str_r ? ::strncmp( str_r, prefix_r.c_str(), prefix_r.size() ) == 0 : prefix_r.empty() ); }
   *
   * bool hasPrefix( const char * str_r, const char * prefix_r )
   * { return( str && prefix_r ? ::strncmp( str_r, prefix_r, ::strlen(prefix_r) ) == 0
   *                           : !((str_r && *str_r) || (prefix_r && *prefix_r)); }
   * \endcode
   *
   * This is where \ref C_Str can help. Constructible from \c std::string and \c char*,
   * it \e reduces the \c std::string to it's \c char*. At the same time it converts
   * \c (char*)0 into an \c "" string.
   *
   * \code
   * bool hasPrefix( const C_Str & str_r, const C_Str & prefix_r )
   * { return( ::strncmp( str_r, prefix_r, prefix_r.size() ) == 0 ); }
   * \endcode
   *
   * \todo Check whether to replace by boost::string_ref
   */
  class C_Str
  {
    public:
      typedef std::string::size_type size_type;

    public:
      C_Str()                            : _val( 0 ),             _sze( 0 ) {}
      C_Str( char * c_str_r )            : _val( c_str_r ),       _sze( std::string::npos ) {}
      C_Str( const char * c_str_r )      : _val( c_str_r ),       _sze( std::string::npos ) {}
      C_Str( const std::string & str_r ) : _val( str_r.c_str() ), _sze( str_r.size() ) {}
      C_Str( const boost::string_ref & str_r ) : _val( str_r.data() ), _sze( str_r.size() ) {}

    public:
      bool      isNull()       const { return !_val; }
      bool      empty()        const { return !(_val && *_val); }
      size_type size()         const
      {
        if ( _sze == std::string::npos )
        { _sze = _val ? ::strlen( _val ) : 0; }
        return _sze;
      };

      operator const char *() const { return c_str(); }
      const char * c_str()    const { return _val ? _val : ""; }

    private:
      const char *const _val;
      mutable size_type _sze;
  };

  /** \relates C_Str Stream output */
  inline std::ostream & operator<<( std::ostream & str, const C_Str & obj )
  { return str << obj.c_str(); }

  ///////////////////////////////////////////////////////////////////
  /** String related utilities and \ref ZYPP_STR_REGEX.
   \see \ref ZYPP_STR_REGEX
  */

  namespace str
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /**
     * Global asString() that works with std::string too
     */
    inline const std::string & asString( const std::string & t )
    { return t; }

#ifndef SWIG // Swig treats it as syntax error
    inline std::string && asString( std::string && t )
    { return std::move(t); }
#endif

    inline std::string asString( const char * t )
    { return t; }

    inline std::string asString( char * t )
    { return t; }

    template<class Tp>
        inline std::string asString( const Tp &t )
        { return t.asString(); }

    template<class Tp>
        inline std::string asString( const intrusive_ptr<Tp> &p )
        { return p->asString(); }

    template<class Tp>
        inline std::string asString( const weak_ptr<Tp> &p )
        { return p->asString(); }

    template<>
        inline std::string asString( const bool &t )
        { return t ? "true" : "false"; }

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
    /// \class Str
    /// \brief Convenient building of std::string via \ref std::ostringstream
    /// Basically a \ref std::ostringstream autoconvertible to \ref std::string
    /// for building string arguments.
    /// \code
    ///   void fnc( const std::string & txt_r );
    ///   fnc( str::Str() << "Hello " << 13 );
    ///
    ///   std::string txt( str::Str() << 45 );
    /// \endcode
    ///////////////////////////////////////////////////////////////////
    struct Str
    {
      template<class Tp>
      Str & operator<<( Tp && val )
      { _str << std::forward<Tp>(val); return *this; }

      Str & operator<<( std::ostream& (*iomanip)( std::ostream& ) )
      { _str << iomanip; return *this; }

      operator std::string() const		{ return _str.str(); }
      std::string str() const			{ return _str.str(); }

      const std::ostream & stream() const	{ return _str; }
      std::ostream & stream()			{ return _str; }

      void clear()				{ _str.str( std::string() ); }

    private:
      std::ostringstream _str;
    };

    /** \relates Str Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Str & obj )
    { return str << obj.str(); }

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

    template<> inline std::string asString( const char & t )			{ return numstring( t ); }
    template<> inline std::string asString( const unsigned char & t )		{ return numstring( t ); }
    template<> inline std::string asString( const short & t )			{ return numstring( t ); }
    template<> inline std::string asString( const unsigned short & t )		{ return numstring( t ); }
    template<> inline std::string asString( const int & t )			{ return numstring( t ); }
    template<> inline std::string asString( const unsigned & t )		{ return numstring( t ); }
    template<> inline std::string asString( const long & t )			{ return numstring( t ); }
    template<> inline std::string asString( const unsigned long & t )		{ return numstring( t ); }
    template<> inline std::string asString( const long long & t )		{ return numstring( t ); }
    template<> inline std::string asString( const unsigned long long & t )	{ return numstring( t ); }
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
    /** String representation of number as bit-string with leading '0's. */
    template <typename TInt>
    std::string binstring( TInt val_r )
    {
      constexpr unsigned bits = sizeof(TInt)*8;
      std::string ret( bits, ' ' );
      TInt bit = 1;
      for ( unsigned pos = bits; pos > 0; )
      { --pos; ret[pos] = ((val_r & bit)?'1':'0'); bit = bit<<1; }
      return ret;
    }
    
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
    template<typename TInt>
      TInt strtonum( const C_Str & str );

    template<>
      inline short              strtonum( const C_Str & str ) { return ::strtol  ( str, NULL, 0 ); }
    template<>
      inline int                strtonum( const C_Str & str ) { return ::strtol  ( str, NULL, 0 ); }
    template<>
      inline long               strtonum( const C_Str & str ) { return ::strtol  ( str, NULL, 0 ); }
    template<>
      inline long long          strtonum( const C_Str & str ) { return ::strtoll ( str, NULL, 0 ); }

    template<>
      inline unsigned short     strtonum( const C_Str & str ) { return ::strtoul ( str, NULL, 0 ); }
    template<>
      inline unsigned           strtonum( const C_Str & str ) { return ::strtoul ( str, NULL, 0 ); }
    template<>
      inline unsigned long      strtonum( const C_Str & str ) { return ::strtoul ( str, NULL, 0 ); }
    template<>
      inline unsigned long long strtonum( const C_Str & str ) { return ::strtoull( str, NULL, 0 ); }

    /** String to integer type detemined 2nd function arg \a i.
     * \code
     * time_t t; strtonum( "42", t );
     * \endcode
    */
    template<typename TInt>
      inline TInt strtonum( const C_Str & str, TInt & i )
      { return i = strtonum<TInt>( str ); }
    //@}

    ///////////////////////////////////////////////////////////////////
    /** Parsing boolean from string.
    */
    //@{
    /** Return \c true if str is <tt>1, true, yes, on</tt> (or a nonzero number). */
    bool strToTrue( const C_Str & str );

    /** Return \c false if str is <tt>0, false, no, off</tt>. */
    bool strToFalse( const C_Str & str );

    /** Parse \c str into a bool depending on the default value.
     * If the \c default is true, look for a legal \c false string.
     * If the \c default is false, look for a legal \c true string.
     */
    inline bool strToBool( const C_Str & str, bool default_r )
    { return( default_r ? strToFalse( str ) : strToTrue( str ) ); }

    /** Parse \c str into a bool if it's a legal \c true or \c false string.
     * If \c str is not a recognized \c true or \c false string, \a return_r
     * is left unchanged.
     */
    inline bool strToBoolNodefault( const C_Str & str, bool & return_r )
    {
      if ( strToTrue( str ) ) return (return_r = true);
      if ( !strToFalse( str ) ) return (return_r = false);
      return return_r;
    }

    /** Parse \c str into a bool if it's a legal \c true or \c false string; else \c indterminate. */
    TriBool strToTriBool( const C_Str & str );

    //@}

    /**
     * \short Return a string with all occurrences of \c from_r replaced with \c to_r.
     */
    std::string gsub( const std::string & str_r, const std::string & from_r, const std::string & to_r );

    /** \overload A function is called on demand to compute each replacement value.
     */
    std::string gsubFun( const std::string & str_r, const std::string & from_r, function<std::string()> to_r );

    /**
     * \short Replace all occurrences of \c from_r with \c to_r in \c str_r (inplace).
     * A reference to \c str_r is also returned for convenience.
     */
    std::string & replaceAll( std::string & str_r, const std::string & from_r, const std::string & to_r );

    /** \overload A function is called on demand to compute each replacement value.
     */
    std::string & replaceAllFun( std::string & str_r, const std::string & from_r, function<std::string()> to_r );

    /** Enhance readability: insert gaps at regular distance
     * \code
     *   // no gaps
     *   Key Fingerprint:  22C07BA534178CD02EFE22AAB88B2FD43DBDC284
     *   // gapify 8
     *   Key Fingerprint:  22C07BA5 34178CD0 2EFE22AA B88B2FD4 3DBDC284
     *   // gapify 4
     *   Key Fingerprint:  22C0 7BA5 3417 8CD0 2EFE 22AA B88B 2FD4 3DBD C284
     *   // gapify 4, '-'
     *   Key Fingerprint:  22C0-7BA5-3417-8CD0-2EFE-22AA-B88B-2FD4-3DBD-C284
     * \endcode
     */
    inline std::string gapify( std::string inp_r, std::string::size_type gap_r = 1, char gapchar = ' ' )
    {
      if ( gap_r &&  inp_r.size() > gap_r )
      {
	inp_r.reserve( inp_r.size() + (inp_r.size()-1)/gap_r );
	for ( std::string::size_type pos = gap_r; pos < inp_r.size(); pos += gap_r+1 )
	  inp_r.insert( pos, 1, gapchar );
      }
      return inp_r;
    }

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
    template<class TOutputIterator>
      unsigned split( const C_Str & line_r, TOutputIterator result_r, const C_Str & sepchars_r = " \t" )
      {
        const char * beg = line_r;
        const char * cur = beg;
        // skip leading sepchars
        while ( *cur && ::strchr( sepchars_r, *cur ) )
          ++cur;
        unsigned ret = 0;
        for ( beg = cur; *beg; beg = cur, ++result_r, ++ret )
          {
            // skip non sepchars
            while( *cur && !::strchr( sepchars_r, *cur ) )
              ++cur;
            // build string
            *result_r = std::string( beg, cur-beg );
            // skip sepchars
            while ( *cur && ::strchr( sepchars_r, *cur ) )
              ++cur;
          }
        return ret;
      }

    /** Split \a line_r into words with respect to escape delimeters.
     * Any sequence of characters in \a sepchars_r is treated as
     * delimiter if not inside \c "" or \c '' or escaped by \c \.
     *
     * \li A non-quoted backslash (\) preserves the literal value of the next character.
     * \li Enclosing characters in single quotes preserves the literal value of each
     *     character within the quotes.  A single quote may not occur between single
     *     quotes, even when preceded by a backslash.
     * \li Enclosing characters in double quotes preserves the literal value of all
     *     characters within the quotes, with the exception of \c \. The backslash
     *     retains its special meaning only when followed by \c " or \c \.
     *
     * The words are passed to OutputIterator \a result_r.
     *
     * \see \ref splitEscaped
     *
     * \code
     * std::vector<std::string> words;
     * str::splitEscaped( "some line", std::back_inserter(words) )
     * \endcode
     *
     * \code
     * example splitted strings
     * normal line -> 2 elements ( "normal", "line" )
     * escaped\ line -> 1 element(escaped line)
     * "quoted line" -> 1 element same as above
     * 'quoted line' -> 1 element same as above
     * "escaped quote\"" -> 1 element (escaped quote")
     *
     * \param line_r   The string to parse.
     * \param result_r
     * \param sepchars_r  String of separator characters.
     * \param withEmpty   Whether to include empty fields between separators in the result.
     *
     * \endcode
     */
    template<class TOutputIterator>
      unsigned splitEscaped( const C_Str & line_r, TOutputIterator result_r, const C_Str & sepchars_r = " \t", bool withEmpty = false)
      {
        const char * beg = line_r;
        const char * cur = beg;
        unsigned ret = 0;

        // skip leading sepchars
        while ( *cur && ::strchr( sepchars_r, *cur ) )
        {
          ++cur;
          if (withEmpty)
          {
            *result_r = "";
            ++ret;
          }
        }

        // there were only sepchars in the string
        if (!*cur && withEmpty)
        {
          *result_r = "";
          return ++ret;
        }

        // after the leading sepchars
	enum class Quote { None, Slash, Single, Double, DoubleSlash };
	std::vector<char> buf;
	Quote quoting = Quote::None;
        for ( beg = cur; *beg; beg = cur, ++result_r, ++ret )
	{
	  // read next value until unquoted sepchar
	  buf.clear();
	  quoting = Quote::None;
	  do {
	    switch ( quoting )
	    {
	      case Quote::None:
		switch ( *cur )
		{
		  case '\\':	quoting = Quote::Slash;		break;
		  case '\'':	quoting = Quote::Single;	break;
		  case '"':	quoting = Quote::Double;	break;
		  default:	buf.push_back( *cur );		break;
		}
		break;

	      case Quote::Slash:
		buf.push_back( *cur );
		quoting = Quote::None;
		break;

	      case Quote::Single:
		switch ( *cur )
		{
		  case '\'':	quoting = Quote::None;		break;
		  default:	buf.push_back( *cur );		break;
		}
		break;

	      case Quote::Double:
		switch ( *cur )
		{
		  case '\"':	quoting = Quote::None;		break;
		  case '\\':	quoting = Quote::DoubleSlash;	break;
		  default:	buf.push_back( *cur );		break;
		}
		break;

	      case Quote::DoubleSlash:
		switch ( *cur )
		{
		  case '\"':	/*fallthrough*/
		  case '\\':	buf.push_back( *cur );		break;
		  default:
		    buf.push_back( '\\' );
		    buf.push_back( *cur );
		    break;
		}
		quoting = Quote::Double;
		break;
	    }
	    ++cur;
	  } while ( *cur && ( quoting != Quote::None || !::strchr( sepchars_r, *cur ) ) );
	  *result_r = std::string( buf.begin(), buf.end() );


	  // skip sepchars
	  if ( *cur && ::strchr( sepchars_r, *cur ) )
	    ++cur;
	  while ( *cur && ::strchr( sepchars_r, *cur ) )
	  {
	    ++cur;
	    if (withEmpty)
	    {
	      *result_r = "";
	      ++ret;
	    }
	  }
	  // the last was a separator => one more field
	  if ( !*cur && withEmpty && ::strchr( sepchars_r, *(cur-1) ) )
	  {
	    *result_r = "";
	    ++ret;
	  }
	}
        return ret;
      }

    /** Split \a line_r into fields.
     * Any single character in \a sepchars_r is treated as a
     * field separator unless \-escaped. The words are passed
     * to OutputIterator.
     * \a result_r.
     * \code
     * ""        -> words 0
     * ":"       -> words 2  |||
     * "a"       -> words 1  |a|
     * ":a"      -> words 2  ||a|
     * "a:"      -> words 2  |a||
     * ":a:"     -> words 3  ||a||
     *
     * \endcode
     *
     * \code
     * std::vector<std::string> words;
     * str::split( "some line", std::back_inserter(words) )
     * \endcode
     *
    */
    template<class TOutputIterator>
      unsigned splitFields( const C_Str & line_r, TOutputIterator result_r, const C_Str & sepchars_r = ":" )
      {
        const char * beg = line_r;
        const char * cur = beg;
        unsigned ret = 0;
        for ( beg = cur; *beg; beg = cur, ++result_r )
          {
            // skip non sepchars
            while( *cur && !::strchr( sepchars_r, *cur ) )
	    {
	      if ( *cur == '\\' && *(cur+1) )
		++cur;
              ++cur;
	    }
            // build string
            *result_r = std::string( beg, cur-beg );
            ++ret;
            // skip sepchar
            if ( *cur )
            {
              ++cur;
              if ( ! *cur )                // ending with sepchar
              {
                *result_r = std::string(); // add final empty field
                ++ret;
                break;
              }
            }
          }
        return ret;
      }

    /**
     * Split \a line_r into fields handling also escaped separators.
     *
     * \see splitFields()
     * \see splitEscaped()
     */
    template<class TOutputIterator>
      unsigned splitFieldsEscaped( const C_Str & line_r, TOutputIterator result_r, const C_Str & sepchars_r = ":" )
      {
        return splitEscaped( line_r, result_r, sepchars_r, true /* withEmpty */ );
      }

    //@}

    ///////////////////////////////////////////////////////////////////
    /** \name Join. */
    //@{
    /** Join strings using separator \a sep_r (defaults to BLANK). */
    template <class TIterator>
      std::string join( TIterator begin, TIterator end, const C_Str & sep_r = " " )
      {
        std::string res;
        for ( TIterator iter = begin; iter != end; ++ iter )
          {
            if ( iter != begin )
              res += sep_r;
            res += asString(*iter);
          }
        return res;
      }

    /** Join strings using separator \a sep_r (defaults to BLANK). */
    template <class TContainer>
      std::string join( const TContainer & cont_r, const C_Str & sep_r = " " )
      { return join( cont_r.begin(), cont_r.end(), sep_r ); }

    /** Join strings using separator \a sep_r, quoting or escaping the values.
     * Separator defaults to BLANK. Use \ref splitEscaped to restore the
     * values.
     */
    template <class TIterator>
      std::string joinEscaped( TIterator begin, TIterator end, const char sep_r = ' ' )
      {
        std::vector<char> buf;
        for ( TIterator iter = begin; iter != end; ++ iter )
        {
          if ( iter != begin )
            buf.push_back( sep_r );

          if ( iter->empty() )
          {
            // empty string goes ""
            buf.push_back( '"' );
            buf.push_back( '"' );
          }
          else
          {
            std::string toadd( asString(*iter) );
            for_( ch, toadd.begin(), toadd.end() )
            {
              switch ( *ch )
              {
                case '"':
                case '\'':
                case '\\':
                  buf.push_back( '\\' );
                  buf.push_back( *ch );
                  break;
                default:
                  if ( *ch == sep_r )
                    buf.push_back( '\\' );
                  buf.push_back( *ch );
              }
            }
          }
        }
        return std::string( buf.begin(), buf.end() );
      }
    //@}


    ///////////////////////////////////////////////////////////////////
    /** \name Indent. */
    //@{
      /** Indent by string ["  "] optionally wrap.
       * Prints nothing for an empty string. Asserts a trainling '\n' on
       * the last line. Optionally wrap lines at ' ' at a given length.
       */
      inline std::ostream & printIndented( std::ostream & str, const std::string & text_r, const std::string & indent_r = "  ", unsigned maxWitdh_r = 0 )
      {
	if ( maxWitdh_r )
	{
	  if ( indent_r.size() >= maxWitdh_r )
	    maxWitdh_r = 0;	// nonsense: indent larger than line witdh
	  else
	    maxWitdh_r -= indent_r.size();
	}
	unsigned width = 0;
	for ( const char * e = text_r.c_str(), * s = e; *e; s = ++e )
	{
	  for ( ; *e && *e != '\n'; ++e ) ;/*searching*/
	  width = e-s;
	  if ( maxWitdh_r && width > maxWitdh_r )
	  {
	    // must break line
	    width = maxWitdh_r;
	    for ( e = s+width; e > s && *e != ' '; --e ) ;/*searching*/
	    if ( e > s )
	      width = e-s;	// on a ' ', replaced by '\n'
	    else
	      e = s+width-1;	// cut line;
	  }
	  str << indent_r;
	  str.write( s, width );
	  str << "\n";
	  if ( !*e )	// on '\0'
	    break;
	}
	return str;
      }
      /** \overload Indent by number of chars [' '] optionally wrap. */
      inline std::ostream & printIndented( std::ostream & str, const std::string & text_r, unsigned indent_r, char indentch_r = ' ', unsigned maxWitdh_r = 0 )
      { return printIndented( str, text_r, std::string( indent_r, indentch_r ), maxWitdh_r ); }
      /** \overload Indent by number of chars [' '] wrap. */
      inline std::ostream & printIndented( std::ostream & str, const std::string & text_r, unsigned indent_r, unsigned maxWitdh_r, char indentch_r = ' ' )
      { return printIndented( str, text_r, std::string( indent_r, indentch_r ), maxWitdh_r ); }

      /** Prefix lines by string computed by function taking line begin/end [std::string(const char*, const char*)]
       * Prints nothing for an empty string. Asserts a trainling '\n' on the last line.
       */
      inline std::ostream & autoPrefix( std::ostream & str, const std::string & text_r, function<std::string(const char*, const char*)> fnc_r )
      {
	for ( const char * e = text_r.c_str(); *e; ++e )
	{
	  const char * s = e;
	  for ( ; *e && *e != '\n'; ++e ) /*searching*/;
	  str << fnc_r( s, e );
	  str.write( s, e-s );
	  str << "\n";
	  if ( !*e )	// on '\0'
	    break;
	}
	return str;
      }
      /** \overload Prefix lines by string generated by function [std::string()] */
      inline std::ostream & autoPrefix0( std::ostream & str, const std::string & text_r, function<std::string()> fnc_r )
      {
	auto wrap = [&fnc_r]( const char*, const char* )-> std::string {
	  return fnc_r();
	};
	return autoPrefix( str, text_r, wrap );
      }
    //@}
    ///////////////////////////////////////////////////////////////////
    /** \name Escape. */
    //@{
      /**
       * Escape desired character \a c using a backslash.
       *
       * For use when printing \a c separated values, and where
       * \ref joinEscaped() is too heavy.
       */
      std::string escape( const C_Str & str_r, const char c = ' ' );

      /** Escape \a next_r and append it to \a str_r using separator \a sep_r. */
      inline void appendEscaped( std::string & str_r, const C_Str & next_r, const char sep_r = ' ' )
      {
        if ( ! str_r.empty() )
          str_r += sep_r;
        if ( next_r.empty() )
          str_r += "\"\"";
        else
          str_r += escape( next_r, sep_r );
      }

      //! \todo unsecape()

    //@}
    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    /** \name Hexencode.
     * Encode all characters other than [a-zA-Z0-9] as %XX.
     * This includes the % character itself, which becomes %25.
     */
    //@{
    /** Encode all characters other than [a-zA-Z0-9] as %XX.
     * This includes the % character itself, which becomes %25.
     */
    std::string hexencode( const C_Str & str_r );
    /** Decode hexencoded %XX sequences. */
    std::string hexdecode( const C_Str & str_r );
    //@}
    ///////////////////////////////////////////////////////////////////

    /** \name Case conversion. */
    //@{
    /** Return lowercase version of \a s
     * \todo improve
    */
    std::string toLower( const std::string & s );
    /** \overload */
    inline std::string toLower( const char * s )
    { return( s ? toLower( std::string(s) ) : std::string() ); }

    /** Return uppercase version of \a s
     * \todo improve
    */
    std::string toUpper( const std::string & s );
    /** \overload */
    inline std::string toUpper( const char * s )
    { return( s ? toUpper( std::string(s) ) : std::string() ); }
    //@}


    /** \name Case insensitive comparison. */
    //@{
    inline int compareCI( const C_Str & lhs, const C_Str & rhs )
    { return ::strcasecmp( lhs, rhs ); }
    //@}

    /** \name Locate substring. */
    //@{
    /** Locate substring case sensitive. */
    inline bool contains( const C_Str & str_r, const C_Str & val_r )
    { return ::strstr( str_r, val_r ); }
    /** Locate substring case insensitive. */
    inline bool containsCI( const C_Str & str_r, const C_Str & val_r )
    { return ::strcasestr( str_r, val_r ); }
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

    std::string trim( const std::string & s, const Trim trim_r = TRIM );

    inline std::string ltrim( const std::string & s )
    { return trim( s, L_TRIM ); }

    inline std::string rtrim( const std::string & s )
    { return trim( s, R_TRIM ); }
    //@}

    std::string stripFirstWord( std::string & line, const bool ltrim_first = true );

    std::string stripLastWord( std::string & line, const bool rtrim_first = true );

    /** Return stream content up to (but not returning) the next newline.
     * \see \ref receiveUpTo
     */
    std::string getline( std::istream & str, bool trim = false );

    /** Return stream content up to (but not returning) the next newline.
     * \see \ref receiveUpTo
     */
    std::string getline( std::istream & str, const Trim trim_r );

    /** Return stream content up to the next ocurrence of \c delim_r or EOF
     * \c delim_r, if found, is always read from the stream. Whether it is
     * also returned in the string depends on \c returnDelim_r.
     * If the stream status is \c good, \c delim_r was found in the stream.
     * If we reached EOF while looking for \c delim_r, \c eof is set; and
     * also \c fail, if we did not read any data before.
     */
    std::string receiveUpTo( std::istream & str, const char delim_r, bool returnDelim_r = false );

    ///////////////////////////////////////////////////////////////////

    /** \name String prefix/suffix handling.
     */
    //@{
    /** Return whether \a str_r has prefix \a prefix_r. */
    inline bool hasPrefix( const C_Str & str_r, const C_Str & prefix_r )
    { return( ::strncmp( str_r, prefix_r, prefix_r.size() ) == 0 ); }

    /** Strip a \a prefix_r from \a str_r and return the resulting string. */
    inline std::string stripPrefix( const C_Str & str_r, const C_Str & prefix_r )
    { return( hasPrefix( str_r, prefix_r ) ? str_r + prefix_r.size() : str_r.c_str() ); }

    /** Return whether \a str_r has suffix \a suffix_r. */
    inline bool hasSuffix( const C_Str & str_r, const C_Str & suffix_r )
    { return( str_r.size() >= suffix_r.size() && ::strncmp( str_r + str_r.size() - suffix_r.size() , suffix_r, suffix_r.size() ) == 0 ); }

    /** Strip a \a suffix_r from \a str_r and return the resulting string. */
    inline std::string stripSuffix( const C_Str & str_r, const C_Str & suffix_r )
    {
      if ( hasSuffix( str_r, suffix_r ) )
        return std::string( str_r, str_r.size() - suffix_r.size() );
      return str_r.c_str();
    }
    /** Return size of the common prefix of \a lhs and \a rhs. */
    inline std::string::size_type commonPrefix( const C_Str & lhs, const C_Str & rhs )
    {
      const char * lp = lhs.c_str();
      const char * rp = rhs.c_str();
      std::string::size_type ret = 0;
      while ( *lp == *rp && *lp != '\0' )
      { ++lp, ++rp, ++ret; }
      return ret;
    }

    /** alias for \ref hasPrefix */
    inline bool startsWith( const C_Str & str_r, const C_Str & prefix_r )
    { return hasPrefix( str_r, prefix_r ); }
    /** alias for \ref hasSuffix */
    inline bool endsWith( const C_Str & str_r, const C_Str & prefix_r )
    { return hasSuffix( str_r, prefix_r ); }
    //@}
  } // namespace str
  ///////////////////////////////////////////////////////////////////

  // drag into zypp:: namespace
  using str::asString;

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_STRING_H
