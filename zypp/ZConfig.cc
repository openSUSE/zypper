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
  */
  class ZConfig::Impl
  {
    public:
      Impl()
      {
        MIL << "ZConfig singleton created." << endl;
        Pathname confpath("/etc/zypp/zypp.conf");
        if ( PathInfo(confpath).isExist())
        {
          InputStream is(confpath);
          dict.read(is);
        }
        else
        {
          MIL << "No /etc/zypp/zypp.conf" << endl;
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
            //MIL << (*it).first << endl;
            if ( section == "main" )
            {
              if ( entry == "arch" )
              {
                cfg_arch = Arch(value);
              }
              else if ( entry == "metadata-path" )
              {
                cfg_metadata_path = Pathname(value);
              }
              else if ( entry == "known-repos-path" )
              {
                cfg_known_repos_path = Pathname(value);
              }
              else if ( entry == "cache-path" )
              {
                cfg_cache_path = Pathname(value);
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
    return ( (_pimpl->cfg_arch == Arch()) ?
        getZYpp()->architecture() : _pimpl->cfg_arch );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZConfig::defaultTextLocale
  //	METHOD TYPE : Locale
  //
  Locale ZConfig::defaultTextLocale() const
  {
    return getZYpp()->getTextLocale();
  }

  Pathname ZConfig::defaultRepoMetadataPath() const
  {
    return ( _pimpl->cfg_metadata_path.empty() 
        ? Pathname("/var/lib/zypp/cache/raw") : _pimpl->cfg_metadata_path );
  }

  Pathname ZConfig::defaultRepoCachePath() const
  {
    return ( _pimpl->cfg_cache_path.empty() 
        ? Pathname("/var/lib/zypp/cache") : _pimpl->cfg_cache_path );
  }

  Pathname ZConfig::defaultKnownReposPath() const
  {
    return ( _pimpl->cfg_known_repos_path.empty() 
        ? Pathname("/etc/zypp/repos.d") : _pimpl->cfg_known_repos_path );
  }

  const std::string & ZConfig::cacheDBSplitJoinSeparator() const
  {
    static std::string s("!@$");
    return s;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
