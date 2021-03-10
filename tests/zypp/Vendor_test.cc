
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
  BOOST_REQUIRE( VendorAttr::instance().equivalent("a", "a") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("a", "aa") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("aa", "a") );

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
  BOOST_REQUIRE( VendorAttr::instance().equivalent("suse", "SuSE as prefix") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("SuSE as prefix", "suse") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("SuSE as prefix","foreign") );
  // bnc#812608: All opensuse projects get their own class (no prefix compare in opensuse namespace)
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse",           "SuSE as prefix") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse as prefix", "SuSE as prefix") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse",           "opensuse as prefix") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse as prefix", "opensuse but different prefix") );

  // bsc#1182629: relaxed equivalence just adds suse/opensuse
  BOOST_REQUIRE( !VendorAttr::instance().relaxedEquivalent("SuSE as prefix","foreign") );
  BOOST_REQUIRE( VendorAttr::instance().relaxedEquivalent("opensuse",           "SuSE as prefix") );
  BOOST_REQUIRE( !VendorAttr::instance().relaxedEquivalent("opensuse as prefix", "SuSE as prefix") );
  BOOST_REQUIRE( !VendorAttr::instance().relaxedEquivalent("opensuse",           "opensuse as prefix") );
  BOOST_REQUIRE( !VendorAttr::instance().relaxedEquivalent("opensuse as prefix", "opensuse but different prefix") );
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

BOOST_AUTO_TEST_CASE(vendor_test3)
{
  VendorAttr::noTargetInstance() = VendorAttr(); // suse defaults; no configfiles read
  VendorAttr::noTargetInstance().addVendorList( { "suse", "opensuse", "opensuse too" } );

  BOOST_REQUIRE( VendorAttr::instance().equivalent("SuSE as prefix", "opensuse") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("SuSE as prefix", "opensuse too") );
  BOOST_REQUIRE( VendorAttr::instance().equivalent("opensuse", "opensuse too") );
  // bnc#812608: All opensuse projects get their own class (no prefix compare in opensuse namespace)
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("SuSE as prefix", "opensuse as prefix") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("SuSE as prefix", "opensuse too as prefix") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse",           "opensuse too as prefix") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse as prefix", "opensuse too") );
  BOOST_REQUIRE( !VendorAttr::instance().equivalent("opensuse as prefix", "opensuse too as prefix") );
}
