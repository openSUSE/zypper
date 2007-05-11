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
#include "zypp2/cache/CacheStore.h"
#include "zypp2/repository/cached/CachedRepositoryImpl.h"
#include "zypp/Url.h"
#include "zypp/NVRA.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"

#include "SimplePackagesParser.h"

using namespace std;
using namespace zypp;
using namespace zypp::repository;
using namespace zypp::repository::cached;
using namespace boost::unit_test;

void cache_write_test(const string &dir)
{
  filesystem::TmpDir tmpdir;
  {
    Pathname nvra_list = Pathname(dir) + "package-set.txt.gz";
    list<MiniResolvable> res_list;
    
    parse_mini_file( nvra_list, res_list );
    
    cache::CacheStore store(tmpdir.path());
    
    data::RecordId catalog_id = store.lookupOrAppendCatalog( Url("http://novell.com"), "/");
    
    zypp::debug::Measure cap_parse_timer("store resolvables");
    for ( list<MiniResolvable>::iterator it = res_list.begin(); it != res_list.end(); it++)
    {
      data::RecordId id = store.appendResolvable( catalog_id,
                                        ResTraits<Package>::kind,
                                        (*it).nvra,
                                        (*it).deps );
    }
  }
  {
    MIL << "now read resolvables" << endl;
    
    CachedRepositoryImpl *repositoryImpl = new CachedRepositoryImpl(tmpdir.path());
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
