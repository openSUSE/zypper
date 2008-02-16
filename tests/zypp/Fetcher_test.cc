#include <stdio.h>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/MediaSetAccess.h"
#include "zypp/Fetcher.h"
#include "zypp/Url.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace zypp::media;
using namespace boost::unit_test;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "/zypp/data/Fetcher/remote-site")

BOOST_AUTO_TEST_CASE(fetcher)
{
  MediaSetAccess media( ("dir:" + DATADIR).asUrl(), "/" );
  Fetcher fetcher;
}

// vim: set ts=2 sts=2 sw=2 ai et:
