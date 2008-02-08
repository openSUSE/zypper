// Arch.cc
//
// tests for Arch
//

#include <iostream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/Arch.h"

// Boost.Test
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using boost::test_tools::close_at_tolerance;

using namespace std;
using namespace zypp;

/******************************************************************
**
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
**
**      DESCRIPTION :
*/
void arch_test()
{
  Arch _arch32( "i386" );

  BOOST_CHECK_EQUAL( _arch32, Arch_i386 );
  BOOST_CHECK_EQUAL( _arch32.asString(), string("i386"));
  BOOST_REQUIRE( _arch32.compatibleWith (Arch_x86_64));
  BOOST_CHECK_THROW( Arch(NULL), exception);
  BOOST_CHECK_EQUAL( Arch(), Arch_noarch );
  BOOST_REQUIRE( Arch("") != Arch_noarch );
  BOOST_REQUIRE( Arch("").empty() );
  BOOST_REQUIRE( ! Arch_noarch.empty() );
  BOOST_REQUIRE( ! ( _arch32.compare(Arch_x86_64) >= 0) );
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "ArchTest" );
    test->add( BOOST_TEST_CASE( &arch_test ), 0 /* expected zero error */ );
    return test;
}
