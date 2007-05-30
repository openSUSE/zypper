/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/ParseException.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/parser/ParseException.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
 
    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseException::ParseException
    //	METHOD TYPE : Ctor
    //
    ParseException::ParseException()
    : Exception( "Parse exception" )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseException::ParseException
    //	METHOD TYPE : Ctor
    //
    ParseException::ParseException( const std::string & msg_r )
    : Exception( msg_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseException::~ParseException
    //	METHOD TYPE : Dtor
    //
    ParseException::~ParseException() throw()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseException::dumpOn
    //	METHOD TYPE : std::ostream &
    //
    std::ostream & ParseException::dumpOn( std::ostream & str ) const
    {
      return Exception::dumpOn( str );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
