#ifndef ZYPPERRICHTEXT_H_
#define ZYPPERRICHTEXT_H_

#include <iosfwd>
#include <zypp/base/String.h>
#include <zypp/base/Regex.h>

/** Print [Rich]Text optionally indented.
 * Richtext is introduced by a '<!-- DT:Rich -->'
 * or '<p>'(bsc#935885) tag.
 */
inline std::ostream & printRichText( std::ostream & str, std::string text, unsigned indent_r = 0U, unsigned width_r = 0U )
{
  std::string processRichText( const std::string& text );

  if ( text.empty() )
    return str;

  static const str::regex rttag("^[ \t\r\n]*<(p|!--[- ]*DT:Rich[ -]*--)>");
  if( zypp::str::regex_match( text, rttag ) )
    text = processRichText( text );

  return str::printIndented( str, text, indent_r, width_r );	// even unindented as it also asserts a trailing '/n'
}

/** Return [Rich]Text optionally indented as string. */
inline std::string printRichText( std::string text, unsigned indent_r = 0U, unsigned width_r = 0U )
{
  str::Str s;
  printRichText( s.stream(), std::move(text), indent_r, width_r );
  return s.str();
}
#endif

