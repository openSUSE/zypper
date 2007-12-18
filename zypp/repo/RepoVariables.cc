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

/*
 * Replaces '$arch' and '$basearch' in the path and query part of the URL
 * with the global ZYpp values. Examples:
 *
 * ftp://user:secret@site.net/$arch/ -> ftp://user:secret@site.net/i686/
 * http://site.net/?basearch=$basearch -> http://site.net/?basearch=i386
 */
Url RepoVariablesUrlReplacer::operator()( const Url &value ) const
{
  Url newurl = value;
  RepoVariablesStringReplacer replacer;
  newurl.setPathData(replacer(value.getPathData()));
  newurl.setQueryString(replacer(value.getQueryString()));

  return newurl;
}

} // ns repo
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
