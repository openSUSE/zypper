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
#include "zypp/data/ResolvableData.h"
#include "zypp2/cache/ResolvableQuery.h"
#include "zypp2/parser/yum/YUMParser.h"

using namespace std;
using namespace zypp;
using namespace zypp::debug;
using namespace zypp::capability;
using namespace zypp::filesystem;
using namespace zypp::cache;
using namespace zypp::parser::yum;
using namespace boost::unit_test;

bool result(const data::RecordId &id, data::ResObject_Ptr ptr )
{
  MIL << "result: " << id << " | " << ptr->name << " | " << ptr->edition << " | " << ptr->arch << endl;
  return true;
}

void resolvable_query_test(const string &dir)
{
  filesystem::TmpDir tmpdir;
  // let the store go out of scope to drop the connection
  {
    cache::CacheStore store(tmpdir.path());
    
    data::RecordId repository_id = store.lookupOrAppendRepository( Url("http://novell.com"), "/");
    
    YUMParser parser( repository_id, store );
    parser.parse(dir);
    store.commit();
  }
  
  ResolvableQuery query(tmpdir.path() );
  query.query(10, &result);
  
  MIL << query.queryTranslatedStringAttribute( 10, "ResObject", "summary" ).text() << endl;
}

test_suite*
init_unit_test_suite( int argc, char *argv[] )
{
  string datadir;
  if (argc < 2)
  {
    datadir = TESTS_SRC_DIR;
    datadir = (Pathname(datadir) + "/repo/yum/data/10.2-updates-subset").asString();
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



