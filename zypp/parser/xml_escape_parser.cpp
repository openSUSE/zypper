/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/xml_escape_parser.cpp
 *
*/

#include <string>
#include "xml_escape_parser.hpp"

namespace iobind{
namespace parser{

std::string xml_escape_parser::escape(const std::string &istr) const
{
  size_t i;
  std::string str = istr;
  i = str.find_first_of("<>&'\"");
  while (i != std::string::npos)
    {
      switch (str[i])
        {
	  case '<': str.replace(i, 1, "&lt;"); i += 3; break;
	  case '>': str.replace(i, 1, "&gt;"); i += 3; break;
	  case '&': str.replace(i, 1, "&amp;"); i += 4; break;
	  case '\'': str.replace(i, 1, "&apos;"); i += 5; break;
	  case '"': str.replace(i, 1, "&quot;"); i += 5; break;
	}
      i = str.find_first_of("<>&'\"", i + 1);
    }
  return str;
}

std::string xml_escape_parser::unescape(const std::string &istr) const
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
}; // parser
}; // iobind
