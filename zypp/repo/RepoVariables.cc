/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include <map>
#include <algorithm>
#include "zypp/base/String.h"
#include "zypp/repo/RepoException.h"
#include "zypp/ZConfig.h"
#include "RepoVariables.h"

using namespace std;

namespace zypp
{
namespace repo
{
  
RepoVariablesStringReplacer::RepoVariablesStringReplacer()
{}

RepoVariablesStringReplacer::~RepoVariablesStringReplacer()
{}

std::string RepoVariablesStringReplacer::operator()( const std::string &value ) const
{
  string newvalue(value);
  
  // $arch
  newvalue = str::gsub( newvalue,
                        "$arch",
                        ZConfig::instance().systemArchitecture().asString() );
  // $basearch
  
  Arch::CompatSet cset( Arch::compatSet( ZConfig::instance().systemArchitecture() ) );
  Arch::CompatSet::const_iterator it = cset.end();
  --it;
  // now at noarch
  --it;
  
  Arch basearch = *it;
  if ( basearch == Arch_noarch )
  {
    basearch = ZConfig::instance().systemArchitecture();
  }

  newvalue = str::gsub( newvalue,
                        "$basearch",
                        basearch.asString() );
  return newvalue;
}

//////////////////////////////////////////////////////////////////////

RepoVariablesUrlReplacer::RepoVariablesUrlReplacer()
{}

RepoVariablesUrlReplacer::~RepoVariablesUrlReplacer()
{}


Url RepoVariablesUrlReplacer::operator()( const Url &value ) const
{
  RepoVariablesStringReplacer replacer;
  string transformed = replacer(value.asString());
  Url newurl;
  try {
    newurl = Url(transformed);
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    // just return what we got
    return value;
  }
  return newurl;
}

} // ns repo
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
