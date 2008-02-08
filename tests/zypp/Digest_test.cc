
#include <iostream>
#include <sstream>
#include <fstream>
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
#include "zypp/PathInfo.h"
#include "zypp/Digest.h"

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;

/**
 * Test case for
 * static std::string digest(const std::string& name, std::istream& is, size_t bufsize = 4096);
 */
void digest_test()
{
  string data("I will test the checksum of this");
  stringstream str1(data);
  stringstream str2(data);
  stringstream str3(data);

  BOOST_CHECK_EQUAL( Digest::digest( "sha1", str1 ), "142df4277c326f3549520478c188cab6e3b5d042" ); 
  BOOST_CHECK_EQUAL( Digest::digest( "md5", str2 ), "f139a810b84d82d1f29fc53c5e59beae" ); 
  // FIXME i think it should throw
  BOOST_CHECK_EQUAL( Digest::digest( "lalala", str3) , "" ); 
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "DigestTest" );
    test->add( BOOST_TEST_CASE( &digest_test ), 0 /* expected zero error */ );
    return test;
}

