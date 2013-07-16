
#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/PublicKey.h"
#include "zypp/TmpPath.h"
#include "zypp/Date.h"

#include <boost/test/auto_unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;

#define DATADIR (Pathname(TESTS_SRC_DIR) +  "/zypp/data/PublicKey")

BOOST_AUTO_TEST_CASE(publickey_test)
{
  // test for a empty key
  zypp::PublicKey empty_key;
  BOOST_REQUIRE( ! empty_key.isValid() );

  BOOST_CHECK_THROW( zypp::PublicKey("nonexistant"), Exception );

  zypp::PublicKey k2(DATADIR/"susekey.asc");
  BOOST_CHECK_EQUAL( k2.id(), "A84EDAE89C800ACA" );
  BOOST_CHECK_EQUAL( k2.name(), "SuSE Package Signing Key <build@suse.de>" );
  BOOST_CHECK_EQUAL( k2.fingerprint(), "79C179B2E1C820C1890F9994A84EDAE89C800ACA" );
  BOOST_CHECK_EQUAL( k2.gpgPubkeyVersion(), "9c800aca" );
  BOOST_CHECK_EQUAL( k2.gpgPubkeyRelease(), "40d8063e" );
  BOOST_CHECK_EQUAL( k2.created(), zypp::Date(1087899198) );
  BOOST_CHECK_EQUAL( k2.expires(), zypp::Date(1214043198) );
//BOOST_CHECK_EQUAL( k2.daysToLive(), "" );
  BOOST_REQUIRE( k2.path() != Pathname() );
  BOOST_REQUIRE( k2 == k2 );

  k2 = zypp::PublicKey(DATADIR/"multikey.asc");
  BOOST_CHECK_EQUAL( k2.id(), "27FA41BD8A7C64F9" );
  BOOST_CHECK_EQUAL( k2.name(), "Unsupported <unsupported@suse.de>" );
  BOOST_CHECK_EQUAL( k2.fingerprint(), "D88811AF6B51852351DF538527FA41BD8A7C64F9" );
  BOOST_CHECK_EQUAL( k2.gpgPubkeyVersion(), "8a7c64f9" );
  BOOST_CHECK_EQUAL( k2.gpgPubkeyRelease(), "4be01af3" );
  BOOST_CHECK_EQUAL( k2.created(), zypp::Date(1272978163) );
  BOOST_CHECK_EQUAL( k2.expires(), zypp::Date(1399122163) );
}

