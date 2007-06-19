
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
  TmpDir tmpKnownReposPath;
  
  BOOST_CHECK_EQUAL( filesystem::copy_dir_content( Pathname(dir) + "/repos.d", tmpKnownReposPath.path() ), 0 );
  
  opts.repoCachePath = tmpCachePath.path();
  opts.repoRawCachePath = tmpRawCachePath.path();
  opts.knownReposPath = tmpKnownReposPath.path();
  
  RepoManager manager(opts);
  
  list<RepoInfo> repos = manager.knownRepositories();
  BOOST_CHECK_EQUAL(repos.size(), (unsigned) 4);
  
  // now add a .repo file with 2 repositories in it
  Url url;
  url.setPathName((Pathname(dir) + "/proprietary.repo").asString());
  url.setScheme("file");

  manager.addRepositories(url);
  
  // check it was not overwriten the proprietary.repo file
  BOOST_CHECK( PathInfo(Pathname(dir) + "/repos.d/proprietary_1.repo").isExist() );
  
  // now there should be 6 repos
  repos = manager.knownRepositories();
  BOOST_CHECK_EQUAL(repos.size(), (unsigned) 6);
  
  // delete the office repo inside the propietary_1.repo
  RepoInfo office;
  office.setAlias("office");
  manager.removeRepository(office);
  // now there should be 5 repos
  repos = manager.knownRepositories();
  BOOST_CHECK_EQUAL(repos.size(), (unsigned) 5);
  // the file still contained one repo, so it should still exists
  BOOST_CHECK( PathInfo(Pathname(dir) + "/repos.d/proprietary_1.repo").isExist() );
  
  // now delete the macromedia one
  RepoInfo macromedia;
  macromedia.setAlias("macromedia");
  manager.removeRepository(macromedia);
  repos = manager.knownRepositories();
  BOOST_CHECK_EQUAL(repos.size(), (unsigned) 4);
  // the file should not exist anymore
  BOOST_CHECK( ! PathInfo(Pathname(dir) + "/repos.d/proprietary_1.repo").isExist() );
  
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
  
  manager.refreshMetadata(repo);

  if ( manager.isCached(repo ) )
  {
    MIL << "Repo already in cache, clean cache"<< endl;
    manager.cleanCache(repo);
  }
  MIL << "Parsing repository metadata..." << endl;
  manager.buildCache(repo);
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

