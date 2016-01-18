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
#include <boost/test/auto_unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

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
BOOST_AUTO_TEST_CASE(arch_test)
{
  //////////////////////////////////////////////////////////////////////
  //
  //////////////////////////////////////////////////////////////////////
  BOOST_REQUIRE( Arch()   == Arch_noarch );
  BOOST_REQUIRE( Arch("") == Arch_empty );
  BOOST_REQUIRE( ! Arch_noarch.empty() );
  BOOST_REQUIRE( Arch_empty.empty() );
  BOOST_REQUIRE( Arch_noarch.isBuiltIn() );
  BOOST_REQUIRE( ! Arch_empty.isBuiltIn() );
  BOOST_REQUIRE( Arch_empty != Arch_noarch );
  //////////////////////////////////////////////////////////////////////
  //
  //////////////////////////////////////////////////////////////////////
  BOOST_CHECK_EQUAL( Arch("i386"), Arch_i386 );
  BOOST_CHECK_EQUAL( Arch("i386").idStr(), "i386" );
  BOOST_CHECK_EQUAL( Arch("i386").asString(), "i386" );
  BOOST_REQUIRE( Arch_i386.isBuiltIn() );

  BOOST_CHECK_EQUAL( Arch("FOO").idStr(), "FOO" );
  BOOST_CHECK_EQUAL( Arch("FOO").asString(), "FOO" );
  BOOST_REQUIRE( ! Arch("FOO").isBuiltIn() );
  //////////////////////////////////////////////////////////////////////
  //
  //////////////////////////////////////////////////////////////////////
  BOOST_REQUIRE( Arch_noarch.compatibleWith( Arch_noarch ) );
  BOOST_REQUIRE( Arch_noarch.compatibleWith( Arch_i386 ) );
  BOOST_REQUIRE( Arch_noarch.compatibleWith( Arch_x86_64 ) );

  BOOST_REQUIRE( ! Arch_i386.compatibleWith( Arch_noarch ) );
  BOOST_REQUIRE( Arch_i386.compatibleWith( Arch_i386 ) );
  BOOST_REQUIRE( Arch_i386.compatibleWith( Arch_x86_64 ) );

  BOOST_REQUIRE( ! Arch_x86_64.compatibleWith( Arch_noarch ) );
  BOOST_REQUIRE( ! Arch_x86_64.compatibleWith( Arch_i386 ) );
  BOOST_REQUIRE( Arch_x86_64.compatibleWith( Arch_x86_64 ) );
  //////////////////////////////////////////////////////////////////////
  //
  //////////////////////////////////////////////////////////////////////
  BOOST_CHECK_EQUAL( Arch::baseArch( Arch_x86_64 ), Arch_x86_64 );
  BOOST_CHECK_EQUAL( Arch::baseArch( Arch_i686 ), Arch_i386 );
  //////////////////////////////////////////////////////////////////////
  //
  //////////////////////////////////////////////////////////////////////
  BOOST_REQUIRE( Arch_i386.compare( Arch_noarch ) >  0 );
  BOOST_REQUIRE( Arch_i386.compare( Arch_i386 )   == 0 );
  BOOST_REQUIRE( Arch_i386.compare( Arch_x86_64 ) <  0 );
}
