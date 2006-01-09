/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CapMatch.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/CapMatch.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  const CapMatch CapMatch::yes( true );
  const CapMatch CapMatch::no( false );
  const CapMatch CapMatch::irrelevant;

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CapMatch & obj )
  {
    if ( obj._result == CapMatch::IRRELEVANT )
      return str << "IRRELEVANT";
    return str << ( obj._result == CapMatch::MATCH ? "MATCH" : "NOMATCH" );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
