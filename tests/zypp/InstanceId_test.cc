#include "TestSetup.h"
#include <zypp/InstanceId.h>

#define BOOST_TEST_MODULE InstanceId

/////////////////////////////////////////////////////////////////////////////
static TestSetup test( TestSetup::initLater );
struct TestInit {
  TestInit() {
    test = TestSetup( Arch_x86_64 );

    // Abuse;) vbox as System repo:
    test.loadTargetRepo( TESTS_SRC_DIR "/data/obs_virtualbox_11_1" );
    test.loadRepo( TESTS_SRC_DIR "/data/openSUSE-11.1", "opensuse" );
    test.loadRepo( TESTS_SRC_DIR "/data/OBS_zypp_svn-11.1", "zyppsvn" );
  }
  ~TestInit() { test.reset(); }
};
BOOST_GLOBAL_FIXTURE( TestInit );
/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(default_constructed)
{
  InstanceId instanceId;
  BOOST_CHECK_EQUAL( instanceId.getNamespace(), std::string() );

  BOOST_CHECK_EQUAL( instanceId.isSystemId( "System" ), false );
  BOOST_CHECK_EQUAL( instanceId.isSystemId( "@System" ), true );

  BOOST_CHECK_EQUAL( instanceId(""), PoolItem() );
  BOOST_CHECK_EQUAL( instanceId(PoolItem()), "" );
}

BOOST_AUTO_TEST_CASE(convert)
{
  InstanceId instanceId;
  instanceId.setNamespace( "SUSE" );
  BOOST_CHECK_EQUAL( instanceId.getNamespace(), "SUSE" );

  ResPool pool( ResPool::instance() );
  for_( it, pool.begin(), pool.end() )
  {
    std::cout << instanceId(*it) << endl;
    BOOST_CHECK_EQUAL( instanceId(instanceId(*it)), *it );
  }
}
