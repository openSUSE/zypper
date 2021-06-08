#include <boost/test/unit_test.hpp>
using boost::unit_test::test_case;

#include <iostream>
#include <vector>
#include <string>
#include "TestSetup.h"
#include <zypp/base/LogTools.h>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using namespace zypp;

static TestSetup test( TestSetup::initLater );
struct TestInit {
  TestInit() {
    test = TestSetup( Arch_x86_64 );
  }
  ~TestInit() { test.reset(); }
};
BOOST_GLOBAL_FIXTURE( TestInit );

void testcase_init()
{
  test.loadTestcaseRepos( TESTS_SRC_DIR"/sat/BlacklistedPool" );
}

std::string blacktag( const PoolItem & pi_r )
{
  char buf[6];
  buf[0] = pi_r.isBlacklisted()	? 'B' : ' ';
  buf[1] = pi_r.isRetracted()	? 'R' : ' ';
  buf[2] = pi_r.isPtf()		? 'P' : ' ';
  buf[3] = pi_r.isPtfMaster()	? 'm' : ' ';
  buf[4] = pi_r.isPtfPackage()	? 'p' : ' ';
  buf[5] = '\0';
  return buf;
}

std::string blacktag( ui::Selectable::Ptr sel_r )
{
  char buf[7];
  buf[0] = sel_r->hasBlacklisted()		? 'B' : ' ';
  buf[1] = sel_r->hasBlacklistedInstalled()	? 'b' : ' ';
  buf[2] = sel_r->hasRetracted()		? 'R' : ' ';
  buf[3] = sel_r->hasRetractedInstalled()	? 'r' : ' ';
  buf[4] = sel_r->hasPtf()			? 'P' : ' ';
  buf[5] = sel_r->hasPtfInstalled()		? 'p' : ' ';
  buf[6] = '\0';
  return buf;
}

BOOST_AUTO_TEST_CASE(BlacklistedPool)
{
  testcase_init();
  ResPoolProxy poolProxy( test.poolProxy() );
  ui::Selectable::Ptr s( poolProxy.lookup( ResKind::package, "package" ) );
  BOOST_REQUIRE( s );

  //cout << blacktag(s) << endl;
  BOOST_CHECK_EQUAL( blacktag(s), "BbRrPp" );

  std::vector<std::string> expect = {
     "BRPmp"	// I__s_(7)package-5-1.x86_64(@System)
    ,"     "	// U__s_(2)package-1-1.x86_64(Repo)
    ,"BRPmp"	// U__s_(6)package-5-1.x86_64(Repo)
    ,"B P p"	// U__s_(5)package-4-1.x86_64(Repo)
    ,"B Pm "	// U__s_(4)package-3-1.x86_64(Repo)
    ,"BR   "	// U__s_(3)package-2-1.x86_64(Repo)
  };
  unsigned idx = unsigned(-1);
  for ( const auto & pi : s->installed() ) {
    //cout << blacktag(pi) << " " << pi << endl;
    BOOST_CHECK_EQUAL( blacktag(pi), expect[++idx] );
  }
  for ( const auto & pi : s->available() ) {
    //cout << blacktag(pi) << " " << pi << endl;
    BOOST_CHECK_EQUAL( blacktag(pi), expect[++idx] );
  }
}

