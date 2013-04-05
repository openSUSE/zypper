
#include <iostream>
#include <list>
#include <string>

// Boost.Test
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/LogControl.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/ZYpp.h"
#include "zypp/VendorAttr.h"

using boost::unit_test::test_case;
using namespace std;
using namespace zypp;

namespace zypp
{
  void reconfigureZConfig( const Pathname & );
}

#define DATADIR (Pathname(TESTS_SRC_DIR) + "/zypp/data/Vendor")

BOOST_AUTO_TEST_CASE(vendor_test1)
{
  reconfigureZConfig( DATADIR / "zypp1.conf" );
  // No vendor definition files has been read. So only suse,opensuse vendors are
  // equivalent
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "suse") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("equal", "equal") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "SuSE") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("opensuse", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("open", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("nothing", "SuSE") );

  // but "opensuse build service" gets its own class:
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse build service", "suse") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse build service", "opensuse") );
  // bnc#812608: All opensuse projects get their own class
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse-education", "suse") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse-education", "opensuse") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse-education", "opensuse build service") );
}

