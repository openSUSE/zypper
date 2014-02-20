#ifndef ZYPPERRICHTEXT_H_
#define ZYPPERRICHTEXT_H_

#include <iosfwd>
#include <zypp/base/String.h>

/** Print [Rich]Text optionally indented. */
inline std::ostream & printRichText( std::ostream & str, std::string text, unsigned indent_r = 0U, unsigned width_r = 0U )
{
  std::string processRichText( const std::string& text );

  if ( text.empty() )
    return str;

  if ( text.find("DT:Rich") != std::string::npos )
    text = processRichText( text );

  return zypp::str::printIndented( str, text, indent_r, width_r );	// even unindented as it also asserts a trailing '/n'
}

#endif

