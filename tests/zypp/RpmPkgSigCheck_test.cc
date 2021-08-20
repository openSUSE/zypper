#include "TestSetup.h"

#include <zypp/target/rpm/RpmDb.h>
using target::rpm::RpmDb;

#define DATADIR (Pathname(TESTS_SRC_DIR) / "/zypp/data/RpmPkgSigCheck")

#ifdef HAVE_RPM_VERIFY_TRANSACTION_STEP
#define HAVE_RPMTSSETVFYFLAGS
#endif

static TestSetup test( TestSetup::initLater );
struct TestInit {
  TestInit() {
    test = TestSetup( );
  }
  ~TestInit() { test.reset(); }
};
BOOST_GLOBAL_FIXTURE( TestInit );

///////////////////////////////////////////////////////////////////
//
// - RpmDb::checkPackage (legacy) and RpmDb::checkPackageSignature are
// expected to produce the same result, except for ...
//
// Result comparison is not very sophisticated. As the detail strings are
// user visible (at least in zypper) we want a notification (breaking testcase)
// if something in the rpm format changes.
//
///////////////////////////////////////////////////////////////////
namespace
{
  struct CheckResult
  {
    CheckResult()
    {}

    CheckResult( RpmDb::CheckPackageResult && result_r )
    : result { std::move(result_r) }
    {}

    CheckResult( RpmDb::CheckPackageResult && result_r,
		 std::vector<std::pair<RpmDb::CheckPackageResult,std::string>> && detail_r )
    : result { std::move(result_r) }
    { static_cast<std::vector<std::pair<RpmDb::CheckPackageResult,std::string>>&>(detail) = std::move(detail_r); }

    RpmDb::CheckPackageResult result;
    RpmDb::CheckPackageDetail detail;
  };

  bool operator==( const CheckResult & lhs, const CheckResult & rhs )
  {
    if ( lhs.result != rhs.result )
      return false;
    // protect against reordered details:

    // there seems to be a backporting of how rpm prints the signature check result
    // breaking our tests here, instead of checking for exact equality we just require
    // that all elements in the lhs are existant in the rhs instance.
    //if ( lhs.detail.size() != rhs.detail.size() )
    //  return false;

    for ( const auto & l : lhs.detail )
    {
      if ( std::find( rhs.detail.begin(), rhs.detail.end(), l ) == rhs.detail.end() )
	return false;
    }
    return true;
  }

  std::ostream & operator<<( std::ostream & str, const CheckResult & obj )
  {
    str << "R: " << obj.result;
    for ( const auto & el : obj.detail )
      str << endl << "   "  << el.first << " | " << el.second;
    return str;
  }

  CheckResult gcheckPackage( const Pathname & path_r )
  {
    CheckResult res;
    res.result = test.target().rpmDb().checkPackage( path_r, res.detail );
//     cout << "==-" << path_r << endl;
//     cout << res << endl;
    return res;
  }

  CheckResult gcheckPackageSignature( const Pathname & path_r )
  {
    CheckResult res;
    res.result = test.target().rpmDb().checkPackageSignature( path_r, res.detail );
//     cout << "==!" << path_r << endl;
//     cout << res << endl;
    return res;
  }
} // namespace


BOOST_AUTO_TEST_CASE(no_pkg)
{
  Pathname rpm { DATADIR/"no.rpm" };
  CheckResult cp { gcheckPackage( rpm ) };
  CheckResult cs { gcheckPackageSignature( rpm ) };
  BOOST_CHECK_EQUAL( cp, cs );

  CheckResult xpct { RpmDb::CHK_ERROR, {/*empty details*/} };
  BOOST_CHECK_EQUAL( xpct, cs );
}

BOOST_AUTO_TEST_CASE(unsigned_pkg)
{
  Pathname rpm { DATADIR/"unsigned.rpm" };
  CheckResult cp { gcheckPackage( rpm ) };
  CheckResult cs { gcheckPackageSignature( rpm ) };
  // For unsigned packages the final result differs!
  // (but only if the digests are OK)
  BOOST_CHECK_EQUAL( cp.result, RpmDb::CHK_OK );
  BOOST_CHECK_EQUAL( cs.result, RpmDb::CHK_NOSIG );
  BOOST_CHECK_EQUAL( cp.detail, cs.detail );

  CheckResult xpct { RpmDb::CHK_NOSIG, {
    { RpmDb::CHK_OK,	"    Header SHA1 digest: OK" },
    { RpmDb::CHK_OK,	"    Header SHA256 digest: OK" },
    { RpmDb::CHK_OK,	"    Payload SHA256 digest: OK" },
    { RpmDb::CHK_OK,	"    MD5 digest: OK" },
    { RpmDb::CHK_NOSIG,	"    Package is not signed!" },
  } };
  BOOST_CHECK_EQUAL( xpct, cs );
}

BOOST_AUTO_TEST_CASE(unsigned_broken_pkg)
{
  Pathname rpm { DATADIR/"unsigned_broken.rpm" };
  CheckResult cp { gcheckPackage( rpm ) };
  CheckResult cs { gcheckPackageSignature( rpm ) };
  // Unsigned, but a broken digest 'superseeds' CHK_NOSIG
  BOOST_CHECK_EQUAL( cp, cs );

  CheckResult xpct { RpmDb::CHK_FAIL, {
    { RpmDb::CHK_OK,	"    Header SHA1 digest: OK" },
    { RpmDb::CHK_OK,	"    Header SHA256 digest: OK" },
    { RpmDb::CHK_FAIL,	"    Payload SHA256 digest: BAD (Expected 6632dfb6e78fd3346baa860da339acdedf6f019fb1b5448ba1baa6cef67de795 != 85156c232f4c76273bbbb134d8d869e93bbfc845dd0d79016856e5356dd33727)" },
    { RpmDb::CHK_FAIL,	"    MD5 digest: BAD (Expected e3f474f75d2d2b267da4ff80fc071dd7 != cebe1e7d39b4356639a0779aa23f6e27)" },
  } };
  BOOST_CHECK_EQUAL( xpct, cs );
}

BOOST_AUTO_TEST_CASE(unsigned_broken_header_pkg)
{
  Pathname rpm { DATADIR/"unsigned_broken_header.rpm" };
  CheckResult cp { gcheckPackage( rpm ) };
  CheckResult cs { gcheckPackageSignature( rpm ) };
  // Unsigned, but a broken digest 'superseeds' CHK_NOSIG
  BOOST_CHECK_EQUAL( cp, cs );

  CheckResult xpct { RpmDb::CHK_FAIL, {
    { RpmDb::CHK_FAIL,	"    Header SHA1 digest: BAD (Expected d6768447e13388b0c35fb151ebfa8f6646a115e9 != dd761ace671a5eb2669b269faf22a3cd72792138)" },
    { RpmDb::CHK_FAIL,	"    Header SHA256 digest: BAD (Expected 2ce9f41bc0de68b4cb1aa1e18c1bea43dfaa01299ae61ef3e4466df332c792e5 != 4a9410db7131cead773afe1876f2490023ccc7dc47cbba47807430c53ea9649d)" },
    { RpmDb::CHK_FAIL,	"    Payload SHA256 digest: BAD (Expected 6632dfb6e78fd3346baa860da339acdedf6f019fb1b5448ba1baa6cef67de795 != 85156c232f4c76273bbbb134d8d869e93bbfc845dd0d79016856e5356dd33727)" },
    { RpmDb::CHK_FAIL,	"    MD5 digest: BAD (Expected e3f474f75d2d2b267da4ff80fc071dd7 != 9afd6b52896d23910280ddded1921071)" },
  } };
  BOOST_CHECK_EQUAL( xpct, cs );
}

BOOST_AUTO_TEST_CASE(signed_pkg_nokey)
{
  Pathname rpm { DATADIR/"signed.rpm" };
  CheckResult cp { gcheckPackage( rpm ) };
  CheckResult cs { gcheckPackageSignature( rpm ) };
  BOOST_CHECK_EQUAL( cp, cs );

  CheckResult xpct { RpmDb::CHK_NOKEY, {
    { RpmDb::CHK_NOKEY,	"    Header V3 RSA/SHA256 Signature, key ID b88b2fd43dbdc284: NOKEY" },
    { RpmDb::CHK_OK,	"    Header SHA1 digest: OK" },
    { RpmDb::CHK_OK,	"    Header SHA256 digest: OK" },
    { RpmDb::CHK_OK,	"    Payload SHA256 digest: OK" },
    { RpmDb::CHK_OK,	"    MD5 digest: OK" },
#ifdef HAVE_RPMTSSETVFYFLAGS
    { RpmDb::CHK_NOKEY,	"    V3 RSA/SHA256 Signature, key ID b88b2fd43dbdc284: NOKEY" },
#endif
  } };
  BOOST_CHECK_EQUAL( xpct, cs );
}

BOOST_AUTO_TEST_CASE(signed_broken_pkg_nokey)
{
  Pathname rpm { DATADIR/"signed_broken.rpm" };
  CheckResult cp { gcheckPackage( rpm ) };
  CheckResult cs { gcheckPackageSignature( rpm ) };
  BOOST_CHECK_EQUAL( cp, cs );

  CheckResult xpct { RpmDb::CHK_FAIL, {
    { RpmDb::CHK_NOKEY,	"    Header V3 RSA/SHA256 Signature, key ID b88b2fd43dbdc284: NOKEY" },
    { RpmDb::CHK_OK,	"    Header SHA1 digest: OK" },
    { RpmDb::CHK_OK,	"    Header SHA256 digest: OK" },
    { RpmDb::CHK_FAIL,	"    Payload SHA256 digest: BAD (Expected 6632dfb6e78fd3346baa860da339acdedf6f019fb1b5448ba1baa6cef67de795 != 85156c232f4c76273bbbb134d8d869e93bbfc845dd0d79016856e5356dd33727)" },
    { RpmDb::CHK_FAIL,	"    MD5 digest: BAD (Expected 8e64684e4d5bd90c3c13f76ecbda9ee2 != 442a473472708c39f3ac2b5eb38b476f)" },
#ifdef HAVE_RPMTSSETVFYFLAGS
    { RpmDb::CHK_FAIL,	"    V3 RSA/SHA256 Signature, key ID b88b2fd43dbdc284: BAD" },
#endif
  } };
  BOOST_CHECK_EQUAL( xpct, cs );
}

BOOST_AUTO_TEST_CASE(signed_broken_header_pkg_nokey)
{
  Pathname rpm { DATADIR/"signed_broken_header.rpm" };
  CheckResult cp { gcheckPackage( rpm ) };
  CheckResult cs { gcheckPackageSignature( rpm ) };
  BOOST_CHECK_EQUAL( cp, cs );

  CheckResult xpct { RpmDb::CHK_FAIL, {
    { RpmDb::CHK_FAIL,	"    Header V3 RSA/SHA256 Signature, key ID b88b2fd43dbdc284: BAD" },
    { RpmDb::CHK_FAIL,	"    Header SHA1 digest: BAD (Expected 9ca2e3aec038e562d33442271ee52c08ded0d637 != 95286fd653f927df0a42746e310861d3f89bb75c)" },
    { RpmDb::CHK_FAIL,	"    Header SHA256 digest: BAD (Expected e88100656c8e06b6e4bb9155f0dd111ef8042866941f02b623cb46e12a82f732 != 76b343bcb9b8aaf9998fdcf7392e234944a0b078c67667fa0d658208b9a66983)" },
    { RpmDb::CHK_FAIL,	"    Payload SHA256 digest: BAD (Expected 6632dfb6e78fd3346baa860da339acdedf6f019fb1b5448ba1baa6cef67de795 != 85156c232f4c76273bbbb134d8d869e93bbfc845dd0d79016856e5356dd33727)" },
    { RpmDb::CHK_FAIL,	"    MD5 digest: BAD (Expected 8e64684e4d5bd90c3c13f76ecbda9ee2 != 81df819a7d94638ff3ffe0bb93a7d177)" },
#ifdef HAVE_RPMTSSETVFYFLAGS
    { RpmDb::CHK_FAIL,	"    V3 RSA/SHA256 Signature, key ID b88b2fd43dbdc284: BAD" },
#endif
  } };
  BOOST_CHECK_EQUAL( xpct, cs );
}


///////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(add_key)
{
  PublicKey key { Pathname(DATADIR)/"signed.key" };
  //cout << key << endl;
  test.target().rpmDb().importPubkey( key );
}
///////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE(signed_pkg_withkey)
{
  Pathname rpm { DATADIR/"signed.rpm" };
  CheckResult cp { gcheckPackage( rpm ) };
  CheckResult cs { gcheckPackageSignature( rpm ) };
  BOOST_CHECK_EQUAL( cp, cs );

  CheckResult xpct { RpmDb::CHK_OK, {
    { RpmDb::CHK_OK,	"    Header V3 RSA/SHA256 Signature, key ID 3dbdc284: OK" },
    { RpmDb::CHK_OK,	"    Header SHA1 digest: OK" },
    { RpmDb::CHK_OK,	"    Header SHA256 digest: OK" },
    { RpmDb::CHK_OK,	"    Payload SHA256 digest: OK" },
    { RpmDb::CHK_OK,	"    MD5 digest: OK" },
#ifdef HAVE_RPMTSSETVFYFLAGS
    { RpmDb::CHK_OK,	"    V3 RSA/SHA256 Signature, key ID 3dbdc284: OK" },
#endif
  } };
  BOOST_CHECK_EQUAL( xpct, cs );
}

BOOST_AUTO_TEST_CASE(signed_broken_pkg_withkey)
{
  Pathname rpm { DATADIR/"signed_broken.rpm" };
  CheckResult cp { gcheckPackage( rpm ) };
  CheckResult cs { gcheckPackageSignature( rpm ) };
  BOOST_CHECK_EQUAL( cp, cs );

  CheckResult xpct { RpmDb::CHK_FAIL, {
    { RpmDb::CHK_OK,	"    Header V3 RSA/SHA256 Signature, key ID b88b2fd43dbdc284: OK" },
    { RpmDb::CHK_OK,	"    Header SHA1 digest: OK" },
    { RpmDb::CHK_OK,	"    Header SHA256 digest: OK" },
    { RpmDb::CHK_FAIL,	"    Payload SHA256 digest: BAD (Expected 6632dfb6e78fd3346baa860da339acdedf6f019fb1b5448ba1baa6cef67de795 != 85156c232f4c76273bbbb134d8d869e93bbfc845dd0d79016856e5356dd33727)" },
    { RpmDb::CHK_FAIL,	"    MD5 digest: BAD (Expected 8e64684e4d5bd90c3c13f76ecbda9ee2 != 442a473472708c39f3ac2b5eb38b476f)" },
#ifdef HAVE_RPMTSSETVFYFLAGS
    { RpmDb::CHK_FAIL,	"    V3 RSA/SHA256 Signature, key ID b88b2fd43dbdc284: BAD" },
#endif
  } };
  BOOST_CHECK_EQUAL( xpct, cs );
}

BOOST_AUTO_TEST_CASE(signed_broken_header_pkg_withkey)
{
  Pathname rpm { DATADIR/"signed_broken_header.rpm" };
  CheckResult cp { gcheckPackage( rpm ) };
  CheckResult cs { gcheckPackageSignature( rpm ) };
  BOOST_CHECK_EQUAL( cp, cs );

  CheckResult xpct { RpmDb::CHK_FAIL, {
    { RpmDb::CHK_FAIL,	"    Header V3 RSA/SHA256 Signature, key ID b88b2fd43dbdc284: BAD" },
    { RpmDb::CHK_FAIL,	"    Header SHA1 digest: BAD (Expected 9ca2e3aec038e562d33442271ee52c08ded0d637 != 95286fd653f927df0a42746e310861d3f89bb75c)" },
    { RpmDb::CHK_FAIL,	"    Header SHA256 digest: BAD (Expected e88100656c8e06b6e4bb9155f0dd111ef8042866941f02b623cb46e12a82f732 != 76b343bcb9b8aaf9998fdcf7392e234944a0b078c67667fa0d658208b9a66983)" },
    { RpmDb::CHK_FAIL,	"    Payload SHA256 digest: BAD (Expected 6632dfb6e78fd3346baa860da339acdedf6f019fb1b5448ba1baa6cef67de795 != 85156c232f4c76273bbbb134d8d869e93bbfc845dd0d79016856e5356dd33727)" },
    { RpmDb::CHK_FAIL,	"    MD5 digest: BAD (Expected 8e64684e4d5bd90c3c13f76ecbda9ee2 != 81df819a7d94638ff3ffe0bb93a7d177)" },
#ifdef HAVE_RPMTSSETVFYFLAGS
    { RpmDb::CHK_FAIL,	"    V3 RSA/SHA256 Signature, key ID b88b2fd43dbdc284: BAD" },
#endif
  } };
  BOOST_CHECK_EQUAL( xpct, cs );
}
