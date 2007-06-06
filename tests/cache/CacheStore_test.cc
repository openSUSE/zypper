#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/base/Random.h"
#include "zypp/base/Logger.h"

#include "zypp/base/Measure.h"
#include "zypp/capability/CapabilityImpl.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/cache/CacheStore.h"
#include "zypp/RepoInfo.h"
#include "zypp/repo/cached/RepoImpl.h"
#include "zypp/Url.h"
#include "zypp/NVRA.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"

#include "SimplePackagesParser.h"

using namespace std;
using namespace zypp;
using namespace zypp::repo;
using namespace zypp::repo::cached;
using namespace boost::unit_test;

void cache_write_test(const string &dir)
{
  data::RecordId repository_id;
  filesystem::TmpDir tmpdir;
  {
    Pathname nvra_list = Pathname(dir) + "package-set.txt.gz";
    list<MiniResolvable> res_list;
    
    parse_mini_file( nvra_list, res_list );
    
    cache::CacheStore store(tmpdir.path());
    
    repository_id = store.lookupOrAppendRepository("novell.com");
    
    zypp::debug::Measure cap_parse_timer("store resolvables");
    for ( list<MiniResolvable>::iterator it = res_list.begin(); it != res_list.end(); it++)
    {
      data::RecordId id = store.appendResolvable( repository_id,
                                        ResTraits<Package>::kind,
                                        (*it).nvra,
                                        (*it).deps );
    }
    store.commit();
  }
  {
    MIL << "now read resolvables" << endl;
    
    cached::RepoImpl *repositoryImpl = new cached::RepoImpl( RepoInfo(), tmpdir.path(), repository_id);
    //RepositoryFactory factory;
    //Repository_Ref repository = factory.createFrom(repositoryImpl);
    repositoryImpl->createResolvables();
    ResStore dbres = repositoryImpl->resolvables();
        
    MIL << dbres.size() << " resolvables" << endl;
  }
}

test_suite*
init_unit_test_suite( int argc, char *argv[] )
{
  string datadir;
  if (argc < 2)
  {
    datadir = TESTS_SRC_DIR;
    datadir = (Pathname(datadir) + "/cache/data").asString();
    cout << "CacheStore_test:"
      " path to directory with test data required as parameter. Using " << datadir  << endl;
    //return (test_suite *)0;
    
  }
  else
  {
    datadir = argv[1];
  }
  
  test_suite* test= BOOST_TEST_SUITE("CacheStore");
  
  std::string const params[] = { datadir };
  test->add(BOOST_PARAM_TEST_CASE(&cache_write_test,
                                 (std::string const*)params, params+1));
  return test;
}


// vim: set ts=2 sts=2 sw=2 ai et:
