/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/PathInfo.cc
 *
*/

#include <zypp/PathInfo.h>
#include <zypp/base/StrMatcher.h>

using std::endl;
using std::string;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace filesystem
  {

    const StrMatcher & matchNoDots()
    {
      static StrMatcher noDots( "[^.]*", Match::GLOB );
      return noDots;
    }

    int dirForEach( const Pathname & dir_r, const StrMatcher & matcher_r, function<bool( const Pathname &, const char *const)> fnc_r )
    {
      if ( ! fnc_r )
	return 0;

      bool nodots = ( &matcher_r == &matchNoDots() );
      return dirForEach( dir_r,
			 [&]( const Pathname & dir_r, const char *const name_r )->bool
			 {
			   if ( ( nodots && name_r[0] == '.' ) || ! matcher_r( name_r ) )
			     return true;
			   return fnc_r( dir_r, name_r );
			 } );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace filesystem
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
