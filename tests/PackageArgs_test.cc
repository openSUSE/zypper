/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "TestSetup.h"
#include "PackageArgs.h"

using namespace zypp;

static TestSetup test;
struct TestInit {
  TestInit() {
    test = TestSetup( Arch_x86_64 );
  }
  ~TestInit() { test.reset(); }
};
BOOST_GLOBAL_FIXTURE( TestInit );

std::ostream & operator<<( std::ostream & str_r, const PackageArgs & obj_r )
{
  std::set<std::string> sorted;
  str::Format fmt { "  %1% %|25t| cap:%2% %3%" };
  for ( const auto & spec : obj_r.dos() )
    sorted.insert( fmt % spec.orig_str % spec.parsed_cap % (spec.modified?"(*)":"") );

  return str_r << "Args " << sorted;
}

// TODO add tests for cases which rely on pool data, like "zypper-1.3.4"
// yielding name=zypper edition=1.3.4 instead of name="zypper-1.3.4" if zypper
// is found in pool
// another example would be bnc #640399

BOOST_AUTO_TEST_CASE(preprocess_test)
{
  {
    std::vector<std::string> rawargs;
    rawargs.push_back("zypper");
    rawargs.push_back("libzypp");
    rawargs.push_back("satsolver-tools");

    PackageArgs args(rawargs);
    std::set<std::string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("zypper") != sargs.end()));
    BOOST_CHECK((sargs.find("libzypp") != sargs.end()));
    BOOST_CHECK((sargs.find("satsolver-tools") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 3);
  }

  {
    std::vector<std::string> rawargs;
    rawargs.push_back("zypper");
    rawargs.push_back("libzypp");
    rawargs.push_back(">=");
    rawargs.push_back("6.30.0");

    PackageArgs args(rawargs);
    std::set<std::string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("zypper") != sargs.end()));
    BOOST_CHECK((sargs.find("libzypp>=6.30.0") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 2);
  }

  {
    std::vector<std::string> rawargs;
    rawargs.push_back("vim");
    rawargs.push_back("zypper>1.4.0");
    rawargs.push_back("libzypp");
    rawargs.push_back("<");
    rawargs.push_back(">");
    rawargs.push_back("6.30.0");

    PackageArgs args(rawargs);
    std::set<std::string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("vim") != sargs.end()));
    BOOST_CHECK((sargs.find("zypper>1.4.0") != sargs.end()));
    BOOST_CHECK((sargs.find("libzypp<>") != sargs.end()));
    BOOST_CHECK((sargs.find("6.30.0") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 4);
  }

  {
    std::vector<std::string> rawargs;
    rawargs.push_back("=");
    rawargs.push_back("zypper=1.4.0");
    rawargs.push_back("libzypp");
    rawargs.push_back(">");
    rawargs.push_back("6.30.0");
    rawargs.push_back("=>");

    PackageArgs args(rawargs);
    std::set<std::string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("=") != sargs.end()));
    BOOST_CHECK((sargs.find("zypper=1.4.0") != sargs.end()));
    BOOST_CHECK((sargs.find("libzypp>6.30.0") != sargs.end()));
    BOOST_CHECK((sargs.find("=>") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 4);
  }

  {
    std::vector<std::string> rawargs;
    rawargs.push_back("=zypper");
    rawargs.push_back("libzypp");
    rawargs.push_back(">6.30.0");
    rawargs.push_back("+vim");
    rawargs.push_back("git<=");
    rawargs.push_back("1.6.4.2");
    rawargs.push_back("tree<=");

    PackageArgs args(rawargs);
    std::set<std::string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("=zypper") != sargs.end()));
    BOOST_CHECK((sargs.find("libzypp>6.30.0") != sargs.end()));
    BOOST_CHECK((sargs.find("+vim") != sargs.end()));
    BOOST_CHECK((sargs.find("git<=1.6.4.2") != sargs.end()));
    BOOST_CHECK((sargs.find("tree<=") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 5);
  }

  {
    std::vector<std::string> rawargs;
    rawargs.push_back("perl(Math::BigInt)");
    rawargs.push_back("pattern:laptop");

    PackageArgs args(rawargs);
    std::set<std::string> sargs = args.asStringSet();

    BOOST_CHECK((sargs.find("perl(Math::BigInt)") != sargs.end()));
    BOOST_CHECK((sargs.find("pattern:laptop") != sargs.end()));
    BOOST_CHECK_EQUAL(sargs.size(), 2);
  }
}

BOOST_AUTO_TEST_CASE(argToCaps_test)
{
  {
    std::vector<std::string> rawargs;
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
    std::vector<std::string> rawargs;
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
    std::vector<std::string> rawargs;
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
    std::vector<std::string> rawargs;
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

BOOST_AUTO_TEST_CASE(kind_tests)
{
  std::vector<std::string> inp {
    ""
    , "name"             , "dep=13"
    , "package:npackage" , "package:dpackage=13"
    , "pattern:npattern" , "pattern:dpattern=13"
    , "product:nproduct" , "product:dproduct=13"
    , "patch:npatch"     , "patch:dpatch=13"
  };

  for ( auto & kindstr : { "package", "pattern", "product", "patch" } )
  {
    ResKind kind { kindstr };
    //cout << "===["<<kind<<"]========================================" << endl;
    PackageArgs args( inp, kind );
    //cout << args << endl;

    // extract the parsed caps...
    CapabilitySet capset;
    for ( const auto & spec : args.dos() )
      capset.insert( spec.parsed_cap );

    // ...and check them...
    BOOST_CHECK_EQUAL( capset.size(), 10 );
    // kind qualified args remain untouched:
    for ( auto & kindstr : { "package", "pattern", "product", "patch" } )
    {
      ResKind tkind { kindstr };
      BOOST_CHECK_EQUAL( capset.count( Capability( "n"+tkind.asString(),  "",   "", tkind ) ), 1 );
      BOOST_CHECK_EQUAL( capset.count( Capability( "d"+tkind.asString(), "=", "13", tkind ) ), 1 );
    }
    // unqualified args get the default kind:
    BOOST_CHECK_EQUAL( capset.count( Capability( "name", "",   "", kind ) ), 1 );
    BOOST_CHECK_EQUAL( capset.count( Capability( "dep", "=", "13", kind ) ), 1 );
  }
}

// vim: set ts=2 sts=8 sw=2 ai et:
