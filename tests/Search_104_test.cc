#include "TestSetup.h"
#include "commands/search/search.h"
#include "commands/query.h"

using namespace zypp;

extern ZYpp::Ptr God;
static TestSetup test( TestSetup::initLater );
struct TestInit {
  TestInit() {
    test = TestSetup( Arch_x86_64 );
    zypp::base::LogControl::instance().logfile( "./zypper_test.log" );
    try
    {
      God = zypp::getZYpp();
    }
    catch ( const ZYppFactoryException & excpt_r )
    {
      ZYPP_CAUGHT (excpt_r);
      cerr <<
        "Could not access the package manager engine."
        " This usually happens when you have another application (like YaST)"
        " using it at the same time. Close the other applications and try again.";
    }
    catch ( const Exception & excpt_r)
    {
      ZYPP_CAUGHT (excpt_r);
      cerr << excpt_r.msg() << endl;
    }

    test.loadRepo(TESTS_SRC_DIR "/data/openSUSE-11.1", "main");

  }
  ~TestInit() { test.reset(); }
};
BOOST_GLOBAL_FIXTURE( TestInit );

void runSearch( ZypperBaseCommand & cmd, const std::vector<std::string> & positionalArgs, int exitInfoExpect, int exitInfoPre = 0 )
{
  Zypper::instance().clearExitInfoCode();  // The only way to reset! setExitInfoCode remembers the fist code set.
  if ( exitInfoPre )
    Zypper::instance().setExitInfoCode( exitInfoPre );
  cmd.setPositionalArguments( positionalArgs );
  int res = cmd.run( Zypper::instance() );
  BOOST_CHECK_EQUAL( res, 0 );
  BOOST_CHECK_EQUAL( Zypper::instance().exitCode(), 0 );
  BOOST_CHECK_EQUAL( Zypper::instance().exitInfoCode(), exitInfoExpect );
}
void runBasiccSearchTest( ZypperBaseCommand && cmd )
{
  Zypper & zypper { Zypper::instance() };
  zypper.configNoConst().ignore_unknown = false;
  runSearch( cmd, { "nomatch" },           104 );
  runSearch( cmd, { "nomatch", "zypper" }, 0 );         // 104 only if the complete search is empty
  runSearch( cmd, { "nomatch" },           106, 106 );  // a previously set exitinfoCode overrules any other
  runSearch( cmd, { "nomatch", "zypper" }, 106, 106 );
  zypper.configNoConst().ignore_unknown = true;         // --ignore-unknown turns 104 into 0
  runSearch( cmd, { "nomatch" },           0 );
  runSearch( cmd, { "nomatch", "zypper" }, 0 );
  runSearch( cmd, { "nomatch" },           106, 106 );
  runSearch( cmd, { "nomatch", "zypper" }, 106, 106 );
}

BOOST_AUTO_TEST_CASE( basic )
{
  // search/info should behave similar
  runBasiccSearchTest( SearchCmd({ "search" }) );
  runBasiccSearchTest( InfoCmd({ "info" }) );
}
