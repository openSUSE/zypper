/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Pool.h
 *
*/
#ifndef ZYPP_SAT_POOL_H
#define ZYPP_SAT_POOL_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Iterator.h"

#include "zypp/AutoDispose.h"

#include "zypp/sat/Repo.h"

///////////////////////////////////////////////////////////////////
extern "C"
{
struct _Pool;
}
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Pool
    //
    /** */
    class Pool
    {
      public:
        /** Default ctor */
        Pool();
        /** Dtor */
        ~Pool();

      public:
        unsigned reposSize() const;
        RepoIterator reposBegin() const;
        RepoIterator reposEnd() const;

        unsigned solvablesSize() const;
        SolvableIterator solvablesBegin() const;
        SolvableIterator solvablesEnd() const;

      public:
        /** Add new empty \ref Repo named \c name_r.
         * \throws Exception if \ref Repo with named \c name_r exists.
         */
        Repo addRepo( const std::string & name_r );

        /** Add new \ref Repo from solv-file.
         * \c name_r defaults to the solvfiles basename.
         * \throws Exception from \ref Pool::addRepo or \ref Repo::addSolv
         */
        Repo addRepoSolv( const Pathname & file_r, const std::string & name_r = std::string() );

     private:
        /** Explicitly shared sat-pool. */
        AutoDispose< ::_Pool *> _raii;
        /** Convenient access. */
        ::_Pool & _pool;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Pool Stream output */
    std::ostream & operator<<( std::ostream & str, const Pool & obj );
#if 0
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : TempRepo
    //
    /** Maintain a temporary \ref Repo.
     * Any included temporary \ref Repo is removed from the \ref Prool
     * upon destruction. This may ease convenient and exception safe
     * creation of repos.
     * \code
     * {
     *   TempRepo tmp( pool.addRepo( "newrepo" ) );
     *
     *   // Exceptions when loading Solvables into "newrepo"
     *   // may bypass, and "newrepo" will be removed from
     *   // the pool.
     *
     *   if ( keep )
     *   {
     *     // If you decide to keep "newrepo", simply
     *     // clear TempRepo to prevent removal.
     *     tmp.reset();
     *   }
     * }
     * \endcode
    */
    class TempRepo : private base::NonCopyable
    {
      public:
        TempRepo( const Repo & repo_r )
        : _repo( repo_r )
        {}

        ~TempRepo()
        {
          if ( _repo )
            _repo.
        }

      private:
        Repo _repo;
    };
    ///////////////////////////////////////////////////////////////////
#endif
    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_POOL_H
