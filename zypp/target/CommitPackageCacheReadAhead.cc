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
    CommitPackageCacheReadAhead::CommitPackageCacheReadAhead( const Pathname &        rootDir_r,
                                                              const PackageProvider & packageProvider_r )
    : CommitPackageCache::Impl( packageProvider_r )
    , _rootDir( rootDir_r )
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
      CacheMap  addToCache;
      ByteCount addSize;

      // Collect all remaining packages to install from
      // _lastInteractive media. (just the PoolItem data)
      for_( it,_commitList.begin(), _commitList.end() )
        {
	  PoolItem pi( *it );
          if ( IMediaKey( pi ) == _lastInteractive
               && isKind<Package>(pi.resolvable())
               && pi.status().isToBeInstalled() )
            {
              if ( _cacheMap.find( pi ) == _cacheMap.end() )
                {
                  addToCache[pi];
                  addSize += pi->downloadSize();
                }
            }
        }

      if ( addToCache.empty() )
        return;
      MIL << "could cache " << _lastInteractive << ": " << addToCache.size() << " items: " <<  addSize << endl;

      // Check whether we can afford caching the items. We cache them all or
      // nothing. It does not make sense to cache only some packages, if a
      // CD change can't be avoided.
      if ( ! _cacheDir )
        {
          _cacheDir.reset( new filesystem::TmpDir( _rootDir, "commitCache." ) );
          PathInfo pi( _cacheDir->path() );
          if ( ! pi.isDir() )
            {
              ERR << "Can not initialize cache dir " << pi << endl;
              return;
            }
        }

      // In case someone removes cacheDir behind our back, df will be
      // -1, so we won't cache.
      ByteCount df( filesystem::df( _cacheDir->path() ) );
      MIL << "available disk space in " << _cacheDir->path() << ": " << df << endl;

      if ( df / 10 < addSize )
        {
          WAR << "cache would require more than 10% of the available " << df << " disk space " << endl;
          WAR << "not caching " << _lastInteractive << endl;
          return;
        }

      // Get all files to cache from the Source and copy them to
      // the cache.
      // NOTE: All files copied to the cache directory are stored in addToCache,
      // which is a local variable. If we throw on error, addToCache will be
      // deleted and all the ManagedFiles stored so far will delete themself.
      // THIS IS EXACTLY WHAT WE WANT.
      for ( CacheMap::iterator it = addToCache.begin(); it != addToCache.end(); ++it )
        {
          // let the source provide the file
          ManagedFile fromSource( sourceProvidePackage( it->first ) );

          // copy it to the cachedir
          std::string destName( str::form( "S%p_%u_%s",
                                           it->first->repository().id(),
                                           it->first->mediaNr(),
                                           fromSource.value().basename().c_str() ) );

          ManagedFile fileInCache( _cacheDir->path() / destName,
                                   filesystem::unlink );

          if ( filesystem::copy( fromSource.value(), fileInCache ) != 0 )
            {
              // copy to cache failed.
              ERR << "Copy to cache failed on " << fromSource.value() << endl;
              ZYPP_THROW( Exception("Copy to cache failed.") );
            }

          // remember the cached file.
          it->second = fileInCache;
        }

      // Here: All files are sucessfully copied to the cache.
      // Update the real cache map.
      _cacheMap.insert( addToCache.begin(), addToCache.end() );
      return;
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
      CacheMap::iterator it = _cacheMap.find( citem_r );
      if ( it != _cacheMap.end() )
        {
          // ManagedFile delivered to the application is removed
          // from the cache. So if the application releases the
          // file, it actually gets deleted from disk.
          ManagedFile cacheHit( it->second );
          _cacheMap.erase( it );

          // safety check whether the file still exists
          PathInfo pi( cacheHit.value() );
          if ( pi.isFile() )
            {
              MIL << "Cache package provide " << cacheHit << endl;
              return cacheHit;
            }

          WAR << "Cached file vanished: " << pi << endl;
        }

      // HERE: It's not in the cache.
      // In case we have to change the media to provide the requested
      // file, try to cache files from the current media, that are
      // required later.
      IMediaKey current( citem_r );
      if ( current != _lastInteractive )
        {
          if ( _lastInteractive != IMediaKey() )
            {
              cacheLastInteractive( citem_r );
            }

          DBG << "Interactive change [" << ++_dbgChanges << "] from " << _lastInteractive
          << " to " << current << endl;
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
