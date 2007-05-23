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

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define  ZYPP_BASE_LOGGER_LOGGROUP "Progress"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ProgressData::report
  //	METHOD TYPE : void
  //
  bool ProgressData::report()
  {
    // DISABLED to get DBG output from 'if ( doReport )'
    //if ( ! _d->_receiver )
    //  return true;

    bool goOn     = true;  // continue per default
    bool doReport = false;

    // compute value and check whether to report it
    if ( hasRange() )
    {
      value_type newVal = _d->_val * 100;
      newVal /= ( _d->_max - _d->_min );

      if ( newVal - _d->_last_val > 20
	   || Date::now() - _d->_last_send > 1
	   || ( newVal == 100 && _d->_last_send != 100 )
	   || _d->_state == END )
      {
	_d->_last_val  = newVal;
	_d->_last_send = Date::now();
	doReport = true;
      }
    }
    else
    {
      if ( Date::now() - _d->_last_send > 1 || _d->_state == END )
      {
	_d->_last_val  = _d->_val;
	_d->_last_send = Date::now();
	doReport = true;
      }
    }

    // report if necessary
    if ( doReport )
    {
      if ( _d->_state == INIT )
      {
	_d->_state = RUN;
      }

      if ( _d->_receiver )
      {
	goOn = _d->_receiver( _d->_last_val );
      }
      else
      {
	DBG << str::form( "{#%u|%s}(%lld%s)",
			numericId(), name().c_str(),
			_d->_last_val, ( hasRange() ? "%" : "!" ) ) << endl;
      }
    }

    // log abort request and return
    if ( ! goOn && _d->_state != END )
    {
      WAR << "User request to ABORT pending action. "
	  << str::form( "{#%u|%s}(%lld%s)",
			numericId(), name().c_str(),
			_d->_last_val, ( hasRange() ? "%" : "!" ) ) << endl;
    }
    return goOn;
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
