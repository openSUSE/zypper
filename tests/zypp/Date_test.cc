#include "zypp/Date.h"

#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_CASE(date_test)
{
  std::string format = "%Y-%m-%d %H:%M:%S";
  std::string date = "2009-02-14 00:31:30";
  BOOST_CHECK_EQUAL(zypp::Date(date,format).form(format), date);
}
