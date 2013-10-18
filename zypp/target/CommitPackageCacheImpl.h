/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/CommitPackageCacheImpl.h
 *
*/
#ifndef ZYPP_TARGET_COMMITPACKAGECACHEIMPL_H
#define ZYPP_TARGET_COMMITPACKAGECACHEIMPL_H

#include <iosfwd>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/target/CommitPackageCache.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CommitPackageCache::Impl
    //
    /** Base for CommitPackageCache implementations (implements no chache).
     *
     * All packages are directly retrieved from the source via the
     * PackageProvider passed to the ctor. The PackageProvider is expected
     * to throw or return an empty ManagedFile if the package can't be provided.
    */
    class CommitPackageCache::Impl
    {
    public:
      typedef CommitPackageCache::PackageProvider  PackageProvider;

    public:
      Impl( const PackageProvider & packageProvider_r )
      : _packageProvider( packageProvider_r )
      {}

      virtual ~Impl()
      {}

    public:
      /** Provide the package.
       * Derived classes overload this.
      */
      virtual ManagedFile get( const PoolItem & citem_r )
      {
        return sourceProvidePackage( citem_r );
      }

      void setCommitList( std::vector<sat::Solvable> commitList_r )
      { _commitList = commitList_r; }

      const std::vector<sat::Solvable> & commitList() const
      { return _commitList; }

    protected:
      /** Let the Source provide the package. */
      virtual ManagedFile sourceProvidePackage( const PoolItem & pi ) const
      {
        if ( ! _packageProvider )
          {
            ZYPP_THROW( Exception("No package provider configured.") );
          }

        ManagedFile ret( _packageProvider( pi, /*cached only*/false ) );
        if ( ret.value().empty() )
          {
            ZYPP_THROW( Exception("Package provider failed.") );
          }

        return ret;
      }

      /** Let the Source provide an already cached package. */
      virtual ManagedFile sourceProvideCachedPackage( const PoolItem & pi ) const
      {
        if ( ! _packageProvider )
          {
            ZYPP_THROW( Exception("No package provider configured.") );
          }

        return _packageProvider( pi, /*cached only*/true );
      }

    private:
      std::vector<sat::Solvable> _commitList;
      PackageProvider _packageProvider;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates CommitPackageCache::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const CommitPackageCache::Impl & obj )
    {
      return str << "CommitPackageCache::Impl";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_COMMITPACKAGECACHEIMPL_H
