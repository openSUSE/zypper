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

    const char * Repo::name() const
    {
      if ( ! _repo ) return "";
      return _repo->name;
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

    void Repo::addSolv( const Pathname & file_r )
    {
#warning add ecxception in repo_add_solv
      AutoDispose<FILE*> file( ::fopen( file_r.c_str(), "r" ), ::fclose );
      if ( file == NULL )
      {
        file.resetDispose();
        throw;
        //return Repo();
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
