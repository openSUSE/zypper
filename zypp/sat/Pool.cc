/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Pool.cc
 *
*/

extern "C"
{
#include <satsolver/pool.h>
#include <satsolver/repo.h>
}
#include <iostream>

#include "zypp/base/Easy.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

#include "zypp/AutoDispose.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/Pool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    const std::string & Pool::systemRepoName()
    {
      static const std::string _val( "@System" );
      return _val;
    }

    ::_Pool * Pool::get() const
    { return myPool().getPool(); }

    Pool::size_type Pool::capacity() const
    { return myPool()->nsolvables; }

    const SerialNumber & Pool::serial() const
    { return myPool().serial(); }

    void Pool::prepare()
    { return myPool().prepare(); }

    bool Pool::reposEmpty() const
    { return myPool()->nrepos; }

    Pool::size_type Pool::reposSize() const
    { return myPool()->nrepos; }

    Pool::RepoIterator Pool::reposBegin() const
    { return RepoIterator( myPool()->repos ); }

    Pool::RepoIterator Pool::reposEnd() const
    { return RepoIterator( myPool()->repos+myPool()->nrepos ); }

    bool Pool::solvablesEmpty() const
    {
      // return myPool()->nsolvables;
      // nsolvables is the array size including
      // invalid Solvables.
      for_( it, reposBegin(), reposEnd() )
      {
        if ( ! it->solvablesEmpty() )
          return false;
      }
      return true;
    }

    Pool::size_type Pool::solvablesSize() const
    {
      // Do not return myPool()->nsolvables;
      // nsolvables is the array size including
      // invalid Solvables.
      size_type ret = 0;
      for_( it, reposBegin(), reposEnd() )
      {
        ret += it->solvablesSize();
      }
      return ret;
    }

    Pool::SolvableIterator Pool::solvablesBegin() const
    { return SolvableIterator( myPool().getFirstId() ); }

    Pool::SolvableIterator Pool::solvablesEnd() const
    { return SolvableIterator(); }

    Repo Pool::reposInsert( const std::string & name_r )
    {
      Repo ret( reposFind( name_r ) );
      if ( ret )
        return ret;

      ret = Repo( myPool()._createRepo( name_r ) );
      if ( name_r == systemRepoName() )
      {
        // autoprovide (dummy) RepoInfo
        ret.setInfo( RepoInfo()
                     .setAlias( name_r )
                     .setName( name_r )
                     .setAutorefresh( true )
                     .setEnabled( true ) );
      }
      return ret;
    }

    Repo Pool::reposFind( const std::string & name_r ) const
    {
      for_( it, reposBegin(), reposEnd() )
      {
        if ( name_r == it->name() )
          return *it;
      }
      return Repo();
    }

    Repo Pool::addRepoSolv( const Pathname & file_r, const std::string & name_r )
    {
      // Using a temporay repo! (The additional parenthesis are required.)
      AutoDispose<Repo> tmprepo( (Repo::EraseFromPool()) );
      *tmprepo = reposInsert( name_r );
      tmprepo->addSolv( file_r );

      // no exceptions so we keep it:
      tmprepo.resetDispose();
      return tmprepo;
    }

    Repo Pool::addRepoSolv( const Pathname & file_r )
    { return addRepoSolv( file_r, file_r.basename() ); }

    Repo Pool::addRepoSolv( const Pathname & file_r, const RepoInfo & info_r )
    {
      Repo ret( addRepoSolv( file_r, info_r.alias() ) );
      ret.setInfo( info_r );
      return ret;
    }


    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Pool & obj )
    {
      return str << "sat::pool(" << obj.serial() << ")["
          << obj.capacity() << "]{"
          << obj.reposSize() << "repos|"
	  << obj.solvablesSize() << "slov}";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
