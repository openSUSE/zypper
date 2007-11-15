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
#include "zypp/base/NonCopyable.h"
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
        bool reposEmpty() const;
        unsigned reposSize() const;
        RepoIterator reposBegin() const;
        RepoIterator reposEnd() const;

        /** Return a \ref Repo named \c name_r.
         * It a such a \ref Repo does not already exist
         * a new empty \ref Repo is created.
         */
        Repo reposInsert( const std::string & name_r );

        /** Find a \ref Repo named \c name_r.
         * Returns \ref norepo if there is no such \ref Repo.
         */
        Repo reposFind( const std::string & name_r ) const;

        /** Remove a \ref Repo named \c name_r. */
        void reposErase( const std::string & name_r )
        { reposErase( reposFind( name_r ) ); }
        /** \overload */
        void reposErase( Repo repo_r )
        { repo_r.eraseFromPool(); }

      public:
        /** Functor removing \ref Repo from it's \ref Pool. */
        struct EraseRepo;

        /** Load \ref Solvables from a solv-file into a \ref Repo named \c name_r.
         * In case of an exception the \ref Repo is removed from the \ref Pool.
         * \throws Exception if loading the solv-file fails.
        */
        Repo addRepoSolv( const Pathname & file_r, const std::string & name_r );
        /** \overload Using the files basename as \ref Repo name. */
        Repo addRepoSolv( const Pathname & file_r )
        { return addRepoSolv( file_r, file_r.basename() ); }

      public:
        bool solvablesEmpty() const;
        unsigned solvablesSize() const;
        SolvableIterator solvablesBegin() const;
        SolvableIterator solvablesEnd() const;

      private:
        /** Explicitly shared sat-pool. */
        AutoDispose< ::_Pool *> _raii;
        /** Convenient access. */
        ::_Pool & _pool;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Pool Stream output */
    std::ostream & operator<<( std::ostream & str, const Pool & obj );

    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Pool::EraseRepo
    //
    /** Functor removing \ref Repo from it's \ref Pool.
     * E.g. used as dispose function in. \ref AutoDispose
     * to provide a convenient and exception safe temporary
     * \ref Repo.
     * \code
     *  sat::Pool satpool;
     *  MIL << "1 " << satpool << endl;
     *  {
     *    AutoDispose<sat::Repo> tmprepo( (sat::Pool::EraseRepo()) );
     *    *tmprepo = satpool.reposInsert( "A" );
     *    tmprepo->addSolv( "sl10.1-beta7-packages.solv" );
     *    DBG << "2 " << satpool << endl;
     *    // Calling 'tmprepo.resetDispose();' here
     *    // would keep the Repo.
     *  }
     *  MIL << "3 " << satpool << endl;
     * \endcode
     * \code
     * 1 sat::pool(){0repos|2slov}
     * 2 sat::pool(){1repos|2612slov}
     * 3 sat::pool(){0repos|2slov}
     * \endcode
     * Leaving the block without calling <tt>tmprepo.resetDispose();</tt>
     * before, will automatically remove the \ref Repo from it's \ref Pool.
     */
    struct Pool::EraseRepo
    {
      void operator()( Repo repo_r ) const
      { repo_r.eraseFromPool(); }
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_POOL_H
