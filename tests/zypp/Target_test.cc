
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
using namespace zypp::filesystem;

BOOST_AUTO_TEST_CASE(target_test)
{

    filesystem::TmpDir tmp;

    ZYpp::Ptr z = getZYpp();

    // create the products.d directory
    assert_dir(tmp.path() / "/etc/products.d" );
    BOOST_CHECK( copy( Pathname(TESTS_SRC_DIR) / "/zypp/data/Target/product.prod",  tmp.path() / "/etc/products.d/product.prod") == 0 );

    // make it the base product
    BOOST_CHECK( symlink(tmp.path() / "/etc/products.d/product.prod", tmp.path() / "/etc/products.d/baseproduct" ) == 0 );

    z->initializeTarget( tmp.path() );

    BOOST_CHECK( ! z->target()->anonymousUniqueId().empty() );
    BOOST_CHECK( PathInfo( tmp.path() / "/var/lib/zypp/AnonymousUniqueId").isExist() );
    BOOST_CHECK( PathInfo( tmp.path() / "/var/lib/zypp/AnonymousUniqueId").size() > 0 );

    // now check the base product
    BOOST_CHECK_EQUAL( z->target()->targetDistribution(), "sle-10-i586");
    BOOST_CHECK_EQUAL( z->target()->targetDistributionRelease(), "special_edition");
    BOOST_CHECK_EQUAL( z->target()->distributionVersion(), "10");

    Target::DistributionLabel dlabel( z->target()->distributionLabel() );
    BOOST_CHECK_EQUAL( dlabel.summary, "A cool distribution" );
    BOOST_CHECK_EQUAL( dlabel.shortName, "" );
}
