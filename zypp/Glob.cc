/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Glob.cc
 *
*/
#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/Glob.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace filesystem
  { /////////////////////////////////////////////////////////////////

    int Glob::add( const std::string & pattern_r, Flags flags_r )
    {
      static Flags _APPEND( GLOB_APPEND ); // not published
      if ( ! flags_r )
        flags_r = _defaultFlags;
      if ( _result )
        flags_r |= _APPEND;
      else
        _result.reset( new ::glob_t );
      return( _lastGlobReturn = ::glob( pattern_r.c_str(), flags_r, NULL, &(*_result) ) );
    }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Glob & obj )
    {
      return dumpRange( str << "(" << obj.size() << ")", obj.begin(), obj.end() );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace filesystem
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
