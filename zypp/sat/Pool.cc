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

#include "zypp/Pathname.h"
#include "zypp/AutoDispose.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/Pool.h"
#include "zypp/sat/Repo.h"
#include "zypp/sat/Solvable.h"

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

    const SerialNumber & Pool::serial() const
    { return myPool().serial(); }

    void Pool::setDirty()
    { return myPool().setDirty(); }

    void Pool::prepare()
    { return myPool().prepare(); }

    bool Pool::reposEmpty() const
    { return myPool()->nrepos; }

    unsigned Pool::reposSize() const
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

    unsigned Pool::solvablesSize() const
    {
      // return myPool()->nsolvables;
      // nsolvables is the array size including
      // invalid Solvables.
      unsigned ret = 0;
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
      myPool().setDirty();
      return Repo( ::repo_create( get(), name_r.c_str() ) );
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

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Pool & obj )
    {
      return str << "sat::pool(" << obj.serial() << "){"
          << obj.reposSize() << "repos|"
	  << obj.solvablesSize() << "slov}" << "whatprovides *"<< obj.get()->whatprovides;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
