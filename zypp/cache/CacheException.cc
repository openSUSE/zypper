/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/CacheException.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"
#include "zypp/cache/CacheException.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////
 
    ///////////////////////////////////////////////////////////////////
    //
    //  CLASS NAME : CacheException
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CacheException::CacheException
    //	METHOD TYPE : Ctor
    //
    CacheException::CacheException()
    : Exception( "Cache exception" )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CacheException::CacheException
    //	METHOD TYPE : Ctor
    //
    CacheException::CacheException( const std::string & msg_r )
    : Exception( msg_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CacheException::~CacheException
    //	METHOD TYPE : Dtor
    //
    CacheException::~CacheException() throw()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CacheException::dumpOn
    //	METHOD TYPE : std::ostream &
    //
    std::ostream & CacheException::dumpOn( std::ostream & str ) const
    {
      return Exception::dumpOn( str );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //  CLASS NAME : CacheRecordNotFoundException
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //  METHOD NAME : CacheRecordNotFoundException::CacheRecordNotFoundException
    //  METHOD TYPE : Ctor
    //
    CacheRecordNotFoundException::CacheRecordNotFoundException()
      : CacheException("Record not found in the cache")
    {}

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
