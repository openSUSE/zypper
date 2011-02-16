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
#include "zypp/ZYppFactory.h"
#include "RepoVariables.h"

using namespace std;

namespace zypp
{
namespace repo
{

RepoVariablesStringReplacer::RepoVariablesStringReplacer()
{
  sysarch = Arch_empty;
  basearch = Arch_empty;
}

RepoVariablesStringReplacer::~RepoVariablesStringReplacer()
{}

void RepoVariablesStringReplacer::resetVarCache( void )
{
  sysarch = Arch_empty;
  basearch = Arch_empty;
  releasever = "";
}

std::string RepoVariablesStringReplacer::operator()( const std::string &value ) const
{
  string newvalue(value);

  // $arch
  if( sysarch.empty() )
    sysarch = ZConfig::instance().systemArchitecture();

  newvalue = str::gsub( newvalue, "$arch", sysarch.asString() );

  // $basearch
  if( basearch.empty() )
    basearch = sysarch.baseArch();

  newvalue = str::gsub( newvalue, "$basearch", basearch.asString() );

  // $releasever (Target::distributionVersion assumes root=/ if target not initialized)
  if ( newvalue.find("$releasever") != string::npos ) {
    if( releasever.empty() )
      releasever = Target::distributionVersion(Pathname()/*guess*/);

    newvalue = str::gsub( newvalue, "$releasever", releasever );
  } 

  return newvalue;
}

//////////////////////////////////////////////////////////////////////

RepoVariablesUrlReplacer::RepoVariablesUrlReplacer()
{}

RepoVariablesUrlReplacer::~RepoVariablesUrlReplacer()
{}

void RepoVariablesUrlReplacer::resetVarCache( void )
{
  replacer.resetVarCache();
}

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
  newurl.setPathData(replacer(value.getPathData()));
  newurl.setQueryString(replacer(value.getQueryString()));

  return newurl;
}

} // ns repo
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
