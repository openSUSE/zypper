
#include <iostream>
#include <list>
#include <string>

// Boost.Test
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using boost::test_tools::close_at_tolerance;

// parameterized test
// http://www.boost.org/libs/test/example/unit_test_example4.cpp

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

using namespace std;
using namespace zypp;


// most frequently you implement test cases as a free functions
void checksum_test()
{
  BOOST_CHECK_THROW( CheckSum( "sha1", "dsdsads" ), Exception ); // wrong size
  BOOST_CHECK_THROW( CheckSum( "sha256", "dsdsads" ), Exception ); // wrong size
  BOOST_CHECK_THROW( CheckSum( "md5", "dsdsads" ), Exception ); // wrong size
  BOOST_CHECK_THROW( CheckSum( "md4", "dsdsads" ), Exception ); // wrong size
  BOOST_CHECK_THROW( CheckSum( "md2", "dsdsads" ), Exception ); // wrong size

}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "CheckSumTest" );
    test->add( BOOST_TEST_CASE( &checksum_test ), 0 /* expected zero error */ );
    return test;
}

