#include <iostream>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/ZYppFactory.h"
#include "zypp/RepoManager.h"
#include "TestSetup.h"

using namespace boost::unit_test;
using namespace zypp;
using std::cout;
using std::endl;

TestSetup test( Arch_x86_64 );
const Pathname DATADIR( TESTS_SRC_DIR "/repo/RepoLicense" );

///////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(repolicense)
{
  std::string repo( "repo" );
  test.loadRepo( DATADIR/repo, repo );
  ResPool pool( ResPool::instance() );
  const RepoInfo & ri( pool.knownRepositoriesBegin()->info() );

  std::string prod;	// <data type="license">
  BOOST_CHECK_EQUAL( ri.hasLicense( prod ), 		true );
  BOOST_CHECK_EQUAL( ri.needToAcceptLicense( prod ), 	false );
  BOOST_CHECK_EQUAL( ri.getLicenseLocales( prod ),	LocaleSet({ Locale(),Locale("de") }) );

  prod = "prod";	// <data type="license-prod">
  BOOST_CHECK_EQUAL( ri.hasLicense( prod ), 		true );
  BOOST_CHECK_EQUAL( ri.needToAcceptLicense( prod ), 	true );
  BOOST_CHECK_EQUAL( ri.getLicenseLocales( prod ),	LocaleSet({ Locale(), Locale("de"), Locale("fr") }) );

  prod = "noprod";	// <data type="license-noprod"> is not available
  BOOST_CHECK_EQUAL( ri.hasLicense( prod ), 		false );
  BOOST_CHECK_EQUAL( ri.needToAcceptLicense( prod ), 	false );
  BOOST_CHECK      ( ri.getLicenseLocales( prod ).empty() );
}
