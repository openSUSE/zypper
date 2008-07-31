
#include <iostream>
#include <list>
#include <string>

// Boost.Test
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/TmpPath.h"

using boost::unit_test::test_case;
using namespace std;
using namespace zypp;


BOOST_AUTO_TEST_CASE(target_test)
{

    filesystem::TmpDir tmp;
    
    ZYpp::Ptr z = getZYpp();
    z->initializeTarget( tmp.path() );
    
    BOOST_CHECK( ! z->target()->anonymousUniqueId().empty() );
    BOOST_CHECK( PathInfo( tmp.path() / "/var/lib/zypp/AnonymousUniqueId").isExist() );
    BOOST_CHECK( PathInfo( tmp.path() / "/var/lib/zypp/AnonymousUniqueId").size() > 0 );
}
