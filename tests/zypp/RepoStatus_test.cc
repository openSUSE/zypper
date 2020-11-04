
#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>
#include <zypp/TmpPath.h>
#include <zypp/RepoStatus.h>
#include <zypp/PathInfo.h>

#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace zypp;
using namespace zypp::filesystem;

BOOST_AUTO_TEST_CASE(repostatus_test)
{
  RepoStatus e;
  RepoStatus E { "", 42 };	// empty refers to the checksum only, not to the timestamp!
  RepoStatus a { "aa", 0 };
  RepoStatus b { "bb", 1 };
  RepoStatus c { "cc", 2 };

  BOOST_CHECK_EQUAL( e.empty(), true );
  BOOST_CHECK_EQUAL( e.timestamp(), 0 );
  BOOST_CHECK_EQUAL( (e && e).empty(), true );

  BOOST_CHECK_EQUAL( E.empty(), true );
  BOOST_CHECK_EQUAL( E.timestamp(), 42 );
  RepoStatus r { E && e };
  BOOST_CHECK_EQUAL( r.empty(), true );
  BOOST_CHECK_EQUAL( r.timestamp(), 42 );
  r = e && E;
  BOOST_CHECK_EQUAL( r.empty(), true );
  BOOST_CHECK_EQUAL( r.timestamp(), 42 );
  r = E && E;
  BOOST_CHECK_EQUAL( r.empty(), true );
  BOOST_CHECK_EQUAL( r.timestamp(), 42 );


  BOOST_CHECK_EQUAL( a.empty(), false );
  BOOST_CHECK_EQUAL( a.timestamp(), 0 );

  r = e && a;
  BOOST_CHECK_EQUAL( r.empty(), false );
  BOOST_CHECK_EQUAL( r.timestamp(), a.timestamp() );	// max timestamp

  r = a && b;
  BOOST_CHECK_EQUAL( r, (b && a) );
  BOOST_CHECK_EQUAL( r.timestamp(), b.timestamp() );	// max timestamp

  r = a && b && c;
  BOOST_CHECK_EQUAL( r, (a && b) && c );
  BOOST_CHECK_EQUAL( r, a && (b && c) );
  BOOST_CHECK_EQUAL( r.timestamp(), c.timestamp() );	// max timestamp
}
