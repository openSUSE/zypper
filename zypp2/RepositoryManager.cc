

#include <iostream>
#include <list>
#include <algorithm>
#include "zypp/base/Exception.h"
#include "zypp/base/Logger.h"
#include "zypp/PathInfo.h"
#include "zypp/parser/IniDict.h"

#include "zypp2/RepositoryManager.h"


using namespace std;
using namespace zypp;
using namespace zypp::filesystem;

namespace zypp {

RepositoryManager::RepositoryManager()
{

}

static std::list<RepositoryInfo> repositories_in_file( const Pathname &file )
{
//   dictionary *d = iniparser_new(file.c_str());
//   
//   if ( d == NULL )
//     ZYPP_THROW(Exception("Failed creating dictionary"));
//   
//   int n = iniparser_getnsec(d);
//   MIL << n << endl;
//   
//   for ( int i = 0; i < n; i++ )
//   {
//     MIL << iniparser_getsecname(d, i) << endl;
//     
//   }
  return std::list<RepositoryInfo>();
}

static std::list<RepositoryInfo> repositories_in_path( const Pathname &dir )
{
  std::list<RepositoryInfo> repos;
  list<Pathname> entries;
  if ( filesystem::readdir( entries, Pathname(dir), false ) != 0 )
    ZYPP_THROW(Exception("failed to read directory"));
  
  for ( list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
  {
    Pathname file = *it;
    std::list<RepositoryInfo> repos_here = repositories_in_file(file);
    std::copy( repos_here.begin(), repos_here.end(), std::back_inserter(repos));
  }
  return repos;
}

std::list<RepositoryInfo> RepositoryManager::knownRepositories()
{
  

  return std::list<RepositoryInfo>();
}

} // ns zypp

