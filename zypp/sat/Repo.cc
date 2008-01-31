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
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

#include "zypp/AutoDispose.h"
#include "zypp/Pathname.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/Repo.h"
#include "zypp/sat/Pool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    const Repo Repo::norepo;

    /////////////////////////////////////////////////////////////////

    ::_Repo * Repo::get() const
    { return myPool().getRepo( _id ); }

#define NO_REPO_RETURN( VAL ) \
    ::_Repo * _repo( get() ); \
    if ( ! _repo ) return VAL

#define NO_REPO_THROW( VAL ) \
    ::_Repo * _repo( get() ); \
    if ( ! _repo ) ZYPP_THROW( VAL )

    bool Repo::isSystemRepo() const
    {
      NO_REPO_RETURN( false );
      return( Pool::systemRepoName() == _repo->name );
    }

    std::string Repo::name() const
    {
      NO_REPO_RETURN( std::string() );
      if ( ! _repo->name )
        return std::string();
      return _repo->name;
    }

    bool Repo::solvablesEmpty() const
    {
      NO_REPO_RETURN( true );
      return _repo->nsolvables;
    }

    Repo::size_type Repo::solvablesSize() const
    {
      NO_REPO_RETURN( 0 );
      return _repo->nsolvables;
    }

    Repo::SolvableIterator Repo::solvablesBegin() const
    {
      NO_REPO_RETURN( make_filter_iterator( detail::ByRepo( *this ),
                                            detail::SolvableIterator(),
                                            detail::SolvableIterator() ) );
      return make_filter_iterator( detail::ByRepo( *this ),
                                   detail::SolvableIterator(_repo->start),
                                   detail::SolvableIterator(_repo->end) );
    }

    Repo::SolvableIterator Repo::solvablesEnd() const
    {
      NO_REPO_RETURN( make_filter_iterator( detail::ByRepo( *this ),
                                            detail::SolvableIterator(),
                                            detail::SolvableIterator() ) );
      return make_filter_iterator(detail::ByRepo( *this ),
                                  detail::SolvableIterator(_repo->end),
                                  detail::SolvableIterator(_repo->end) );
    }

    void Repo::eraseFromPool()
    {
      NO_REPO_RETURN();
      myPool()._deleteRepo( _repo );
      _id = detail::noRepoId;
    }

    void Repo::addSolv( const Pathname & file_r )
    {
      NO_REPO_THROW( Exception( _("Can't add solvables to norepo.") ) );

      AutoDispose<FILE*> file( ::fopen( file_r.c_str(), "r" ), ::fclose );
      if ( file == NULL )
      {
        file.resetDispose();
        ZYPP_THROW( Exception( _("Can't read solv-file: ")+file_r.asString() ) );
      }

      myPool()._addSolv( _repo, file );
    }

    detail::SolvableIdType Repo::addSolvables( unsigned count_r )
    {
      NO_REPO_THROW( Exception( _("Can't add solvables to norepo.") ) );
      return myPool()._addSolvables( _repo, count_r );
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
