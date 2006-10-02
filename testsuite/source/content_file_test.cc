#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"

#include <zypp/source/susetags/ProductMetadataParser.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

#include <string>
#include <list>
#include <iostream>

#include <boost/test/unit_test.hpp>

using namespace zypp;
using namespace zypp::source;
using namespace zypp::source::susetags;

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

void replace_test()
{
 ProductMetadataParser p;
  p.parse( "data/products/content_toreplace.txt", Source_Ref::noSource );
  Arch sysarch( getZYpp()->architecture() );
  BOOST_REQUIRE( p.result->releaseNotesUrl().asString() == ("http://www.suse.com/relnotes/" + sysarch.asString() + "/SUSE-SLES/9/release-notes.rpm") );
  MIL << p.result->releaseNotesUrl().asString() << std::endl;
}

test_suite*
init_unit_test_suite( int, char* [] )
{
  test_suite* test= BOOST_TEST_SUITE( "ContentFileTest" );
  test->add( BOOST_TEST_CASE( &replace_test ), 0 /* expected zero error */ );
  return test;
}

// vim: set ts=2 sts=2 sw=2 ai et:
