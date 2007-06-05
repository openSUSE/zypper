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

#include "zypp/ZConfig.h"
#include "zypp/ZYppFactory.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZConfig::systemArchitecture
  //	METHOD TYPE : Arch
  //
  Arch ZConfig::systemArchitecture() const
  {
    return getZYpp()->architecture();
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

  Pathname ZConfig::defaultRepoRawCachePath() const
  {
    return Pathname("/var/lib/zypp/cache/raw");
  }
      
  Pathname ZConfig::defaultRepoCachePath() const
  {
    return Pathname("/var/lib/zypp/cache");
  }
  
  Pathname ZConfig::defaultKnownReposPath() const
  {
    return Pathname("/etc/zypp/repos.d");
  }
  
  const std::string & ZConfig::cacheDBSplitJoinSeparator() const
  {
    static std::string s("!@$");
    return s;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
