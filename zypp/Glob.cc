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

    int Glob::add( const char * pattern_r, Flags flags_r )
    {
      static Flags kAppend( GLOB_APPEND ); // not published
      if ( ! flags_r )
        flags_r = _defaultFlags;
      if ( _result )
        flags_r |= kAppend;
      else
        _result.reset( new ::glob_t );
      return( _lastGlobReturn = ::glob( pattern_r, flags_r, NULL, &(*_result) ) );
    }

    void Glob::clear()
    {
      if ( _result )
      {
        ::globfree( &(*_result) );
        _result.reset();
        _lastGlobReturn = 0;
      }
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
