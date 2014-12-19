#include "TestSetup.h"
#include <zypp/parser/ProductFileReader.h>

//static TestSetup test( Arch_x86_64 );

// Must be the first test!
BOOST_AUTO_TEST_CASE(basic)
{
  parser::ProductFileData data;
  BOOST_CHECK( data.empty() );

  data = parser::ProductFileReader::scanFile( TESTS_SRC_DIR "/parser/ProductFileReader_test.dat" );
  BOOST_REQUIRE( ! data.empty() );

  BOOST_CHECK_EQUAL( data.vendor(), "Novell" );
  BOOST_CHECK_EQUAL( data.name(), "SUSE_SLES" );
  BOOST_CHECK_EQUAL( data.edition(), "11-0" );
  BOOST_CHECK_EQUAL( data.arch(), Arch_i586 );
  BOOST_CHECK_EQUAL( data.productline(), "" );
  BOOST_CHECK_EQUAL( data.registerTarget(), "sle-11-i586" );
  BOOST_CHECK_EQUAL( data.registerRelease(), "whatever" );
  BOOST_CHECK_EQUAL( data.registerFlavor(), "module" );
  BOOST_CHECK_EQUAL( data.updaterepokey(), "A43242DKD" );

  BOOST_REQUIRE_EQUAL( data.upgrades().size(), 2 );

  BOOST_CHECK_EQUAL( data.upgrades()[0].name(), "openSUSE_11.1" );
  BOOST_CHECK_EQUAL( data.upgrades()[0].summary(), "openSUSE 11.1" );
  BOOST_CHECK_EQUAL( data.upgrades()[0].repository(), "http://download.opensuse.org/distribution/openSUSE/11.1" );
  BOOST_CHECK_EQUAL( data.upgrades()[0].product(), "used on entreprise products" );
  BOOST_CHECK_EQUAL( data.upgrades()[0].notify(), true );
  BOOST_CHECK_EQUAL( data.upgrades()[0].status(), "stable" );

  BOOST_CHECK_EQUAL( data.upgrades()[1].name(), "openSUSE_Factory" );
  BOOST_CHECK_EQUAL( data.upgrades()[1].summary(), "openSUSE Factory" );
  BOOST_CHECK_EQUAL( data.upgrades()[1].repository(), "http://download.opensuse.org/distribution/openSUSE/Factory" );
  BOOST_CHECK_EQUAL( data.upgrades()[1].product(), "" );
  BOOST_CHECK_EQUAL( data.upgrades()[1].notify(), false );
  BOOST_CHECK_EQUAL( data.upgrades()[1].status(), "unstable" );
}
