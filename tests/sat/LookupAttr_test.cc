#include "TestSetup.h"
#include <zypp/sat/LookupAttr.h>

#define LABELED(V) #V << ":\t" << V

static TestSetup test( Arch_x86_64 );

// Must be the first test!
BOOST_AUTO_TEST_CASE(bnc_435838)
{
  // On the fly check that findSystemRepo does not
  // cause loading the SystemRepo. check 2 times.
  BOOST_REQUIRE( ! test.satpool().findSystemRepo() );
  BOOST_REQUIRE( ! test.satpool().findSystemRepo() );

  // empty @system to pool
  test.satpool().systemRepo();
  BOOST_REQUIRE( test.satpool().findSystemRepo() );

  // bnc_435838 crashes if iterating a just created repo.
  sat::LookupAttr q( sat::SolvAttr::name );
  for_( it, q.begin(),q.end() )
    ;
}

BOOST_AUTO_TEST_CASE(LookupAttr_init)
{
  test.loadTarget(); // initialize and load target
  test.loadRepo( TESTS_SRC_DIR "/data/openSUSE-11.1" );
  test.loadRepo( TESTS_SRC_DIR "/data/OBS:VirtualBox-11.1" );
  test.loadRepo( TESTS_SRC_DIR "/data/11.0-update" );
}

BOOST_AUTO_TEST_CASE(LookupAttr_defaultconstructed)
{
  sat::LookupAttr q;
  BOOST_CHECK( q.empty() );
  BOOST_CHECK( q.size() == 0 );
  BOOST_CHECK_EQUAL( q.begin(), q.end() );
}

BOOST_AUTO_TEST_CASE(LookupAttr_nonexistingattr)
{
  sat::LookupAttr q( sat::SolvAttr("nonexistingattr") );
  BOOST_CHECK( q.empty() );
  BOOST_CHECK( q.size() == 0 );
  BOOST_CHECK_EQUAL( q.begin(), q.end() );
}

BOOST_AUTO_TEST_CASE(LookupAttr_existingattr)
{
  sat::LookupAttr q( sat::SolvAttr::name );
  BOOST_CHECK( ! q.empty() );
  BOOST_CHECK( q.size() != 0 );
  BOOST_CHECK_NE( q.begin(), q.end() );
}

BOOST_AUTO_TEST_CASE(LookupAttr_)
{
//   base::LogControl::TmpLineWriter shutUp( new log::FileLineWriter( "/tmp/YLOG" ) );
//   sat::LookupAttr q( sat::SolvAttr::name );
//   MIL << "HI" << endl;
}