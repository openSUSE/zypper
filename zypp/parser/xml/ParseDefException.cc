/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/xml/ParseDefException.cc
 *
*/
#include "zypp/parser/xml/ParseDefException.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseDefException::ParseDefException
    //	METHOD TYPE : Constructor
    //
    ParseDefException::ParseDefException( const std::string & what_r )
    : Exception( what_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseDefBuildException::ParseDefBuildException
    //	METHOD TYPE : Constructor
    //
    ParseDefBuildException::ParseDefBuildException( const std::string & what_r )
    : ParseDefException( what_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseDefValidateException::ParseDefValidateException
    //	METHOD TYPE : Constructor
    //
    ParseDefValidateException::ParseDefValidateException( const std::string & what_r )
    : ParseDefException( what_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseDefDataException::ParseDefDataException
    //	METHOD TYPE : Constructor
    //
    ParseDefDataException::ParseDefDataException( const std::string & what_r )
    : ParseDefException( what_r )
    {}

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
