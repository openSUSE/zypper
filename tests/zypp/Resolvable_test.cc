
#include <iostream>
#include <list>
#include <string>

// Boost.Test
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"
#include "zypp/Pattern.h"

#include "TestSetup.h"

using boost::unit_test::test_case;
using namespace std;
using namespace zypp;


BOOST_AUTO_TEST_CASE(resolvable_test)
{
    TestSetup test( Arch_x86_64 );
    // test.loadTarget(); // initialize and load target
    test.loadRepo( TESTS_SRC_DIR"/data/openSUSE-11.1" );

    int pattern_count = 0;
    for_( pitem, test.pool().begin(), test.pool().end() )
    {
        if ( isKind<Pattern>(pitem->resolvable()) )
        {
            //BOOST_CHECK( ! asKind<Pattern>(pitem->resolvable())->contents().empty() );
            MIL << asKind<Pattern>(pitem->resolvable()) << endl;
            pattern_count++;
        }        
    }
    BOOST_CHECK(pattern_count > 0);
}
