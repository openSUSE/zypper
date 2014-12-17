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

  splitEscaped( s, std::back_inserter(v) );
  BOOST_CHECK_EQUAL( v.size(), 3 );
  BOOST_CHECK_EQUAL( v[0], "simple" );
  BOOST_CHECK_EQUAL( v[1], "non-escaped" );
  BOOST_CHECK_EQUAL( v[2], "string" );

  v.clear();
  s = string( "\"escaped sentence \"" );
  splitEscaped( s, std::back_inserter(v) );
  BOOST_CHECK_EQUAL( v.size(), 1 );
  BOOST_CHECK_EQUAL( v[0], "escaped sentence " );

  v.clear();
  s = string( "\"escaped \\\\sent\\\"ence \\\\\"" );
  splitEscaped( s, std::back_inserter(v) );
  BOOST_CHECK_EQUAL( v.size(), 1 );
  BOOST_CHECK_EQUAL( v[0], "escaped \\sent\"ence \\" );

  v.clear();
  s = string( "escaped sentence\\ with\\ space" );
  splitEscaped( s, std::back_inserter(v) );
  BOOST_CHECK_EQUAL( v.size(), 2 );
  BOOST_CHECK_EQUAL( v[0], "escaped" );
  BOOST_CHECK_EQUAL( v[1], "sentence with space" );

  // split - join
  v.clear();
  s = "some line \"\" foo\\ a foo\\\\ b";
  str::splitEscaped( s, std::back_inserter(v) );
  BOOST_CHECK_EQUAL( v.size(), 6 );
  BOOST_CHECK_EQUAL( v[0], "some" );
  BOOST_CHECK_EQUAL( v[1], "line" );
  BOOST_CHECK_EQUAL( v[2], "" );
  BOOST_CHECK_EQUAL( v[3], "foo a" );
  BOOST_CHECK_EQUAL( v[4], "foo\\" );
  BOOST_CHECK_EQUAL( v[5], "b" );
  BOOST_CHECK_EQUAL( s, str::joinEscaped( v.begin(), v.end() ) );

  // split - join using alternate sepchar
  s = str::joinEscaped( v.begin(), v.end(), 'o' );
  v.clear();
  str::splitEscaped( s, std::back_inserter(v), "o" );
  BOOST_CHECK_EQUAL( v.size(), 6 );
  BOOST_CHECK_EQUAL( v[0], "some" );
  BOOST_CHECK_EQUAL( v[1], "line" );
  BOOST_CHECK_EQUAL( v[2], "" );
  BOOST_CHECK_EQUAL( v[3], "foo a" );
  BOOST_CHECK_EQUAL( v[4], "foo\\" );
  BOOST_CHECK_EQUAL( v[5], "b" );
  BOOST_CHECK_EQUAL( s, str::joinEscaped( v.begin(), v.end(), 'o' ) );
}

BOOST_AUTO_TEST_CASE(bnc_909772)
{
  // While \-escaping processes single-quote, double-quote, backslash and sepchar[ ]
  // deescaping failed to process the quotes correctly.
  std::string s;
  std::vector<std::string> v;

  v.clear();
  v.push_back("");
  v.push_back("'\" \\");
  v.push_back("\\'\\\"\\ \\\\");
  s = str::joinEscaped( v.begin(), v.end() );
  BOOST_CHECK_EQUAL( s, "\"\""  " "  "\\'\\\"\\ \\\\"  " "  "\\\\\\'\\\\\\\"\\\\\\ \\\\\\\\" );

  s += " ";
  s += "'"   "\\\\\" \\ \\\\"   "'\\ single";	// single quote: all literal, no ' inside

  s += " ";
  s += "\""   "\\'\\\" \\ \\\\"   "\"\\ double";// double quote: all literal except \\ \"

  v.clear();
  splitEscaped( s, std::back_inserter(v) );
  BOOST_CHECK_EQUAL( v.size(), 5 );
  BOOST_CHECK_EQUAL( v[0], "" );
  BOOST_CHECK_EQUAL( v[1], "'\" \\" );
  BOOST_CHECK_EQUAL( v[2], "\\'\\\"\\ \\\\" );
  BOOST_CHECK_EQUAL( v[3], "\\\\\" \\ \\\\ single" );
  BOOST_CHECK_EQUAL( v[4], "\\'\" \\ \\ double" );
}

BOOST_AUTO_TEST_CASE(testsplitEscapedWithEmpty)
{
  string s( "simple:non-escaped:string" );
  vector<string> v;

  BOOST_CHECK_EQUAL(splitFieldsEscaped(s, std::back_inserter(v)), 3);
  BOOST_CHECK_EQUAL(v.size(), 3);

  v.clear();
  s = "non-escaped:with::spaces:";
  BOOST_CHECK_EQUAL(splitFieldsEscaped(s, std::back_inserter(v)), 5);
  BOOST_CHECK_EQUAL(v.size(), 5);

  v.clear();
  s = "::";
  BOOST_CHECK_EQUAL(splitFieldsEscaped(s, std::back_inserter(v)), 3);
  BOOST_CHECK_EQUAL(v.size(), 3);

  v.clear();
  s = ":escaped::with\\:spaces";
  BOOST_CHECK_EQUAL(splitFieldsEscaped(s, std::back_inserter(v)), 4);
  BOOST_CHECK_EQUAL(v.size(), 4);
}

BOOST_AUTO_TEST_CASE(test_escape)
{
  string badass = "bad|ass\\|worse";
  string escaped = str::escape(badass, '|');

  BOOST_CHECK_EQUAL( escaped, "bad\\|ass\\\\\\|worse" );
}

BOOST_AUTO_TEST_CASE(conversions)
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

    BOOST_CHECK_EQUAL(str::compareCI("TeST", "test"), 0);
    BOOST_CHECK_EQUAL(str::compareCI("TeST", "test"), 0);
}

BOOST_AUTO_TEST_CASE(conversions_to_bool)
{
  // true iff true-string {1,on,yes,true}
  BOOST_CHECK_EQUAL( str::strToTrue("1"),     true );
  BOOST_CHECK_EQUAL( str::strToTrue("42"),    true );
  BOOST_CHECK_EQUAL( str::strToTrue("ON"),    true );
  BOOST_CHECK_EQUAL( str::strToTrue("YES"),   true );
  BOOST_CHECK_EQUAL( str::strToTrue("TRUE"),  true );
  BOOST_CHECK_EQUAL( str::strToTrue("0"),     false );
  BOOST_CHECK_EQUAL( str::strToTrue("OFF"),   false );
  BOOST_CHECK_EQUAL( str::strToTrue("NO"),    false );
  BOOST_CHECK_EQUAL( str::strToTrue("FALSE"), false );
  BOOST_CHECK_EQUAL( str::strToTrue(""),      false );
  BOOST_CHECK_EQUAL( str::strToTrue("foo"),   false );

  // false iff false-string {0,off,no,false}
  BOOST_CHECK_EQUAL( str::strToFalse("1"),     true );
  BOOST_CHECK_EQUAL( str::strToFalse("42"),    true );
  BOOST_CHECK_EQUAL( str::strToFalse("ON"),    true );
  BOOST_CHECK_EQUAL( str::strToFalse("YES"),   true );
  BOOST_CHECK_EQUAL( str::strToFalse("TRUE"),  true );
  BOOST_CHECK_EQUAL( str::strToFalse("0"),     false );
  BOOST_CHECK_EQUAL( str::strToFalse("OFF"),   false );
  BOOST_CHECK_EQUAL( str::strToFalse("NO"),    false );
  BOOST_CHECK_EQUAL( str::strToFalse("FALSE"), false );
  BOOST_CHECK_EQUAL( str::strToFalse(""),      true );
  BOOST_CHECK_EQUAL( str::strToFalse("foo"),   true );

  // true iff true-string
  BOOST_CHECK_EQUAL( str::strToBool("TRUE",  false), true );
  BOOST_CHECK_EQUAL( str::strToBool("FALSE", false), false );
  BOOST_CHECK_EQUAL( str::strToBool("",      false), false );
  BOOST_CHECK_EQUAL( str::strToBool("foo",   false), false );

  // false iff false-string
  BOOST_CHECK_EQUAL( str::strToBool("TRUE",  true),  true );
  BOOST_CHECK_EQUAL( str::strToBool("FALSE", true),  false );
  BOOST_CHECK_EQUAL( str::strToBool("",      true),  true );
  BOOST_CHECK_EQUAL( str::strToBool("foo",   true),  true );

  // true/false iff true/false-string, else unchanged
  bool ret;
  ret = true; BOOST_CHECK_EQUAL( str::strToBoolNodefault("TRUE",  ret),  true );
  ret = true; BOOST_CHECK_EQUAL( str::strToBoolNodefault("FALSE", ret),  false );
  ret = true; BOOST_CHECK_EQUAL( str::strToBoolNodefault("",      ret),  true );
  ret = true; BOOST_CHECK_EQUAL( str::strToBoolNodefault("foo",   ret),  true );

  ret = false; BOOST_CHECK_EQUAL( str::strToBoolNodefault("TRUE",  ret),  true );
  ret = false; BOOST_CHECK_EQUAL( str::strToBoolNodefault("FALSE", ret),  false );
  ret = false; BOOST_CHECK_EQUAL( str::strToBoolNodefault("",      ret),  false );
  ret = false; BOOST_CHECK_EQUAL( str::strToBoolNodefault("foo",   ret),  false );
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

  BOOST_CHECK_EQUAL( str::commonPrefix("", ""),		0 );
  BOOST_CHECK_EQUAL( str::commonPrefix("a", ""),	0 );
  BOOST_CHECK_EQUAL( str::commonPrefix("", "b"),	0 );
  BOOST_CHECK_EQUAL( str::commonPrefix("a", "b"),	0 );
  BOOST_CHECK_EQUAL( str::commonPrefix("c", "c"),	1 );
  BOOST_CHECK_EQUAL( str::commonPrefix("ca", "cb"),	1 );
}

BOOST_AUTO_TEST_CASE(hexencode_hexdecode)
{
  std::string o;
  o.reserve( 256 );
  for ( unsigned i = 1; i < 256; ++i )
    o += i;

  std::string e( str::hexencode( o ) );
  // encoded contains nothing but [%a-zA-Z0-9]
  for ( unsigned i = 0; i < 255; ++i )
  {
    char ch = e[i];
    BOOST_CHECK( ch == '%'
                 || ( 'a' <= ch && ch <= 'z' )
                 || ( 'A' <= ch && ch <= 'Z' )
                 || ( '0' <= ch && ch <= '9' ) );
  }

  std::string d( str::hexdecode( e ) );
  // decoded equals original
  BOOST_CHECK( o == d );

  // Test %XX is decoded for hexdigits only
  const char *const dig = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  for ( const char * d1 = dig; *d1; ++d1 )
    for ( const char * d2 = dig; *d2; ++d2 )
    {
      std::string eu( "%" );
      eu += *d1; eu += *d2;
      std::string el( str::toLower(eu) );

      std::string u( str::hexdecode( eu ) );
      std::string l( str::hexdecode( el ) );

      if ( *d1 <= 'F' &&  *d2 <= 'F' )
      {
	BOOST_CHECK_EQUAL( u, l );		// no matter if upper or lower case hexdigit
	BOOST_CHECK_EQUAL( u.size(), 1 );	// size 1 == decoded
      }
      else
      {
	BOOST_CHECK_EQUAL( u, eu );		// no hexdigits remain unchanged
	BOOST_CHECK_EQUAL( l, el );
     }
    }
}
