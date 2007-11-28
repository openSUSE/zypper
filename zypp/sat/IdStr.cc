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

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    const IdStr IdStr::Null( unsigned(STRID_NULL) );
    const IdStr IdStr::Empty( unsigned(STRID_EMPTY) );

    /////////////////////////////////////////////////////////////////

    IdStr::IdStr( const char * str_r )
      : _id( ::str2id( myPool().getPool(), str_r, true ) )
    {}

    IdStr::IdStr( const std::string & str_r )
      : _id( ::str2id( myPool().getPool(), str_r.c_str(), true ) )
    {}

    const char * IdStr::c_str() const
    {
      return ::id2str( myPool().getPool(), _id );
    }

    std::string IdStr::string() const
    {
      return ::id2str( myPool().getPool(), _id );
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
