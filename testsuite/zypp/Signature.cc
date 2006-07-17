
#include <iostream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/Signature.h"

#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;


void signature_test()
{  
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "SignaureTest" );
    test->add( BOOST_TEST_CASE( &signature_test ), 0 /* expected zero error */ );
    return test;
}

