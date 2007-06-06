
#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/KeyRing.h"
#include "zypp/PublicKey.h"
#include "zypp/TmpPath.h"
#include "zypp/ResStore.h"

#include "zypp/RepoManager.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "KeyRingTestReceiver.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using namespace boost::unit_test::log;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;
using namespace zypp::repo;

void repomanager_test( const string &dir )
{
  RepoManagerOptions opts;
  
  TmpDir tmpCachePath;
  TmpDir tmpRawCachePath;
  
  opts.repoCachePath = tmpCachePath.path();
  opts.repoRawCachePath = tmpRawCachePath.path();
  opts.knownReposPath = Pathname(dir) + "/repos.d";
  
  RepoManager manager(opts);
  
  list<RepoInfo> repos = manager.knownRepositories();
  
  BOOST_CHECK_EQUAL(repos.size(), (unsigned) 3);
  
  RepoInfo repo(repos.front());
  manager.refreshMetadata(repo);
  
  Repository repository;
  try {
    repository = manager.createFromCache(repo);
  }
  catch ( const RepoNotCachedException &e )
  {
    ZYPP_CAUGHT(e);
    MIL << "repo " << repo.alias() << " not cached yet. Caching..." << endl;
    manager.buildCache(repo);
    repository = manager.createFromCache(repo);
  }
  
  ResStore store = repository.resolvables();
  MIL << store.size() << " resolvables" << endl;
}

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  string datadir;
  if (argc < 2)
  {
    datadir = TESTS_SRC_DIR;
    datadir = (Pathname(datadir) + "/zypp/data/RepoManager").asString();
    cout << "repomanager_test:"
      " path to directory with test data required as parameter. Using " << datadir  << endl;
    //return (test_suite *)0;
  }
  else
  {
    datadir = argv[1];
  }

  std::string const params[] = { datadir };
    //set_log_stream( std::cout );
  test_suite* test= BOOST_TEST_SUITE( "RepoManagerTest" );
  test->add(BOOST_PARAM_TEST_CASE( &repomanager_test,
                              (std::string const*)params, params+1));
  return test;
}

