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

enum class E
{
  _0	= 0,
  _1	= 1 << 0,
  _2	= 1 << 1,
  _3	= _2 | _1,
  _4	= 1 << 2,
  _5	= _4 | _1,
  _8	= 1 << 3,
};

ZYPP_DECLARE_FLAGS( TFlags, E );
ZYPP_DECLARE_OPERATORS_FOR_FLAGS( TFlags );

inline std::ostream & operator<<( std::ostream & str, const E & obj )
{ return str << TFlags(obj); }

static const TFlags T_6( E::_4 | E::_2 );
static const TFlags T_7( E::_4 | E::_2 | E::_1 );


BOOST_AUTO_TEST_CASE(flags)
{
  TFlags t0;
  BOOST_CHECK_EQUAL( t0, 0 );
  BOOST_CHECK_EQUAL( E::_0, t0 );
  BOOST_CHECK_EQUAL( t0, E::_0 );
  BOOST_CHECK_EQUAL( t0, TFlags(0) );
  BOOST_CHECK_EQUAL( ~t0, ~0 );
  BOOST_CHECK_EQUAL( ~~t0, 0 );
  BOOST_CHECK_EQUAL( ~t0, ~E::_0 );
  BOOST_CHECK_EQUAL( ~~t0, E::_0 );

  BOOST_CHECK_EQUAL( TFlags::none(), TFlags(0) );
  BOOST_CHECK_EQUAL( TFlags::all(), ~TFlags(0) );

  TFlags t1( E::_1 );
  BOOST_CHECK_EQUAL( t1, 1 );
  BOOST_CHECK_EQUAL( t1, E::_1 );
  BOOST_CHECK_EQUAL( t1, TFlags(1) );

  TFlags t;
  //t = 1;	// must not compile: assign from int_type
  t = E::_2;	// = enum
  BOOST_CHECK_EQUAL( t, E::_2 );
  t = T_6;	// = TFlags
  BOOST_CHECK_EQUAL( t, T_6 );

  // enum op enum
  t = ~E::_1;		BOOST_CHECK_EQUAL( ~t, E::_1 );
  t = E::_1 & E::_2;	BOOST_CHECK_EQUAL( t, E::_0 );
  t = E::_1 | E::_2;	BOOST_CHECK_EQUAL( t, E::_3 );
  t = E::_1 ^ E::_2;	BOOST_CHECK_EQUAL( t, E::_3 );

  // enum op TFlags
  t = E::_2 & T_6;	BOOST_CHECK_EQUAL( t, E::_2 );
  t = E::_2 | T_6;	BOOST_CHECK_EQUAL( t, T_6 );
  t = E::_2 ^ T_6;	BOOST_CHECK_EQUAL( t, E::_4 );

  // TFlags op enum
  t = ~T_7;		BOOST_CHECK_EQUAL( ~t, T_7 );
  t = T_7 & E::_2;	BOOST_CHECK_EQUAL( t, E::_2 );
  t = T_7 | E::_2;	BOOST_CHECK_EQUAL( t, T_7 );
  t = T_7 ^ E::_2;	BOOST_CHECK_EQUAL( t, E::_5 );

  // TFlags op TFlags
  t = T_7 & T_6;	BOOST_CHECK_EQUAL( t, T_6 );
  t = T_7 | T_6;	BOOST_CHECK_EQUAL( t, T_7 );
  t = T_7 ^ T_7;	BOOST_CHECK_EQUAL( t, E::_0 );

  t = E::_3;
  BOOST_CHECK( ! t.testFlag( E::_0 ) );	// fails as t != 0
  BOOST_CHECK( t.testFlag( E::_1 ) );
  BOOST_CHECK( t.testFlag( E::_2 ) );
  BOOST_CHECK( t.testFlag( E::_3 ) );
  t.unsetFlag( E::_2 );		BOOST_CHECK( t.testFlag( E::_1 ) );
  t.setFlag( E::_1, false );	BOOST_CHECK( t.testFlag( E::_0 ) );		// succeed as t == 0
  t.setFlag( E::_3, true );	BOOST_CHECK( t.testFlag( E::_3 ) );
}
