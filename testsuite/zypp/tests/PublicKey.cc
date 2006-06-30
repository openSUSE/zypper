
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

//BOOST_CHECK( m_account.balance() >= 0.0 );
        // BOOST_REQUIRE
        //BOOST_CHECK_MESSAGE( m_account.balance() > 1.0,
        //                     "Initial balance should be more then 1, was " << m_account.balance() );
        //BOOST_CHECK_EQUAL( m_account.balance(), 5.0 );
        //BOOST_CHECK_CLOSE( m_account.balance(), 10.0, /* tolerance */ 1e-10 );
        //BOOST_REQUIRE_MESSAGE( m_account.balance() > 500.1,
        //                       "Balance should be more than 500.1, was " << m_account.balance());

        //BOOST_REQUIRE_PREDICATE( std::not_equal_to<double>(), (m_account.balance())(999.9) );
        //BOOST_REQUIRE_PREDICATE( close_at_tolerance<double>( 1e-9 ), (m_account.balance())(605.5) );
        // BOOST_CHECK_THROW( m_account.withdraw( m_account.balance() + 1 ), std::runtime_error );

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

