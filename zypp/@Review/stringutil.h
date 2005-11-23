/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:       stringutil.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Contains 'std::string form(const char * format, ...)' for
  printf style creation of strings and some more string utility
  functions.

/-*/
#ifndef stringutil_h
#define stringutil_h

#include <cstdio>
#include <cstdarg>

#include <iosfwd>
#include <vector>
#include <string>
#include <list>

/**
 * Utility functions for std::strings. Most of them based on stringutil::form.
 **/
///////////////////////////////////////////////////////////////////
namespace stringutil {
;//////////////////////////////////////////////////////////////////

enum Trim {
  NO_TRIM = 0x00,
  L_TRIM  = 0x01,
  R_TRIM  = 0x02,
  TRIM    = (L_TRIM|R_TRIM)
};

inline std::string form( const char * format, ... )
    __attribute__ ((format (printf, 1, 2)));

/**
 * Printf style building of strings via format string.
 * <PRE>
 * std::string ex( stringutil::form( "Example number %d", 1 ) );
 * std::cout << ex << stringutil::form( " and number %d.", 2 ) << endl;
 *
 * Will print: Example number 1 and number 2.
 * </PRE>
 **/
inline std::string form( const char * format, ... ) {
  char * buf = 0;
  std::string val;

  va_list ap;
  va_start( ap, format );

#if 1
  vasprintf( &buf, format, ap );
  if ( buf ) {
    val = buf;
    free( buf );
  }
#else
  // Don't know wheter we actually nedd two va_lists, one to
  // evaluate the buffer size needed, and one to actually fill
  // the buffer. Maybe there's a save way to reuse a va_lists.
  va_list ap1;
  va_start( ap1, format );
  buf = new char[vsnprintf( NULL, 0, format, ap ) + 1];
  vsprintf( buf, format, ap1 );
  val = buf;
  delete [] buf;
  va_end( ap1 );
#endif

  va_end( ap );
  return val;
}

/**
 * Print number. Optional second argument sets the minimal string width (' ' padded).
 * Negative values will cause the number to be left adjusted within the string. Default
 * width is 0.
 * <PRE>
 * numstring(42)           -> "42"
 * numstring(42, 4)        -> "  42"
 * numstring(42,-4)        -> "42  "
 * </PRE>
 **/
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

/**
 * Print number as hex value with leading '0x'. Optional second argument sets the minimal
 * string width (0 padded). Negative values will cause the number to be left adjusted
 * within the string. Default width is 10 (4 for char).
 * <PRE>
 * hexstring(42)           -> "0x0000002a"
 * hexstring(42, 4)        -> "0x2a"
 * hexstring(42,-4)        -> "0x2a"
 * </PRE>
 **/
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

/**
 * Print number as octal value with leading '0'. Optional second argument sets the minimal
 * string width (0 padded). Negative values will cause the number to be left adjusted
 * within the string. Default width is 5 (4 for char).
 * <PRE>
 * octstring(42)           -> "00052"
 * octstring(42, 4)        -> "0052"
 * octstring(42,-4)        -> "052 "
 * </PRE>
 **/
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

/**
 * String to integer type determined by template arg: time_t t = strtonum<time_t>( "42" );
 **/
template<typename _It>
  inline _It strtonum( const std::string & str );

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

/**
 * String to integer type detemined function arg: time_t t; strtonum( "42", t );
 **/
template<typename _It>
  inline _It strtonum( const std::string & str, _It & i ) { return i = strtonum<_It>( str ); }

/** \brief read one line from a stream
 * Return one line read from istream. Afterwards the streampos is behind the delimiting '\n'
 * (or at EOF). The delimiting '\n' is <b>not</b> returned.
 *
 * If trim is true, the string returned is trimmed (surrounding whitespaces removed).
 * <PRE>
 * ifstream s( "somefile" );
 *
 * while ( s ) {
 *   string l = getline( s );
 *   if ( !(s.fail() || s.bad()) ) {
 *
 *     // l contains valid data to be consumed.
 *     // In case it makes any difference to you:
 *     if ( s.good() ) {
 *       // A delimiting '\n' was read.
 *     } else {
 *       // s.eof() is set: There's no '\n' at the end of file.
 *       // Note: The line returned may netvertheless be empty if trimed is true.
 *     }
 *   }
 * }
 * </PRE>
 **/
extern std::string getline( std::istream & str, bool trim = false );

/** \brief read one line from a stream
 *
 * like above but with allows to specify trimming direction
 * */
extern std::string getline( std::istream & str, const Trim trim_r );

/**
 * Split line into words
 *
 * <b>singlesep_r = false</b>: Separator is any nonenmpty sequence of characters listed in sep_t.
 * Leading trailing separators are ignored.
 *
 * <b>Example:</b> singlesep_r = false, sep_t = ":"
 * <PRE>
 * ""        -> words 0
 * ":"       -> words 0
 * "a"       -> words 1  |a|
 * "::a"     -> words 1  |a|
 * "::a::"   -> words 1  |a|
 * ":a::b:c:"-> words 3  |a|b|c|
 * </PRE>
 *
 * <b>singlesep_r = true</b>: Separator is any single character occuring in sep_t.
 * Leading trailing separators are not ignored (i.e will cause an empty word).
 *
 * <b>Example:</b> singlesep_r = true, sep_t = ":"
 * <PRE>
 * ""        -> words 0
 * ":"       -> words 2  |||
 * "a"       -> words 1  |a|
 * ":a"      -> words 2  ||a|
 * "a:"      -> words 2  |a||
 * ":a:"     -> words 3  ||a||
 * </PRE>
 *
 **/
extern unsigned split( const std::string          line_r,
                       std::vector<std::string> & words_r,
                       const std::string &        sep_t       = " \t",
                       const bool                 singlesep_r = false );

/**
 * Join strinngs in words_r using separator sep_r
 **/
extern std::string join( const std::vector<std::string> & words_r,
			 const std::string & sep_r = " " );


/**
 * Split string into a list of lines using <b>any<\b> char in sep_r as line
 * delimiter. The delimiter is stripped from the line.
 *
 * <PRE>
 * splitToLines( "start\n\nend" ) -> { "start", "", "end" }
 * </PRE>
 **/
inline std::list<std::string> splitToLines( const std::string text_r, const std::string & sep_r = "\n" )
{
  std::vector<std::string> lines;
  stringutil::split( text_r, lines, "\n", true );
  std::list<std::string> ret;
  for ( unsigned i = 0; i < lines.size(); ++i ) {
    ret.push_back( lines[i] );
  }
  return ret;
}

/**
 * Strip the first word (delimited by blank or tab) from value, and return it.
 * Adjust value to start with the second word afterwards.
 *
 * If value starts with blank or tab, the <b>first word is empty</b> and value will be
 * ltrimmed afterwards.
 *
 * If ltrim_first is true, value will be ltrimmed before stripping the first word. Thus
 * first word is empty, iff value is empty or contains whitespace only.
 *
 * <PRE>
 * stripFirstWord( "1st" )             ==  "1st" and value truncated to ""
 * stripFirstWord( "1st word" )        ==  "1st" and value truncated to "word"
 * stripFirstWord( " 1st word" )       ==  ""    and value truncated to "1st word"
 * stripFirstWord( " 1st word", true ) ==  "1st" and value truncated to "word"
 * </PRE>
 **/
extern std::string stripFirstWord( std::string & value, const bool ltrim_first = false );

/**
 * Return string with leading/trailing/surrounding whitespace removed
 **/
extern std::string ltrim( const std::string & s );
extern std::string rtrim( const std::string & s );
inline std::string  trim( const std::string & s, const Trim trim_r = TRIM ) {
  switch ( trim_r ) {
  case L_TRIM:
    return ltrim( s );
  case R_TRIM:
    return rtrim( s );
  case TRIM:
    return ltrim( rtrim( s ) );
  case NO_TRIM:
    break;
  }
  return s;
}

/**
 * Return string converted to lower/upper case
 **/
extern std::string toLower( const std::string & s );
extern std::string toUpper( const std::string & s );

/**
 * Helper for stream output
 **/
extern std::ostream & dumpOn( std::ostream & str, const std::list<std::string> & l, const bool numbered = false );
extern std::ostream & dumpOn( std::ostream & str, const std::vector<std::string> & l, const bool numbered = false );

///////////////////////////////////////////////////////////////////
}  // namespace stringutil
///////////////////////////////////////////////////////////////////

#endif // stringutil_h
