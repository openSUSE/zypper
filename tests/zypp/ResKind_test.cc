#include <boost/test/auto_unit_test.hpp>
#include "zypp/base/Logger.h"
#include "zypp/ResKind.h"

using boost::unit_test::test_case;
using namespace std;
using namespace zypp;

BOOST_AUTO_TEST_CASE(reskind_test)
{
  // Default construced is empty ""
  BOOST_CHECK_EQUAL( ResKind(), "" );
  // boolean context
  BOOST_CHECK( ! ResKind() );
  BOOST_CHECK( ! ResKind(0) );  // id NULL
  BOOST_CHECK( ! ResKind(1) );  // id ""
  BOOST_CHECK( ! ResKind("") ); // ""
  BOOST_CHECK( ResKind(2) );
  BOOST_CHECK( ResKind("FOO") );
  // Internal representation is lowercased
  BOOST_CHECK_EQUAL( ResKind("FOO").asString(), "foo" );
  // Caseinsensitive comparison
  BOOST_CHECK_EQUAL( ResKind("FOO"), ResKind("foo") );
  BOOST_CHECK_EQUAL( ResKind("FOO"), string("Foo") );
  BOOST_CHECK_EQUAL( ResKind("FOO"), "Foo" );
  BOOST_CHECK_EQUAL( ResKind("FOO"), string("foo") );
  BOOST_CHECK_EQUAL( ResKind("FOO"), "foo" );
  BOOST_CHECK_EQUAL( string("foo"), ResKind("FOO") );
  BOOST_CHECK_EQUAL( "foo", ResKind("FOO") );

  BOOST_CHECK_EQUAL( ResKind::compare( "FOO", "foo" ), 0 );

}
