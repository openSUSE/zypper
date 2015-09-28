#include <set>
#include <boost/test/auto_unit_test.hpp>
#include "zypp/base/LogTools.h"
#include "zypp/base/SetTracker.h"

typedef std::set<int> SetType;
namespace std
{
  inline ostream & operator<<( ostream & str, const SetType & obj )
  { return zypp::dumpRangeLine( str, obj.begin(), obj.end() );  }
}
typedef zypp::base::SetTracker<SetType> Tracker;

std::set<int> s;
std::set<int> s1	({1});
std::set<int> s2	({2});
std::set<int> s3	({3});
std::set<int> s12	({1,2});
std::set<int> s13	({1,3});
std::set<int> s23	({2,3});
std::set<int> s123	({1,2,3});


BOOST_AUTO_TEST_CASE(basic)
{
  Tracker t;
  BOOST_CHECK_EQUAL( t.current(),	s	);
  BOOST_CHECK_EQUAL( t.added(),		s	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  BOOST_CHECK( t.add( 1 ) );
  BOOST_CHECK_EQUAL( t.current(),	s1	);
  BOOST_CHECK_EQUAL( t.added(),		s1	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  BOOST_CHECK( !t.add( 1 ) );
  BOOST_CHECK_EQUAL( t.current(),	s1	);
  BOOST_CHECK_EQUAL( t.added(),		s1	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  BOOST_CHECK( t.add( 2 ) );
  BOOST_CHECK_EQUAL( t.current(),	s12	);
  BOOST_CHECK_EQUAL( t.added(),		s12	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  BOOST_CHECK( t.remove( 1 ) );
  BOOST_CHECK_EQUAL( t.current(),	s2	);
  BOOST_CHECK_EQUAL( t.added(),		s2	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  BOOST_CHECK( t.set( s3 ) );
  BOOST_CHECK_EQUAL( t.current(),	s3	);
  BOOST_CHECK_EQUAL( t.added(),		s3	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  BOOST_CHECK( t.add( 2 ) );
  BOOST_CHECK_EQUAL( t.current(),	s23	);
  BOOST_CHECK_EQUAL( t.added(),		s23	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  BOOST_CHECK( t.remove( 2 ) );
  BOOST_CHECK_EQUAL( t.current(),	s3	);
  BOOST_CHECK_EQUAL( t.added(),		s3	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  BOOST_CHECK( ! t.remove( 2 ) );
  BOOST_CHECK_EQUAL( t.current(),	s3	);
  BOOST_CHECK_EQUAL( t.added(),		s3	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  BOOST_CHECK( t.set( s ) );
  BOOST_CHECK_EQUAL( t.current(),	s	);
  BOOST_CHECK_EQUAL( t.added(),		s	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  //----------------------------------------------------------------------

  BOOST_CHECK( t.setInitial( s2 ) );
  BOOST_CHECK_EQUAL( t.current(),	s2	);
  BOOST_CHECK_EQUAL( t.added(),		s	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  BOOST_CHECK( t.set( s13 ) );
  BOOST_CHECK_EQUAL( t.current(),	s13	);
  BOOST_CHECK_EQUAL( t.added(),		s13	);
  BOOST_CHECK_EQUAL( t.removed(),	s2	);

  BOOST_CHECK( t.set( s123 ) );
  BOOST_CHECK_EQUAL( t.current(),	s123	);
  BOOST_CHECK_EQUAL( t.added(),		s13	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

  BOOST_CHECK( t.set( s ) );
  BOOST_CHECK_EQUAL( t.current(),	s	);
  BOOST_CHECK_EQUAL( t.added(),		s	);
  BOOST_CHECK_EQUAL( t.removed(),	s2	);

  BOOST_CHECK( t.set( s2 ) );
  BOOST_CHECK_EQUAL( t.current(),	s2	);
  BOOST_CHECK_EQUAL( t.added(),		s	);
  BOOST_CHECK_EQUAL( t.removed(),	s	);

}
