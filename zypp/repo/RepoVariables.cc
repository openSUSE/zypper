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
{}

RepoVariablesStringReplacer::~RepoVariablesStringReplacer()
{}

std::string RepoVariablesStringReplacer::operator()( const std::string &value ) const
{
  string newvalue(value);

  // $arch
  Arch sysarch( ZConfig::instance().systemArchitecture() );
  newvalue = str::gsub( newvalue, "$arch", sysarch.asString() );

  // $basearch
  Arch basearch( sysarch.baseArch( ) );

  newvalue = str::gsub( newvalue, "$basearch", basearch.asString() );

  // $releasever (Target::distributionVersion assumes root=/ if target not initialized)
  newvalue = str::gsub( newvalue, "$releasever", Target::distributionVersion(Pathname()/*guess*/) );

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
