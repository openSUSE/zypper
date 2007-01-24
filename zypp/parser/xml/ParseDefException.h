/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/xml/ParseDefException.h
 *
*/
#ifndef ZYPP_PARSER_XML_PARSEDEFEXCEPTION_H
#define ZYPP_PARSER_XML_PARSEDEFEXCEPTION_H

#include <string>

#include "zypp/base/Exception.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefException
    //
    /** Common base class for \ref ParseDef exceptions. */
    struct ParseDefException : public Exception
    {
      ParseDefException( const std::string & what_r );
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefBuildException
    //
    /** Exceptions when building a ParseDef tree. */
    struct ParseDefBuildException : public ParseDefException
    {
      ParseDefBuildException( const std::string & what_r );
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefValidateException
    //
    /** Parse exceptions related to the documents node structure. */
    struct ParseDefValidateException : public ParseDefException
    {
      ParseDefValidateException( const std::string & what_r );
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefDataException
    //
    /** Parse exceptions related to the nodes content. */
    struct ParseDefDataException : public ParseDefException
    {
      ParseDefDataException( const std::string & what_r );
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace xml
    ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_XML_PARSEDEFEXCEPTION_H
