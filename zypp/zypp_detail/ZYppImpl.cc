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
#include "zypp/NVRAD.h"
#include "zypp/Language.h"
#include "zypp/DiskUsageCounter.h"
#include "zypp/NameKindProxy.h"
#include "zypp/Locks.h"

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
      MIL << "Initializing keyring..." << std::endl;
      //_keyring = new KeyRing(homePath() + Pathname("/keyring/all"), homePath() + Pathname("/keyring/trusted"));
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

    void ZYppImpl::initializeTarget(const Pathname & root)
    {
      MIL << "initTarget( " << root << endl;
      if (_target) {
	if (_target->root() == root) {
	    MIL << "Repeated call to initializeTarget()" << endl;
	    return;
	}
#warning NEED SOME NEW WAY TO INDICATE NEDD OF TARGET RELOAD
#if 0
	removeInstalledResolvables( );
#endif
      }
      _target = new Target( root );
    }

    void ZYppImpl::finishTarget()
    {
#warning NEED SOME NEW WAY TO UNLOAD THE POOL
#if 0
      if (_target)
	removeInstalledResolvables();
#endif
      _target = 0;
    }

    //------------------------------------------------------------------------
    // commit

    /** \todo Remove workflow from target, lot's of it could be done here,
     * and target used for transact. */
    ZYppCommitResult ZYppImpl::commit( const ZYppCommitPolicy & policy_r )
    {
      if ( getenv("ZYPP_TESTSUITE_FAKE_ARCH") )
      {
        ZYPP_THROW( Exception("ZYPP_TESTSUITE_FAKE_ARCH set. Commit not allowed and disabled.") );
      }

      MIL << "Attempt to commit (" << policy_r << ")" << endl;
      if (! _target)
	ZYPP_THROW( Exception("Target not initialized.") );

      ZYppCommitResult res = _target->_pimpl->commit( pool(), policy_r );

      if (! policy_r.dryRun() ) {
        // Tag target data invalid, so they are reloaded on the next call to
        // target->resolvables(). Actually the target should do this without
        // foreign help.
        _target->reset();
#warning NEED SOME NEW WAY TO INDICATE NEDD OF TARGET RELOAD
#if 0
	removeInstalledResolvables();
        if ( policy_r.syncPoolAfterCommit() )
          {
            // reload new status from target
            addResolvables( _target->resolvables(), true );
          }
#endif
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

    //------------------------------------------------------------------------
    // locales

    /** */
    void ZYppImpl::setRequestedLocales( const LocaleSet & locales_r )
    {
#warning REIMPLEMENT WITHOUT LANGUAGE RESOLVABLE
#if 0
     ResPool mpool( ResPool::instance() );
      // assert all requested are available
      for ( LocaleSet::const_iterator it = locales_r.begin();
            it != locales_r.end(); ++it )
        {
          NameKindProxy select( nameKindProxy<Language>( mpool, it->code() ) );
          if ( select.installedEmpty() && select.availableEmpty() )
            _pool.insert( Language::availableInstance( *it ) );
        }

      // now adjust status
      for ( ResPool::byKind_iterator it = mpool.byKindBegin<Language>();
            it != mpool.byKindEnd<Language>(); ++it )
        {
          NameKindProxy select( nameKindProxy<Language>( mpool, (*it)->name() ) );
          if ( locales_r.find( Locale( (*it)->name() ) ) != locales_r.end() )
            {
              // Language is requested
              if ( select.installedEmpty() )
                {
                  if ( select.availableEmpty() )
                    {
                      // no item ==> provide available to install
                      _pool.insert( Language::availableInstance( Locale((*it)->name()) ) );
                      select = nameKindProxy<Language>( mpool, (*it)->name() );
                    }
                  // available only ==> to install
                  select.availableBegin()->status().setTransactValue( ResStatus::TRANSACT, ResStatus::USER );
                }
              else
                {
                  // installed ==> keep it
                  select.installedBegin()->status().setTransactValue( ResStatus::KEEP_STATE, ResStatus::USER );
                  if ( ! select.availableEmpty() )
                    {
                      // both items ==> keep
                      select.availableBegin()->status().resetTransact( ResStatus::USER );
                    }
                }
            }
          else
            {
              // Language is NOT requested
              if ( ! select.installedEmpty() )
                select.installedBegin()->status().setTransactValue( ResStatus::TRANSACT, ResStatus::USER );
              if ( ! select.availableEmpty() )
                select.availableBegin()->status().resetTransact( ResStatus::USER );
            }
        }
#endif
    }

    /** */
    ZYppImpl::LocaleSet ZYppImpl::getAvailableLocales() const
    {
      ZYpp::LocaleSet ret;
#warning REIMPLEMENT WITHOUT LANGUAGE RESOLVABLE
#if 0
      ResPool mpool( ResPool::instance() );
      for ( ResPool::byKind_iterator it = mpool.byKindBegin<Language>();
            it != mpool.byKindEnd<Language>(); ++it )
        {
          if ( (*it).status().isUninstalled() ) // available!
            ret.insert( Locale( (*it)->name() ) );
        }
#endif
      return ret;
    }

    /** */
    ZYppImpl::LocaleSet ZYppImpl::getRequestedLocales() const
    {
      ZYpp::LocaleSet ret;
#warning REIMPLEMENT WITHOUT LANGUAGE RESOLVABLE
#if 0
     ResPool mpool( ResPool::instance() );
      for ( ResPool::byKind_iterator it = mpool.byKindBegin<Language>();
            it != mpool.byKindEnd<Language>(); ++it )
        {
          NameKindProxy select( nameKindProxy<Language>( mpool, (*it)->name() ) );
          if ( ! select.installedEmpty()
               && select.installedBegin()->status().getTransactValue() != ResStatus::TRANSACT )
            ret.insert( Locale( (*it)->name() ) );
          else if ( ! select.availableEmpty()
                    && select.availableBegin()->status().getTransactValue() == ResStatus::TRANSACT )
            ret.insert( Locale( (*it)->name() ) );
        }
#endif
      return ret;
    }

    void ZYppImpl::availableLocale( const Locale & locale_r )
    {
#warning REIMPLEMENT WITHOUT LANGUAGE RESOLVABLE
#if 0
      _pool.insert( Language::availableInstance( locale_r ) );
#endif
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

    int ZYppImpl::applyLocks()
    {
      Pathname locksrcPath( "/etc/zypp/locks" );
      try
      {
        Target_Ptr trg( target() );
        if ( trg )
          locksrcPath = trg->root() / locksrcPath;
      }
      catch ( ... )
      {
        // noop: Someone decided to let target() throw if the ptr is NULL ;(
      }

      int num=0;
      PathInfo locksrc( locksrcPath );
      if ( locksrc.isFile() )
      {
        MIL << "Reading locks from '" << locksrcPath << "'" << endl;
        num = zypp::locks::readLocks( pool(), locksrcPath );
        MIL << num << " items locked." << endl;
      }
      else
      {
        MIL << "No file '" << locksrcPath << "' to read locks from" << endl;
      }
      return num;
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
