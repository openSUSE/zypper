#include "TestSetup.h"
#include "zypp/ResPool.h"
#include "zypp/ui/Selectable.h"

#define BOOST_TEST_MODULE Selectable

/////////////////////////////////////////////////////////////////////////////

static TestSetup test;

BOOST_AUTO_TEST_CASE(testcase_init)
{
  zypp::base::LogControl::instance().logToStdErr();
  test.loadTestcaseRepos( TESTS_SRC_DIR"/data/TCSelectable" );

/*  dumpRange( USR, test.pool().knownRepositoriesBegin(),
                  test.pool().knownRepositoriesEnd() ) << endl;
  USR << "pool: " << test.pool() << endl;*/
}
/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(candiadate)
{
  ResPoolProxy poolProxy( test.poolProxy() );
  ui::Selectable::Ptr s( poolProxy.lookup( ResKind::package, "candidate" ) );
  //   (I 1) {
  //   I__s_(8)candidate-1-1.i586(@System)(openSUSE)
  // } (A 6) {
  //   U__s_(2)candidate-4-1.x86_64(RepoHIGH)(unkown)
  //   U__s_(3)candidate-4-1.i586(RepoHIGH)(unkown)
  //   U__s_(6)candidate-0-1.x86_64(RepoMID)(SUSE)
  //   U__s_(7)candidate-0-1.i586(RepoMID)(SUSE) <- candidate (highrst prio matching arch and vendor)
  //   U__s_(4)candidate-2-1.x86_64(RepoLOW)(openSUSE)
  //   U__s_(5)candidate-2-1.i586(RepoLOW)(openSUSE)
  // }
  BOOST_CHECK_EQUAL( s->candidateObj()->repoInfo().alias(), "RepoMID" );
  BOOST_CHECK_EQUAL( s->candidateObj()->edition(), Edition("0-1") );
  // no updateCandidate due to low version
  BOOST_CHECK_EQUAL( s->updateCandidateObj(), PoolItem() );
}
