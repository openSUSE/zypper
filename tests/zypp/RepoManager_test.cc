
#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include "zypp/base/LogTools.h"
#include "zypp/base/Exception.h"
#include "zypp/KeyRing.h"
#include "zypp/PublicKey.h"
#include "zypp/TmpPath.h"
#include "zypp/PathInfo.h"

#include "zypp/RepoManager.h"

#include <boost/test/auto_unit_test.hpp>


#include "KeyRingTestReceiver.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;
using namespace zypp::repo;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "/zypp/data/RepoManager")

BOOST_AUTO_TEST_CASE(repomanager_test)
{
  RepoManagerOptions opts;

  TmpDir tmpCachePath;
  TmpDir tmpRawCachePath;
  TmpDir tmpKnownReposPath;

  BOOST_CHECK_EQUAL( filesystem::copy_dir_content( DATADIR + "/repos.d", tmpKnownReposPath.path() ), 0 );

  opts.repoCachePath = tmpCachePath.path();
  opts.repoRawCachePath = tmpRawCachePath.path();
  opts.knownReposPath = tmpKnownReposPath.path();

  RepoManager manager(opts);

  list<RepoInfo> repos = manager.knownRepositories();
  BOOST_CHECK_EQUAL(repos.size(), (unsigned) 4);

  // now add a .repo file with 2 repositories in it
  Url url;
  url.setPathName((DATADIR + "/proprietary.repo").asString());
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
  Url repourl("dir:" + string(TESTS_SRC_DIR) + "/repo/yum/data/10.2-updates-subset");
  //Url repourl("dir:/mounts/dist/install/stable-x86/suse");
  //BOOST_CHECK_MESSAGE(0, repourl.asString());
  repo.setBaseUrl(repourl);

  KeyRingTestReceiver keyring_callbacks;
  KeyRingTestSignalReceiver receiver;

  // disable sgnature checking
  keyring_callbacks.answerTrustKey(true);
  keyring_callbacks.answerAcceptVerFailed(true);
  keyring_callbacks.answerAcceptUnknownKey(true);

  // We have no metadata and cache yet
  BOOST_CHECK_MESSAGE( !manager.isCached(repo), "Repo should not yet be cached" );

  // This should download metadata and build the cache
  manager.buildCache(repo);

  // Now we have metadata and cache
  BOOST_CHECK_MESSAGE( manager.isCached(repo), "Repo should be cached now" );

  // Metadata are up to date
  SEC << endl
      << manager.checkIfToRefreshMetadata(repo, *repo.baseUrlsBegin()) << endl;
  BOOST_CHECK_MESSAGE( !manager.checkIfToRefreshMetadata(repo, *repo.baseUrlsBegin()), "Metadata should be up to date" );

   // the solv file should exists now
  Pathname base = (opts.repoCachePath + repo.alias());
  Pathname solvfile = base.extend(".solv");
  Pathname cookiefile = base.extend(".cookie");
  BOOST_CHECK_MESSAGE( PathInfo(solvfile).isExist(), "Solv file is created after caching: " + solvfile.asString());
  BOOST_CHECK_MESSAGE( PathInfo(cookiefile).isExist(), "Cookie file is created after caching: " + cookiefile.asString());

  MIL << "Repo already in cache, clean cache"<< endl;
  manager.cleanCache(repo);

  BOOST_CHECK_MESSAGE( !manager.isCached(repo),
                       "Repo cache was just deleted, should not be cached now" );

  // now cache should build normally
  manager.buildCache(repo);

  manager.loadFromCache(repo);

  if ( manager.isCached(repo ) )
  {
    MIL << "Repo already in cache, clean cache"<< endl;
    manager.cleanCache(repo);
  }
  MIL << "Parsing repository metadata..." << endl;
  manager.buildCache(repo);
}

