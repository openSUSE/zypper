#include <boost/test/unit_test.hpp>
#include <iostream>
#include <sstream>
#include <optional>
#include <zypp/base/StringV.h>

using std::cout;
using std::endl;
using namespace zypp::strv;

namespace tools
{
  // Something convertible to bool
  struct False {
    operator bool() { return false; }
  };

  template <typename T>
  std::ostream & outsopt( std::ostream & str, const std::optional<T> & v )
  {
    str << "{";
    if ( v.has_value() )
      str << v.value();
    else
      str << "@";
    return str << "}";
  }

  // CB data tracker
  void outs( std::ostream & str,
	     std::optional<std::string_view> w = std::optional<std::string_view>(),
	     std::optional<unsigned> i = std::optional<unsigned>(),
	     std::optional<bool> f = std::optional<bool>() )
  { outsopt(str,w); outsopt(str,i); outsopt(str,f) << endl; }

} // namespace tools

BOOST_AUTO_TEST_CASE(WordConsumerSignature)
{
  std::string l { "0b2" };
  regex r { "^|b|$" };

  // Check different CB signatures are correctly overloaded and invoked.
  // Trace the CB calls and args passed in one big string and compare it
  // with the expected.
  // std::ostream & str { cout };
  std::ostringstream str;
  // CB returning false
  BOOST_CHECK_EQUAL( 2, splitRx( l, r, [&str](std::string_view w, unsigned i, bool f) -> bool { tools::outs(str,w,i,f); return tools::False(); } ) );
  BOOST_CHECK_EQUAL( 2, splitRx( l, r, [&str](std::string_view w, unsigned i) -> bool { tools::outs(str,w,i); return tools::False(); } ) );
  BOOST_CHECK_EQUAL( 2, splitRx( l, r, [&str](std::string_view w) -> bool { tools::outs(str,w); return tools::False(); } ) );
  BOOST_CHECK_EQUAL( 2, splitRx( l, r, [&str]() -> bool { tools::outs(str); return tools::False(); } ) );
  // CB returning true
  BOOST_CHECK_EQUAL( 4, splitRx( l, r, [&str](std::string_view w, unsigned i, bool f) -> void { tools::outs(str,w,i,f); } ) );
  BOOST_CHECK_EQUAL( 4, splitRx( l, r, [&str](std::string_view w, unsigned i) -> void { tools::outs(str,w,i); } ) );
  BOOST_CHECK_EQUAL( 4, splitRx( l, r, [&str](std::string_view w) -> void { tools::outs(str,w); } ) );
  BOOST_CHECK_EQUAL( 4, splitRx( l, r, [&str]() -> void { tools::outs(str); } ) );
  // no CB
  BOOST_CHECK_EQUAL( 4, splitRx( l, r ) );

  // Trace:
  BOOST_CHECK_EQUAL( str.str(),
		     "{}{0}{0}\n"	// F1
		     "{0b2}{1}{1}\n"
		     "{}{0}{@}\n"	// F2
		     "{0b2}{1}{@}\n"
		     "{}{@}{@}\n"	// F3
		     "{0b2}{@}{@}\n"
		     "{@}{@}{@}\n"	// F4
		     "{@}{@}{@}\n"
		     "{}{0}{0}\n"	// T1
		     "{0}{1}{0}\n"
		     "{2}{2}{0}\n"
		     "{}{3}{1}\n"
		     "{}{0}{@}\n"	// T2
		     "{0}{1}{@}\n"
		     "{2}{2}{@}\n"
		     "{}{3}{@}\n"
		     "{}{@}{@}\n"	// T3
		     "{0}{@}{@}\n"
		     "{2}{@}{@}\n"
		     "{}{@}{@}\n"
		     "{@}{@}{@}\n"	// T4
		     "{@}{@}{@}\n"
		     "{@}{@}{@}\n"
		     "{@}{@}{@}\n"
  );
}

namespace tools
{
  using zypp::strv::detail::WordConsumer;

  /* Call splitter_r and check whether CB results match word_r. May CB abort at stopAt_r. */
  void checkSplit( std::function<unsigned(WordConsumer&&)> splitter_r, const std::initializer_list<const char*> & words_r,
		   unsigned stopAt_r = -1U )
  {
    //cout << "??? " << words_r.size() << endl;
    auto next = words_r.begin();
    unsigned ret = splitter_r( [&]( std::string_view w, unsigned i, bool f )->bool {
      //cout << "  ["<<i<<"] <"<<w<<">"<<endl;
      BOOST_REQUIRE( next != words_r.end() );
      BOOST_CHECK_EQUAL( *next, w );
      ++next;
      BOOST_CHECK_EQUAL( f, next == words_r.end() );
      return i != stopAt_r;
    });
    //cout << " => " << ret << endl;
    BOOST_CHECK_EQUAL( next, words_r.end() );
    BOOST_CHECK_EQUAL( ret, words_r.size() );
  }

  void expectRx( const std::string & l, const regex & rx, const std::initializer_list<const char*> & words, unsigned stopAt = -1U )
  { checkSplit( [&](WordConsumer && wc) { return splitRx( l, rx, wc ); }, words, stopAt ); }

  void expectSp( std::string_view l, std::string_view sep, const std::initializer_list<const char*> & words, unsigned stopAt = -1U )
  { checkSplit( [&](WordConsumer && wc) { return split( l, sep, wc ); }, words, stopAt ); }

  void expectTp( std::string_view l, std::string_view sep, const std::initializer_list<const char*> & words, unsigned stopAt = -1U )
  { checkSplit( [&](WordConsumer && wc) { return split( l, sep, Trim::trim, wc ); }, words, stopAt ); }
} // namespace tools


BOOST_AUTO_TEST_CASE(splitRxResults)
{
  using tools::expectRx ;

  regex rx;	// no regex, no match ==> input reported
  expectRx( "",  rx, {""} );
  expectRx( "0",  rx, {"0"} );
  expectRx( "01",  rx, {"01"} );
  expectRx( "012",  rx, {"012"} );

  rx = "";	// Empty regex, empty matches
  expectRx( "",  rx, {"",""} );
  expectRx( "0",  rx, {"","0",""} );
  expectRx( "01",  rx, {"","0","1",""} );
  expectRx( "012",  rx, {"","0","1","2",""} );

  rx = "x*";	// Empty matches as above
  expectRx( "",  rx, {"",""} );
  expectRx( "0",  rx, {"","0",""} );
  expectRx( "01",  rx, {"","0","1",""} );
  expectRx( "012",  rx, {"","0","1","2",""} );

  rx = "1*";
  expectRx( "",  rx, {"",""} );
  expectRx( "0",  rx, {"","0",""} );
  expectRx( "01",  rx, {"","0",""} );
  expectRx( "012",  rx, {"","0","2",""} );
  expectRx( "012\n",  rx, {"","0","2","\n"} );	// no match behind a trailing NL
  expectRx( "012\n\n",  rx, {"","0","2","\n","\n"} );	// no match behind a trailing NL

  rx = "^|1|$";
  expectRx( "",  rx, {"",""} );
  expectRx( "0",  rx, {"","0",""} );
  expectRx( "01",  rx, {"","0",""} );
  expectRx( "012",  rx, {"","0","2",""} );
  expectRx( "012\n",  rx, {"","0","2","\n"} );	// no match behind a trailing NL
  expectRx( "012\n\n",  rx, {"","0","2","\n","\n"} );	// no match behind a trailing NL
}

BOOST_AUTO_TEST_CASE(splitResults)
{
  using tools::expectSp;

  std::string sep;	// empty sep; split [[:blank:]]+; report not empty words
  expectSp( "",  sep, {} );
  expectSp( " ",  sep, {} );
  expectSp( "  ",  sep, {} );
  expectSp( " 1 ",  sep, {"1"} );
  expectSp( " 1 2 ",  sep, {"1","2"} );
  expectSp( " 1  2 ",  sep, {"1","2"} );
  expectSp( " 1  2 ",  sep, {"1","2"}, 0 );	// CB aborted
  expectSp( " 1  2 ",  sep, {"1","2"}, 1 );	// CB aborted

  sep = ",";	// not empty seps....
  expectSp( "",  sep, {""} );
  expectSp( ",",  sep, {"",""} );
  expectSp( ",,",  sep, {"","",""} );
  expectSp( "1",  sep, {"1"} );
  expectSp( ",1,",  sep, {"","1",""} );
  expectSp( ",1,2,",  sep, {"","1","2",""} );
  expectSp( ",1,,2,",  sep, {"","1","","2",""} );

  expectSp( " ",  sep, {" "} );
  expectSp( " , ",  sep, {" "," "} );
  expectSp( " , , ",  sep, {" "," "," "} );
  expectSp( " 1 ",  sep, {" 1 "} );
  expectSp( " , 1 , ",  sep, {" "," 1 "," "} );
  expectSp( " , 1 , 2 , ",  sep, {" "," 1 "," 2 "," "} );
  expectSp( " , 1 , , 2 , ",  sep, {" "," 1 "," "," 2 "," "} );
  expectSp( " , 1 , , 2 , ",  sep, {" "," 1 , , 2 , "}, 0 );	// CB aborted

  using tools::expectTp;	// trimmed split
  expectTp( " ",  sep, {""} );
  expectTp( " , ",  sep, {"",""} );
  expectTp( " , , ",  sep, {"","",""} );
  expectTp( " 1 ",  sep, {"1"} );
  expectTp( " , 1 , ",  sep, {"","1",""} );
  expectTp( " , 1 , 2 , ",  sep, {"","1","2",""} );
  expectTp( " , 1 , , 2 , ",  sep, {"","1","","2",""} );
  expectTp( " , 1 , , 2 , ",  sep, {"","1 , , 2 ,"}, 0 );	// CB aborted
}

BOOST_AUTO_TEST_CASE(trimming)
{
  BOOST_CHECK_EQUAL( ltrim(""),    "" );
  BOOST_CHECK_EQUAL( ltrim(" "),   "" );
  BOOST_CHECK_EQUAL( ltrim("1"),   "1" );
  BOOST_CHECK_EQUAL( ltrim(" 1"),  "1" );
  BOOST_CHECK_EQUAL( ltrim("1 "),  "1 " );
  BOOST_CHECK_EQUAL( ltrim(" 1 "), "1 " );

  BOOST_CHECK_EQUAL( rtrim(""),    "" );
  BOOST_CHECK_EQUAL( rtrim(" "),   "" );
  BOOST_CHECK_EQUAL( rtrim("1"),   "1" );
  BOOST_CHECK_EQUAL( rtrim(" 1"),  " 1" );
  BOOST_CHECK_EQUAL( rtrim("1 "),  "1" );
  BOOST_CHECK_EQUAL( rtrim(" 1 "), " 1" );

  BOOST_CHECK_EQUAL(  trim(""),     "" );
  BOOST_CHECK_EQUAL(  trim(" "),    "" );
  BOOST_CHECK_EQUAL(  trim("1"),    "1" );
  BOOST_CHECK_EQUAL(  trim(" 1"),   "1" );
  BOOST_CHECK_EQUAL(  trim("1 "),   "1" );
  BOOST_CHECK_EQUAL(  trim(" 1 "),  "1" );

  BOOST_CHECK_EQUAL( ltrim(" 1 "), trim(" 1 ", Trim::left) );
  BOOST_CHECK_EQUAL( rtrim(" 1 "), trim(" 1 ", Trim::right) );
  BOOST_CHECK_EQUAL(  trim(" 1 "), trim(" 1 ", Trim::left|Trim::right) );

  BOOST_CHECK_EQUAL(  trim(" \t1\t "),         "1" );
  BOOST_CHECK_EQUAL(  trim(" \t1\t ", " "),  "\t1\t" );
  BOOST_CHECK_EQUAL(  trim(" \t1\t ", ""),  " \t1\t " );
}
