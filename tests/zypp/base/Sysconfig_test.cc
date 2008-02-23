
#include <iostream>
#include <fstream>
#include <map>
#include <string>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/TmpPath.h"
#include "zypp/PathInfo.h"

#include "zypp/base/Sysconfig.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using namespace boost::unit_test;

using namespace std;
using namespace zypp;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "/zypp/base/data/Sysconfig")

BOOST_AUTO_TEST_CASE(Sysconfig)
{
  Pathname file = DATADIR + "proxy";
  map<string,string> values = zypp::base::sysconfig::read(file);
  BOOST_CHECK_EQUAL( values.size(), 6 );
  BOOST_CHECK_EQUAL( values["PROXY_ENABLED"], "no");
  BOOST_CHECK_EQUAL( values["GOPHER_PROXY"], "");
  BOOST_CHECK_EQUAL( values["NO_PROXY"], "localhost, 127.0.0.1");
}

