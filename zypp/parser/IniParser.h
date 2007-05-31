/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/IniParser.h
 *
*/
#ifndef ZYPP_PARSER_INIPARSER_H
#define ZYPP_PARSER_INIPARSER_H

#include <iosfwd>
#include <string>
#include <list>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/InputStream.h"
#include "zypp/ProgressData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace parser
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : IniParser
//
/** Basic SUSEtags parser.
 * Will replace parser/tagfile/ and  parser/taggedfile/ stuff.
*/
class IniParser : private base::NonCopyable
{
public:
  /** Default ctor */
  IniParser();
  /** Dtor */
  virtual ~IniParser();
  /** Parse the stream.
   * \throw ParseException on errors. Invoke \ref consume
   * for each tag. \ref consume might throw other exceptions
   * as well.
  */
  void parse( const InputStream & imput_r, const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() );

public:
  /** Called when start parsing. */
  virtual void beginParse();
  /** Called when a section is found. */
  virtual void consume( const std::string &section );
  /** Called when a key value is found. */
  virtual void consume( const std::string &section, const std::string &key, const std::string &value );
  /** Called when the parse is done. */
  virtual void endParse();
public:
  /** Name of the current InputStream. */
  const std::string & inputname() const
  {
    return _inputname;
  }

private:
  std::string _inputname;
  std::string _current_section;
  int _line_nr;
  //ProgressData _ticks;
};

/////////////////////////////////////////////////////////////////
} // namespace parser
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_INIPARSER_H
