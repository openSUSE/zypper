/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/UserRequestException.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/base/UserRequestException.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : UserRequestException::UserRequestException
  //	METHOD TYPE : Ctor
  //
  UserRequestException::UserRequestException( const std::string & msg_r )
  : Exception( msg_r ), _kind( UNSPECIFIED )
  {}

  UserRequestException::UserRequestException( const std::string & msg_r, const Exception & history_r )
  : Exception( msg_r, history_r ), _kind( UNSPECIFIED )
  {}

  UserRequestException::UserRequestException( Kind kind_r, const std::string & msg_r )
  : Exception( msg_r ), _kind( kind_r )
  {}

  UserRequestException::UserRequestException( Kind kind_r, const std::string & msg_r, const Exception & history_r )
  : Exception( msg_r, history_r ), _kind( kind_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : UserRequestException::dumpOn
  //	METHOD TYPE : std::ostream &
  //
  std::ostream & UserRequestException::dumpOn( std::ostream & str ) const
  {
    switch ( _kind )
    {
      case UNSPECIFIED: str << "UNSPECIFIED"; break;
      case IGNORE:      str << "IGNORE";      break;
      case SKIP:        str << "SKIP";        break;
      case RETRY:       str << "RETRY";       break;
      case ABORT:       str << "ABORT";       break;
	// no default !
    }
    return str << " request: " << msg();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
