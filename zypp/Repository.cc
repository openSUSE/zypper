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
#include <climits>
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Xml.h"

#include "zypp/AutoDispose.h"
#include "zypp/Pathname.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/Repository.h"
#include "zypp/sat/Pool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    const Repository Repository::noRepository;

    const std::string & Repository::systemRepoAlias()
    { return sat::detail::PoolImpl::systemRepoAlias(); }

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
	return myPool().isSystemRepo( _repo );
    }

    std::string Repository::alias() const
    {
      NO_REPOSITORY_RETURN( std::string() );
      if ( ! _repo->name )
        return std::string();
      return _repo->name;
    }

    std::string Repository::name() const
    { return info().name(); }

    int Repository::satInternalPriority() const
    {
      NO_REPOSITORY_RETURN( INT_MIN );
      return _repo->priority;
    }

    int Repository::satInternalSubPriority() const
    {
      NO_REPOSITORY_RETURN( INT_MIN );
      return _repo->subpriority;
    }

    Repository::ContentRevision contentRevision() const
    {
      NO_REPOSITORY_RETURN( ContentRevision() );
      sat::LookupRepoAttr q( sat::SolvAttr::repositoryRevision, *this );
      return q.empty() ? std::string() : q.begin().asString();
    }

    Repository::ContentIdentifier Repository::contentIdentifier() const
    {
      NO_REPOSITORY_RETURN( ContentIdentifier() );
      sat::LookupRepoAttr q( sat::SolvAttr::repositoryRepoid, *this );
      return q.empty() ? std::string() : q.begin().asString();
    }

    zypp::Date Repository::generatedTimestamp() const
    {
      NO_REPOSITORY_RETURN( 0 );
      sat::LookupRepoAttr q( sat::SolvAttr::repositoryTimestamp, *this );
      return( q.empty() ? 0 : q.begin().asUnsigned() );
    }

    zypp::Date Repository::suggestedExpirationTimestamp() const
    {
      NO_REPOSITORY_RETURN( 0 );
      Date generated = generatedTimestamp();
      if ( ! generated )
        return 0; // do not calculate over a missing generated timestamp

      sat::LookupRepoAttr q( sat::SolvAttr::repositoryExpire, *this );
      if ( q.empty() )
        return 0;

      return generated + Date(q.begin().asUnsigned());
    }

    Repository::Keywords Repository::keywords() const
    {
      NO_REPOSITORY_RETURN( Keywords() );
      return Keywords( sat::SolvAttr::repositoryKeywords, *this, sat::LookupAttr::REPO_ATTR );
    }

    bool Repository::hasKeyword( const std::string & val_r ) const
    {
      for ( const auto & val : keywords() )
	if ( val == val_r )
	  return true;
      return false;
    }

    bool Repository::maybeOutdated() const
    {
      NO_REPOSITORY_RETURN( false );
      // system repo is not mirrored
      if ( isSystemRepo() )
        return false;

      Date suggested = suggestedExpirationTimestamp();

      // if no data, don't suggest
      if ( ! suggested )
        return false;

      return suggestedExpirationTimestamp() < Date::now();
    }

    bool Repository::providesUpdatesFor( const std::string &key ) const
    {
      NO_REPOSITORY_RETURN( false );

      for_( it,
            updatesProductBegin(),
            updatesProductEnd() )
      {
        // FIXME implement real CPE matching here
        // someday
        if ( key == it.cpeId() )
          return true;
      }

      return false;
    }

    bool Repository::isUpdateRepo() const
    {
      NO_REPOSITORY_RETURN( false );
      return ( updatesProductBegin() != updatesProductEnd() );
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

    Repository::ProductInfoIterator Repository::compatibleWithProductBegin() const
    {
      NO_REPOSITORY_RETURN( ProductInfoIterator() );
      return ProductInfoIterator( sat::SolvAttr::repositoryDistros, *this );
    }

    Repository::ProductInfoIterator Repository::compatibleWithProductEnd() const
    {
      return ProductInfoIterator();
    }

    Repository::ProductInfoIterator Repository::updatesProductBegin() const
    {
      NO_REPOSITORY_RETURN( ProductInfoIterator() );
      return ProductInfoIterator( sat::SolvAttr::repositoryUpdates, *this );
    }

    Repository::ProductInfoIterator Repository::updatesProductEnd() const
    {
      return ProductInfoIterator();
    }

    RepoInfo Repository::info() const
    {
      NO_REPOSITORY_RETURN( RepoInfo() );
      return myPool().repoInfo( _repo );
    }

    void Repository::setInfo( const RepoInfo & info_r )
    {
	NO_REPOSITORY_THROW( Exception( "Can't set RepoInfo for norepo." ) );
	if ( info_r.alias() != alias() )
	{
	    ZYPP_THROW( Exception( str::form( "RepoInfo alias (%s) does not match repository alias (%s)",
					      info_r.alias().c_str(), alias().c_str() ) ) );
	}
	myPool().setRepoInfo( _repo, info_r );
        MIL << *this << endl;
    }

    void Repository::clearInfo()
    {
	NO_REPOSITORY_RETURN();
	myPool().setRepoInfo( _repo, RepoInfo() );
    }

    void Repository::eraseFromPool()
    {
	NO_REPOSITORY_RETURN();
        MIL << *this << " removed from pool" << endl;
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

    void Repository::addSolv( const Pathname & file_r )
    {
      NO_REPOSITORY_THROW( Exception( "Can't add solvables to norepo." ) );

      AutoDispose<FILE*> file( ::fopen( file_r.c_str(), "re" ), ::fclose );
      if ( file == NULL )
      {
        file.resetDispose();
        ZYPP_THROW( Exception( "Can't open solv-file: "+file_r.asString() ) );
      }

      if ( myPool()._addSolv( _repo, file ) != 0 )
      {
        ZYPP_THROW( Exception( "Error reading solv-file: "+file_r.asString() ) );
      }

      MIL << *this << " after adding " << file_r << endl;
    }

    void Repository::addHelix( const Pathname & file_r )
    {
      NO_REPOSITORY_THROW( Exception( "Can't add solvables to norepo." ) );

      std::string command( file_r.extension() == ".gz" ? "zcat " : "cat " );
      command += file_r.asString();

      AutoDispose<FILE*> file( ::popen( command.c_str(), "re" ), ::pclose );
      if ( file == NULL )
      {
        file.resetDispose();
        ZYPP_THROW( Exception( "Can't open helix-file: "+file_r.asString() ) );
      }

      if ( myPool()._addHelix( _repo, file ) != 0 )
      {
        ZYPP_THROW( Exception( "Error reading helix-file: "+file_r.asString() ) );
      }

      MIL << *this << " after adding " << file_r << endl;
    }

    sat::detail::SolvableIdType Repository::addSolvables( unsigned count_r )
    {
	NO_REPOSITORY_THROW( Exception( "Can't add solvables to norepo.") );
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

	return str << "sat::repo(" << obj.alias() << ")"
		   << "{"
                   << "prio " << obj.get()->priority << '.' << obj.get()->subpriority
		   << ", size " << obj.solvablesSize()
		   << "}";
    }

    std::ostream & dumpAsXmlOn( std::ostream & str, const Repository & obj )
    {
      return xmlout::node( str, "repository", {
	{ "alias", obj.name() },
	{ "name", obj.alias() }
      } );
    }

    //////////////////////////////////////////////////////////////////
    namespace detail
    {
      void RepositoryIterator::increment()
      {
	if ( base() )
	{
	  ::_Pool * satpool = sat::Pool::instance().get();
	  do {
	    ++base_reference();
	  } while ( base() < satpool->repos+satpool->nrepos && !*base() );
	}
      }
    } // namespace detail
    //////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    // Repository::ProductInfoIterator
    //
    ///////////////////////////////////////////////////////////////////

    Repository::ProductInfoIterator::ProductInfoIterator( sat::SolvAttr attr_r, Repository repo_r )
    { base_reference() = sat::LookupRepoAttr( attr_r, repo_r ).begin(); }

    std::string Repository::ProductInfoIterator::label() const
    { return base_reference().subFind( sat::SolvAttr::repositoryProductLabel ).asString(); }

    std::string Repository::ProductInfoIterator::cpeId() const
    { return base_reference().subFind( sat::SolvAttr::repositoryProductCpeid ).asString(); }

    /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
