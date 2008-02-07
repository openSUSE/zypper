/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/detail/PoolImpl.h
 *
*/
#ifndef ZYPP_SAT_DETAIL_POOLIMPL_H
#define ZYPP_SAT_DETAIL_POOLIMPL_H
extern "C"
{
#include <satsolver/pool.h>
#include <satsolver/repo.h>
#include <satsolver/solvable.h>
#include <satsolver/poolarch.h>
#include <satsolver/repo_solv.h>
}
#include <iosfwd>

#include "zypp/base/NonCopyable.h"
#include "zypp/base/SerialNumber.h"
#include "zypp/sat/detail/PoolMember.h"
#include "zypp/RepoInfo.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PoolImpl
      //
      /** */
      class PoolImpl : private base::NonCopyable
      {
        public:
          /** Default ctor */
          PoolImpl();

          /** Dtor */
          ~PoolImpl();

          /** Pointer style access forwarded to sat-pool. */
           ::_Pool * operator->()
           { return _pool; }

        public:
          /** Serial number changing whenever the content changes. */
          const SerialNumber & serial() const
          { return _serial; }


          /** Update housekeeping data (e.g. whatprovides).
           * \todo actually requires a watcher.
          */
          void prepare();

        public:
          /** Invalidate housekeeping data (e.g. whatprovides).
          */
          void setDirty( const char * a1 = 0, const char * a2 = 0, const char * a3 = 0 );

        public:
          /** \name Actions invalidating housekeeping data.
           *
           * All methods expect valid arguments being passed.
           */
          //@{
          /** Creating a new repo named \a name_r. */
          RepoIdType _createRepo( const std::string & name_r )
          {
            setDirty(__FUNCTION__, name_r.c_str() );
            return ::repo_create( _pool, name_r.c_str() );
          }

          /** Creating a new repo named \a name_r. */
          void _deleteRepo( ::_Repo * repo_r )
          {
            setDirty(__FUNCTION__, repo_r->name );
            ::repo_free( repo_r, /*reuseids*/false );
            eraseRepoInfo( repo_r );
          }

          /** Adding solv file to a repo. */
          int _addSolv( ::_Repo * repo_r, FILE * file_r )
          {
            setDirty(__FUNCTION__, repo_r->name );
            return ::repo_add_solv( repo_r , file_r  );
          }
          /** Adding Solvables to a repo. */
          detail::SolvableIdType _addSolvables( ::_Repo * repo_r, unsigned count_r )
          {
            setDirty(__FUNCTION__, repo_r->name );
            return ::repo_add_solvable_block( repo_r, count_r );
          }
          //@}

        public:
          /** a \c valid \ref Solvable has a non NULL repo pointer. */
          bool validSolvable( const ::_Solvable & slv_r ) const
          { return slv_r.repo; }
          /** \overload Check also for id_r being in range of _pool->solvables. */
          bool validSolvable( SolvableIdType id_r ) const
          { return id_r < unsigned(_pool->nsolvables) && validSolvable( _pool->solvables[id_r] ); }
          /** \overload Check also for slv_r being in range of _pool->solvables. */
          bool validSolvable( const ::_Solvable * slv_r ) const
          { return _pool->solvables <= slv_r && slv_r <= _pool->solvables+_pool->nsolvables && validSolvable( *slv_r ); }

        public:
          ::_Pool * getPool() const
          { return _pool; }

          /** \todo a quick check whether the repo was meanwhile deleted. */
          ::_Repo * getRepo( RepoIdType id_r ) const
          { return id_r; }

          /** Return pointer to the sat-solvable or NULL if it is not valid.
           * \see \ref validSolvable.
           */
          ::_Solvable * getSolvable( SolvableIdType id_r ) const
          {
            if ( validSolvable( id_r ) )
              return &_pool->solvables[id_r];
            return 0;
          }

        public:
          /** Get id of the first valid \ref Solvable.
           * This is the next valid after the system solvable.
          */
          SolvableIdType getFirstId()  const
          { return getNextId( 1 ); }

          /** Get id of the next valid \ref Solvable.
           * This goes round robbin. At the end it returns \ref noSolvableId.
           * Passing \ref noSolvableId it returns the 1st valid  \ref Solvable.
           * \see \ref validSolvable.
           */
          SolvableIdType getNextId( SolvableIdType id_r ) const
          {
            for( ++id_r; id_r < unsigned(_pool->nsolvables); ++id_r )
            {
              if ( validSolvable( _pool->solvables[id_r] ) )
                return id_r;
            }
            return noSolvableId;
          }

        public:
          /** */
          const RepoInfo & repoInfo( RepoIdType id_r )
          { return _repoinfos[id_r]; }
          /** */
          void setRepoInfo( RepoIdType id_r, const RepoInfo & info_r )
          { _repoinfos[id_r] = info_r; }
          /** */
          void eraseRepoInfo( RepoIdType id_r )
          { _repoinfos.erase( id_r ); }

        private:
          /** sat-pool. */
          ::_Pool * _pool;
          /** Serial number. */
          SerialNumber _serial;
          /** Watch serial number. */
          SerialNumberWatcher _watcher;
          /** Additional \ref RepoInfo. */
          std::map<RepoIdType,RepoInfo> _repoinfos;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#define POOL_SETDIRTY
#endif // ZYPP_SAT_DETAIL_POOLIMPL_H
