
#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/ProgressData.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using namespace boost::unit_test::log;

using namespace std;
using namespace zypp;

void progressdata_test()
{
  {
    ProgressData progress(100);
    CombinedProgressData sub1rcv(progress, 80);
    
    ProgressData sub1progress(100);
    sub1progress.sendTo(sub1rcv);
    
    // task 1 goes to 50%
    sub1progress.set(50);
    // which is 50% of 80% in task 1
    BOOST_CHECK_EQUAL( progress.val(), 40 );
  }
  
  {
    ProgressData progress(40000);
    CombinedProgressData sub2rcv(progress, 10000);
    
    ProgressData sub2progress(500);
    sub2progress.sendTo(sub2rcv);
    sub2progress.set(250);
    
    // which is 50% of 80% in task 1
    BOOST_CHECK_EQUAL( progress.val(), 5000 );
  }
  
  {
    ProgressData progress(20000,60000);
    CombinedProgressData sub2rcv(progress, 10000);
    
    ProgressData sub2progress(500);
    sub2progress.sendTo(sub2rcv);
    sub2progress.set(250);
    
    // which is 50% of 80% in task 1
    BOOST_CHECK_EQUAL( progress.val(), 25000 );
  }
  
}

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  test_suite* test= BOOST_TEST_SUITE( "ProgressData_test" );
    test->add( BOOST_TEST_CASE( &progressdata_test ), 0 /* expected zero error */ );
    return test;
}

