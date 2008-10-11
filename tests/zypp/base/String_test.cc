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

BOOST_AUTO_TEST_CASE(gsubTest)
{
  string olds = "olds";
  string news = "new string";

  BOOST_CHECK_EQUAL(gsub("test olds string",olds,news), "test new string string");
  BOOST_CHECK_EQUAL(gsub("no string",olds,news),"no string");
  BOOST_CHECK_EQUAL(gsub("oldsolds",olds,news),"new stringnew string");
}

BOOST_AUTO_TEST_CASE(replaceAllTest)
{
  string olds = "olds";
  string news = "new string";
  string tests;

  tests = "test olds string";
  replaceAll(tests,olds,news);
  BOOST_CHECK_EQUAL(tests, "test new string string");

  tests = "no string";
  replaceAll(tests,olds,news);
  BOOST_CHECK_EQUAL(tests, "no string");

  tests = "oldsolds";
  replaceAll(tests,olds,news);
  BOOST_CHECK_EQUAL(tests, "new stringnew string");
}

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

   // split - join
   v.clear();
   s = "some line \"\" foo\\ a foo\\\\ b";
   str::splitEscaped( s, std::back_inserter(v) );
   BOOST_CHECK_EQUAL( s, str::joinEscaped( v.begin(), v.end() ) );

   // split - join using alternate sepchar
   s = str::joinEscaped( v.begin(), v.end(), 'o' );
   v.clear();
   str::splitEscaped( s, std::back_inserter(v), "o" );
   BOOST_CHECK_EQUAL( s, str::joinEscaped( v.begin(), v.end(), 'o' ) );
}

BOOST_AUTO_TEST_CASE(test_escape)
{
  string badass = "bad|ass\\|worse";
  string escaped = str::escape(badass, '|');

  BOOST_CHECK_EQUAL( escaped, "bad\\|ass\\\\\\|worse" );
}

BOOST_AUTO_TEST_CASE(convertions)
{
    BOOST_CHECK_EQUAL(str::numstring(42), "42");
    BOOST_CHECK_EQUAL(str::numstring(42, 4), "  42");
    BOOST_CHECK_EQUAL(str::numstring(42, -4), "42  ");

    BOOST_CHECK_EQUAL(str::hexstring(42), "0x0000002a");
    BOOST_CHECK_EQUAL(str::hexstring(42, 4), "0x2a");
    BOOST_CHECK_EQUAL(str::hexstring(42, -4), "0x2a");

    BOOST_CHECK_EQUAL(str::octstring(42), "00052");
    //BOOST_CHECK_EQUAL(str::octstring(42, 4), "0052");
    //BOOST_CHECK_EQUAL(str::octstring(42, -4), "052");

    BOOST_CHECK_EQUAL(str::strtonum<int>("42"), 42);

    BOOST_CHECK_EQUAL(str::toLower("This IS A TeST"), "this is a test");
    BOOST_CHECK_EQUAL(str::toUpper("This IS A TeST"), "THIS IS A TEST");
    BOOST_CHECK_EQUAL(str::compareCI("TeST", "test"), 0);
}

BOOST_AUTO_TEST_CASE(operations)
{
    //BOOST_CHECK_EQUAL(str::ltrim("  foo "), "foo");
    //BOOST_CHECK_EQUAL(str::rtrim(" f ffo  "), "f ffo");
    BOOST_CHECK_EQUAL(str::trim(" f ffo  "), "f ffo");
    BOOST_CHECK_EQUAL(str::trim("  f ffo  "), "f ffo");

    // strip
    {
        string tostrip("Oh! la la");
        str::stripFirstWord(tostrip, false);        
        BOOST_CHECK_EQUAL(tostrip, "la la");
    }
    // strip
    {
        string tostrip("Oh! la la");
        str::stripLastWord(tostrip, false);        
        BOOST_CHECK_EQUAL(tostrip, "Oh! la");
    }

    BOOST_CHECK( ! str::hasPrefix("foolala", "oo"));
    BOOST_CHECK( str::hasPrefix("foolala", "foo"));


}

