/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/TargetImpl.h
 *
*/
#ifndef ZYPP_TARGET_TARGETIMPL_H
#define ZYPP_TARGET_TARGETIMPL_H

#include <iosfwd>
#include <set>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/PoolItem.h"
#include "zypp/ZYppCommit.h"

#include "zypp/Pathname.h"
#include "zypp/media/MediaAccess.h"
#include "zypp/Target.h"
#include "zypp/target/rpm/RpmDb.h"
#include "zypp/target/TargetException.h"
#include "zypp/target/RequestedLocalesFile.h"
#include "zypp/target/HardLocksFile.h"
#include "zypp/ManagedFile.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(TargetImpl);
    class CommitPackageCache;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : TargetImpl
    //
    /** Base class for concrete Target implementations.
     *
     * Constructed by \ref TargetFactory. Public access via \ref Target
     * interface.
    */
    class TargetImpl : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const TargetImpl & obj );

    public:
      /** list of pool items  */
      typedef std::list<PoolItem> PoolItemList;

      /** set of pool items  */
      typedef std::set<PoolItem> PoolItemSet;

    public:
      /** Ctor. */
      TargetImpl(const Pathname & root_r = "/", bool doRebuild_r = false );
      /** Dtor. */
      virtual ~TargetImpl();

      /** Null implementation */
      static TargetImpl_Ptr nullimpl();

      /**
       * generates the unique anonymous id which is called
       * when creating the target
       */
      void createAnonymousId() const;

      /**
       * generates a cache of the last product flavor
       */
      void createLastDistributionFlavorCache() const;

      /** \name Solv file handling.
       * If target solv file is outdated, but (non-root-)user has
       * no permission to  create it at the default location, we
       * use a temporary one.
       */
      //@{
    private:
      /** The systems default solv file location. */
      Pathname defaultSolvfilesPath() const;

      /** The solv file location actually in use (default or temp). */
      Pathname solvfilesPath() const
      { return solvfilesPathIsTemp() ? _tmpSolvfilesPath : defaultSolvfilesPath(); }

      /** Whether we're using a temp. solvfile. */
      bool solvfilesPathIsTemp() const
      { return ! _tmpSolvfilesPath.empty(); }

      Pathname _tmpSolvfilesPath;

    public:
      void load( bool force = true );

      void unload();

      void reload();

      void clearCache();

      bool buildCache();
      //@}

    public:

      /** The root set for this target */
      Pathname root() const
      { return _root; }

      /** The directory to store things. */
      Pathname home() const
      { return home( _root ); }

      static Pathname home( const Pathname & root_r )
      { return root_r / "/var/lib/zypp"; }

      /** Commit changes in the pool */
      ZYppCommitResult commit( ResPool pool_r, const ZYppCommitPolicy & policy_r );

      /** Install a source package on the Target. */
      void installSrcPackage( const SrcPackage_constPtr & srcPackage_r );

      /** Provides a source package on the Target. */
      ManagedFile provideSrcPackage( const SrcPackage_constPtr & srcPackage_r );

      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const
      {
        return str << "TargetImpl";
      }

      /** The RPM database */
      rpm::RpmDb & rpm();

      /** If the package is installed and provides the file
      Needed to evaluate split provides during Resolver::Upgrade() */
      bool providesFile (const std::string & path_str, const std::string & name_str) const;

      /** Return name of package owning \a path_str
       * or empty string if no installed package owns \a path_str. */
      std::string whoOwnsFile (const std::string & path_str) const
      { return _rpm.whoOwnsFile (path_str); }

      /** return the last modification date of the target */
      Date timestamp() const;

      /** \copydoc Target::baseProduct() */
      Product::constPtr baseProduct() const;

      /** \copydoc Target::requestedLocales() */
      LocaleSet requestedLocales() const
      { return _requestedLocalesFile.locales(); }
      /** \overload */
      static LocaleSet requestedLocales( const Pathname & root_r );

      /** \copydoc Target::targetDistribution() */
      std::string targetDistribution() const;
      /** \overload */
      static std::string targetDistribution( const Pathname & root_r );

      /** \copydoc Target::targetDistributionRelease()*/
      std::string targetDistributionRelease() const;
      /** \overload */
      static std::string targetDistributionRelease( const Pathname & root_r );

      /** \copydoc Target::distributionVersion()*/
      Target::DistributionLabel distributionLabel() const;
      /** \overload */
      static Target::DistributionLabel distributionLabel( const Pathname & root_r );

      /** \copydoc Target::distributionVersion()*/
      std::string distributionVersion() const;
      /** \overload */
      static std::string distributionVersion( const Pathname & root_r );

      /** \copydoc Target::distributionFlavor() */
      std::string distributionFlavor() const;
      /** \overload */
      static std::string distributionFlavor( const Pathname & root_r );

      /** \copydoc Target::anonymousUniqueId() */
      std::string anonymousUniqueId() const;
      /** \overload */
      static std::string anonymousUniqueId( const Pathname & root_r );

    private:
      /** Commit ordered changes (internal helper) */
      void commit( const ZYppCommitPolicy & policy_r,
		   CommitPackageCache & packageCache_r,
		   ZYppCommitResult & result_r );

      /** Commit helper checking for file conflicts after download. */
      void commitFindFileConflicts( const ZYppCommitPolicy & policy_r, ZYppCommitResult & result_r );
    protected:
      /** Path to the target */
      Pathname _root;
      /** RPM database */
      rpm::RpmDb _rpm;
      /** Requested Locales database */
      RequestedLocalesFile _requestedLocalesFile;
      /** Hard-Locks database */
      HardLocksFile _hardLocksFile;
      /** Cache distributionVersion */
      mutable std::string _distributionVersion;

    private:
      /** Null implementation */
      static TargetImpl_Ptr _nullimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates TargetImpl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const TargetImpl & obj )
    {
      return obj.dumpOn( str );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_TARGETIMPL_H
