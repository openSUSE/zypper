/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Repo.cc
 *
*/
extern "C"
{
#include <satsolver/pool.h>
#include <satsolver/repo.h>
#include <satsolver/repo_solv.h>
}
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

#include "zypp/AutoDispose.h"

#include "zypp/sat/Repo.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    const Repo Repo::norepo( NULL );

    std::string Repo::name() const
    {
      if ( ! _repo || ! _repo->name ) return std::string();
      return _repo->name;
    }

    bool Repo::solvablesEmpty() const
    {
      if ( ! _repo ) return true;
      return _repo->nsolvables;
    }

    unsigned Repo::solvablesSize() const
    {
      if ( ! _repo ) return 0;
      return _repo->nsolvables;
    }

    SolvableIterator Repo::solvablesBegin() const
    {
      if ( ! _repo ) return SolvableIterator();
      return SolvableIterator( _repo->pool->solvables+_repo->start );
    }

    SolvableIterator Repo::solvablesEnd() const
    {
      if ( ! _repo ) return SolvableIterator();
      return SolvableIterator( _repo->pool->solvables+_repo->start+_repo->nsolvables );
    }

    void Repo::eraseFromPool()
    {
      if ( ! _repo ) return;
      ::repo_free( _repo, /*reuseids*/false );
    }

    void Repo::addSolv( const Pathname & file_r )
    {
      if ( ! _repo )
      {
        ZYPP_THROW( Exception( "Can't add solvables to noepo." ) );
      }

      AutoDispose<FILE*> file( ::fopen( file_r.c_str(), "r" ), ::fclose );
      if ( file == NULL )
      {
        file.resetDispose();
        ZYPP_THROW( Exception( "Can't read solv-file "+file_r.asString() ) );
      }

      ::repo_add_solv( _repo, file );
    }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Repo & obj )
    {
      if ( ! obj )
        return str << "sat::repo()";

      return str << "sat::repo(" << obj.name() << ")"
          << "{"
          << obj.solvablesSize()
          << ' ' << obj.get()->start << ' ' << obj.get()->end << ' '
          << (obj.get()->start < 0      ? "_START_":"")
          << (obj.get()->nsolvables < 0 ?"_NUMSOLV_":"")
          <<"}";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
