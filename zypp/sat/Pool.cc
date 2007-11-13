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
#include <stdio.h>
#include <satsolver/repo_solv.h>
#include <satsolver/pool.h>
}
#include <cstdio>
#include <iostream>
#include <set>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

#include "zypp/sat/Pool.h"
#include "zypp/sat/Repo.h"

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
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Pool::Pool
    //	METHOD TYPE : Ctor
    //
    Pool::Pool()
      : _raii( ::pool_create(), ::pool_free )
      , _pool( *_raii.value() )
    {
      if ( _raii == NULL )
      {
        _raii.resetDispose(); // no call to ::pool_free
        ZYPP_THROW( Exception( _("Can not create sat-pool.") ) );
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Pool::~Pool
    //	METHOD TYPE : Dtor
    //
    Pool::~Pool()
    {}

    unsigned Pool::reposSize() const
    { return _pool.nrepos; }

    RepoIterator Pool::reposBegin() const
    { return make_transform_iterator( _pool.repos, detail::mkRepo() ); }

    RepoIterator Pool::reposEnd() const
    { return make_transform_iterator( _pool.repos+_pool.nrepos, detail::mkRepo() ); }


    unsigned Pool::solvablesSize() const
    { return _pool.nsolvables;}

    SolvableIterator Pool::solvablesBegin() const
    { return SolvableIterator( _pool.solvables ); }

    SolvableIterator Pool::solvablesEnd() const
    { return SolvableIterator( _pool.solvables+_pool.nsolvables ); }


    void Pool::t() const
    {
      if ( _pool.nrepos )
      {
        SEC << (void*)(*_pool.repos)<< Repo( *_pool.repos ) << " " << (*_pool.repos)->name << std::endl;
      }
      else
      {
        SEC << "NOREPO" << std::endl;
      }

    }

    Repo Pool::addRepoSolv( const Pathname & file_r )
    {
      AutoDispose<FILE*> file( ::fopen( file_r.c_str(), "r" ), ::fclose );
      if ( file == NULL )
      {
        file.resetDispose();
        return Repo();
      }

#warning Workaround sat-repo not doing strdup on name.
      // simply spend a static array of reponames
      static std::set<std::string> _reponames;
      return ::pool_addrepo_solv( &_pool, file, _reponames.insert( file_r.asString() ).first->c_str() );
    }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Pool & obj )
    {
      return str << "sat::pool(){"
          << obj.reposSize() << "repos|"
          << obj.solvablesSize() << "slov}";

    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
