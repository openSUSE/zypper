/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/CommitPackageCache.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/target/CommitPackageCache.h"
#include "zypp/target/CommitPackageCacheImpl.h"
#include "zypp/target/CommitPackageCacheReadAhead.h"

using std::endl;

#include "zypp/target/rpm/librpmDb.h"
#include "zypp/repo/PackageProvider.h"
#include "zypp/repo/DeltaCandidates.h"
#include "zypp/ResPool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace {
      ///////////////////////////////////////////////////////////////////
      /// \class QueryInstalledEditionHelper
      /// \short Helper for PackageProvider queries during download.
      ///////////////////////////////////////////////////////////////////
      struct QueryInstalledEditionHelper
      {
	bool operator()( const std::string & name_r, const Edition & ed_r, const Arch & arch_r ) const
	{
	  rpm::librpmDb::db_const_iterator it;
	  for ( it.findByName( name_r ); *it; ++it )
	  {
	    if ( arch_r == it->tag_arch()
	      && ( ed_r == Edition::noedition || ed_r == it->tag_edition() ) )
	    {
	      return true;
	    }
	  }
	  return false;
	}
      };
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	class RepoProvidePackage
    //
    ///////////////////////////////////////////////////////////////////

    struct RepoProvidePackage::Impl
    {
      repo::RepoMediaAccess _access;
      std::list<Repository> _repos;
      repo::PackageProviderPolicy _packageProviderPolicy;
    };

    RepoProvidePackage::RepoProvidePackage()
      : _impl( new Impl )
    {
       const ResPool & pool( ResPool::instance() );
      _impl->_repos.insert( _impl->_repos.begin(), pool.knownRepositoriesBegin(), pool.knownRepositoriesEnd() );
      _impl->_packageProviderPolicy.queryInstalledCB( QueryInstalledEditionHelper() );
    }

    RepoProvidePackage::~RepoProvidePackage()
    {}

    ManagedFile RepoProvidePackage::operator()( const PoolItem & pi, bool fromCache_r )
    {
      Package::constPtr p = asKind<Package>(pi.resolvable());
      if ( fromCache_r )
      {
	repo::PackageProvider pkgProvider( _impl->_access, p, repo::DeltaCandidates(), _impl->_packageProviderPolicy );
	return pkgProvider.providePackageFromCache();
      }
      else
      {
	repo::DeltaCandidates deltas( _impl->_repos, p->name() );
	repo::PackageProvider pkgProvider( _impl->_access, p, deltas, _impl->_packageProviderPolicy );
	return pkgProvider.providePackage();
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CommitPackageCache
    //
    ///////////////////////////////////////////////////////////////////

    CommitPackageCache::CommitPackageCache( Impl * pimpl_r )
    : _pimpl( pimpl_r )
    {
      assert( _pimpl );
    }

    CommitPackageCache::CommitPackageCache( const Pathname &        rootDir_r,
                                            const PackageProvider & packageProvider_r )
    {
      if ( getenv("ZYPP_COMMIT_NO_PACKAGE_CACHE") )
        {
          MIL << "$ZYPP_COMMIT_NO_PACKAGE_CACHE is set." << endl;
          _pimpl.reset( new Impl( packageProvider_r ) ); // no cache
        }
      else
        {
          _pimpl.reset( new CommitPackageCacheReadAhead( rootDir_r, packageProvider_r ) );
        }
      assert( _pimpl );
    }

    CommitPackageCache::~CommitPackageCache()
    {}

    void CommitPackageCache::setCommitList( std::vector<sat::Solvable> commitList_r )
    {
      _pimpl->setCommitList( commitList_r );
    }

    ManagedFile CommitPackageCache::get( const PoolItem & citem_r )
    { return _pimpl->get( citem_r ); }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const CommitPackageCache & obj )
    { return str << *obj._pimpl; }

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
