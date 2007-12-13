/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Capabilities.cc
 *
*/
#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/sat/Capabilities.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////
  
    Capabilities:: Capabilities( const detail::IdType * base_r, detail::IdType skip_r )
    : _begin( base_r )
    {
      if ( ! _begin )
        return;

      if ( skip_r )
      {
        for ( const detail::IdType * end = _begin; *end; ++end )
	{
          if ( *end == skip_r )
          {
	    _begin = end+1;
	    return;
	  }
        }
      }
      // skipp all ==> empty
      _begin = 0;
    }


    Capabilities::size_type Capabilities::size() const
    {
      if ( ! _begin )
        return 0;

      // jump over satsolvers internal ids.
      Capabilities::size_type ret = 0;
      for ( const detail::IdType * end = _begin; *end; ++end )
      {
        if ( ! detail::isDepMarkerId( *end ) )
          ++ret;
      }
      return ret;
    }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Capabilities & obj )
    {
      return dumpRange( str << "(" << obj.size() << ")", obj.begin(), obj.end() );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
