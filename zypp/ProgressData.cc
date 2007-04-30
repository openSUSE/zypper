/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ProgressData.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/String.h"

#include "zypp/ProgressData.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ProgressData::report
  //	METHOD TYPE : void
  //
  void ProgressData::report()
  {
    if ( hasRange() )
    {
      value_type newVal = _d->_val * 100;
      newVal /= ( _d->_max - _d->_min );

      if ( newVal - _d->_last_val > 5 || Date::now() - _d->_last_send > 1 || _d->_state == END )
      {
	DBG << str::form( "{%u|%s}(%d%%)",
			  numericId(), name().c_str(), newVal ) << endl;
	_d->_last_val  = newVal;
	_d->_last_send = Date::now();
	if ( _d->_state == INIT )
	{
	  _d->_state = RUN;
	}
      }
    }
    else
    {
      if ( Date::now() - _d->_last_send > 1 || _d->_state == END )
      {
	DBG << str::form( "{%u|%s}(%d!)",
			numericId(), name().c_str(), _d->_val ) << endl;
	_d->_last_val  = _d->_val;
	_d->_last_send = Date::now();
	if ( _d->_state == INIT )
	{
	  _d->_state = RUN;
	}
      }
    }
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const ProgressData & obj )
  {
    if ( obj.hasRange() )
    {
      return str << str::form( "{%u|%s}[%lld,%lld](%lld)",
			       obj.numericId(), obj.name().c_str(),
			       obj.min(), obj.max(), obj.val() );
    }
    return str << str::form( "{%u|%s}[-,-](%lld)",
			     obj.numericId(), obj.name().c_str(),
			     obj.val() );
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  ProgressData makeProgressData( const InputStream & input_r )
  {
    ProgressData ret;
    ret.name( input_r.name() );
    if ( input_r.size() > 0 )
      ret.range( input_r.size() );
    return ret;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
