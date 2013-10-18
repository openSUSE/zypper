/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/CommitPackageCacheReadAhead.h
 *
*/
#ifndef ZYPP_TARGET_COMMITPACKAGECACHEREADAHEAD_H
#define ZYPP_TARGET_COMMITPACKAGECACHEREADAHEAD_H

#include <map>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/Repository.h"
#include "zypp/TmpPath.h"
#include "zypp/target/CommitPackageCacheImpl.h"

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
    /** Helper storing a source id and media number. */
    struct IMediaKey
    {
      IMediaKey()
      {}

      explicit
      IMediaKey( const PoolItem & obj_r )
      : _repo( obj_r->repository() )
      , _mediaNr( obj_r->mediaNr() )
      {}

      explicit
      IMediaKey( const ResObject::constPtr & obj_r )
      : _repo( obj_r->repository() )
      , _mediaNr( obj_r->mediaNr() )
      {}

      IMediaKey( const Repository & repo, unsigned mediaNr_r )
      : _repo( repo )
      , _mediaNr( mediaNr_r )
      {}

      bool operator==( const IMediaKey & rhs ) const
      { return( _repo == rhs._repo && _mediaNr == rhs._mediaNr ); }

      bool operator!=( const IMediaKey & rhs ) const
      { return ! operator==( rhs ); }

      bool operator<( const IMediaKey & rhs ) const
      {
        return( _repo.id() < rhs._repo.id()
                || ( _repo.id() == rhs._repo.id()
                     && _mediaNr < rhs._mediaNr ) );
      }

      Repository                  _repo;
      DefaultIntegral<unsigned,0> _mediaNr;
    };
    ///////////////////////////////////////////////////////////////////

    std::ostream & operator<<( std::ostream & str, const IMediaKey & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CommitPackageCacheReadAhead
    //
    /** */
    class CommitPackageCacheReadAhead : public CommitPackageCache::Impl
    {
    public:
      CommitPackageCacheReadAhead( const Pathname &        /*rootDir_r*/,
                                   const PackageProvider & packageProvider_r );

    public:
      /** Provide the package. Either from Source or from cache. */
      virtual ManagedFile get( const PoolItem & citem_r );

    private:
      /** Return whether \a pi is located on a CD/DVD */
      bool onInteractiveMedia( const PoolItem & pi ) const;

    private:
      /** Fill the cache.
       * Called before changing from one interactive media to another.
       * Performs the read ahead of packages trying to avoid the necessity
       * of switching back to the current media later.
      */
      void cacheLastInteractive( const PoolItem & citem_r );

      /** cacheLastInteractive helper . */
      void doCacheLastInteractive( const PoolItem & citem_r );

    private:
      DefaultIntegral<unsigned,0> _dbgChanges;
      IMediaKey                   _lastInteractive;
      //Pathname                    _rootDir;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_COMMITPACKAGECACHEREADAHEAD_H
