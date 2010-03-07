/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "TestSetup.h"
#include "PackageArgs.h"

using namespace std;
using namespace zypp;

BOOST_AUTO_TEST_CASE(preprocess_test)
{
  {
    vector<string> rawargs;
    rawargs.push_back("zypper");
    rawargs.push_back("libzypp");
    rawargs.push_back("satsolver-tools");

    PackageArgs args(rawargs);
    set<string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("zypper") != sargs.end()));
    BOOST_CHECK((sargs.find("libzypp") != sargs.end()));
    BOOST_CHECK((sargs.find("satsolver-tools") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 3);
  }

  {
    vector<string> rawargs;
    rawargs.push_back("zypper");
    rawargs.push_back("libzypp");
    rawargs.push_back(">=");
    rawargs.push_back("6.30.0");

    PackageArgs args(rawargs);
    set<string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("zypper") != sargs.end()));
    BOOST_CHECK((sargs.find("libzypp>=6.30.0") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 2);
  }

  {
    vector<string> rawargs;
    rawargs.push_back("vim");
    rawargs.push_back("zypper>1.4.0");
    rawargs.push_back("libzypp");
    rawargs.push_back("<");
    rawargs.push_back(">");
    rawargs.push_back("6.30.0");

    PackageArgs args(rawargs);
    set<string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("vim") != sargs.end()));
    BOOST_CHECK((sargs.find("zypper>1.4.0") != sargs.end()));
    BOOST_CHECK((sargs.find("libzypp<>") != sargs.end()));
    BOOST_CHECK((sargs.find("6.30.0") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 4);
  }

  {
    vector<string> rawargs;
    rawargs.push_back("=");
    rawargs.push_back("zypper=1.4.0");
    rawargs.push_back("libzypp");
    rawargs.push_back(">");
    rawargs.push_back("6.30.0");
    rawargs.push_back("=>");

    PackageArgs args(rawargs);
    set<string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("=") != sargs.end()));
    BOOST_CHECK((sargs.find("zypper=1.4.0") != sargs.end()));
    BOOST_CHECK((sargs.find("libzypp>6.30.0") != sargs.end()));
    BOOST_CHECK((sargs.find("=>") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 4);
  }

  {
    vector<string> rawargs;
    rawargs.push_back("=zypper");
    rawargs.push_back("libzypp");
    rawargs.push_back(">6.30.0");
    rawargs.push_back("+vim");
    rawargs.push_back("git<=");
    rawargs.push_back("1.6.4.2");
    rawargs.push_back("tree<=");

    PackageArgs args(rawargs);
    set<string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("=zypper") != sargs.end()));
    BOOST_CHECK((sargs.find("libzypp>6.30.0") != sargs.end()));
    BOOST_CHECK((sargs.find("+vim") != sargs.end()));
    BOOST_CHECK((sargs.find("git<=1.6.4.2") != sargs.end()));
    BOOST_CHECK((sargs.find("tree<=") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 5);
  }

  {
    vector<string> rawargs;
    rawargs.push_back("perl(Math::BigInt)");
    rawargs.push_back("pattern:laptop");

    PackageArgs args(rawargs);
    set<string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("perl(Math::BigInt)") != sargs.end()));
    BOOST_CHECK((sargs.find("pattern:laptop") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 2);
  }
}

BOOST_AUTO_TEST_CASE(argToCaps_test)
{
  {
    vector<string> rawargs;
    rawargs.push_back("zypper>=1.4.0");
    rawargs.push_back("perl(Math::BigInt)");
    rawargs.push_back("pattern:laptop");
    rawargs.push_back("-irda");

    PackageArgs args(rawargs);
    const PackageArgs::CapRepoPairSet & caps = args.doCaps();
    BOOST_CHECK(caps.find(PackageArgs::CapRepoPair(
        Capability("", "zypper", ">=", "1.4.0"),"")) != caps.end());
    BOOST_CHECK(caps.find(PackageArgs::CapRepoPair(
        Capability("perl(Math::BigInt)"),"")) != caps.end());
    BOOST_CHECK(caps.find(PackageArgs::CapRepoPair(
        Capability("laptop", ResKind::pattern),"")) != caps.end());
    BOOST_CHECK_EQUAL(caps.size(), 3);

    const PackageArgs::CapRepoPairSet & dontcaps = args.dontCaps();
    BOOST_CHECK(dontcaps.find(PackageArgs::CapRepoPair(
        Capability("irda"),"")) != dontcaps.end());
    BOOST_CHECK_EQUAL(dontcaps.size(), 1);
  }
}

// vim: set ts=2 sts=8 sw=2 ai et:
