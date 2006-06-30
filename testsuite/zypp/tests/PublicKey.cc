
#include <iostream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/PublicKey.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;

void publickey_test()
{
  BOOST_CHECK_THROW( zypp::devel::PublicKey("nonexistant"), Exception );
  
  zypp::devel::PublicKey k1("publickey-1.asc");
  
  BOOST_CHECK_EQUAL( k1.id(), "CD1EB6A9667E42D1");
  BOOST_CHECK_EQUAL( k1.name(), "Duncan Mac-Vicar Prett <duncan@puc.cl>" );
  BOOST_CHECK_EQUAL( k1.fingerprint(), "75DA7B971FD6ADB9A880BA9FCD1EB6A9667E42D1" );
  
  zypp::devel::PublicKey k2("publickey-suse.asc");
  
  BOOST_CHECK_EQUAL( k2.id(), "A84EDAE89C800ACA" );
  BOOST_CHECK_EQUAL( k2.name(), "SuSE Package Signing Key <build@suse.de>" );
  BOOST_CHECK_EQUAL( k2.fingerprint(), "79C179B2E1C820C1890F9994A84EDAE89C800ACA" );
    
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "PublicKeyTest" );
    test->add( BOOST_TEST_CASE( &publickey_test ), 0 /* expected zero error */ );
    return test;
}

