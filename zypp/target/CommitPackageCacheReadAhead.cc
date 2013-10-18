/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/CommitPackageCacheReadAhead.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/PathInfo.h"
#include "zypp/RepoInfo.h"
#include "zypp/Package.h"
#include "zypp/target/CommitPackageCacheReadAhead.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : IMediaKey
    //
    ///////////////////////////////////////////////////////////////////

    std::ostream & operator<<( std::ostream & str, const IMediaKey & obj )
    {
      return str << "[S" << obj._repo.id() << ":" << obj._mediaNr << "]"
                 << " " << obj._repo.info().alias();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CommitPackageCacheReadAhead
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CommitPackageCacheReadAhead::CommitPackageCacheReadAhead
    //	METHOD TYPE : Ctor
    //
    CommitPackageCacheReadAhead::CommitPackageCacheReadAhead( const Pathname &        /*rootDir_r*/,
                                                              const PackageProvider & packageProvider_r )
    : CommitPackageCache::Impl( packageProvider_r )
    //, _rootDir( rootDir_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CommitPackageCacheReadAhead::onInteractiveMedia
    //	METHOD TYPE : bool
    //
    bool CommitPackageCacheReadAhead::onInteractiveMedia( const PoolItem & pi ) const
    {
      if ( pi->mediaNr() == 0 ) // no media access at all
        return false;
      if ( pi->repoInfo().baseUrlsEmpty() )
        return false; // no Url - should actually not happen
      std::string scheme( pi->repoInfo().baseUrlsBegin()->getScheme() );
      return ( scheme == "dvd" || scheme == "cd" );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CommitPackageCacheReadAhead::cacheLastInteractive
    //	METHOD TYPE : void
    //
    void CommitPackageCacheReadAhead::cacheLastInteractive( const PoolItem & citem_r )
    {
      // Fill cache errors are never proagated.
      try
        {
          doCacheLastInteractive( citem_r );
        }
      catch ( const Exception & excpt_r )
        {
          ZYPP_CAUGHT( excpt_r );
          WAR << "Failed to cache " << _lastInteractive << endl;
        }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CommitPackageCacheReadAhead::doCacheLastInteractive
    //	METHOD TYPE : void
    //
    void CommitPackageCacheReadAhead::doCacheLastInteractive( const PoolItem & citem_r )
    {
      unsigned  addToCache = 0;
      bool      sawCitem = false;

      // Collect all remaining packages to install from
      // _lastInteractive media. (just the PoolItem data)
      for_( it, commitList().begin(), commitList().end() )
      {
	PoolItem pi( *it );
	if ( ! sawCitem )
	{
	  if ( pi == citem_r )
	    sawCitem = true;
	  continue;
	}
	if ( IMediaKey( pi ) == _lastInteractive
	  && pi.status().isToBeInstalled()
	  && isKind<Package>(pi.resolvable()) )
	{
	  if ( ! pi->asKind<Package>()->isCached() )
	  {
	    ManagedFile fromSource( sourceProvidePackage( pi ) );
	    if ( fromSource->empty() )
	    {
	      ERR << "Copy to cache failed on " << fromSource << endl;
	      ZYPP_THROW( Exception("Copy to cache failed.") );
	    }
	    fromSource.resetDispose(); // keep the package file in the cache
	    ++addToCache;
	  }
	}
      }

      if ( addToCache )
	MIL << "Cached " << _lastInteractive << ": " << addToCache << " items." << endl;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CommitPackageCacheReadAhead::get
    //	METHOD TYPE : ManagedFile
    //
    ManagedFile CommitPackageCacheReadAhead::get( const PoolItem & citem_r )
    {
      // Non CD/DVD media provide their packages without cache.
      if ( ! onInteractiveMedia( citem_r ) )
      {
	return sourceProvidePackage( citem_r );
      }

      // Check whether it's cached.
      ManagedFile ret( sourceProvideCachedPackage( citem_r ) );
      if ( ! ret->empty() )
	return ret;

      IMediaKey current( citem_r );
      if ( current != _lastInteractive )
      {
	if ( _lastInteractive != IMediaKey() )
	{
	  cacheLastInteractive( citem_r );
	}

	DBG << "Interactive change [" << ++_dbgChanges << "] from " << _lastInteractive << " to " << current << endl;
	_lastInteractive = current;
      }

      // Provide and return the file from media.
      return sourceProvidePackage( citem_r );
    }


    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
