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
#include <iostream>
#include <fstream>
//#include "zypp/base/Logger.h"

#include "zypp/zypp_detail/ZYppImpl.h"
#include "zypp/detail/LanguageImpl.h"
#include "zypp/detail/ResImplTraits.h"
#include "zypp/solver/detail/Helper.h"
#include "zypp/NVRAD.h"
#include "zypp/Language.h"
#include "zypp/DiskUsageCounter.h"

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
    , _resolver( new Resolver(_pool.accessor()) )
    , _disk_usage()
    , _target(0)
    {
      MIL << "defaultTextLocale: '" << _textLocale << "'" << endl;

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

    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ZYppImpl::~ZYppImpl
    //	METHOD TYPE : Destructor
    //
    ZYppImpl::~ZYppImpl()
    {
    }
    
    void ZYppImpl::reset()
    {
	// TODO: check the memory is released
	_textLocale = defaultTextLocale();
	_pool = ResPoolManager();
	_sourceFeed = SourceFeed_Ref(_pool);
	_resolver = new Resolver(_pool.accessor());
	_disk_usage = DiskUsageCounter();
	_target = 0;
    }

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
    ZYpp::CommitResult ZYppImpl::commit( int medianr_r )
    {
      MIL << "Attempt to commit (medianr " << medianr_r << ")" << endl;
      if (! _target)
	ZYPP_THROW( Exception("Target not initialized.") );

      ZYpp::CommitResult res;

      // must redirect to Target::Impl. This kind of commit should not be
      // in the Target interface.

      res._result = _target->commit( pool(), medianr_r,
                                     res._errors, res._remaining, res._srcremaining );

      // reload new status from target

      removeInstalledResolvables();
      addResolvables( _target->resolvables(), true );

      MIL << "Commit (medianr " << medianr_r << ") returned: "
          << res._result
          << " (errors " << res._errors.size()
          << ", remaining " << res._remaining.size()
          << ", srcremaining " << res._srcremaining.size()
          << ")" << endl;

      return res;
    }


    //------------------------------------------------------------------------
    // locales

    /** */
    void ZYppImpl::setRequestedLocales( const LocaleSet & locales_r )
    {
	// check if each requested is also possible.

	LocaleSet possible = getPossibleLocales();
	bool changed = false;
	for (LocaleSet::const_iterator it = locales_r.begin(); it != locales_r.end(); ++it) {
	    changed = possible.insert( *it ).second;
	    if ( (it->code() != it->language().code()) ) {
		changed = possible.insert( Locale( it->language().code() ) ).second;
	    }
	}

	// oops, some requested are not possbile, make them possible
	//  this will actually generate 'uninstalled' language items we need below

	if (changed) {
	    setPossibleLocales( possible );
	}
	
	// now select the requested items for selection

	for (LocaleSet::const_iterator it = locales_r.begin(); it != locales_r.end(); ++it) {
	    MIL << "Requested locale '" << *it << "'" << endl;

// remove unwanted ?	    PoolItem installed( Helper::findInstalledByNameAndKind( _pool.accessor(), it->code(), ResTraits<Language>::kind ) );
	    PoolItem uninstalled( solver::detail::Helper::findUninstalledByNameAndKind( _pool.accessor(), it->code(), ResTraits<Language>::kind ) );
	    if (uninstalled) {
		if (!uninstalled.status().isLocked()) {
		    uninstalled.status().setTransact( true, ResStatus::USER );
		}
	    }

	    // if lang_country is given, also enable lang (i.e. if "de_DE" is given, also enable "de")
	    if ( (it->code() != it->language().code()) ) {
		MIL << "Auto requesting locale '" << it->language().code() << "'" << endl;
		uninstalled = solver::detail::Helper::findUninstalledByNameAndKind( _pool.accessor(), it->language().code(), ResTraits<Language>::kind );
		if (uninstalled) {
		    if (!uninstalled.status().isLocked()) {
			uninstalled.status().setTransact( true, ResStatus::USER );
		    }
		}
	    }
	}

	_requested_locales = locales_r;

    }

    /** */
    void ZYppImpl::setPossibleLocales( const LocaleSet & locales_r )
    {
	removeResolvables( _possible_locales );
	_possible_locales.clear();

	for (LocaleSet::const_iterator it = locales_r.begin(); it != locales_r.end(); ++it) {
	    NVRA nvra( it->code(), Edition(), Arch_noarch );
	    NVRAD ldata( nvra, Dependencies() );
	    detail::ResImplTraits<detail::LanguageImpl>::Ptr limpl = new detail::LanguageImpl();
	    Language::Ptr language = detail::makeResolvableFromImpl( ldata, limpl );
	    _possible_locales.insert( language );
	}
	addResolvables( _possible_locales, false );
    }

    /** */
    ZYppImpl::LocaleSet ZYppImpl::getPossibleLocales() const
    {
	LocaleSet lset;
	for (ResStore::const_iterator it = _possible_locales.begin(); it != _possible_locales.end(); ++it) {
	    lset.insert( Locale( (*it)->name() ) );
	}
	return lset;
    }

    /** */
    ZYppImpl::LocaleSet ZYppImpl::getAvailableLocales() const
    {
	return _available_locales;
    }

    void ZYppImpl::availableLocale( const Locale & locale_r )
    {
	_available_locales.insert( locale_r );
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
