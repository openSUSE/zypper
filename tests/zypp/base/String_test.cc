#include <string>
#include <vector>
#include <iterator>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/String.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using namespace boost::unit_test;

using namespace std;
using namespace zypp;
using namespace zypp::str;

BOOST_AUTO_TEST_CASE(testsplitEscaped)
{
  string s( "simple non-escaped string" );
  vector<string> v;

  insert_iterator<vector<string> > ii (v,v.end());
  splitEscaped( s, ii );
  BOOST_CHECK_EQUAL( v.size(), 3 );

  v.clear();
  s = string( "\"escaped sentence \"" );
  ii = insert_iterator<vector<string> >( v, v.end() );
  splitEscaped( s, ii );
  BOOST_CHECK_EQUAL( v.size(), 1 );
  BOOST_CHECK_EQUAL( v.front(), string( "escaped sentence " ) ); 

   v.clear();
   s = string( "\"escaped \\\\sent\\\"ence \\\\\"" );
   ii = insert_iterator<vector<string> >( v, v.end() );
   splitEscaped( s, ii );
   BOOST_CHECK_EQUAL( v.size(), 1 );
   BOOST_CHECK_EQUAL( v.front(), string( "escaped \\sent\"ence \\" ) );

  
   v.clear();
   s = string( "escaped sentence\\ with\\ space" );
   ii = insert_iterator<vector<string> >( v, v.end() );
   splitEscaped( s, ii );
   BOOST_CHECK_EQUAL( v.size(), 2 );
   BOOST_CHECK_EQUAL( v[1], string( "sentence with space" ) );
}
