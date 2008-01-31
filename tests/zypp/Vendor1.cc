
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

#include "zypp/base/LogControl.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/ZYpp.h"
#include "zypp/VendorAttr.h"

using namespace std;
using namespace zypp;


// most frequently you implement test cases as a free functions
void vendor_test()
{
  // No vendor definition files has been readed. So only suse,opensuse vendors are
  // equivalent
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "suse") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("equal", "equal") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "SuSE") ); 
  BOOST_REQUIRE( VendorAttr::instance().equivalent("opensuse", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("open", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("nothing", "SuSE") );
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    setenv("ZYPP_CONF", "./../../tests/zypp/data/Vendor/zypp1.conf", 1 );
    zypp::base::LogControl::instance().logfile( "-" );    
    test_suite* test= BOOST_TEST_SUITE( "VendorTest" );
    test->add( BOOST_TEST_CASE( &vendor_test ), 5 /* expected zero error */ , 0);
    return test;
}

