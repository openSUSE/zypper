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
#include "zypp/Pathname.h"

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

        void t() const;

      public:
        Repo addRepoSolv( const Pathname & file_r );

      private:
        /** Explicitly shared sat-pool. */
        AutoDispose< ::_Pool *> _raii;
        /** Convenient access. */
        ::_Pool & _pool;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Pool Stream output */
    std::ostream & operator<<( std::ostream & str, const Pool & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_POOL_H
