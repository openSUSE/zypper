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

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/UserRequestException.h"

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

  namespace {
    inline const std::string & keyGarbage()
    {
      static const std::string & _val( ",|/\\" );
      return _val;
    }
  } //namespace

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

void IniParser::garbageLine( const std::string &section, const std::string &line )
{
  std::string msg = str::form("%s: Section [%s]: Line %d contains garbage (no '=' or '%s' in key)",
			      _inputname.c_str(), section.c_str(), _line_nr, keyGarbage().c_str());
  ZYPP_THROW(ParseException(msg));
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : IniParser::parse
//	METHOD TYPE : void
//
void IniParser::parse( const InputStream & input_r, const ProgressData::ReceiverFnc & progress )
{
  MIL << "Start parsing " << input_r << endl;
  _inputname = input_r.name();
  beginParse();

  ProgressData ticks( makeProgressData( input_r ) );
  ticks.sendTo(progress);
  ticks.toMin();

  iostr::EachLine line( input_r );
  for ( ; line; line.next() )
  {
    std::string trimmed = str::trim(*line);

    if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#')
      continue ; /* Comment lines */

    if (trimmed[0] == '[')
    {
      std::string::size_type pos = trimmed.find(']');
      if ( pos != std::string::npos )
      {
	std::string section = trimmed.substr(1, pos-1);
	consume(section);
	section.swap(_current_section);
      }
      else
      {
	_line_nr = line.lineNo();
	garbageLine( _current_section, trimmed );
      }
      continue;
    }

    std::string::size_type pos = trimmed.find('=');
    if ( pos == std::string::npos || trimmed.find_first_of( keyGarbage() ) < pos )
    {
      _line_nr = line.lineNo();
      garbageLine( _current_section, trimmed );	// may or may not throw
    }
    else
    {
      std::string key = str::rtrim(trimmed.substr(0, pos));
      std::string value = str::ltrim(trimmed.substr(pos+1));
      consume( _current_section, key, value);
    }

    // set progress and allow cancel
    if ( ! ticks.set( input_r.stream().tellg() ) )
      ZYPP_THROW(AbortRequestException());
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
