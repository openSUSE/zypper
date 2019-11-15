#include <boost/test/auto_unit_test.hpp>
#define BOOST_CHECK_MODULE Resolver
using namespace boost::unit_test;

#include "TestSetup.h"
#include "zypp/ResPool.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/pool/PoolStats.h"
#include "zypp/ui/Selectable.h"

static TestSetup test( TestSetup::initLater );

struct BAD_TESTCASE {};

typedef std::set<PoolItem> PoolItemSet;

constexpr const unsigned onlyRequires	= 0x1;
constexpr const unsigned inrMode	= 0x2;

PoolItemSet resolve( unsigned flags_r = 0 )
{
  test.resolver().setOnlyRequires            ( flags_r & onlyRequires );
  test.resolver().setIgnoreAlreadyRecommended( ! ( flags_r & inrMode ) );
  if ( ! test.resolver().resolvePool() )
    throw BAD_TESTCASE();

  return { make_filter_begin<resfilter::ByTransact>(test.pool()), make_filter_end<resfilter::ByTransact>(test.pool()) };
}

inline PoolItem getPi( const std::string & name_r, bool installed_r )
{
  for ( const auto & pi : test.pool().byName( name_r ) )
  { if ( pi.isSystem() == installed_r ) return pi; }
  throw BAD_TESTCASE();
}
inline PoolItem getIPi( const std::string & name_r )
{ return getPi( name_r, true ); }
inline PoolItem getAPi( const std::string & name_r )
{ return getPi( name_r, false ); }

/////////////////////////////////////////////////////////////////////////////
// Pool content:
PoolItem Ip;	// IA: aspell
PoolItem Ap;
PoolItem Ipen;	// IA: aspell-en	(wanted locale)
PoolItem Apen;
PoolItem Apde;	//  A: aspell-de	(wanted locale)
PoolItem Apfr;	//  A: aspell-fr	(unwanted locale)
PoolItem Aprec;	//  A: recommended-pkg	(by aspell)

struct TestInit {
  TestInit() {
    test = TestSetup( );

    test.loadTestcaseRepos( TESTS_SRC_DIR"/data/TCNamespaceRecommends" );
    Ip	= getIPi( "aspell" );
    Ap	= getAPi( "aspell" );
    Ipen	= getIPi( "aspell-en" );
    Apen	= getAPi( "aspell-en" );
    Apde	= getAPi( "aspell-de" );
    Apfr	= getAPi( "aspell-fr" );
    Aprec	= getAPi( "recommended-pkg" );
  }
  ~TestInit() { test.reset(); }
};
BOOST_GLOBAL_FIXTURE( TestInit );

/////////////////////////////////////////////////////////////////////////////

inline void BOOST_checkresult( const PoolItemSet & resolved_r, const PoolItemSet & expected_r )
{ BOOST_CHECK_EQUAL( resolved_r, expected_r ); }


BOOST_AUTO_TEST_CASE(install)
{
  Ap.status().setTransact( true, ResStatus::USER );
  // Upadte aspell, add all recommends
  BOOST_checkresult( resolve(), { Ap, Ip, Apde, Aprec } );
  Ap.status().setTransact( false, ResStatus::USER );
}

BOOST_AUTO_TEST_CASE(installOnlyRequires)
{
  Ap.status().setTransact( true, ResStatus::USER );
  // Upadte aspell, add only namespace recommends
  BOOST_checkresult( resolve( onlyRequires ), { Ap, Ip, Apde } );
  Ap.status().setTransact( false, ResStatus::USER );
}

BOOST_AUTO_TEST_CASE(inr)
{
  // Fillup all recommends
  BOOST_checkresult( resolve( inrMode ), { Apde, Aprec } );
}

BOOST_AUTO_TEST_CASE(inrOnlyRequires)
{
  // Fillup only namespace recommends
  BOOST_checkresult( resolve( inrMode|onlyRequires ), { Apde } );
}
