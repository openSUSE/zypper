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

#include "TestSetup.h"

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

  sat::Pool pool(sat::Pool::instance());

  TestSetup test( Arch_x86_64 );
  test.loadRepo(Url(string("dir:") + repodir.absolutename().asString()), "updates");

  Repository repo = pool.reposFind("updates");
  
  BOOST_CHECK_EQUAL( repo.generatedTimestamp(), Date(1222083131) );
  BOOST_CHECK_EQUAL( repo.suggestedExpirationTimestamp(), Date(1222083131 + 3600) );  
}
