#include "TestSetup.h"
#include <zypp/sat/LookupAttr.h>

BOOST_AUTO_TEST_CASE(idstring)
{
  // id 0 ==> NULL
  // id 1 ==> ""
  // evaluates id in a boolean context
  IdString a0( 0 );
  IdString a1( 1 );

  BOOST_CHECK_EQUAL( a0.id(), 0 );
  BOOST_CHECK_EQUAL( a1.id(), 1 );

  BOOST_CHECK( !a0 );
  BOOST_CHECK( a1 );

  BOOST_CHECK( !(a0 == a1) );
  BOOST_CHECK(  (a0 != a1) );
  BOOST_CHECK(  (a0 < a1) );
  BOOST_CHECK(  (a1 > a0) );
  BOOST_CHECK( !(a0 >= a1) );
  BOOST_CHECK( !(a1 <= a0) );

  BOOST_CHECK_EQUAL( a0.compare( (const char *)0 ), 0 );
  BOOST_CHECK_EQUAL( a0.compare( "" ), -1 );
  BOOST_CHECK_EQUAL( a0.compare( a1 ), -1 );

  BOOST_CHECK_EQUAL( a1.compare( (const char *)0 ), 1 );
  BOOST_CHECK_EQUAL( a1.compare( "" ), 0 );
  BOOST_CHECK_EQUAL( a1.compare( a0 ), 1 );
}

BOOST_AUTO_TEST_CASE(idstringtype)
{
  sat::SolvAttr a0( sat::SolvAttr::allAttr );
  sat::SolvAttr a1( sat::SolvAttr::noAttr );

  BOOST_CHECK_EQUAL( a0.id(), 0 );
  BOOST_CHECK_EQUAL( a1.id(), 1 );

  BOOST_CHECK( !a0 );
  BOOST_CHECK( !a1 ); // evaluates empty string (id 0/1) in a boolean context

  BOOST_CHECK( !(a0 == a1) );
  BOOST_CHECK(  (a0 != a1) );
  BOOST_CHECK(  (a0 < a1) );
  BOOST_CHECK(  (a1 > a0) );
  BOOST_CHECK( !(a0 >= a1) );
  BOOST_CHECK( !(a1 <= a0) );

  BOOST_CHECK_EQUAL( a0.compare( (const char *)0 ), 0 );
  BOOST_CHECK_EQUAL( a0.compare( "" ), -1 );
  BOOST_CHECK_EQUAL( a0.compare( a1 ), -1 );

  BOOST_CHECK_EQUAL( a1.compare( (const char *)0 ), 1 );
  BOOST_CHECK_EQUAL( a1.compare( "" ), 0 );
  BOOST_CHECK_EQUAL( a1.compare( a0 ), 1 );
}
