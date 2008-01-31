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
#include <iostream>
#include "zypp/base/Logger.h"
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
        : repo_add_probe          ( false )
        , repo_refresh_delay      ( 10 )
        , download_use_patchrpm   ( true )
        , download_use_deltarpm   ( true )

      {
        MIL << "ZConfig singleton created." << endl;

	// ZYPP_CONF might override /etc/zypp/zypp.conf

        const char *env_confpath = getenv( "ZYPP_CONF" );

        Pathname confpath( env_confpath ? env_confpath : "/etc/zypp/zypp.conf" );
        if ( PathInfo(confpath).isExist())
        {
          InputStream is(confpath);
          dict.read(is);
        }
        else
        {
          MIL << confpath << " not found, using defaults instead." << endl;
          return;
        }

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
                cfg_arch = Arch(value);
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
            }
          }
        }
      }

      ~Impl()
      {}

    public:
    parser::IniDict dict;

    Arch cfg_arch;

    Pathname cfg_metadata_path;
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
  {

  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZConfig::~ZConfig
  //	METHOD TYPE : Dtor
  //
  ZConfig::~ZConfig( )
  {}

  ///////////////////////////////////////////////////////////////////
#warning change methods to use the singleton

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZConfig::systemArchitecture
  //	METHOD TYPE : Arch
  //
  Arch ZConfig::systemArchitecture() const
  {
    // get architecture from ZYpp() if not overriden,
    //  ZYpp() knows how to retrieve the client arch and check cpu flags
    return ( (_pimpl->cfg_arch == Arch()) ?
        getZYpp()->architecture() : _pimpl->cfg_arch );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZConfig::overrideSystemArchitecture
  //	METHOD TYPE : void
  //
  void ZConfig::overrideSystemArchitecture(const Arch & arch)
  {
     WAR << "Overriding system architecture with " << arch << endl;
     _pimpl->cfg_arch = arch;
     getZYpp()->setArchitecture( arch );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZConfig::defaultTextLocale
  //	METHOD TYPE : Locale
  //
  Locale ZConfig::textLocale() const
  {
    return getZYpp()->getTextLocale();
  }

  Pathname ZConfig::repoMetadataPath() const
  {
    return ( _pimpl->cfg_metadata_path.empty()
        ? Pathname("/var/cache/zypp/raw") : _pimpl->cfg_metadata_path );
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
