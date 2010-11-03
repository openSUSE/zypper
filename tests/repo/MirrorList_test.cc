#include <iostream>
#include <vector>
#include <boost/test/auto_unit_test.hpp>

#include "WebServer.h"

#include "zypp/repo/RepoMirrorList.cc"

using namespace std;
using namespace zypp;
using namespace zypp::repo;

BOOST_AUTO_TEST_CASE(get_mirrorlist)
{
  WebServer web((Pathname(TESTS_SRC_DIR) + "/data/Mirrorlist/remote-site").c_str(), 10001);
  web.start();

  Url weburl1 (web.url());
  Url weburl2 (web.url());

  weburl1.setPathName("/metalink.xml");
  weburl2.setPathName("/mirrors.txt");

  RepoMirrorList rml1 (weburl1);
  RepoMirrorList rml2 (weburl2);

  BOOST_CHECK(rml1.getUrls().begin()->asString() == "http://ftp-stud.hs-esslingen.de/pub/fedora/linux/updates/13/x86_64/");
  BOOST_CHECK(rml2.getUrls().begin()->asString() == "http://ftp-stud.hs-esslingen.de/pub/fedora/linux/updates/13/x86_64/");

  BOOST_CHECK(rml1.getUrls().size() == 4);
  BOOST_CHECK(rml2.getUrls().size() == 4);

  web.stop();
}
