
#include <iostream>
#include <list>
#include <string>

// Boost.Test
#include <boost/test/unit_test.hpp>

#include "zypp/base/LogControl.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/ZYpp.h"
#include "zypp/VendorAttr.h"

using boost::unit_test::test_case;
using namespace std;
using namespace zypp;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "/zypp/data/Vendor")

BOOST_AUTO_TEST_CASE(vendor_test1)
{
  setenv("ZYPP_CONF", (DATADIR + "zypp1.conf").c_str(), 1 );
  // No vendor definition files has been readed. So only suse,opensuse vendors are
  // equivalent
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "suse") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("equal", "equal") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "SuSE") ); 
  BOOST_REQUIRE( VendorAttr::instance().equivalent("opensuse", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("open", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("nothing", "SuSE") );
}

BOOST_AUTO_TEST_CASE(vendor_test2)
{
  setenv("ZYPP_CONF", (DATADIR + "zypp2.conf").c_str(), 1 );
  // No vendor definition files has been readed. So only suse,opensuse vendors are
  // equivalent
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "suse") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("equal", "equal") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "SuSE") ); 
  BOOST_REQUIRE( VendorAttr::instance().equivalent("opensuse", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("open", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("nothing", "SuSE") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("nvidia", "SuSE") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("nvidia_new_new", "SuSE") );  
  BOOST_REQUIRE( VendorAttr::instance().equivalent("nvidia", "opensuse") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("ati", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("ati", "nvidia") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("ati_new", "ati") );  
}

