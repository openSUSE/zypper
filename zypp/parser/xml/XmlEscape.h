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

#include <string>
#include "zypp/APIConfig.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace xml
  {

    /** Escape xml special charaters (<tt>& -> &amp;</tt>; from IoBind library). */
    ZYPP_API std::string escape( const std::string & in_r );

    /** Unescape xml special charaters (<tt>&amp; -> &</tt>; from IoBind library) */
    ZYPP_API std::string unescape( const std::string & in_r );

  } // namespace xml
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_XML_XMLESCAPE_H
