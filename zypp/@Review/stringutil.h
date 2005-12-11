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

///////////////////////////////////////////////////////////////////
}  // namespace stringutil
///////////////////////////////////////////////////////////////////

#endif // stringutil_h
