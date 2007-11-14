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
}
#include <iostream>

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


    Repo Pool::addRepo( const std::string & name_r )
    {
#warning Implement name check
      return ::repo_create( &_pool, name_r.c_str() );
    }

    Repo Pool::addRepoSolv( const Pathname & file_r, const std::string & name_r )
    {
      Repo repo( addRepo( name_r.empty() ? file_r.basename() : name_r ) );
      try
      {
        repo.addSolv( file_r );
      }
      catch ( ... )
      {
#warning use RAII to avoid cleanup catch
        ::repo_free( repo.get() );
        throw;
      }
      return repo;
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
