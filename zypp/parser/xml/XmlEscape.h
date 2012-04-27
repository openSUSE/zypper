/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xml/XmlEscape.h
 *
*/
#ifndef ZYPP_PARSER_XML_XMLESCAPE_H
#define ZYPP_PARSER_XML_XMLESCAPE_H

// from IoBind Library:
#include "zypp/parser/xml/xml_escape_parser.hpp"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    /** Escape xml special charaters (<tt>& -> &amp;</tt>; from IoBind library). */
    inline std::string escape( const std::string & in_r )
    { return iobind::parser::xml_escape_parser().escape( in_r ); }

    /** Unescape xml special charaters (<tt>&amp; -> &</tt>; from IoBind library) */
    inline std::string unescape( const std::string & in_r )
    { return iobind::parser::xml_escape_parser().unescape( in_r ); }

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_XML_XMLESCAPE_H
