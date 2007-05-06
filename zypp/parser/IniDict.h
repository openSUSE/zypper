/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/IniDict.h
 *
*/
#ifndef ZYPP_PARSER_INIDICT_H
#define ZYPP_PARSER_INIDICT_H

#include <iosfwd>
#include <map>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/InputStream.h"
#include "zypp/parser/IniParser.h"

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
    /**
     * 
     */
    class IniDict : public IniParser
    {
      friend std::ostream & operator<<( std::ostream & str, const IniDict & obj );
    public:
      /** Default ctor */
      IniDict( const InputStream &is );
      /** Dtor */
      ~IniDict();

    public:

      /** Called when a section is found. */
      virtual void consume( const std::string &section );
      /** Called when a key value is found. */
      virtual void consume( const std::string &section, const std::string &key, const std::string &value );

    private:
      std::map<std::string, std::map<std::string, std::string> > _dict;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates IniDict Stream output */
    std::ostream & operator<<( std::ostream & str, const IniDict & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_INIDICT_H
