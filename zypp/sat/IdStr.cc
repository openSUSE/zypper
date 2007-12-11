/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/IdStr.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/IdStr.h"
#include "zypp/sat/Pool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    const IdStr IdStr::Null( STRID_NULL );
    const IdStr IdStr::Empty( STRID_EMPTY );

    /////////////////////////////////////////////////////////////////

    IdStr::IdStr( const char * str_r )
      : _id( ::str2id( myPool().getPool(), str_r, /*create*/true ) )
    {}

    IdStr::IdStr( const std::string & str_r )
      : _id( ::str2id( myPool().getPool(), str_r.c_str(), /*create*/true ) )
    {}

    unsigned IdStr::size() const
    { return ::strlen( c_str() ); }

    const char * IdStr::c_str() const
    { return ::id2str( myPool().getPool(), _id ); }

    std::string IdStr::string() const
    { return ::id2str( myPool().getPool(), _id ); }

    int IdStr::compare( const IdStr & rhs ) const
    {
      if ( _id == rhs._id )
        return 0;
      // Explicitly handle IdStr::Null because
      // it's string representation is "<NULL>"
      // and not something less than "".
      if ( ! _id )
        return -1;
      if ( ! rhs._id )
        return 1;
      return ::strcmp( c_str(), rhs.c_str() );
    }

    int IdStr::compare( const char * rhs ) const
    {
      // Explicitly handle IdStr::Null == (const char *)0
      if ( ! _id )
        return rhs ? -1 : 0;
      if ( ! rhs )
        return _id ? 1 : 0;
      return ::strcmp( c_str(), rhs );
    }

   /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const IdStr & obj )
    {
      return str << obj.c_str();
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
