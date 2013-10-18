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
                          const PackageProvider & packageProvider_r );

      /** Dtor */
      ~CommitPackageCache();

    public:
      /** Download(commit) sequence of solvables to compute read ahead. */
      void setCommitList( std::vector<sat::Solvable> commitList_r );
      /** \overload */
      template <class _Iterator>
      void setCommitList( _Iterator begin_r, _Iterator end_r )
      { setCommitList( std::vector<sat::Solvable>( begin_r, end_r  ) ); }

      /** Provide a package. */
      ManagedFile get( const PoolItem & citem_r );
      /** \overload */
      ManagedFile get( sat::Solvable citem_r )
      { return get( PoolItem(citem_r) ); }

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
