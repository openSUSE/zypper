#include "TestSetup.h"
#include "zypp/ResPool.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/pool/PoolStats.h"
#include "zypp/ui/Selectable.h"

#define BOOST_TEST_MODULE Dup

/////////////////////////////////////////////////////////////////////////////

static TestSetup test;

template <class TIterator>
std::ostream & vdumpPoolStats( std::ostream & str, TIterator begin_r, TIterator end_r )
{
  pool::PoolStats stats;
  for_( it, begin_r, end_r )
  {
    str << *it << endl;
    stats( *it );
  }
  return str << stats;
}

bool upgrade()
{
  bool rres = false;
  {
    rres = getZYpp()->resolver()->doUpgrade();
  }
  if ( ! rres )
  {
    ERR << "upgrade " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "upgrade " << rres << endl;
  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(test.pool()),
                  make_filter_end<resfilter::ByTransact>(test.pool()) ) << endl;
  return true;
}


BOOST_AUTO_TEST_CASE(testcase_init)
{
  //zypp::base::LogControl::instance().logToStdErr();
  test.loadTestcaseRepos( TESTS_SRC_DIR"/data/TCdup" );
  dumpRange( USR, test.pool().knownRepositoriesBegin(),
                  test.pool().knownRepositoriesEnd() ) << endl;
  USR << "pool: " << test.pool() << endl;
  BOOST_REQUIRE( upgrade() );
}
/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(orphaned)
{
  ResPoolProxy proxy( test.poolProxy() );
  BOOST_CHECK_EQUAL( proxy.lookup( ResKind::package, "glibc" )->status(),		ui::S_KeepInstalled );
  BOOST_CHECK_EQUAL( proxy.lookup( ResKind::package, "release-package" )->status(),	ui::S_AutoUpdate );
  BOOST_CHECK_EQUAL( proxy.lookup( ResKind::package, "dropped_required" )->status(),	ui::S_KeepInstalled );
  BOOST_CHECK_EQUAL( proxy.lookup( ResKind::package, "dropped" )->status(),		ui::S_AutoDel );
}
