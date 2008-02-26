// Capabilities.cc
//
// tests for Capabilities
//
#include <iostream>
#include <string>

// Boost.Test
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/Capability.h"
#include "zypp/Capabilities.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using boost::test_tools::close_at_tolerance;

using namespace std;
using namespace zypp;

BOOST_AUTO_TEST_CASE(capabilities_test)
{
//     Resolvable::Kind kind = ResTraits<zypp::Package>::kind;
//     CapFactory factory;
//
//     Edition edition ("1.0", "42");
//     Capability cap = factory.parse ( kind, "foo", "=", "1.0-42");
//     BOOST_CHECK_EQUAL( cap.asString(), "foo == 1.0-42" );
//     BOOST_CHECK_EQUAL( cap.index(), "foo");
//     BOOST_CHECK_EQUAL( cap.op(), Rel::EQ);
//     BOOST_CHECK_EQUAL( cap.edition(), edition);
//
//     Capability cap2 = factory.parse ( kind, "foo", Rel::EQ, edition);
//     BOOST_CHECK_EQUAL( cap2.index(), cap.index());
//     BOOST_CHECK_EQUAL( cap2.op(), cap.op());
//     BOOST_CHECK_EQUAL( cap2.edition(), cap.edition());
//
//     Capability cap3 = factory.parse ( kind, "foo = 1.0-42");
//     BOOST_CHECK_EQUAL( cap3.index(), cap.index());
//     BOOST_CHECK_EQUAL( cap3.op(), cap.op());
//     BOOST_CHECK_EQUAL( cap3.edition(), cap.edition());
//
//     Capability cap6 = factory.parse ( kind, "kdelibs* > 1.5");
//     BOOST_CHECK_EQUAL( cap6.index(), "kdelibs*");
//     BOOST_CHECK_EQUAL( cap6.op(), Rel::GT);
//     BOOST_CHECK_EQUAL( cap6.edition(), Edition("1.5"));
//
//
//     string bash = "/bin/bash";
//     Capability cap4 = factory.parse ( kind, bash);
//     BOOST_CHECK_EQUAL(cap4.index(), bash);
//     BOOST_CHECK_EQUAL(cap4.op(), Rel::NONE);
//     BOOST_CHECK_EQUAL(cap4.edition(), Edition::noedition);
//
//     string hal = "hal(smp)";
//     Capability cap5 = factory.parse ( kind, hal);
//     BOOST_CHECK_EQUAL(cap5.index(), "hal()");
//     BOOST_CHECK_EQUAL(cap5.op(), Rel::NONE);
//     BOOST_CHECK_EQUAL(cap5.edition(), Edition::noedition);

  Capability c0( 0 ); // id 0
  Capability c1( 1 ); // id 1
  Capability cD;      // default constructed empty
  Capability cE( "" );// empty

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


  Capability r( "req" );
  Capability p( "prereq" );

  sat::detail::IdType caps[10];
  caps[0] = r.id();
  caps[1] = sat::detail::solvablePrereqMarker;
  caps[2] = p.id();
  caps[3] = 0;

  // Capabilities with and without prereq (skip marker in ++)
  Capabilities c( caps );
  cout << c << endl;
  BOOST_CHECK_EQUAL( c.size(), 2 );
  Capabilities::const_iterator it( c.begin() );
  BOOST_CHECK_EQUAL( *it, r );
  BOOST_CHECK_EQUAL( it.tagged(), false );
  ++it;
  BOOST_CHECK_EQUAL( *it, p );
  BOOST_CHECK_EQUAL( it.tagged(), true );

  // Capabilities with prereq only (skip marker in ctor)
  c = Capabilities( caps+1 );
  cout << c << endl;
  BOOST_CHECK_EQUAL( c.size(), 1 );
  it = c.begin();
  BOOST_CHECK_EQUAL( *it, p );
  BOOST_CHECK_EQUAL( it.tagged(), true );

}

