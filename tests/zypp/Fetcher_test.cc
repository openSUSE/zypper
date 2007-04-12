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

void fetcher_simple_test()
{
  //TESTS_SRC_DIR
}

test_suite*
init_unit_test_suite( int argc, char *argv[] )
{
//   if (argc < 2)
//   {
//     cout << "mediasetaccesstest:"
//       " path to directory with test data required as parameter" << endl;
//     return (test_suite *)0;
//   }

  test_suite* test= BOOST_TEST_SUITE("Fetcher_test");

  // simple test
  test->add(BOOST_TEST_CASE(&fetcher_simple_test));

  return test;
}

// vim: set ts=2 sts=2 sw=2 ai et:
