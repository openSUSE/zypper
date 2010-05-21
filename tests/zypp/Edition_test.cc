// Edition.cc
//
// tests for Edition
//

#include "zypp/base/Logger.h"
#include "zypp/Edition.h"

#include <boost/test/auto_unit_test.hpp>

using boost::unit_test::test_case;

using namespace std;
using namespace zypp;

BOOST_AUTO_TEST_CASE(edition)
{
  Edition _ed1 ("1");
  Edition _ed2 ("1.1");
  Edition _ed3 ("1:1");
  Edition _ed4 ("2:1-1");

  BOOST_CHECK_EQUAL(_ed2.version(), "1.1");
  BOOST_CHECK_EQUAL(_ed2.release(), "");
  BOOST_CHECK_EQUAL(_ed2.epoch(), 0U);
  BOOST_CHECK_EQUAL(_ed4.epoch(), 2U);

  BOOST_CHECK_EQUAL(_ed1, Edition ("1", ""));
  BOOST_CHECK_EQUAL(_ed2, Edition ("1.1", ""));
  BOOST_CHECK_EQUAL(_ed2, Edition ("1_1", "")); // Edition strings may differ in separator (non alphanum)
  BOOST_CHECK_EQUAL(_ed2, Edition ("0:1.1")); // epoch 0 is no epoch
  BOOST_CHECK_EQUAL(_ed3, Edition ("1", "", "1"));
  BOOST_CHECK_EQUAL(_ed3, Edition ("1", "", 1));
  BOOST_CHECK_EQUAL(_ed4, Edition ("1", "1", 2));

  BOOST_CHECK_EQUAL( Edition::compare("1:1-1","2:1-1"), -1 );
  BOOST_CHECK_EQUAL( Edition::compare("2:1-1","2:1-1"), 0 );
  BOOST_CHECK_EQUAL( Edition::compare("3:1-1","2:1-1"), 1 );
}
