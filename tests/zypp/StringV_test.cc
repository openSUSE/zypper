#include <boost/test/unit_test.hpp>
#include <iostream>
#include <sstream>
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

BOOST_AUTO_TEST_CASE(splitRxCallbacks)
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
  void expect( const std::string & l, const regex & rx, const std::initializer_list<const char*> & words )
  {
    //cout << "??? <" << rx << "> <" << l << "> " << words.size() << endl;
    auto next = words.begin();
    unsigned ret = splitRx( l, rx, [&]( std::string_view w, unsigned i ) {
      //cout << "  ["<<i<<"] <"<<w<<">"<<endl;
      BOOST_REQUIRE( next != words.end() );
      BOOST_CHECK_EQUAL( *next, w );
      ++next;
    });
    //cout << " => " << ret << endl;
    BOOST_CHECK_EQUAL( next, words.end() );
    BOOST_CHECK_EQUAL( ret, words.size() );
  }
} // namespace tools

BOOST_AUTO_TEST_CASE(splitRxResults)
{
  using tools::expect;

  regex rx;	// no regex, no match ==> input reported
  expect( "",  rx, {""} );
  expect( "0",  rx, {"0"} );
  expect( "01",  rx, {"01"} );
  expect( "012",  rx, {"012"} );

  rx = "";	// Empty regex, empty matches
  expect( "",  rx, {"",""} );
  expect( "0",  rx, {"","0",""} );
  expect( "01",  rx, {"","0","1",""} );
  expect( "012",  rx, {"","0","1","2",""} );

  rx = "x*";	// Empty matches as above
  expect( "",  rx, {"",""} );
  expect( "0",  rx, {"","0",""} );
  expect( "01",  rx, {"","0","1",""} );
  expect( "012",  rx, {"","0","1","2",""} );

  rx = "1*";
  expect( "",  rx, {"",""} );
  expect( "0",  rx, {"","0",""} );
  expect( "01",  rx, {"","0",""} );
  expect( "012",  rx, {"","0","2",""} );
  expect( "012\n",  rx, {"","0","2","\n"} );	// no match behind a trailing NL
  expect( "012\n\n",  rx, {"","0","2","\n","\n"} );	// no match behind a trailing NL

  rx = "^|1|$";
  expect( "",  rx, {"",""} );
  expect( "0",  rx, {"","0",""} );
  expect( "01",  rx, {"","0",""} );
  expect( "012",  rx, {"","0","2",""} );
  expect( "012\n",  rx, {"","0","2","\n"} );	// no match behind a trailing NL
  expect( "012\n\n",  rx, {"","0","2","\n","\n"} );	// no match behind a trailing NL
}
