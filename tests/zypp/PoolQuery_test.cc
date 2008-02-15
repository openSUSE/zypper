#include <stdio.h>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/ZYppFactory.h"
#include "zypp/PoolQuery.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;

bool result_cb( const ResObject::Ptr &r )
{
  cout << r << endl;
  return true;
}

void poolquery_simple_test()
{
  Pathname dir(TESTS_SRC_DIR);
  dir += "/zypp/data/PoolQuery";

  ZYpp::Ptr z = getZYpp();
    
  sat::Pool::instance().addRepoSolv(dir + "foo.solv");

  PoolQuery query;
  //query.setInstalledOnly();
  query.execute("kde", &result_cb);

  cout << "search done." << endl;

  query.setMatchExact(true);
  query.execute("kde", &result_cb);
  
  cout << "search done." << endl;
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

  test_suite* test= BOOST_TEST_SUITE("PoolQuery_test");

  // simple test
  test->add(BOOST_TEST_CASE(&poolquery_simple_test));

  return test;
}

// vim: set ts=2 sts=2 sw=2 ai et:
