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

bool init_function() {
  framework::master_test_suite().add( BOOST_TEST_CASE( boost::bind( &poolquery_simple_test) ) );
  return true;
} 

int
main( int argc, char* argv[] )
{
return ::boost::unit_test::unit_test_main( &init_function, argc, argv );
} 

// vim: set ts=2 sts=2 sw=2 ai et:
