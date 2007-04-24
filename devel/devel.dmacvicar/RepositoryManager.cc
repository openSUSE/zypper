






#include <iostream>
#include <list>
#include <algorithm>
#include "RepositoryManager.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Logger.h"
#include "zypp/PathInfo.h"
#include "zypp/parser/inifile/iniparser.h"

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;
using namespace zypp::source;

namespace zypp {

RepositoryManager::RepositoryManager()
{

}

static std::list<source::SourceInfo> repositories_in_file( const Pathname &file )
{
  dictionary *d = iniparser_new(file.c_str());
  
  if ( d == NULL )
    ZYPP_THROW(Exception("Failed creating dictionary"));
  
  int n = iniparser_getnsec(d);
  MIL << n << endl;
  
  for ( int i = 0; i < n; i++ )
  {
    MIL << iniparser_getsecname(d, i) << endl;
    
  }
  return std::list<source::SourceInfo>();
}

static std::list<source::SourceInfo> repositories_in_path( const Pathname &dir )
{
  std::list<source::SourceInfo> repos;
  list<Pathname> entries;
  if ( filesystem::readdir( entries, Pathname(dir), false ) != 0 )
    ZYPP_THROW(Exception("failed to read directory"));
  
  for ( list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
  {
    Pathname file = *it;
    std::list<source::SourceInfo> repos_here = repositories_in_file(file);
    std::copy( repos_here.begin(), repos_here.end(), std::back_inserter(repos));
  }
  return repos;
}

std::list<source::SourceInfo> RepositoryManager::knownRepositories()
{
  

  return std::list<source::SourceInfo>();
}

} // ns zypp