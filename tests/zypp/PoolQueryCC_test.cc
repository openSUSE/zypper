#include "TestSetup.h"
#include <zypp/base/String.h>
#include <zypp/base/LogTools.h>

#include "zypp/PoolQuery.h"
#include "zypp/PoolQueryUtil.tcc"

#define BOOST_TEST_MODULE PoolQuery_CC

using boost::unit_test::test_case;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using namespace zypp;

template <class TCont>
std::ostream & nlist( std::ostream & str, const TCont & set_r )
{
  str << "[" << set_r.size() << "]: ";
  for ( const auto & solv : set_r )
    str << " \"" << solv.name() << "\"";
  return str << endl;
}

/////////////////////////////////////////////////////////////////////////////

static TestSetup test( TestSetup::initLater );
struct TestInit {
  TestInit() {
    test = TestSetup( Arch_x86_64 );

    test.loadTargetHelix( TESTS_SRC_DIR "/zypp/data/PoolQueryCC/rxnames.xml" );
    nlist( cout << "repo ", ResPool::instance() );
  }
  ~TestInit() { test.reset(); }
};
BOOST_GLOBAL_FIXTURE( TestInit );

/////////////////////////////////////////////////////////////////////////////
// Basic issue: Multiple match strings are compiled into a singe regex. The
// semantic of the individual match strings must be preserved. I.e. a literal
// "." must become "\.". Globbing patterns must match the whole string, so they
// need to be anchored within the regex. Etc.
/////////////////////////////////////////////////////////////////////////////
static const unsigned qtestSIZEMISS	= unsigned(-1);
static const unsigned qtestRXFAIL	= unsigned(-2);
static const unsigned qtestRXFAILCOMB	= unsigned(-3);

unsigned qtest( const std::string & pattern_r, Match::Mode mode_r, bool verbose_r = false )
{
  static constexpr const bool noMatchInvalidRegexException = false;

  typedef std::set<sat::Solvable> Result;
  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name);
  switch ( mode_r )
  {
    case Match::STRING:		q.setMatchExact();	break;
    case Match::SUBSTRING:	q.setMatchSubstring();	break;
    case Match::OTHER:		q.setMatchWord();	break;	// OTHER missused for matchWord()
    case Match::GLOB:		q.setMatchGlob();	break;
    case Match::REGEX:		q.setMatchRegex();	break;
    default:
      throw( "unhandled match mode" );
      break;
  }
  q.addString( pattern_r );
  Result o;
  try {
    o = Result( q.begin(), q.end() );	// original query
  }
  catch ( const zypp::MatchInvalidRegexException & excpt )
  {
    cout << "Caught: " << excpt << endl;
    return qtestRXFAIL;
  }

  q.addString( "more" );
  try {
    Result r( q.begin(), q.end() );	// compiles into RX (o|more)

    BOOST_CHECK( o == r );
    if ( o != r || verbose_r )
    {
      cout << '"' << pattern_r << "\"  " << mode_r << endl;
      nlist( cout << "    o", o );
      nlist( cout << "    r", r );
      if ( ! verbose_r )
	return qtestSIZEMISS;
    }
  }
  catch ( const zypp::MatchInvalidRegexException & excpt )
  {
    BOOST_CHECK( noMatchInvalidRegexException );
    cout << "Caught: " << excpt << endl;
    return qtestRXFAILCOMB;
  }

  return o.size();
}

inline unsigned qtest( const std::string & pattern_r, bool verbose_r = false )
{ return qtest( pattern_r, Match::SUBSTRING, verbose_r ); }

/////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(pool_query_init)
{
  // NOTE: qtest( , Match::OTHER ) is missused for matchWord()
  BOOST_CHECK_EQUAL( qtest( "?", Match::SUBSTRING ),	1 );
  BOOST_CHECK_EQUAL( qtest( "?", Match::STRING ),	1 );
  BOOST_CHECK_EQUAL( qtest( "?", Match::OTHER ),	0 );	// not word boundary
  BOOST_CHECK_EQUAL( qtest( "?", Match::GLOB ),		15 );
  BOOST_CHECK_EQUAL( qtest( "\\?", Match::GLOB ),	1 );
  BOOST_CHECK_EQUAL( qtest( "?", Match::REGEX ),	qtestRXFAIL );
  BOOST_CHECK_EQUAL( qtest( "\\?", Match::REGEX ),	1 );

  BOOST_CHECK_EQUAL( qtest( "A", Match::SUBSTRING ),	4 );
  BOOST_CHECK_EQUAL( qtest( "A", Match::OTHER ),	2 );
  BOOST_CHECK_EQUAL( qtest( "A*", Match::OTHER ),	0 );
  BOOST_CHECK_EQUAL( qtest( "*A", Match::OTHER ),	0 );
  BOOST_CHECK_EQUAL( qtest( "A*", Match::GLOB ),	2 );
  BOOST_CHECK_EQUAL( qtest( "*A", Match::GLOB ),	1 );
}

/////////////////////////////////////////////////////////////////////////////
