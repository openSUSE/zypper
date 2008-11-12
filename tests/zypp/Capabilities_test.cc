// Capabilities.cc
//
// tests for Capabilities
//
#include <iostream>
#include <string>

// Boost.Test
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/Arch.h"
#include "zypp/Capability.h"
#include "zypp/Capabilities.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using boost::test_tools::close_at_tolerance;

using namespace std;
using namespace zypp;

BOOST_AUTO_TEST_CASE(capabilities_test)
{
  //////////////////////////////////////////////////////////////////////
  // Id 0 and 1 are nor equal, but share the same representation ""/NOCAP
  //////////////////////////////////////////////////////////////////////

  Capability c0( 0 );  // id 0
  Capability c1( 1 );  // id 1
  Capability cD;       // default constructed empty
  Capability cE( "" ); // empty

  BOOST_CHECK_EQUAL( c0.id(), 0 );
  BOOST_CHECK_EQUAL( c1.id(), 1 );
  BOOST_CHECK_EQUAL( Capability().id(), 1 );   // default constructed empty
  BOOST_CHECK_EQUAL( Capability("").id(), 1 ); // empty

  BOOST_CHECK_EQUAL( c0.asString(), "" );
  BOOST_CHECK_EQUAL( c1.asString(), "" );

  BOOST_CHECK_EQUAL( c0.empty(), true );
  BOOST_CHECK_EQUAL( c1.empty(), true );

  BOOST_CHECK_EQUAL( c0.detail().kind(), CapDetail::NOCAP );
  BOOST_CHECK_EQUAL( c1.detail().kind(), CapDetail::NOCAP );

  BOOST_CHECK_EQUAL( ( c0 == c1 ), false );
  BOOST_CHECK_EQUAL( Capability::matches( c0, c1 ), CapMatch::yes );

  //////////////////////////////////////////////////////////////////////
  // skipping internal marker in Capabilities
  //////////////////////////////////////////////////////////////////////

  Capability r( "req" );
  Capability p( "prereq" );

  sat::detail::IdType caps[10];
  caps[0] = r.id();
  caps[1] = sat::detail::solvablePrereqMarker;
  caps[2] = p.id();
  caps[3] = 0;

  // Capabilities with and without prereq (skip marker in ++)
  Capabilities c( caps );
  //cout << c << endl;
  BOOST_CHECK_EQUAL( c.size(), 2 );
  Capabilities::const_iterator it( c.begin() );
  BOOST_CHECK_EQUAL( *it, r );
  BOOST_CHECK_EQUAL( it.tagged(), false );
  ++it;
  BOOST_CHECK_EQUAL( *it, p );
  BOOST_CHECK_EQUAL( it.tagged(), true );

  // Capabilities with prereq only (skip marker in ctor)
  c = Capabilities( caps+1 );
  //cout << c << endl;
  BOOST_CHECK_EQUAL( c.size(), 1 );
  it = c.begin();
  BOOST_CHECK_EQUAL( *it, p );
  BOOST_CHECK_EQUAL( it.tagged(), true );


  //////////////////////////////////////////////////////////////////////
  //
  //////////////////////////////////////////////////////////////////////
  Capability n( "na.me" );
  Capability na( "na.me.i386" );
  Capability noe( "na.me == 1" );
  Capability naoe( "na.me.i386 == 1" );

  BOOST_CHECK_EQUAL( n.detail().kind(), CapDetail::NAMED );
  BOOST_CHECK_EQUAL( na.detail().kind(), CapDetail::NAMED );
  BOOST_CHECK_EQUAL( noe.detail().kind(), CapDetail::VERSIONED );
  BOOST_CHECK_EQUAL( naoe.detail().kind(), CapDetail::VERSIONED );

  BOOST_CHECK_EQUAL( n.detail().hasArch(), false );
  BOOST_CHECK_EQUAL( na.detail().hasArch(), true );
  BOOST_CHECK_EQUAL( noe.detail().hasArch(), false );
  BOOST_CHECK_EQUAL( naoe.detail().hasArch(), true );

  BOOST_CHECK      ( n.detail().arch().empty() );
  BOOST_CHECK_EQUAL( na.detail().arch(), Arch_i386.idStr() );
  BOOST_CHECK      ( noe.detail().arch().empty() );
  BOOST_CHECK_EQUAL( naoe.detail().arch(), Arch_i386.idStr() );

  BOOST_CHECK_EQUAL( Capability( "",     "na.me", "",   "" ), n );
  BOOST_CHECK_EQUAL( Capability( "i386", "na.me", "",   "" ), na );
  BOOST_CHECK_EQUAL( Capability( "",     "na.me", "==", "1" ), noe );
  BOOST_CHECK_EQUAL( Capability( "i386", "na.me", "==", "1" ), naoe );

  // explicit arch
  BOOST_CHECK_EQUAL( Capability( Arch_i386, "na.me" ), na );
  BOOST_CHECK_EQUAL( Capability( Arch_i386, "na.me == 1" ), naoe );
}

