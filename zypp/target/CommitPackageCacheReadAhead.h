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

#include "zypp/Source.h"
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
      IMediaKey( const ResObject::constPtr & obj_r )
      : _source( obj_r->source() )
      , _mediaNr( obj_r->sourceMediaNr() )
      {}

      IMediaKey( const Source_Ref & source_r, unsigned mediaNr_r )
      : _source( source_r )
      , _mediaNr( mediaNr_r )
      {}

      bool operator==( const IMediaKey & rhs ) const
      { return( _source == rhs._source && _mediaNr == rhs._mediaNr ); }

      bool operator!=( const IMediaKey & rhs ) const
      { return ! operator==( rhs ); }

      bool operator<( const IMediaKey & rhs ) const
      {
        return( _source.numericId() < rhs._source.numericId()
                || ( _source.numericId() == rhs._source.numericId()
                     && _mediaNr < rhs._mediaNr ) );
      }

      Source_Ref                  _source;
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
      typedef std::map<PoolItem,ManagedFile>     CacheMap;

    public:
      CommitPackageCacheReadAhead( const_iterator          begin_r,
                                   const_iterator          end_r,
                                   const Pathname &        rootDir_r,
                                   const PackageProvider & packageProvider_r );

    public:
      /** Provide the package. Either from Source or from cache. */
      virtual ManagedFile get( const_iterator citem_r );

    private:
      /** Return whether \a pi is located on a CD/DVD */
      bool onInteractiveMedia( const PoolItem & pi ) const;

    private:
      /** Fill the cache.
       * Called before changing from one interactive media to another.
       * Performs the read ahead of packages trying to avoid the necessity
       * of switching back to the current media later.
      */
      void cacheLastInteractive( const_iterator citem_r );

      /** cacheLastInteractive helper . */
      void doCacheLastInteractive( const_iterator citem_r );

    private:
      DefaultIntegral<unsigned,0> _dbgChanges;

      const_iterator       _commitListEnd;
      IMediaKey            _lastInteractive;

      Pathname                       _rootDir;
      shared_ptr<filesystem::TmpDir> _cacheDir;
      CacheMap                       _cacheMap;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_COMMITPACKAGECACHEREADAHEAD_H
