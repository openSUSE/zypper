
#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/RepoInfo.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "KeyRingTestReceiver.h"

#include "WebServer.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using namespace boost::unit_test::log;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;
using namespace zypp::repo;

BOOST_AUTO_TEST_CASE(repoinfo_test)
{
  WebServer web((Pathname(TESTS_SRC_DIR) + "/data/Mirrorlist/remote-site").c_str(), 10001);
  web.start();

  Url weburl (web.url());
  weburl.setPathName("/metalink.xml");

  RepoInfo ri;

  ri.setMirrorListUrl(weburl);

  BOOST_CHECK(ri.url().asString() == "http://ftp-stud.hs-esslingen.de/pub/fedora/linux/updates/13/x86_64/");

  ostringstream ostr;
  ri.dumpAsIniOn(ostr);

  BOOST_CHECK( ostr.str().find("baseurl=") == string::npos );

  web.stop();
}
