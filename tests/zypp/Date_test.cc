//
// tests for zypp/Date.h
//

#include <iostream>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/Date.h"

// Boost.Test
#include <boost/test/auto_unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;

BOOST_AUTO_TEST_CASE(date_test)
{
  string format = "%Y-%m-%d %H:%M:%S";
  string date = "2009-02-14 00:31:30";
  BOOST_CHECK_EQUAL(Date(date,format).form(format), date);
}
