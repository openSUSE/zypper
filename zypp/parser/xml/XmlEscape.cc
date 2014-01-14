/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xml/XmlEscape.cc
 *
*/

#include <string>
#include "zypp/parser/xml/XmlEscape.h"

/*
IoBind Library License:
--------------------------

The zlib/libpng License Copyright (c) 2003 Jonathan de Halleux

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution
*/
///////////////////////////////////////////////////////////////////
namespace iobind
{
  ///////////////////////////////////////////////////////////////////
  namespace parser
  {
    struct ZYPP_LOCAL xml_escape_parser
    {
      std::string unescape(const std::string &istr) const
      {
	size_t i;
	std::string str = istr;
	i = str.find_first_of("&");
	while (i != std::string::npos)
	{
	  if (str[i] == '&')
	  {
	    if (!str.compare(i + 1, 3, "lt;"))
	      str.replace(i, 4, 1, '<');
	    else if (!str.compare(i + 1, 3, "gt;"))
	      str.replace(i, 4, 1, '>');
	    else if (!str.compare(i + 1, 4, "amp;"))
	      str.replace(i, 5, 1, '&');
	    else if (!str.compare(i + 1, 5, "apos;"))
	      str.replace(i, 6, 1, '\'');
	    else if (!str.compare(i + 1, 5, "quot;"))
	      str.replace(i, 6, 1, '"');
	  }
	  i = str.find_first_of("&", i + 1);
	}
	return str;
      }
    };
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
} // namespace iobind
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace xml
  {
    ///////////////////////////////////////////////////////////////////
    namespace detail
    {
      std::ostream & EscapedString::dumpOn( std::ostream & str ) const
      {
	typedef unsigned char uchar;
	for ( char ch : _in )
	{
	  switch ( ch )
	  {
	    case '<':	str << "&lt;";		break;
	    case '>':	str << "&gt;";		break;
	    case '&':	str << "&amp;";		break;
	    case '"':	str << "&quot;";	break;
	    case '\'':	str << "&apos;";	break;

	    // control chars we allow:
	    case '\n':
	    case '\r':
	    case '\t':
	      str << ch;
	      break;

	    default:
	      if ( uchar(ch) < 32u )
		str << '?'; // filter problematic control chars (XML1.0)
	      else
		str << ch;
	      break;
	  }
	}
	return str;
      }

    } // detail
    ///////////////////////////////////////////////////////////////////

    std::string unescape( const std::string & in_r )
    { return iobind::parser::xml_escape_parser().unescape( in_r ); }

  } // namespace xml
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
