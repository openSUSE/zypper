
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <cstdio>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/TmpPath.h"
#include "zypp/PathInfo.h"

#include "zypp/base/Sysconfig.h"
#include "zypp/base/InterProcessMutex.h"


using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using namespace boost::unit_test;

using namespace std;
using namespace zypp;
using namespace zypp::base;


BOOST_AUTO_TEST_CASE(Abort)
{
    int r = 0;
    int status = 0;
    { 
        MIL << "ready to fork" << endl;
    
        r = fork();
  
        if ( r < 0 )
        {
            BOOST_ERROR("Can't fork process");
            return;
        }
        else if ( r == 0 )
        {
            MIL << "child, PID: " << getpid() << endl;
            // child
            sleep(3);
            BOOST_REQUIRE_THROW( InterProcessMutex( InterProcessMutex::Options(InterProcessMutex::Reader,"testcase", 0)), ZYppLockedException);
            //InterProcessMutex mutex2("testcase");
        }
        else
        {
            MIL << "parent: " << getpid() << endl;
            InterProcessMutex mutex( InterProcessMutex::Options(InterProcessMutex::Writer, "testcase"));
            // parent
            sleep(6);
            wait(NULL);
            MIL << "first lock will go out of scope" << endl;
            
        }        
    }
    //if ( r > 0 )
        
}



