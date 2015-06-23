//
// tests for Flags
//
#include <boost/test/auto_unit_test.hpp>
#include "zypp/base/Logger.h"
#include "zypp/base/Flags.h"
#include <iostream>

using boost::unit_test::test_case;
using std::cout;
using std::endl;
using namespace zypp;

enum TBits
{
  T_0	= 0,
  T_1	= 1 << 0,
  T_2	= 1 << 1,
  T_3	= T_2 | T_1,
  T_4	= 1 << 2,
  T_5	= T_4 | T_1,
  T_8	= 1 << 3,
};

ZYPP_DECLARE_FLAGS( TFlags, TBits );
ZYPP_DECLARE_OPERATORS_FOR_FLAGS( TFlags );

static const TFlags T_6( T_4 | T_2 );
static const TFlags T_7( T_4 | T_2 | T_1 );

BOOST_AUTO_TEST_CASE(flags)
{
  TFlags t0;
  BOOST_CHECK_EQUAL( t0, 0 );
  BOOST_CHECK_EQUAL( t0, T_0 );
  BOOST_CHECK_EQUAL( t0, TFlags(0) );
  BOOST_CHECK_EQUAL( ~t0, ~0 );
  BOOST_CHECK_EQUAL( ~~t0, 0 );
  BOOST_CHECK_EQUAL( ~t0, ~T_0 );
  BOOST_CHECK_EQUAL( ~~t0, T_0 );

  TFlags t1( T_1 );
  BOOST_CHECK_EQUAL( t1, 1 );
  BOOST_CHECK_EQUAL( t1, T_1 );
  BOOST_CHECK_EQUAL( t1, TFlags(1) );

  TFlags t;
  // t = 1;	// must not compile: assign from int_type
  t = T_2;	// = enum
  BOOST_CHECK_EQUAL( t, T_2 );
  t = T_6;	// = TFlags
  BOOST_CHECK_EQUAL( t, T_6 );

  // enum op enum
  t = ~T_1;	 BOOST_CHECK_EQUAL( ~t, T_1 );
  t = T_1 & T_2; BOOST_CHECK_EQUAL( t, T_0 );
  t = T_1 | T_2; BOOST_CHECK_EQUAL( t, T_3 );
  t = T_1 ^ T_2; BOOST_CHECK_EQUAL( t, T_3 );

  // enum op TFlags
  t = T_2 & T_6; BOOST_CHECK_EQUAL( t, T_2 );
  t = T_2 | T_6; BOOST_CHECK_EQUAL( t, T_6 );
  t = T_2 ^ T_6; BOOST_CHECK_EQUAL( t, T_4 );

  // TFlags op enum
  t = ~T_7;	 BOOST_CHECK_EQUAL( ~t, T_7 );
  t = T_7 & T_2; BOOST_CHECK_EQUAL( t, T_2 );
  t = T_7 | T_2; BOOST_CHECK_EQUAL( t, T_7 );
  t = T_7 ^ T_2; BOOST_CHECK_EQUAL( t, T_5 );

  // TFlags op enum
  t = T_7 & T_6; BOOST_CHECK_EQUAL( t, T_6 );
  t = T_7 | T_6; BOOST_CHECK_EQUAL( t, T_7 );
  t = T_7 ^ T_7; BOOST_CHECK_EQUAL( t, T_0 );


  t = T_3;
  BOOST_CHECK( ! t.testFlag( T_0 ) );	// fails as T_3 != 0
  BOOST_CHECK( t.testFlag( T_1 ) );
  BOOST_CHECK( t.testFlag( T_2 ) );
  BOOST_CHECK( t.testFlag( T_3 ) );
  t.unsetFlag( T_2 ); 		BOOST_CHECK( t.testFlag( T_1 ) );
  t.setFlag( T_1, false );	BOOST_CHECK( t.testFlag( T_0 ) );		// succeed as T_3 == 0
  t.setFlag( T_3, true );	BOOST_CHECK( t.testFlag( T_3 ) );
}
