/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/IniDict.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"
#include <map>
#include <string>
#include "zypp/parser/IniDict.h"

using namespace std;
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : IniDict
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : IniDict::IniDict
    //	METHOD TYPE : Ctor
    //
    IniDict::IniDict( const InputStream &is )
    {
      parse(is);
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : IniDict::~IniDict
    //	METHOD TYPE : Dtor
    //
    IniDict::~IniDict()
    {}

    void IniDict::consume( const std::string &section )
    {
      // do nothing for now.
    }

    void IniDict::consume( const std::string &section, const std::string &key, const std::string &value )
    {
      _dict[section][key] = value;
    }


    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const IniDict & obj )
    {
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
