
#include <iostream>
#include <list>
#include <string>

// Boost.Test
#include <boost/test/unit_test.hpp>

#include <zypp/base/LogControl.h>
#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>
#include <zypp/VendorAttr.h>

using boost::unit_test::test_case;
using namespace zypp;
using std::cout;
using std::endl;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "/zypp/data/Vendor")

BOOST_AUTO_TEST_CASE(vendor_empty)
{
  VendorAttr::noTargetInstance() = VendorAttr(); // suse defaults; no configfiles read

  BOOST_REQUIRE( VendorAttr::instance().equivalent("", "") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("a", "") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("", "a") );

  BOOST_REQUIRE( VendorAttr::instance().equivalent( IdString::Null, IdString::Null ) );
  BOOST_REQUIRE( VendorAttr::instance().equivalent( IdString::Empty, IdString::Null ) );
  BOOST_REQUIRE( VendorAttr::instance().equivalent( IdString::Null, IdString::Empty ) );
  BOOST_REQUIRE( VendorAttr::instance().equivalent( IdString::Empty, IdString::Empty ) );
}

BOOST_AUTO_TEST_CASE(vendor_test1)
{

  VendorAttr::noTargetInstance() = VendorAttr(); // suse defaults; no configfiles read

  // bsc#1030686: Remove legacy vendor equivalence between 'suse' and 'opensuse'
  // No vendor definition files has been read. So only suse* vendors are
  // equivalent
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "suse") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("equal", "equal") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse", "SuSE") );
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

BOOST_AUTO_TEST_CASE(vendor_test2)
{
  VendorAttr::noTargetInstance() = VendorAttr( DATADIR / "vendors.d" );

  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "suse") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("equal", "equal") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("open", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("nothing", "SuSE") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("nvidia", "SuSE") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("nvidia_new_new", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("nvidia", "opensuse") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("ati", "SuSE") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("ati", "nvidia") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("ati_new", "ati") );
}

