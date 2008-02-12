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
#include <iostream>
#include <boost/mpl/int.hpp>

#include "zypp/base/Easy.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

#include "zypp/ZConfig.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/Repo.h"
#include "zypp/Capability.h"

using std::endl;

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::satpool"

// ///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////

      // MPL checks for satlib constants we redefine to avoid
      // includes and defines.
      BOOST_MPL_ASSERT_RELATION( noId,                 ==, STRID_NULL );
      BOOST_MPL_ASSERT_RELATION( emptyId,              ==, STRID_EMPTY );

      BOOST_MPL_ASSERT_RELATION( solvablePrereqMarker, ==, SOLVABLE_PREREQMARKER );
      BOOST_MPL_ASSERT_RELATION( solvableFileMarker,   ==, SOLVABLE_FILEMARKER );

      BOOST_MPL_ASSERT_RELATION( CapDetail::CAP_AND,       ==, REL_AND );
      BOOST_MPL_ASSERT_RELATION( CapDetail::CAP_OR,        ==, REL_OR );
      BOOST_MPL_ASSERT_RELATION( CapDetail::CAP_WITH,      ==, REL_WITH );
      BOOST_MPL_ASSERT_RELATION( CapDetail::CAP_NAMESPACE, ==, REL_NAMESPACE );

     /////////////////////////////////////////////////////////////////

      static void logSat( struct _Pool *, void *data, int type, const char *logString )
      {
	  if ((type & (SAT_FATAL|SAT_ERROR))) {
	      _ERR("satsolver") << logString;
	  } else {
	      _MIL("satsolver") << logString;
	  }
      }

      static detail::IdType nsCallback( struct _Pool *, void *data, detail::IdType lhs, detail::IdType rhs )
      {
        //T << Cability( lhs ) << (const char *)data << Capability( rhs ) << endl;
        return 0;
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
        ::pool_setdebuglevel( _pool, verbose ? 5 : 1 );
        ::pool_setdebugcallback( _pool, logSat, NULL );

        // set pool architecture
        ::pool_setarch( _pool,  ZConfig::instance().systemArchitecture().asString().c_str() );

        // set namespace callback
        _pool->nscallback = &nsCallback;
        _pool->nscallbackdata = (void*)" NAMESPACE ";
        SEC << _pool->nscallback << endl;
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

     ///////////////////////////////////////////////////////////////////

      void PoolImpl::setDirty( const char * a1, const char * a2, const char * a3 )
      {
        if ( a1 )
        {
          if      ( a3 ) DBG << a1 << " " << a2 << " " << a3 << endl;
          else if ( a2 ) DBG << a1 << " " << a2 << endl;
          else           DBG << a1 << endl;
        }
        _serial.setDirty();
        ::pool_freewhatprovides( _pool );
      }

      void PoolImpl::prepare()
      {
        if ( _watcher.remember( _serial ) )
        {
           // sat solver claims to handle this on it's own:
           ::pool_createwhatprovides( _pool );
        }
      }

      ///////////////////////////////////////////////////////////////////

      int PoolImpl::_addSolv( ::_Repo * repo_r, FILE * file_r )
      {
        setDirty(__FUNCTION__, repo_r->name );
        int ret = ::repo_add_solv( repo_r , file_r  );

        if ( ret == 0 )
        {
          // Filter out unwanted archs
          std::set<detail::IdType> sysids;
          {
            Arch::CompatSet sysarchs( Arch::compatSet( ZConfig::instance().systemArchitecture() ) );
            for_( it, sysarchs.begin(), sysarchs.end() )
              sysids.insert( it->idStr().id() );
            // unfortunately satsolver treats src/nosrc as architecture:
            sysids.insert( ARCH_SRC );
            sysids.insert( ARCH_NOSRC );
          }

          detail::IdType blockBegin = 0;
          unsigned       blockSize  = 0;
          for ( detail::IdType i = repo_r->start; i < repo_r->end; ++i )
          {
            ::_Solvable * s( _pool->solvables + i );
            if ( s->repo == repo_r && sysids.find( s->arch ) == sysids.end() )
            {
              // Remember an unwanted arch entry:
              if ( ! blockBegin )
                blockBegin = i;
              ++blockSize;
            }
            else if ( blockSize )
            {
              // Free remembered entries
              ::repo_free_solvable_block( repo_r, blockBegin, blockSize, /*reuseids*/false );
              blockBegin = blockSize = 0;
            }
          }
          if ( blockSize )
          {
            // Free remembered entries
            ::repo_free_solvable_block( repo_r, blockBegin, blockSize, /*reuseids*/false );
            blockBegin = blockSize = 0;
          }
        }

        return ret;
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
