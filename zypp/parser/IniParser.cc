/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/IniParser.cc
 *
*/
#include <iostream>
#include <sstream>

#include <boost/regex.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/IOStream.h"

#include "zypp/parser/ParseException.h"
#include "zypp/parser/IniParser.h"
#include "zypp/ProgressData.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace parser
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : IniParser::IniParser
//	METHOD TYPE : Ctor
//
IniParser::IniParser()
  : _line_nr(0)
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : IniParser::~IniParser
//	METHOD TYPE : Dtor
//
IniParser::~IniParser()
{}

void IniParser::beginParse()
{}

void IniParser::consume( const std::string &section, const std::string &key, const std::string &value )
{}

void IniParser::consume( const std::string &section )
{}

void IniParser::endParse()
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : IniParser::parse
//	METHOD TYPE : void
//
void IniParser::parse( const InputStream & input_r )
{
  boost::regex rxSection("^\\[(.+)\\]$");
  boost::regex rxKeyValue("^(.+)[[:space:]]*=[[:space:]]*(.+)$");
  
  MIL << "Start parsing " << input_r << endl;
  _inputname = input_r.name();
  beginParse();

  ProgressData ticks( makeProgressData( input_r ) );
  ticks.toMin();

  iostr::EachLine line( input_r );
  for ( ; line; line.next() )
  {
    std::string trimmed = str::trim(*line);
    const char *where = trimmed.c_str(); /* Skip leading spaces */
    if (*where==';' || *where=='#' || *where==0)
      continue ; /* Comment lines */
    else
    {
      if (*where=='[' )
      {
        boost::smatch what;
        if(boost::regex_match(trimmed, what, rxSection, boost::match_extra))
        {
          //DBG << what << endl;
          std::string section = what[1];
          consume(section);
          section.swap(_current_section);
        }
      }
      else
      {
        boost::smatch what;
        if(boost::regex_match(trimmed, what, rxKeyValue, boost::match_extra))
        {
          //DBG << what << endl;
          consume( _current_section, what[1], what[2] );
        }
      }
    }
    ticks.set( input_r.stream().tellg() );
  }
  ticks.toMax();

  endParse();
  _inputname.clear();
  MIL << "Done parsing " << input_r << endl;
}

/////////////////////////////////////////////////////////////////
} // namespace parser
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
