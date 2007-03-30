
#include <iostream>
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
#include "zypp/TmpPath.h"

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;

/**
 * Test case for
 * bool is_checksum( const Pathname & file, const CheckSum &checksum );
 * std::string checksum( const Pathname & file, const std::string &algorithm );
 */
void pathinfo_checksum_test()
{
  const char *buffer = "I will test the checksum of this";
  TmpFile file;
  ofstream str(file.path().asString().c_str(),ofstream::out);

  if (!str.good())
    ZYPP_THROW(Exception("cant open file"));

  str << buffer;
  str.flush();
  str.close();

  CheckSum file_sha1("sha1", "142df4277c326f3549520478c188cab6e3b5d042");
  CheckSum file_md5("md5", "f139a810b84d82d1f29fc53c5e59beae");

  BOOST_CHECK_EQUAL( checksum( file.path(), "sha1"), "142df4277c326f3549520478c188cab6e3b5d042" );
  BOOST_CHECK_EQUAL( checksum( file.path(), "md5"), "f139a810b84d82d1f29fc53c5e59beae" );

  BOOST_REQUIRE( is_checksum( file.path(), file_sha1 ) );
  BOOST_REQUIRE( is_checksum( file.path(), file_md5 ) );
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "PathInfoTest" );
    test->add( BOOST_TEST_CASE( &pathinfo_checksum_test ), 0 /* expected zero error */ );
    return test;
}

