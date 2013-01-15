/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "TestSetup.h"
#include "locales.h"
#include "repos.h"

using namespace std;
using namespace zypp;

static TestSetup test( Arch_x86_64 );


BOOST_AUTO_TEST_CASE( setup )
{
  zypp::base::LogControl::instance().logfile( "./zypper_test.log" );

  MIL << "*** Starting locale tests" << endl;

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

  init_target( test.zypper() );
}

BOOST_AUTO_TEST_CASE( add_locales )
{
  vector<string> localeArgs;
  localeArgs.push_back( "de" );
  localeArgs.push_back( "en" );
  localeArgs.push_back( "invalid" );
  // try to add locales "de", "en" and "invalid" 
  std::map<std::string, bool> result = addLocales( test.zypper(), localeArgs );

  BOOST_CHECK( (result.find("de") != result.end()) && result["de"] );
  BOOST_CHECK( (result.find("en") != result.end()) && result["en"] );
  BOOST_CHECK( result.find("invalid") == result.end() ); // not in result, sorted out

  localeArgs.clear();
  localeArgs.push_back("de");
  // try to add "de" again
  result = addLocales( test.zypper(), localeArgs );

  BOOST_CHECK( (result.find("de") != result.end()) && !result["de"] );
}

BOOST_AUTO_TEST_CASE( remove_locales )
{
  vector<string> localeArgs;
  localeArgs.push_back( "de" );
  localeArgs.push_back( "cs" );
  // remove "de" and "cs"
  std::map<std::string, bool> result = removeLocales( test.zypper(), localeArgs );

  BOOST_CHECK( (result.find("de") != result.end()) && result["de"] );
  BOOST_CHECK( (result.find("cs") != result.end()) && !result["cs"] );
}
