
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
#include "zypp/PathInfo.h"

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
  BOOST_CHECK( PathInfo(tmpKnownReposPath.path() + "/proprietary.repo_1").isExist() );
  
  // now there should be 6 repos
  repos = manager.knownRepositories();
  BOOST_CHECK_EQUAL(repos.size(), (unsigned) 6);
  
  RepoInfo office_dup;
  office_dup.setAlias("office");
  BOOST_CHECK_THROW(manager.addRepository(office_dup), RepoAlreadyExistsException);

  // delete the office repo inside the propietary_1.repo
  RepoInfo office;
  office.setAlias("office");
  manager.removeRepository(office);
  // now there should be 5 repos
  repos = manager.knownRepositories();
  BOOST_CHECK_EQUAL(repos.size(), (unsigned) 5);
  // the file still contained one repo, so it should still exists
  BOOST_CHECK( PathInfo(tmpKnownReposPath.path() + "/proprietary.repo_1").isExist() );
  
  // now delete the macromedia one
  RepoInfo macromedia;
  macromedia.setAlias("macromedia");
  manager.removeRepository(macromedia);
  repos = manager.knownRepositories();
  BOOST_CHECK_EQUAL(repos.size(), (unsigned) 4);
  // the file should not exist anymore
  BOOST_CHECK( ! PathInfo(tmpKnownReposPath.path() + "/proprietary.repo_1").isExist() );
  

  // let test cache creation

  RepoInfo repo;
  repo.setAlias("foo");
  //Url repourl("dir:" + string(TESTS_SRC_DIR) + "/repo/yum/data/10.2-updates-subset");
  Url repourl("dir:/mounts/dist/install/stable-x86/suse");
  //BOOST_CHECK_MESSAGE(0, repourl.asString());
  repo.setBaseUrl(repourl);

  KeyRingTestReceiver keyring_callbacks;
  KeyRingTestSignalReceiver receiver;
  
  // disable sgnature checking
  keyring_callbacks.answerTrustKey(true);
  keyring_callbacks.answerAcceptVerFailed(true);
  keyring_callbacks.answerAcceptUnknownKey(true);

  manager.refreshMetadata(repo);
  
  BOOST_CHECK_MESSAGE( ! manager.isCached(repo),
                       "Repo is not yet cached" );
  manager.buildCache(repo);

  // we have no metadata yet so this should throw
  //BOOST_CHECK_THROW( manager.buildCache(repo),
  //                   RepoMetadataException );

  return;

  Repository repository;

  // it is not cached, this should throw
  BOOST_CHECK_THROW( manager.createFromCache(repo),
                     RepoNotCachedException );

  MIL << "repo " << repo.alias() << " not cached yet. Caching..." << endl;
  manager.buildCache(repo);

  // the solv file should exists now
  Pathname solvfile = (opts.repoCachePath + repo.alias()).extend(".solv");
  BOOST_CHECK_MESSAGE( !PathInfo(solvfile).isExist(), "Solv file is created after caching");

  repository = manager.createFromCache(repo);
  
  BOOST_CHECK_MESSAGE( manager.isCached(repo),
                       "Repo is cached" );

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

