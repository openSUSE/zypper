/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/zypp_detail/ZYppImpl.cc
 *
*/

#include <iostream>
#include "zypp/TmpPath.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"

#include "zypp/zypp_detail/ZYppImpl.h"
#include "zypp/solver/detail/Helper.h"
#include "zypp/target/TargetImpl.h"
#include "zypp/ZYpp.h"
#include "zypp/DiskUsageCounter.h"
#include "zypp/ZConfig.h"
#include "zypp/sat/Pool.h"
#include "zypp/PoolItem.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace zypp_detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ZYppImpl::ZYppImpl
    //	METHOD TYPE : Constructor
    //
    ZYppImpl::ZYppImpl()
    : _target(0)
    , _resolver( new Resolver( ResPool::instance()) )
    {
      ZConfig::instance().about( MIL );
      MIL << "Initializing keyring..." << std::endl;
      _keyring = new KeyRing(tmpPath());
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ZYppImpl::~ZYppImpl
    //	METHOD TYPE : Destructor
    //
    ZYppImpl::~ZYppImpl()
    {}

    //------------------------------------------------------------------------
    // add/remove resolvables

    DiskUsageCounter::MountPointSet ZYppImpl::diskUsage()
    {
      if ( ! _disk_usage )
      {
        setPartitions( DiskUsageCounter::detectMountPoints() );
      }
      return _disk_usage->disk_usage(pool());
    }

    void ZYppImpl::setPartitions(const DiskUsageCounter::MountPointSet &mp)
    {
      _disk_usage.reset(new DiskUsageCounter());
      _disk_usage->setMountPoints(mp);
    }

    DiskUsageCounter::MountPointSet ZYppImpl::getPartitions() const
    {
      if (_disk_usage)
        return _disk_usage->getMountPoints();
      else
        return DiskUsageCounter::detectMountPoints();
    }

    //------------------------------------------------------------------------
    // target

    Target_Ptr ZYppImpl::target() const
    {
      if (! _target)
	ZYPP_THROW(Exception("Target not initialized."));
      return _target;
     }

    void ZYppImpl::initializeTarget( const Pathname & root, bool doRebuild_r )
    {
      MIL << "initTarget( " << root << (doRebuild_r?", rebuilddb":"") << ")" << endl;
      if (_target) {
          if (_target->root() == root) {
              MIL << "Repeated call to initializeTarget()" << endl;
              return;
          }

          _target->unload();

      }
      _target = new Target( root, doRebuild_r );
      _target->buildCache();
    }

    void ZYppImpl::finishTarget()
    {
      if (_target)
          _target->unload();

      _target = 0;
    }

    //------------------------------------------------------------------------
    // commit

    /** \todo Remove workflow from target, lot's of it could be done here,
     * and target used for transact. */
    ZYppCommitResult ZYppImpl::commit( const ZYppCommitPolicy & policy_r )
    {
      setenv( "ZYPP_IS_RUNNING", str::numstring(getpid()).c_str(), 1 );

      if ( getenv("ZYPP_TESTSUITE_FAKE_ARCH") )
      {
        ZYPP_THROW( Exception("ZYPP_TESTSUITE_FAKE_ARCH set. Commit not allowed and disabled.") );
      }

      MIL << "Attempt to commit (" << policy_r << ")" << endl;
      if (! _target)
	ZYPP_THROW( Exception("Target not initialized.") );

      ZYppCommitResult res = _target->_pimpl->commit( pool(), policy_r );

      if (! policy_r.dryRun() )
      {
        if ( policy_r.syncPoolAfterCommit() )
          {
            // reload new status from target
            DBG << "reloading " << sat::Pool::instance().systemRepoAlias() << " repo to pool" << endl;
            _target->load();
          }
        else
          {
            DBG << "unloading " << sat::Pool::instance().systemRepoAlias() << " repo from pool" << endl;
            _target->unload();
          }
      }

      MIL << "Commit (" << policy_r << ") returned: "
          << res << endl;
      return res;
    }

    void ZYppImpl::installSrcPackage( const SrcPackage_constPtr & srcPackage_r )
    {
      if (! _target)
        ZYPP_THROW( Exception("Target not initialized.") );
      _target->_pimpl->installSrcPackage( srcPackage_r );
    }

    ManagedFile ZYppImpl::provideSrcPackage( const SrcPackage_constPtr & srcPackage_r )
    {
      if (! _target)
        ZYPP_THROW( Exception("Target not initialized.") );
      return _target->_pimpl->provideSrcPackage( srcPackage_r );
    }

    //------------------------------------------------------------------------
    // target store path

    Pathname ZYppImpl::homePath() const
    { return _home_path.empty() ? Pathname("/var/lib/zypp") : _home_path; }

    void ZYppImpl::setHomePath( const Pathname & path )
    { _home_path = path; }

    Pathname ZYppImpl::tmpPath() const
    {
      static TmpDir zypp_tmp_dir( TmpPath::defaultLocation(), "zypp." );
      return zypp_tmp_dir.path();
    }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const ZYppImpl & obj )
    {
      return str << "ZYppImpl";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace zypp_detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
