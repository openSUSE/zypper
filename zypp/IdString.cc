/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/IdString.cc
 *
*/
#include <iostream>
#include <boost/mpl/int.hpp>

#include "zypp/IdString.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/Pool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  const IdString IdString::Null ( sat::detail::noId );
  const IdString IdString::Empty( sat::detail::emptyId );

  /////////////////////////////////////////////////////////////////

  IdString::IdString( const char * str_r )
  : _id( ::pool_str2id( myPool().getPool(), str_r, /*create*/true ) )
  {}

  IdString::IdString( const char * str_r, unsigned len_r )
  : _id( ::pool_strn2id( myPool().getPool(), str_r, len_r, /*create*/true ) )
  {}

  IdString::IdString( const std::string & str_r )
  : IdString( str_r.c_str(), str_r.length() )
  {}

  IdString::IdString( boost::string_ref str_r )
  : IdString( str_r.data(), str_r.length() )
  {}

  unsigned IdString::size() const
  { return ::strlen( c_str() ); }

  const char * IdString::c_str() const
  { return _id ? ::pool_id2str( myPool().getPool(), _id ) : ""; }

  int IdString::compare( const IdString & rhs ) const
  {
    if ( _id == rhs._id )
      return 0;
    // Explicitly handle IdString::Null < ""
    if ( ! _id )
      return -1;
    if ( ! rhs._id )
      return 1;
    return ::strcmp( c_str(), rhs.c_str() );
  }

  int IdString::compare( const char * rhs ) const
  {
    // Explicitly handle IdString::Null == (const char *)0
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
  std::ostream & operator<<( std::ostream & str, const IdString & obj )
  {
    return str << obj.c_str();
  }

  std::ostream & dumpOn( std::ostream & str, const IdString & obj )
  {
    return str << '(' << obj.id() << ')' << obj.c_str();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
