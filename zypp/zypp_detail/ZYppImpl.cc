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

#include <sys/utsname.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"

#include "zypp/zypp_detail/ZYppImpl.h"
#include "zypp/detail/ResImplTraits.h"
#include "zypp/solver/detail/Helper.h"
#include "zypp/target/TargetImpl.h"
#include "zypp/ZYpp.h"
#include "zypp/NVRAD.h"
#include "zypp/Language.h"
#include "zypp/DiskUsageCounter.h"
#include "zypp/NameKindProxy.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace zypp_detail
  { /////////////////////////////////////////////////////////////////

    inline Locale defaultTextLocale()
    {
      Locale ret( "en" );
      char * envlist[] = { "LC_ALL", "LC_CTYPE", "LANG", NULL };
      for ( char ** envvar = envlist; *envvar; ++envvar )
        {
          char * envlang = getenv( *envvar );
          if ( envlang )
            {
              std::string envstr( envlang );
              if ( envstr != "POSIX" && envstr != "C" )
                {
                  Locale lang( envlang );
                  if ( lang != Locale::noCode )
                    {
                      ret = lang;
                      break;
                    }
                }
            }
        }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ZYppImpl::ZYppImpl
    //	METHOD TYPE : Constructor
    //
    ZYppImpl::ZYppImpl()
    : _textLocale( defaultTextLocale() )
    , _pool()
    , _sourceFeed( _pool )
    , _target(0)
    , _resolver( new Resolver(_pool.accessor()) )
    , _disk_usage()
    {
      MIL << "defaultTextLocale: '" << _textLocale << "'" << endl;
      
      MIL << "initializing keyring..." << std::endl;
      //_keyring = new KeyRing(homePath() + Pathname("/keyring/all"), homePath() + Pathname("/keyring/trusted"));
      _keyring = new KeyRing(tmpPath());

      // detect the true architecture
      struct utsname buf;
      if (uname (&buf) < 0) {
        ERR << "Can't determine system architecture" << endl;
      }
      else {
        _architecture = Arch( buf.machine );

        MIL << "uname architecture is '" << buf.machine << "'" << endl;

        // some CPUs report i686 but dont implement cx8 and cmov
        // check for both flags in /proc/cpuinfo and downgrade
        // to i586 if either is missing (cf bug #18885)

        if (_architecture == Arch_i686)
        {
            std::ifstream cpuinfo ("/proc/cpuinfo");
            if (!cpuinfo)
            {
                ERR << "Cant open /proc/cpuinfo" << endl;
            }
            else
            {
                char infoline[1024];
                while (cpuinfo.good())
                {
                    if (!cpuinfo.getline (infoline, 1024, '\n'))
                    {
                        if (cpuinfo.eof())
                            break;
                    }
                    if (strncmp (infoline, "flags", 5) == 0)
                    {
                        std::string flagsline (infoline);
                        if ((flagsline.find( "cx8" ) == std::string::npos)
                            || (flagsline.find( "cmov" ) == std::string::npos))
                        {
                            _architecture = Arch_i586;
                        }
                        break;
                    } // flags found
                } // read proc/cpuinfo
            } // proc/cpuinfo opened
        } // i686 extra flags check

        MIL << "System architecture is '" << _architecture << "'" << endl;
      }
    

      
      if ( getenv("ZYPP_TESTSUITE_FAKE_ARCH") ) 
      {
        Arch already_set = _architecture;
        
        std::string fakearch(getenv("ZYPP_TESTSUITE_FAKE_ARCH"));
        try
        { 
          _architecture = Arch( fakearch );
          MIL << "ZYPP_TESTSUITE_FAKE_ARCH: Setting fake system architecture for test purpuses (warning! commit will be disabled!) to: '" << _architecture << "'" << endl;
        }
        catch(...)
        {
          ERR << "ZYPP_TESTSUITE_FAKE_ARCH: Wrong architecture specified on env variable, using: '" << _architecture << "'" << endl;
        }
      }
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

    void ZYppImpl::addResolvables (const ResStore& store, bool installed)
    {
	_pool.insert(store.begin(), store.end(), installed);
    }

    void ZYppImpl::removeResolvables (const ResStore& store)
    {
        for (ResStore::iterator it = store.begin(); it != store.end(); ++it)
	{
    	    _pool.erase(*it);
	}
    }

    void ZYppImpl::removeInstalledResolvables ()
    {
        for (ResPool::const_iterator it = pool().begin(); it != pool().end();)
	{
	    ResPool::const_iterator next = it; ++next;
	    if (it->status().isInstalled())
		_pool.erase( *it );
	    it = next;
	}
    }

    DiskUsageCounter::MountPointSet ZYppImpl::diskUsage()
    { return _disk_usage.disk_usage(pool()); }

    void ZYppImpl::setPartitions(const DiskUsageCounter::MountPointSet &mp)
    { _disk_usage.setMountPoints(mp); }

    //------------------------------------------------------------------------
    // target

    Target_Ptr ZYppImpl::target() const
    {
      if (! _target)
	ZYPP_THROW(Exception("Target not initialized."));
      return _target;
     }

    void ZYppImpl::initTarget(const Pathname & root, bool commit_only)
    {
      MIL << "initTarget( " << root << ", " << commit_only << ")" << endl;
      if (_target) {
	if (_target->root() == root) {
	    MIL << "Repeated call to initTarget()" << endl;
	    return;
	}
	removeInstalledResolvables( );
      }
      _target = new Target( root );
      if (!commit_only)
      {
	_target->enableStorage( root );
	addResolvables( _target->resolvables(), true );
      }
    }

    void ZYppImpl::finishTarget()
    {
      if (_target)
	removeInstalledResolvables();
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
	// reload new status from target
	removeInstalledResolvables();
	addResolvables( _target->resolvables(), true );
      }

      MIL << "Commit (" << policy_r << ") returned: "
          << res << endl;
      return res;
    }


    //------------------------------------------------------------------------
    // locales

    /** */
    void ZYppImpl::setRequestedLocales( const LocaleSet & locales_r )
    {
      ResPool mpool( pool() );
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
    }

    /** */
    ZYppImpl::LocaleSet ZYppImpl::getAvailableLocales() const
    {
      ZYpp::LocaleSet ret;
      ResPool mpool( pool() );
      for ( ResPool::byKind_iterator it = mpool.byKindBegin<Language>();
            it != mpool.byKindEnd<Language>(); ++it )
        {
          if ( (*it).status().isUninstalled() ) // available!
            ret.insert( Locale( (*it)->name() ) );
        }
      return ret;
    }

    /** */
    ZYppImpl::LocaleSet ZYppImpl::getRequestedLocales() const
    {
      ZYpp::LocaleSet ret;
      ResPool mpool( pool() );
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
      return ret;
    }

    void ZYppImpl::availableLocale( const Locale & locale_r )
    {
      _pool.insert( Language::availableInstance( locale_r ) );
    }

    //------------------------------------------------------------------------
    // architecture

    void ZYppImpl::setArchitecture( const Arch & arch )
    {
	_architecture = arch;
	if (_resolver) _resolver->setArchitecture( arch );
    }

    //------------------------------------------------------------------------
    // target store path

    Pathname ZYppImpl::homePath() const
    { return _home_path.empty() ? Pathname("/var/lib/zypp") : _home_path; }

    void ZYppImpl::setHomePath( const Pathname & path )
    { _home_path = path; }

    Pathname ZYppImpl::tmpPath() const
    { 
      static TmpDir zypp_tmp_dir("/var/tmp", "zypp.");
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
