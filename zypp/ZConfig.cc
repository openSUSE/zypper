/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZConfig.cc
 *
*/
extern "C"
{
#include <sys/utsname.h>
#include <unistd.h>
}
#include <iostream>
#include <fstream>
#include "zypp/base/Logger.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/String.h"

#include "zypp/ZConfig.h"
#include "zypp/ZYppFactory.h"
#include "zypp/PathInfo.h"
#include "zypp/parser/IniDict.h"

using namespace std;
using namespace zypp::filesystem;
using namespace zypp::parser;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    /** Determine system architecture evaluating \c uname and \c /proc/cpuinfo.
    */
    Arch _autodetectSystemArchitecture()
    {
      struct ::utsname buf;
      if ( ::uname( &buf ) < 0 )
      {
        ERR << "Can't determine system architecture" << endl;
        return Arch_noarch;
      }

      Arch architecture( buf.machine );
      MIL << "Uname architecture is '" << buf.machine << "'" << endl;

      // some CPUs report i686 but dont implement cx8 and cmov
      // check for both flags in /proc/cpuinfo and downgrade
      // to i586 if either is missing (cf bug #18885)
      if ( architecture == Arch_i686 )
      {
        std::ifstream cpuinfo( "/proc/cpuinfo" );
        if ( cpuinfo )
        {
          for( iostr::EachLine in( cpuinfo ); in; in.next() )
          {
            if ( str::hasPrefix( *in, "flags" ) )
            {
              if (    in->find( "cx8" ) == std::string::npos
                   || in->find( "cmov" ) == std::string::npos )
              {
                architecture = Arch_i586;
                WAR << "CPU lacks 'cx8' or 'cmov': architecture downgraded to '" << architecture << "'" << endl;
              }
              break;
            }
          }
        }
        else
        {
          ERR << "Cant open " << PathInfo("/proc/cpuinfo") << endl;
        }
      }
      return architecture;
    }

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
    Locale _autodetectTextLocale()
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
            Locale lang( envstr );
            if ( ! lang.code().empty() )
            {
              MIL << "Found " << *envvar << "=" << envstr << endl;
              ret = lang;
              break;
            }
          }
        }
      }
      MIL << "Default text locale is '" << ret << "'" << endl;
      return ret;
    }

   /////////////////////////////////////////////////////////////////
  } // namespace zypp
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZConfig::Impl
  //
  /** ZConfig implementation.
   * \todo Enrich section and entry definition by some comment
   * (including the default setting and provide some method to
   * write this into a sample zypp.conf.
  */
  class ZConfig::Impl
  {
    public:
      Impl( const Pathname & override_r = Pathname() )
        : cfg_arch                	( defaultSystemArchitecture() )
        , cfg_textLocale          	( defaultTextLocale() )
        , repo_add_probe          	( false )
        , repo_refresh_delay      	( 10 )
        , download_use_deltarpm   	( true )
	, solver_onlyRequires   	( false )
        , apply_locks_file              ( true )

      {
        MIL << "libzypp: " << VERSION << " built " << __DATE__ << " " <<  __TIME__ << endl;

	// override_r has higest prio
        // ZYPP_CONF might override /etc/zypp/zypp.conf
        Pathname confpath( override_r );
        if ( confpath.empty() )
        {
          const char *env_confpath = getenv( "ZYPP_CONF" );
          confpath = env_confpath ? env_confpath : "/etc/zypp/zypp.conf";
        }
        else
        {
          // Inject this into ZConfig. Be shure this is
          // allocated via new. See: reconfigureZConfig
          INT << "Reconfigure to " << confpath << endl;
          ZConfig::instance()._pimpl.reset( this );
        }
        if ( PathInfo(confpath).isExist() )
        {
          parser::IniDict dict( confpath );
          //InputStream is(confpath);

          for ( IniDict::section_const_iterator sit = dict.sectionsBegin();
                sit != dict.sectionsEnd();
                ++sit )
          {
            string section(*sit);
            //MIL << section << endl;
            for ( IniDict::entry_const_iterator it = dict.entriesBegin(*sit);
                  it != dict.entriesEnd(*sit);
                  ++it )
            {
              string entry(it->first);
              string value(it->second);
              //DBG << (*it).first << "=" << (*it).second << endl;
              if ( section == "main" )
              {
                if ( entry == "arch" )
                {
                  Arch carch( value );
                  if ( carch != cfg_arch )
                  {
                    WAR << "Overriding system architecture (" << cfg_arch << "): " << carch << endl;
                    cfg_arch = carch;
                  }
                }
                else if ( entry == "cachedir" )
                {
                  cfg_cache_path = Pathname(value);
                }
                else if ( entry == "metadatadir" )
                {
                  cfg_metadata_path = Pathname(value);
                }
                else if ( entry == "solvfilesdir" )
                {
                  cfg_solvfiles_path = Pathname(value);
                }
                else if ( entry == "packagesdir" )
                {
                  cfg_packages_path = Pathname(value);
                }
                else if ( entry == "configdir" )
                {
                  cfg_config_path = Pathname(value);
                }
                else if ( entry == "reposdir" )
                {
                  cfg_known_repos_path = Pathname(value);
                }
                else if ( entry == "repo.add.probe" )
                {
                  repo_add_probe = str::strToBool( value, repo_add_probe );
                }
                else if ( entry == "repo.refresh.delay" )
                {
                  str::strtonum(value, repo_refresh_delay);
                }
                else if ( entry == "download.use_deltarpm" )
                {
                  download_use_deltarpm = str::strToBool( value, download_use_deltarpm );
                }
                else if ( entry == "vendordir" )
                {
                  cfg_vendor_path = Pathname(value);
                }
                else if ( entry == "productsdir" )
                {
                  cfg_products_path = Pathname(value);
                }
                else if ( entry == "solver.onlyRequires" )
                {
                  solver_onlyRequires = str::strToBool( value, solver_onlyRequires );
                }
                else if ( entry == "solver.checkSystemFile" )
                {
                  solver_checkSystemFile = Pathname(value);
                }
                else if ( entry == "multiversion" )
                {
		  std::list<std::string> multi;
                  str::split( value, back_inserter(multi), ", \t" );
		  for ( std::list<string>::const_iterator it = multi.begin();
			it != multi.end(); it++) {
		      multiversion.insert (IdString(*it));
		  }
                }
                else if ( entry == "locksfile.path" )
                {
                  locks_file = Pathname(value);
                }
                else if ( entry == "locksfile.apply" )
                {
                  apply_locks_file = str::strToBool( value, apply_locks_file );
                }
                else if ( entry == "update.datadir" )
                {
                  update_data_path = Pathname(value);
                }
                else if ( entry == "update.scriptsdir" )
                {
                  update_scripts_path = Pathname(value);
                }
                else if ( entry == "update.messagessdir" )
                {
                  update_messages_path = Pathname(value);
                }
                else if ( entry == "rpm.install.excludedocs" )
                {
                  rpmInstallFlags.setFlag( target::rpm::RPMINST_EXCLUDEDOCS );
                }
                else if ( entry == "history.logfile" )
                {
                  history_log_path = Pathname(value);
                }
              }
            }
          }
        }
        else
        {
          MIL << confpath << " not found, using defaults instead." << endl;
        }

        // legacy:
        if ( getenv( "ZYPP_TESTSUITE_FAKE_ARCH" ) )
        {
          Arch carch( getenv( "ZYPP_TESTSUITE_FAKE_ARCH" ) );
          if ( carch != cfg_arch )
          {
            WAR << "ZYPP_TESTSUITE_FAKE_ARCH: Overriding system architecture (" << cfg_arch << "): " << carch << endl;
            cfg_arch = carch;
          }
        }

        MIL << "ZConfig singleton created." << endl;
        MIL << "defaultTextLocale: '" << cfg_textLocale << "'" << endl;
        MIL << "System architecture is '" << cfg_arch << "'" << endl;
      }

      ~Impl()
      {}

    public:
    Arch     cfg_arch;
    Locale   cfg_textLocale;

    Pathname cfg_cache_path;
    Pathname cfg_metadata_path;
    Pathname cfg_solvfiles_path;
    Pathname cfg_packages_path;

    Pathname cfg_config_path;
    Pathname cfg_known_repos_path;
    Pathname cfg_known_services_path;
    Pathname cfg_vendor_path;
    Pathname cfg_products_path;
    Pathname locks_file;

    Pathname update_data_path;
    Pathname update_scripts_path;
    Pathname update_messages_path;

    bool repo_add_probe;
    unsigned repo_refresh_delay;

    bool download_use_deltarpm;

    bool solver_onlyRequires;
    Pathname solver_checkSystemFile;

    std::set<IdString> multiversion;

    bool apply_locks_file;

    target::rpm::RpmInstFlags rpmInstallFlags;
    
    Pathname history_log_path;
  };
  ///////////////////////////////////////////////////////////////////

  // Backdoor to redirect ZConfig from within the running
  // TEST-application. HANDLE WITH CARE!
  void reconfigureZConfig( const Pathname & override_r )
  {
    // ctor puts itself unter smart pointer control.
    new ZConfig::Impl( override_r );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZConfig::instance
  //	METHOD TYPE : ZConfig &
  //
  ZConfig & ZConfig::instance()
  {
    static ZConfig _instance; // The singleton
    return _instance;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZConfig::ZConfig
  //	METHOD TYPE : Ctor
  //
  ZConfig::ZConfig()
  : _pimpl( new Impl )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZConfig::~ZConfig
  //	METHOD TYPE : Dtor
  //
  ZConfig::~ZConfig( )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  // system architecture
  //
  ///////////////////////////////////////////////////////////////////

  Arch ZConfig::defaultSystemArchitecture()
  {
    static Arch _val( _autodetectSystemArchitecture() );
    return _val;
  }

  Arch ZConfig::systemArchitecture() const
  { return _pimpl->cfg_arch; }

  void ZConfig::setSystemArchitecture( const Arch & arch_r )
  {
    if ( arch_r != _pimpl->cfg_arch )
    {
      WAR << "Overriding system architecture (" << _pimpl->cfg_arch << "): " << arch_r << endl;
      _pimpl->cfg_arch = arch_r;
    }
  }

  ///////////////////////////////////////////////////////////////////
  //
  // text locale
  //
  ///////////////////////////////////////////////////////////////////

  Locale ZConfig::defaultTextLocale()
  {
    static Locale _val( _autodetectTextLocale() );
    return _val;
  }

  Locale ZConfig::textLocale() const
  { return _pimpl->cfg_textLocale; }

  void ZConfig::setTextLocale( const Locale & locale_r )
  {
    if ( locale_r != _pimpl->cfg_textLocale )
    {
      WAR << "Overriding text locale (" << _pimpl->cfg_textLocale << "): " << locale_r << endl;
      _pimpl->cfg_textLocale = locale_r;
    }
  }

  ///////////////////////////////////////////////////////////////////

  Pathname ZConfig::repoCachePath() const
  {
    return ( _pimpl->cfg_cache_path.empty()
        ? Pathname("/var/cache/zypp") : _pimpl->cfg_cache_path );
  }

  Pathname ZConfig::repoMetadataPath() const
  {
    return ( _pimpl->cfg_metadata_path.empty()
        ? (repoCachePath()/"raw") : _pimpl->cfg_metadata_path );
  }

  Pathname ZConfig::repoSolvfilesPath() const
  {
    return ( _pimpl->cfg_solvfiles_path.empty()
        ? (repoCachePath()/"solv") : _pimpl->cfg_solvfiles_path );
  }

  Pathname ZConfig::repoPackagesPath() const
  {
    return ( _pimpl->cfg_packages_path.empty()
        ? (repoCachePath()/"packages") : _pimpl->cfg_packages_path );
  }

  ///////////////////////////////////////////////////////////////////

  Pathname ZConfig::configPath() const
  {
    return ( _pimpl->cfg_config_path.empty()
        ? Pathname("/etc/zypp") : _pimpl->cfg_config_path );
  }

  Pathname ZConfig::knownReposPath() const
  {
    return ( _pimpl->cfg_known_repos_path.empty()
        ? (configPath()/"repos.d") : _pimpl->cfg_known_repos_path );
  }

  Pathname ZConfig::knownServicesPath() const
  {
    return ( _pimpl->cfg_known_services_path.empty()
        ? (configPath()/"services.d") : _pimpl->cfg_known_repos_path );
  }

  Pathname ZConfig::vendorPath() const
  {
    return ( _pimpl->cfg_vendor_path.empty()
        ? (configPath()/"vendors.d") : _pimpl->cfg_vendor_path );
  }

  Pathname ZConfig::productsPath() const
  {
    return ( _pimpl->cfg_products_path.empty()
        ? (configPath()/"products.d") : _pimpl->cfg_products_path );
  }

  Pathname ZConfig::locksFile() const
  {
    return ( _pimpl->locks_file.empty()
        ? (configPath()/"locks") : _pimpl->locks_file );
  }

  ///////////////////////////////////////////////////////////////////

  bool ZConfig::repo_add_probe() const
  {
    return _pimpl->repo_add_probe;
  }

  unsigned ZConfig::repo_refresh_delay() const
  {
    return _pimpl->repo_refresh_delay;
  }

  bool ZConfig::download_use_deltarpm() const
  { return _pimpl->download_use_deltarpm; }


  bool ZConfig::solver_onlyRequires() const
  { return _pimpl->solver_onlyRequires; }

  Pathname ZConfig::solver_checkSystemFile() const
  { return _pimpl->solver_checkSystemFile; }


  std::set<IdString> ZConfig::multiversion() const
  { return _pimpl->multiversion; }

  void ZConfig::addMultiversion(std::string &name)
  { _pimpl->multiversion.insert(IdString(name)); }

  bool ZConfig::removeMultiversion(std::string &name)
  { return _pimpl->multiversion.erase(IdString(name)); }

  bool ZConfig::apply_locks_file() const
  {
    return _pimpl->apply_locks_file;
  }

  Pathname ZConfig::update_dataPath() const
  {
    return ( _pimpl->update_data_path.empty()
        ? Pathname("/var/adm") : _pimpl->update_data_path );
  }

  Pathname ZConfig::update_messagesPath() const
  {
    return ( _pimpl->update_messages_path.empty()
             ? Pathname(update_dataPath()/"update-messages") : _pimpl->update_messages_path );
  }


  Pathname ZConfig::update_scriptsPath() const
  {
    return ( _pimpl->update_scripts_path.empty()
             ? Pathname(update_dataPath()/"update-scripts") : _pimpl->update_scripts_path );
  }

  ///////////////////////////////////////////////////////////////////

  target::rpm::RpmInstFlags ZConfig::rpmInstallFlags() const
  { return _pimpl->rpmInstallFlags; }


  Pathname ZConfig::historyLogFile() const
  {
    return ( _pimpl->history_log_path.empty() ?
        Pathname("/var/log/zypp/history") : _pimpl->history_log_path );
  }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
