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
      Impl()
        : cfg_arch                ( defaultSystemArchitecture() )
        , cfg_textLocale          ( defaultTextLocale() )
        , repo_add_probe          ( false )
        , repo_refresh_delay      ( 10 )
        , download_use_patchrpm   ( true )
        , download_use_deltarpm   ( true )

      {
        MIL << "libzypp: " << VERSION << " built " << __DATE__ << " " <<  __TIME__ << endl;

	// ZYPP_CONF might override /etc/zypp/zypp.conf
        const char *env_confpath = getenv( "ZYPP_CONF" );
        Pathname confpath( env_confpath ? env_confpath : "/etc/zypp/zypp.conf" );
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
                else if ( entry == "metadatadir" )
                {
                  cfg_metadata_path = Pathname(value);
                }
                else if ( entry == "reposdir" )
                {
                  cfg_known_repos_path = Pathname(value);
                }
                else if ( entry == "cachedir" )
                {
                  cfg_cache_path = Pathname(value);
                }
                else if ( entry == "repo.add.probe" )
                {
                  repo_add_probe = str::strToBool( value, repo_add_probe );
                }
                else if ( entry == "repo.refresh.delay" )
                {
                  str::strtonum(value, repo_refresh_delay);
                }
                else if ( entry == "download.use_patchrpm" )
                {
                  download_use_patchrpm = str::strToBool( value, download_use_patchrpm );
                }
                else if ( entry == "download.use_deltarpm" )
                {
                  download_use_deltarpm = str::strToBool( value, download_use_deltarpm );
                }
                else if ( entry == "vendordir" )
                {
                  cfg_vendor_path = Pathname(value);
                }
                else if ( entry == "packagesdir" )
                {
                  cfg_packages_path = Pathname(value);
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

    Pathname cfg_metadata_path;
    Pathname cfg_packages_path;
    Pathname cfg_cache_path;
    Pathname cfg_known_repos_path;
    Pathname cfg_vendor_path;

    bool repo_add_probe;
    unsigned repo_refresh_delay;

    bool download_use_patchrpm;
    bool download_use_deltarpm;


  };
  ///////////////////////////////////////////////////////////////////

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

  Pathname ZConfig::repoMetadataPath() const
  {
    return ( _pimpl->cfg_metadata_path.empty()
        ? Pathname("/var/cache/zypp/raw") : _pimpl->cfg_metadata_path );
  }

  Pathname ZConfig::repoPackagesPath() const
  {
    return ( _pimpl->cfg_packages_path.empty()
        ? Pathname("/var/cache/zypp/packages") : _pimpl->cfg_packages_path );
  }

  Pathname ZConfig::repoCachePath() const
  {
    return ( _pimpl->cfg_cache_path.empty()
        ? Pathname("/var/cache/zypp") : _pimpl->cfg_cache_path );
  }

  Pathname ZConfig::knownReposPath() const
  {
    return ( _pimpl->cfg_known_repos_path.empty()
        ? Pathname("/etc/zypp/repos.d") : _pimpl->cfg_known_repos_path );
  }

  const std::string & ZConfig::cacheDBSplitJoinSeparator() const
  {
    static std::string s("!@$");
    return s;
  }

  bool ZConfig::repo_add_probe() const
  {
    return _pimpl->repo_add_probe;
  }

  unsigned ZConfig::repo_refresh_delay() const
  {
    return _pimpl->repo_refresh_delay;
  }

  bool ZConfig::download_use_patchrpm() const
  { return _pimpl->download_use_patchrpm; }

  bool ZConfig::download_use_deltarpm() const
  { return _pimpl->download_use_deltarpm; }

  Pathname ZConfig::vendorPath() const
  {
    return ( _pimpl->cfg_vendor_path.empty()
        ? Pathname("/etc/zypp/vendors.d") : _pimpl->cfg_vendor_path );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
