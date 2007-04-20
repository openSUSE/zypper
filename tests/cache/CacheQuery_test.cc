#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/base/Measure.h"
#include "zypp/base/Logger.h"
#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/TmpPath.h"

#include "zypp2/cache/CacheStore.h"
#include "zypp2/cache/CapabilityQuery.h"
#include "zypp/data/ResolvableData.h"
#include "zypp2/cache/ResolvableQuery.h"

#include "SimplePackagesParser.h"

using namespace std;
using namespace zypp;
using namespace zypp::debug;
using namespace zypp::capability;
using namespace zypp::filesystem;
using namespace zypp::cache;
using namespace boost::unit_test;

bool result(const data::RecordId &id, data::ResObject_Ptr ptr )
{
  MIL << "result: " << id << " | " << ptr->name << " | " << ptr->edition << " | " << ptr->arch << endl;
}

void resolvable_query_test(const string &dir)
{
  Pathname nvra_list = Pathname(dir) + "package-set.txt.gz";
  
  MIL << "parsing " << nvra_list << endl;
  
  list<MiniResolvable> res_list;
  
  parse_mini_file( nvra_list, res_list );
  
  filesystem::TmpDir tmpdir;
  // let the store go out of scope to drop the connection
  {
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
    
    MIL << "packages writen to store" << endl;
  }
  
  ResolvableQuery query(tmpdir.path(), &result);
  query.query("lib");
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
  test->add(BOOST_PARAM_TEST_CASE(&resolvable_query_test,
                                 (std::string const*)params, params+1));
  return test;
}



