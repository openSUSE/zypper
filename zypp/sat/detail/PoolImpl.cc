/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/detail/PoolImpl.cc
 *
*/
extern "C"
{
#include <satsolver/solvable.h>
#include <satsolver/repo.h>
#include <satsolver/pool.h>
#include <satsolver/sat_debug.h>
}

#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

#include "zypp/sat/detail/PoolImpl.h"

using std::endl;

// ///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////

      void logSat( char *logString )
      {
        _MIL("satsolver") << logString;
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PoolMember::myPool
      //	METHOD TYPE : PoolImpl
      //
      PoolImpl & PoolMember::myPool()
      {
        static PoolImpl _global;
        return _global;
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PoolImpl::PoolImpl
      //	METHOD TYPE : Ctor
      //
      PoolImpl::PoolImpl()
      : _pool( ::pool_create() )
      {
        if ( ! _pool )
        {
          ZYPP_THROW( Exception( _("Can not create sat-pool.") ) );
        }
        // initialialize logging
        bool verbose = ( getenv("ZYPP_FULLLOG") || getenv("ZYPP_LIBSAT_FULLLOG") );
        ::sat_set_debug( verbose ? DEBUG_5 : ERROR, 1 ); // logging linenumer, function,....
        ::sat_set_debugCallback( logSat );
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PoolImpl::~PoolImpl
      //	METHOD TYPE : Dtor
      //
      PoolImpl::~PoolImpl()
      {
        ::pool_free( _pool );
      }

      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
