
#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include <boost/test/auto_unit_test.hpp>

#include "zypp/ProgressData.h"

using boost::unit_test::test_case;

using namespace std;
using namespace zypp;

BOOST_AUTO_TEST_CASE(progressdata_test)
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
   

