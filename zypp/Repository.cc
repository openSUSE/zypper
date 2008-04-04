/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Repository.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

#include "zypp/AutoDispose.h"
#include "zypp/Pathname.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/Repository.h"
#include "zypp/sat/Pool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////

    const Repository Repository::noRepository;

    /////////////////////////////////////////////////////////////////

    ::_Repo * Repository::get() const
    { return myPool().getRepo( _id ); }

#define NO_REPOSITORY_RETURN( VAL ) \
    ::_Repo * _repo( get() ); \
    if ( ! _repo ) return VAL

#define NO_REPOSITORY_THROW( VAL ) \
    ::_Repo * _repo( get() ); \
    if ( ! _repo ) ZYPP_THROW( VAL )

    bool Repository::isSystemRepo() const
    {
	NO_REPOSITORY_RETURN( false );
	return( sat::Pool::systemRepoName() == _repo->name );
    }

    std::string Repository::name() const
    {
	NO_REPOSITORY_RETURN( std::string() );
	if ( ! _repo->name )
	    return std::string();
	return _repo->name;
    }

    bool Repository::solvablesEmpty() const
    {
	NO_REPOSITORY_RETURN( true );
	return !_repo->nsolvables;
    }

    Repository::size_type Repository::solvablesSize() const
    {
	NO_REPOSITORY_RETURN( 0 );
	return _repo->nsolvables;
    }

    Repository::SolvableIterator Repository::solvablesBegin() const
    {
	NO_REPOSITORY_RETURN( make_filter_iterator( detail::ByRepository( *this ),
					      sat::detail::SolvableIterator(),
					      sat::detail::SolvableIterator() ) );
	return make_filter_iterator( detail::ByRepository( *this ),
				     sat::detail::SolvableIterator(_repo->start),
				     sat::detail::SolvableIterator(_repo->end) );
    }

    Repository::SolvableIterator Repository::solvablesEnd() const
    {
	NO_REPOSITORY_RETURN( make_filter_iterator( detail::ByRepository( *this ),
					      sat::detail::SolvableIterator(),
					      sat::detail::SolvableIterator() ) );
	return make_filter_iterator(detail::ByRepository( *this ),
				    sat::detail::SolvableIterator(_repo->end),
				    sat::detail::SolvableIterator(_repo->end) );
    }

    RepoInfo Repository::info() const
    {
	NO_REPOSITORY_RETURN( RepoInfo() );
	return myPool().repoInfo( _repo );
    }

    void Repository::setInfo( const RepoInfo & info_r )
    {
	NO_REPOSITORY_THROW( Exception( _("Can't set RepoInfo for norepo.") ) );
	if ( info_r.alias() != name() )
	{
	    ZYPP_THROW( Exception( str::form( _("RepoInfo alias (%s) does not match repository name (%s)"),
					      info_r.alias().c_str(), name().c_str() ) ) );
	}
	myPool().setRepoInfo( _repo, info_r );

        // satsolver priority is based on '<', while yum's repoinfo
        // uses 1(highest)->99(lowest). Thus we use -info_r.priority.
        _repo->priority = -info_r.priority();
    }

    void Repository::clearInfo()
    {
	NO_REPOSITORY_RETURN();
	myPool().setRepoInfo( _repo, RepoInfo() );
    }

    void Repository::eraseFromPool()
    {
	NO_REPOSITORY_RETURN();
	myPool()._deleteRepo( _repo );
	_id = sat::detail::noRepoId;
    }

    Repository Repository::nextInPool() const
    {
      NO_REPOSITORY_RETURN( noRepository );
      for_( it, sat::Pool::instance().reposBegin(), sat::Pool::instance().reposEnd() )
      {
        if ( *it == *this )
        {
          if ( ++it != _for_end )
            return *it;
          break;
        }
      }
      return noRepository;
    }

#warning NEED POOL MANIP EXEPTIONS
    void Repository::addSolv( const Pathname & file_r )
    {
	NO_REPOSITORY_THROW( Exception( _("Can't add solvables to norepo.") ) );

	AutoDispose<FILE*> file( ::fopen( file_r.c_str(), "r" ), ::fclose );
	if ( file == NULL )
	{
	    file.resetDispose();
	    ZYPP_THROW( Exception( _("Can't open solv-file: ")+file_r.asString() ) );
	}

	if ( myPool()._addSolv( _repo, file, isSystemRepo() ) != 0 )
	{
	    ZYPP_THROW( Exception( _("Error reading solv-file: ")+file_r.asString() ) );
	}
    }

    sat::detail::SolvableIdType Repository::addSolvables( unsigned count_r )
    {
	NO_REPOSITORY_THROW( Exception( _("Can't add solvables to norepo.") ) );
	return myPool()._addSolvables( _repo, count_r );
    }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
     */
    std::ostream & operator<<( std::ostream & str, const Repository & obj )
    {
	if ( ! obj )
	    return str << "noRepository";

	return str << "sat::repo(" << obj.name() << ")"
		   << "{"
                   << "prio " << obj.get()->priority
		   << ", size " << obj.solvablesSize()
		   <<"}";
    }


    /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
