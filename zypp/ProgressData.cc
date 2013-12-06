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
    Date now = Date::now();

    // compute value and check whether to report it
    if ( hasRange() )
    {
      value_type newVal = _d->_val * 100 / ( _d->_max - _d->_min );

      if ( newVal - _d->_last_val > 10
	   || now - _d->_last_send > 1
	   || ( _d->_last_val == 0 && newVal > 0 )
	   || ( newVal == 100 && _d->_last_val != 100 )
	   || ( newVal != 100 && _d->_last_val == 100 )
	   || _d->_state != RUN /*INIT||END*/ )
      {
	_d->_last_val  = newVal;
	_d->_last_send = now;
      }
      else
	return true;	// skip report, continue per default
    }
    else
    {
      if ( now - _d->_last_send > 1 || _d->_state != RUN /*INIT||END*/ )
      {
	_d->_last_val  = _d->_val;
	_d->_last_send = now;
      }
      else
	return true;	// skip report, continue per default
    }

    // now send report
    if ( _d->_state == INIT )
    {
      _d->_state = RUN;
    }
    // XXX << str::form( "{#%u|%s}(%lld%s)", numericId(), name().c_str(), _d->_last_val, ( hasRange() ? "%" : "!" ) ) << endl;

    if ( _d->_receiver )
    {
      if ( ! _d->_receiver( *this ) )
      {
	if ( _d->_state != END )
	{
	  WAR << "User request to ABORT pending action. "
	  << str::form( "{#%u|%s}(%lld%s)", numericId(), name().c_str(),
			_d->_last_val, ( hasRange() ? "%" : "!" ) ) << endl;
	}
	return false;	// aborted by user
      }
    }
    else if ( _d->_state == END )
    {
      DBG << str::form( "{#%u|%s}END", numericId(), name().c_str() ) << endl;
    }

    return true;	// continue per default
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
      return str << str::form( "{%u|%s}[%lld,%lld](%lld)%lld%%)",
			       obj.numericId(), obj.name().c_str(),
			       obj.min(), obj.max(), obj.val(), obj.reportValue() );
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

  CombinedProgressData::CombinedProgressData( ProgressData &pd,
                                              ProgressData::value_type weight )
    : _weight(weight),
      _last_value(0),
      _pd(pd)
  {

  }

  bool CombinedProgressData::operator()( const ProgressData &progress )
  {
    if ( progress.reportAlive() || ( _weight == 0 ) )
      return _pd.tick();

    // factor [0,1] of increase in subtask ( ie: before 0,2 now 0.5 )
    float increment = ((float)(progress.val() - _last_value))/(progress.max() - progress.min());
    // how much the subtask affects the parent task ie: 0,1
    float parent_factor = (float)(_weight)/(_pd.max() - _pd.min());
    // real increment of the parent task
    float real_increment = parent_factor*increment;
    _last_value = progress.val();
    return _pd.incr( (int)( (_pd.max()-_pd.min()) * real_increment) );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
