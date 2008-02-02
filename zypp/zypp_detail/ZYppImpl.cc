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

    /** The locale to be used for texts and messages.
     *
     * For the encoding to be used the preference is
     *
     *    LC_ALL, LC_CTYPE, LANG
     *
     * For the language of the messages to be used, the preference is
     *
     *    LANGUAGE, LC_ALL, LC_MESSAGES, LANG
     *
     * Note that LANGUAGE can contain more than one locale name, it can be
     * a list of locale names like for example
     *
     *    LANGUAGE=ja_JP.UTF-8:de_DE.UTF-8:fr_FR.UTF-8

     * \todo Support dynamic fallbacklists defined by LANGUAGE
     */
    inline Locale defaultTextLocale()
    {
      Locale ret( "en" );
      const char * envlist[] = { "LC_ALL", "LC_MESSAGES", "LANG", NULL };
      for ( const char ** envvar = envlist; *envvar; ++envvar )
        {
	  const char * envlang = getenv( *envvar );
          if ( envlang )
            {
              std::string envstr( envlang );
              if ( envstr != "POSIX" && envstr != "C" )
                {
                  Locale lang( envlang );
                  if ( ! lang.code().empty() )
                    {
                      ret = lang;
                      break;
                    }
                }
            }
        }
      return ret;
    }

    Arch defaultArchitecture()
    {
      Arch architecture;

      // detect the true architecture
      struct utsname buf;
      if ( uname( &buf ) < 0 )
        {
          ERR << "Can't determine system architecture" << endl;
        }
      else
        {
          architecture = Arch( buf.machine );
          DBG << "uname architecture is '" << buf.machine << "'" << endl;

          // some CPUs report i686 but dont implement cx8 and cmov
          // check for both flags in /proc/cpuinfo and downgrade
          // to i586 if either is missing (cf bug #18885)

          if ( architecture == Arch_i686 )
            {
              std::ifstream cpuinfo( "/proc/cpuinfo" );
              if ( !cpuinfo )
                {
                  ERR << "Cant open /proc/cpuinfo" << endl;
                }
              else
                {
                  char infoline[1024];
                  while ( cpuinfo.good() )
                    {
                      if ( !cpuinfo.getline( infoline, 1024, '\n' ) )
                        {
                          if ( cpuinfo.eof() )
                            break;
                        }
                      if ( strncmp( infoline, "flags", 5 ) == 0 )
                        {
                          std::string flagsline( infoline );
                          if ( flagsline.find( "cx8" ) == std::string::npos
                               || flagsline.find( "cmov" ) == std::string::npos )
                            {
                              architecture = Arch_i586;
                              DBG << "CPU lacks 'cx8' or 'cmov': architecture downgraded to '" << architecture << "'" << endl;
                            }
                          break;
                        } // flags found
                    } // read proc/cpuinfo
                } // proc/cpuinfo opened
            } // i686 extra flags check
        }

      if ( getenv( "ZYPP_TESTSUITE_FAKE_ARCH" ) )
      {
        architecture = Arch( getenv( "ZYPP_TESTSUITE_FAKE_ARCH" ) );
        WAR << "ZYPP_TESTSUITE_FAKE_ARCH: Setting fake system architecture for test purpuses to: '" << architecture << "'" << endl;
      }

      return architecture;
    }
    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ZYppImpl::ZYppImpl
    //	METHOD TYPE : Constructor
    //
    ZYppImpl::ZYppImpl()
    : _textLocale( defaultTextLocale() )
    , _target(0)
    , _resolver( new Resolver( ResPool::instance()) )
    , _architecture( defaultArchitecture() )
    {
      MIL << "libzypp: " << VERSION << " built " << __DATE__ << " " <<  __TIME__ << endl;
      MIL << "defaultTextLocale: '" << _textLocale << "'" << endl;
      MIL << "System architecture is '" << _architecture << "'" << endl;

      MIL << "initializing keyring..." << std::endl;
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
    // architecture

    void ZYppImpl::setArchitecture( const Arch & arch )
    {
	_architecture = arch;
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
