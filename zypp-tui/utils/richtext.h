#ifndef ZYPPERRICHTEXT_H_
#define ZYPPERRICHTEXT_H_

#include <iosfwd>
#include <zypp/base/String.h>

/** Convert . */
//std::string processRichText( const std::string& text );

/** Print [Rich]Text optionally indented. */
inline std::ostream & printRichText( std::ostream & str, std::string text, unsigned indent_r = 0U )
{
  std::string processRichText( const std::string& text );

  if ( text.empty() )
    return str;

  if ( text.find("DT:Rich") != std::string::npos )
    text = processRichText( text );

  return str::printIndented( str, text, indent_r );	// even unindented as it also asaerts a trailing '/n'
}


#endif

