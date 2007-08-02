/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/SerialNumber.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/base/SerialNumber.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SerialNumber
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SerialNumber::SerialNumber
  //	METHOD TYPE : Ctor
  //
  SerialNumber::SerialNumber( bool dirty_r )
    : _dirty( dirty_r )
    , _serial( 0 )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SerialNumber::~SerialNumber
  //	METHOD TYPE : Dtor
  //
  SerialNumber::~SerialNumber()
  {}

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const SerialNumber & obj )
  {
    return str << "SERIAL" << (obj._dirty?"*":"(") << obj._serial << (obj._dirty?"*":")");
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SerialNumberWatcher
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SerialNumberWatcher::SerialNumberWatcher
  //	METHOD TYPE : Ctor
  //
  SerialNumberWatcher::SerialNumberWatcher( unsigned serial_r )
  : _serial( serial_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SerialNumberWatcher::SerialNumberWatcher
  //	METHOD TYPE : Ctor
  //
  SerialNumberWatcher::SerialNumberWatcher( const SerialNumber & serial_r )
  : _serial( serial_r.serial() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SerialNumber::~SerialNumber
  //	METHOD TYPE : Dtor
  //
  SerialNumberWatcher::~SerialNumberWatcher()
  {}

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const SerialNumberWatcher & obj )
  {
    return str << "LAST_SERIAL(" << obj._serial << ")";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
