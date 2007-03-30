// Capabilities.cc
//
// tests for Capabilities 
//
#include <string>

// Boost.Test
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/CapFactory.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using boost::test_tools::close_at_tolerance;

using namespace std;
using namespace zypp;

void capabilities_test()
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
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "CapabilitiesTest" );
    test->add( BOOST_TEST_CASE( &capabilities_test ), 0 /* expected zero error */ );
    return test;
}
