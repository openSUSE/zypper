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
#include "zypp/repo/RepoException.h"
#include "zypp/ZConfig.h"
#include "RepoVariables.h"

using namespace std;

namespace zypp
{
namespace repo
{

static string
gsub(const string& sData,
     const string& sFrom,
     const string& sTo)
{
  string sNew = sData;
  
  if (! sNew.empty())
  {
    string::size_type toLen = sTo.length();
    string::size_type frLen = sFrom.length();
    string::size_type loc = 0;
    
    while (string::npos != (loc = sNew.find(sFrom, loc)))
    {
      sNew.replace(loc, frLen, sTo);
      loc += toLen;
      
      if (loc >= sNew.length())
      break;
    }
  }

  return sNew;
}
  
RepoVariablesStringReplacer::RepoVariablesStringReplacer()
{}

RepoVariablesStringReplacer::~RepoVariablesStringReplacer()
{}

std::string RepoVariablesStringReplacer::operator()( const std::string &value ) const
{
  string newvalue(value);
  
  // $arch
  newvalue = gsub( newvalue,
                   "$arch",
                   ZConfig::instance().systemArchitecture().asString() );
  // $basearch
  ZConfig::instance().systemArchitecture();
  
  ZConfig::instance();
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
