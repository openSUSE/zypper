/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* Strictly for internal use!
*/
#ifndef ZYPP_TUI_UTILS_RICHTEXT_H_
#define ZYPP_TUI_UTILS_RICHTEXT_H_

#include <iosfwd>
#include <zypp-core/base/String.h>
#include <zypp-core/base/Regex.h>

namespace ztui {

/** Print [Rich]Text optionally indented.
 * Richtext is introduced by a '<!-- DT:Rich -->'
 * or '<p>'(bsc#935885) tag.
 */
inline std::ostream & printRichText( std::ostream & str, std::string text, unsigned indent_r = 0U, unsigned width_r = 0U )
{
  std::string processRichText( const std::string& text );

  if ( text.empty() )
    return str;

  static const zypp::str::regex rttag("^[ \t\r\n]*<(p|!--[- ]*DT:Rich[ -]*--)>");
  if( zypp::str::regex_match( text, rttag ) )
    text = processRichText( text );

  return zypp::str::printIndented( str, text, indent_r, width_r );	// even unindented as it also asserts a trailing '/n'
}

/** Return [Rich]Text optionally indented as string. */
inline std::string printRichText( std::string text, unsigned indent_r = 0U, unsigned width_r = 0U )
{
  zypp::str::Str s;
  printRichText( s.stream(), std::move(text), indent_r, width_r );
  return s.str();
}

}

#endif

