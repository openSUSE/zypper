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

#include "zypp/base/Easy.h"
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

    bool Pool::reposEmpty() const
    { return _pool.nrepos; }

    unsigned Pool::reposSize() const
    { return _pool.nrepos; }

    RepoIterator Pool::reposBegin() const
    { return make_transform_iterator( _pool.repos, detail::mkRepo() ); }

    RepoIterator Pool::reposEnd() const
    { return make_transform_iterator( _pool.repos+_pool.nrepos, detail::mkRepo() ); }


    bool Pool::solvablesEmpty() const
    { return _pool.nsolvables;}

    unsigned Pool::solvablesSize() const
    { return _pool.nsolvables;}

    SolvableIterator Pool::solvablesBegin() const
    { return SolvableIterator( _pool.solvables ); }

    SolvableIterator Pool::solvablesEnd() const
    { return SolvableIterator( _pool.solvables+_pool.nsolvables ); }

    Repo Pool::reposInsert( const std::string & name_r )
    {
      Repo ret( reposFind( name_r ) );
      if ( ret )
        return ret;
      return ::repo_create( &_pool, name_r.c_str() );
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
      AutoDispose<Repo> tmprepo( (EraseRepo()) );
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
