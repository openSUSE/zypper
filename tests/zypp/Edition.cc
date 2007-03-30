// Edition.cc
//
// tests for Edition
//

#include "zypp/base/Logger.h"
#include "zypp/Edition.h"

#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;

void edition_test()
{
  BOOST_CHECK_THROW( Edition(string("A::foo--foo")), exception );
  
  Edition _ed1 ("1");
  Edition _ed2 ("1.1");
  Edition _ed3 ("1:1");
  Edition _ed4 ("1:1-1");

  BOOST_CHECK_EQUAL(_ed2.version(), "1.1");
  BOOST_CHECK_EQUAL(_ed2.release(), "");
  BOOST_CHECK_EQUAL(_ed2.epoch(), 0);
  BOOST_CHECK_EQUAL(_ed4.epoch(), 1);

  BOOST_CHECK_EQUAL(_ed1, Edition ("1", ""));
  BOOST_CHECK_EQUAL(_ed2, Edition ("1.1", ""));
  BOOST_CHECK_EQUAL(_ed3, Edition ("1", "", "1"));
  BOOST_CHECK_EQUAL(_ed3, Edition ("1", "", 1));
  BOOST_CHECK_EQUAL(_ed4, Edition ("1", "1", 1));
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "EditionTest" );
    test->add( BOOST_TEST_CASE( &edition_test ), 0 /* expected zero error */ );
    return test;
}