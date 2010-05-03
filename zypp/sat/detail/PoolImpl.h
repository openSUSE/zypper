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

#include "zypp/base/Tr1hash.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/SerialNumber.h"
#include "zypp/sat/detail/PoolMember.h"
#include "zypp/RepoInfo.h"
#include "zypp/Locale.h"
#include "zypp/Capability.h"
#include "zypp/IdString.h"

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
          void prepare() const;

        private:
          /** Invalidate housekeeping data (e.g. whatprovides) if the
           *  pools content changed.
           */
          void setDirty( const char * a1 = 0, const char * a2 = 0, const char * a3 = 0 );

          /** Invalidate housekeeping data (e.g. whatprovides) if dependencies changed.
           */
          void depSetDirty( const char * a1 = 0, const char * a2 = 0, const char * a3 = 0 );

          /** Callback to resolve namespace dependencies (language, modalias, filesystem, etc.). */
          static detail::IdType nsCallback( ::_Pool *, void * data, detail::IdType lhs, detail::IdType rhs );

        public:
          /** Reserved system repository alias \c @System. */
          static const std::string & systemRepoAlias();

          bool isSystemRepo( ::_Repo * repo_r ) const
          { return repo_r && _pool->installed == repo_r; }

          ::_Repo * systemRepo() const
          { return _pool->installed; }

        public:
          /** \name Actions invalidating housekeeping data.
           *
           * All methods expect valid arguments being passed.
           */
          //@{
          /** Creating a new repo named \a name_r. */
          ::_Repo * _createRepo( const std::string & name_r );

          /** Creating a new repo named \a name_r. */
          void _deleteRepo( ::_Repo * repo_r );

          /** Adding solv file to a repo.
           * Except for \c isSystemRepo_r, solvables of incompatible architecture
           * are filtered out.
          */
          int _addSolv( ::_Repo * repo_r, FILE * file_r );

          /** Adding helix file to a repo.
           * Except for \c isSystemRepo_r, solvables of incompatible architecture
           * are filtered out.
          */
          int _addHelix( ::_Repo * repo_r, FILE * file_r );

          /** Adding Solvables to a repo. */
          detail::SolvableIdType _addSolvables( ::_Repo * repo_r, unsigned count_r );
          //@}

          /** Helper postprocessing the repo after adding solv or helix files. */
          void _postRepoAdd( ::_Repo * repo_r );

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
          /** Also adjust repo priority and subpriority accordingly. */
          void setRepoInfo( RepoIdType id_r, const RepoInfo & info_r );
          /** */
          void eraseRepoInfo( RepoIdType id_r )
          { _repoinfos.erase( id_r ); }

        public:
          /** Returns the id stored at \c offset_r in the internal
           * whatprovidesdata array.
          */
          const sat::detail::IdType whatProvidesData( unsigned offset_r )
          { return _pool->whatprovidesdata[offset_r]; }

          /** Returns offset into the internal whatprovidesdata array.
           * Use \ref whatProvidesData to get the stored Id.
          */
          unsigned whatProvides( Capability cap_r )
          { prepare(); return ::pool_whatprovides( _pool, cap_r.id() ); }

        public:
          /** \name Requested locales. */
          //@{
	  void setTextLocale( const Locale & locale_r );
          void setRequestedLocales( const LocaleSet & locales_r );
          bool addRequestedLocale( const Locale & locale_r );
          bool eraseRequestedLocale( const Locale & locale_r );

          const LocaleSet & getRequestedLocales() const
          { return _requestedLocales; }

          bool isRequestedLocale( const Locale & locale_r ) const
          {
            LocaleSet::const_iterator it( _requestedLocales.find( locale_r ) );
            return it != _requestedLocales.end();
          }

          const LocaleSet & getAvailableLocales() const;

          bool isAvailableLocale( const Locale & locale_r ) const
          {
            const LocaleSet & avl( getAvailableLocales() );
            LocaleSet::const_iterator it( avl.find( locale_r ) );
            return it != avl.end();
          }
          //@}

        public:
          /** \name Multiversion install. */
          //@{
          typedef IdStringSet MultiversionList;

          const MultiversionList & multiversionList() const
          {
            if ( ! _multiversionListPtr )
              multiversionListInit();
            return *_multiversionListPtr;
          }

          bool isMultiversion( IdString ident_r ) const
          {
            const MultiversionList & l( multiversionList() );
            return l.find( ident_r ) != l.end();
          }
          //@}

        public:
          /** \name Installed on behalf of a user request hint. */
          //@{
          typedef IdStringSet OnSystemByUserList;

          const OnSystemByUserList & onSystemByUserList() const
          {
            if ( ! _onSystemByUserListPtr )
	      onSystemByUserListInit();
	    return *_onSystemByUserListPtr;
          }

          bool isOnSystemByUser( IdString ident_r ) const
          {
            const OnSystemByUserList & l( onSystemByUserList() );
            return l.find( ident_r ) != l.end();
          }
          //@}

        private:
          /** sat-pool. */
          ::_Pool * _pool;
          /** Serial number. */
          SerialNumber _serial;
          /** Watch serial number. */
          SerialNumberWatcher _watcher;
          /** Additional \ref RepoInfo. */
          std::map<RepoIdType,RepoInfo> _repoinfos;

          /**  */
          LocaleSet _requestedLocales;
          mutable scoped_ptr<LocaleSet> _availableLocalesPtr;
          mutable std::tr1::unordered_set<IdString> _locale2Solver;

          /**  */
          void multiversionListInit() const;
          mutable scoped_ptr<MultiversionList> _multiversionListPtr;

          /**  */
          void onSystemByUserListInit() const;
          mutable scoped_ptr<OnSystemByUserList> _onSystemByUserListPtr;
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
