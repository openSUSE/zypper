#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/LogTools.h"
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
    BOOST_CHECK_EQUAL(str::numstring(42),     "42");
    BOOST_CHECK_EQUAL(str::numstring(42, 6),  "    42");
    BOOST_CHECK_EQUAL(str::numstring(42, -6), "42    ");

    BOOST_CHECK_EQUAL(str::hexstring(42),     "0x0000002a");
    BOOST_CHECK_EQUAL(str::hexstring(42, 6),  "0x002a");
    BOOST_CHECK_EQUAL(str::hexstring(42, -6), "0x2a  ");

    BOOST_CHECK_EQUAL(str::octstring(42),     "00052");
    BOOST_CHECK_EQUAL(str::octstring(42, 6),  "000052");
    BOOST_CHECK_EQUAL(str::octstring(42, -6), "052   ");

    BOOST_CHECK_EQUAL(str::strtonum<int>("42"), 42);

    BOOST_CHECK_EQUAL(str::toLower("This IS A TeST"), "this is a test");
    BOOST_CHECK_EQUAL(str::toUpper("This IS A TeST"), "THIS IS A TEST");
    BOOST_CHECK_EQUAL(str::compareCI("TeST", "test"), 0);
}

BOOST_AUTO_TEST_CASE(operations)
{
    BOOST_CHECK_EQUAL(str::ltrim(" \t f \t ffo \t "), "f \t ffo \t ");
    BOOST_CHECK_EQUAL(str::rtrim(" \t f \t ffo \t "), " \t f \t ffo");
    BOOST_CHECK_EQUAL(str::trim(" \t f \t ffo \t "),  "f \t ffo");

    // strip first
    {
        string tostrip(" Oh! la la ");
        string word( str::stripFirstWord(tostrip, true) ); // ltrim first
        BOOST_CHECK_EQUAL(word, "Oh!");
        BOOST_CHECK_EQUAL(tostrip, "la la ");
    }
    {
        string tostrip(" Oh! la la ");
        string word( str::stripFirstWord(tostrip, false) ); // no ltrim first
        BOOST_CHECK_EQUAL(word, "");
        BOOST_CHECK_EQUAL(tostrip, "Oh! la la ");
    }

    // strip last
    {
        string tostrip(" Oh! la la ");
        string word( str::stripLastWord(tostrip, true) ); // rtrim first
        BOOST_CHECK_EQUAL(word, "la");
        BOOST_CHECK_EQUAL(tostrip, " Oh! la");
    }
    {
        string tostrip(" Oh! la la ");
        string word( str::stripLastWord(tostrip, false) ); // no rtrim first
        BOOST_CHECK_EQUAL(word, "");
        BOOST_CHECK_EQUAL(tostrip, " Oh! la la");
    }
}

BOOST_AUTO_TEST_CASE(prefix_suffix)
{
  BOOST_CHECK( str::hasPrefix("abcXabcYabc", "abcX") );
  BOOST_CHECK( str::hasSuffix("abcXabcYabc", "Yabc") );

  BOOST_CHECK_EQUAL( str::stripPrefix("abcXabcYabc", "abcX"),  "abcYabc" );
  BOOST_CHECK_EQUAL( str::stripSuffix("abcXabcYabc", "Yabc"),  "abcXabc" );

  BOOST_CHECK( ! str::hasPrefix("abcXabcYabc", "ac") );
  BOOST_CHECK( ! str::hasSuffix("abcXabcYabc", "ac") );

  BOOST_CHECK_EQUAL( str::stripPrefix("abcXabcYabc", "ac"),  "abcXabcYabc" );
  BOOST_CHECK_EQUAL( str::stripSuffix("abcXabcYabc", "ac"),  "abcXabcYabc" );

  BOOST_CHECK( str::startsWith("abcXabcYabc", "abc") );
  BOOST_CHECK( str::endsWith("abcXabcYabc", "abc") );

  BOOST_CHECK( str::contains("abcXabcYabc", "XabcY") );
  BOOST_CHECK( ! str::contains("abcXabcYabc", "xabcy") );
  BOOST_CHECK( str::containsCI("abcXabcYabc", "xabcy") );
}
