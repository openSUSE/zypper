/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Edition.cc
 *
*/
extern "C"
{
#include <satsolver/evr.h>
}
#include "zypp/base/String.h"

#include "zypp/Edition.h"
#include "zypp/sat/Pool.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    inline std::string makeevrstr( const std::string & version_r,
                                   const std::string & release_r,
                                   Edition::epoch_t epoch_r )
    {
      std::string ret( version_r );
      if ( ! release_r.empty() )
      {
        ret += "-";
        ret += release_r;
      }
      return ( epoch_r ? str::numstring( epoch_r ) + ":" + ret
                       : ret );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  const Edition Edition::noedition;

  ///////////////////////////////////////////////////////////////////

  Edition::Edition( const std::string & version_r,
                    const std::string & release_r,
                    epoch_t epoch_r )
  : _str( makeevrstr( version_r, release_r, epoch_r ) )
  {}

  Edition::Edition( const std::string & version_r,
                    const std::string & release_r,
                    const std::string & epoch_r )
  : _str( makeevrstr( version_r, release_r, str::strtonum<epoch_t>( epoch_r ) ) )
  {}

  Edition::epoch_t Edition::epoch() const
  {
    const char * str( c_str() );
    const char * sep = str;
    // skip edition
    for ( ; *sep >= '0' && *sep <= '9'; ++sep )
      ; // NOOP
    if ( *sep == ':' )
      return str::strtonum<epoch_t>( std::string( str, sep-str ) );
    return 0;
  }

  const std::string & Edition::version() const
  {
    const char * str( c_str() );
    const char * sep = str;
    // skip edition
    for ( ; *sep >= '0' && *sep <= '9'; ++sep )
      ; // NOOP
    if ( *sep == ':' )
      str = sep+1;
    // strip release
    sep = ::strrchr( str, '-' );
    if ( sep )
      return std::string( str, sep-str );
    return str;
  }

  const std::string & Edition::release() const
  {
    const char * str( c_str() );
    const char * sep = ::strrchr( str, '-' );
    // get release
    if ( sep )
      return sep+1;
    return std::string();
  }

  int Edition::_doCompareC( const char * rhs ) const
  {
    return ::evrcmp_str( sat::Pool::instance().get(),
                         c_str(), rhs, EVRCMP_COMPARE );
  }

  int Edition::_doMatchC( const char * rhs )  const
  {
    return ::evrcmp_str( sat::Pool::instance().get(),
                         c_str(), rhs, EVRCMP_MATCH );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////


