#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <string>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"
#include "zypp/RepoManager.h"
#include "zypp/sat/Pool.h"
#include "KeyRingTestReceiver.h"

using boost::unit_test::test_case;

using namespace std;
using namespace zypp;
using namespace zypp::repo;
using namespace zypp::filesystem;

#define TEST_DIR TESTS_SRC_DIR "/repo/yum/data/extensions"

BOOST_AUTO_TEST_CASE(extensions)
{
  KeyRingTestReceiver rec;
  //rec.answerAcceptUnknownKey(true);
  rec.answerAcceptUnsignedFile(true);
  

//  rec.answerImportKey(true);
  Pathname repodir(TEST_DIR );

  TmpDir tmpCachePath;
  RepoManagerOptions opts( RepoManagerOptions::makeTestSetup( tmpCachePath ) ) ;
  RepoManager rm(opts);

  RepoInfo updates;
  updates.setAlias("updates");
  updates.addBaseUrl(Url(string("dir:") + repodir.absolutename().asString() ));

  try
  {
    rm.buildCache(updates);
    rm.loadFromCache(updates);
  }
  catch (const Exception & e)
  {
    BOOST_FAIL( string("Problem getting the data: ")+ e.msg()) ;
  }
  sat::Pool pool(sat::Pool::instance());

  Repository repo = pool.reposFind("updates");
  
  BOOST_CHECK_EQUAL( repo.generatedTimestamp(), Date(1215823454) );
  BOOST_CHECK_EQUAL( repo.suggestedExpirationTimestamp(), Date(1215823454 + 3600) );  

  rm.cleanCache(updates);
}
