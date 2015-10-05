/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/CommitPackageCache.h
 *
*/
#ifndef ZYPP_TARGET_COMMITPACKAGECACHE_H
#define ZYPP_TARGET_COMMITPACKAGECACHE_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Function.h"

#include "zypp/PoolItem.h"
#include "zypp/Pathname.h"
#include "zypp/ManagedFile.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /// \class RepoProvidePackage
    /// \short Default PackageProvider for \ref CommitPackageCache
    ///
    /// \p pool_r \ref ResPool used to get candidates
    /// \p pi item to be commited
    ///////////////////////////////////////////////////////////////////
    class RepoProvidePackage
    {
    public:
      RepoProvidePackage();
      ~RepoProvidePackage();

      /** Provide package optionally fron cache only. */
      ManagedFile operator()( const PoolItem & pi, bool fromCache_r );

    private:
      struct Impl;
      RW_pointer<Impl> _impl;
    };

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CommitPackageCache
    //
    /** Target::commit helper optimizing package provision.
    */
    class CommitPackageCache
    {
      friend std::ostream & operator<<( std::ostream & str, const CommitPackageCache & obj );

    public:
      typedef function<ManagedFile( const PoolItem & pi, bool fromCache_r )> PackageProvider;

    public:
      /** Ctor */
      CommitPackageCache( const Pathname &        rootDir_r,
                          const PackageProvider & packageProvider_r = RepoProvidePackage() );

      /** Dtor */
      ~CommitPackageCache();

    public:
      /** Download(commit) sequence of solvables to compute read ahead. */
      void setCommitList( std::vector<sat::Solvable> commitList_r );
      /** \overload */
      template <class TIterator>
      void setCommitList( TIterator begin_r, TIterator end_r )
      { setCommitList( std::vector<sat::Solvable>( begin_r, end_r  ) ); }

      /** Provide a package. */
      ManagedFile get( const PoolItem & citem_r );
      /** \overload */
      ManagedFile get( sat::Solvable citem_r )
      { return get( PoolItem(citem_r) ); }

      /** Whether preloaded hint is set.
       * If preloaded the cache tries to avoid trigering the infoInCache CB,
       * based on the assumption this was already done when preloading the cache.
       */
      bool preloaded() const;
      /** Set preloaded hint. */
      void preloaded( bool newval_r );

    public:
      /** Implementation. */
      class Impl;
      /** Ctor taking an implementation. */
      explicit CommitPackageCache( Impl * pimpl_r );
    private:
      /** Pointer to implementation. */
      RW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates CommitPackageCache Stream output */
    std::ostream & operator<<( std::ostream & str, const CommitPackageCache & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_COMMITPACKAGECACHE_H
