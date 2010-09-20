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

static TestSetup test(Arch_x86_64);

// TODO add tests for cases which rely on pool data, like "zypper-1.3.4"
// yielding name=zypper edition=1.3.4 instead of name="zypper-1.3.4" if zypper
// is found in pool
// another example would be bnc #640399

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
    const PackageArgs::PackageSpecSet & specs = args.dos();
    {
      PackageSpec spec;
      spec.orig_str = "zypper>=1.4.0";
      spec.parsed_cap = Capability("", "zypper", ">=", "1.4.0");
      BOOST_CHECK(specs.find(spec) != specs.end());
    }
    {
      PackageSpec spec;
      spec.orig_str = "perl(Math::BigInt)";
      spec.parsed_cap = Capability("perl(Math::BigInt)");
      BOOST_CHECK(specs.find(spec) != specs.end());
    }
    {
      PackageSpec spec;
      spec.orig_str = "pattern:laptop";
      spec.parsed_cap = Capability("laptop", ResKind::pattern);
      BOOST_CHECK(specs.find(spec) != specs.end());
    }
    BOOST_CHECK_EQUAL(specs.size(), 3);

    const PackageArgs::PackageSpecSet & dontspecs = args.donts();
    {
      PackageSpec spec;
      spec.orig_str = "-irda";
      spec.parsed_cap = Capability("irda");
      BOOST_CHECK(dontspecs.find(spec) != dontspecs.end());
    }
    BOOST_CHECK_EQUAL(dontspecs.size(), 1);
  }
}

BOOST_AUTO_TEST_CASE(dupes_test)
{
  {
    vector<string> rawargs;
    rawargs.push_back("zypper>=1.4.0");
    rawargs.push_back("zypper");
    rawargs.push_back(">=");
    rawargs.push_back("1.4.0");
    rawargs.push_back("-zypper>=1.4.0");

    PackageArgs args(rawargs);
    BOOST_CHECK(args.empty());
  }
}


BOOST_AUTO_TEST_CASE(dont_by_default_test)
{
  {
    vector<string> rawargs;
    rawargs.push_back("package");
    rawargs.push_back("-simon");
    rawargs.push_back("+garfunkel");

    PackageArgs::Options argopts;
    argopts.do_by_default = false;
    PackageArgs args(rawargs, ResKind::package, argopts);

    BOOST_CHECK_EQUAL(args.donts().size(), 2);
    BOOST_CHECK_EQUAL(args.dos().size(), 1);
  }
}

BOOST_AUTO_TEST_CASE(argToCaps_with_patch_test)
{
  {
    vector<string> rawargs;
    rawargs.push_back("openssl-CVE-2009-4355.patch");

    PackageArgs args(rawargs, ResKind::patch);
    const PackageArgs::PackageSpecSet & specs = args.dos();
    {
      PackageSpec spec;
      spec.orig_str = "openssl-CVE-2009-4355.patch";
      // if 'patch' were a known architecture, zypp would have parsed it as arch
      // but since it's not, it's considered a part of the name
      spec.parsed_cap = Capability("", "openssl-CVE-2009-4355.patch", "", "", ResKind::patch);
      BOOST_CHECK(specs.find(spec) != specs.end());
    }
  }
}

// vim: set ts=2 sts=8 sw=2 ai et:
