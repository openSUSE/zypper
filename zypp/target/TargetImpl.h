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
#include <list>
#include <set>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/DefaultFalseBool.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/PoolItem.h"
#include "zypp/ZYppCommit.h"

#include "zypp/Pathname.h"
#include "zypp/media/MediaAccess.h"
#include "zypp/Target.h"
#include "zypp/target/rpm/RpmDb.h"
#include "zypp/target/TargetException.h"
#include "zypp/target/RequestedLocalesFile.h"
#include "zypp/target/SoftLocksFile.h"
#include "zypp/target/HardLocksFile.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(TargetImpl);

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
      void load();

      void unload();

      void clearCache();

      void buildCache();
      //@}

      std::string anonymousUniqueId() const;

    public:

      /** The root set for this target */
      Pathname root() const
      { return _root; }

      /** The directory to store things. */
      Pathname home() const
      { return _root / "/var/lib/zypp"; }

      /** Commit changes in the pool */
      ZYppCommitResult commit( ResPool pool_r, const ZYppCommitPolicy & policy_r );

      ZYPP_DEPRECATED int commit( ResPool pool_r, unsigned int medianr,
                                  PoolItemList & errors_r,
                                  PoolItemList & remaining_r,
                                  PoolItemList & srcremaining_r,
                                  bool dry_run = false )
      {
        ZYppCommitPolicy policy;
        policy.restrictToMedia( medianr ).dryRun( dry_run );
        ZYppCommitResult res = commit( pool_r, policy );
        errors_r.swap( res._errors );
        remaining_r.swap( res._remaining );
        srcremaining_r.swap( res._srcremaining );
        return res._result;
      }

      /** Commit ordered changes
       *  @param pool_r only needed for #160792
       *  @return uncommitted ones (due to error)
       */
      PoolItemList commit( const PoolItemList & items_r, const ZYppCommitPolicy & policy_r, const ResPool & pool_r );

      /** Install a source package on the Target. */
      void installSrcPackage( const SrcPackage_constPtr & srcPackage_r );

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

      /** \copydoc Target::release() */
      std::string release() const;

      /** \copydoc Target::targetDistribution() */
      std::string targetDistribution() const;

      /** \copydoc Target::targetDistributionRelease()*/
      std::string targetDistributionRelease() const;

      /** \copydoc Target::distributionVersion()*/
      std::string distributionVersion() const;

      /** \copydoc Target::distributionFlavor() */
      std::string distributionFlavor() const;

    protected:
      /** Path to the target */
      Pathname _root;
      /** RPM database */
      rpm::RpmDb _rpm;
      /** Requested Locales database */
      RequestedLocalesFile _requestedLocalesFile;
      /** Soft-locks database */
      SoftLocksFile _softLocksFile;
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
