#include "TestSetup.h"
#include <zypp/Repository.h>
#include <zypp/sat/Pool.h>

static TestSetup test( Arch_x86_64 );

namespace zypp { namespace detail {
  /** \relates RepositoryIterator Stream output */
  inline std::ostream & operator<<( std::ostream & str, const RepositoryIterator & obj )
  {
    str << "RI["<< *obj <<"]";
    return str;
  }
}}

// Must be the first test!
BOOST_AUTO_TEST_CASE(findSystemRepo)
{
  // On the fly check that findSystemRepo does not
  // cause loading the SystemRepo. check 2 times.
  BOOST_REQUIRE( ! test.satpool().findSystemRepo() );
  BOOST_REQUIRE( ! test.satpool().findSystemRepo() );
}

void checkRepoIter()
{
  sat::Pool satpool( test.satpool() );
  sat::Pool::size_type count = satpool.reposSize();
  for_( it, satpool.reposBegin(), satpool.reposEnd() )
  {
    cout << "- " << count << " " << *it << endl;
    BOOST_CHECK( *it );
    --count;
  }
  BOOST_CHECK_EQUAL( count, 0 );
}


BOOST_AUTO_TEST_CASE(repolist)
{
  // libzypp-11: underlying libsolv changed the pools repository
  // pointer list. It may now contain emebeded NULLs which have
  // to be skipped when iterating the repos.
  //
  sat::Pool satpool( test.satpool() );
  BOOST_CHECK( satpool.reposEmpty() );
  BOOST_CHECK_EQUAL( satpool.reposSize(), 0 );
  checkRepoIter();

  // empty @system to pool
  test.satpool().systemRepo();
  BOOST_REQUIRE( satpool.findSystemRepo() );
  BOOST_CHECK( !satpool.reposEmpty() );
  BOOST_CHECK_EQUAL( satpool.reposSize(), 1 );
  checkRepoIter();

  test.loadRepo( TESTS_SRC_DIR "/data/obs_virtualbox_11_1" );
  BOOST_CHECK( !satpool.reposEmpty() );
  BOOST_CHECK_EQUAL( satpool.reposSize(), 2 );
  checkRepoIter();

  test.loadRepo( TESTS_SRC_DIR "/data/11.0-update" );
  BOOST_CHECK( !satpool.reposEmpty() );
  BOOST_CHECK_EQUAL( satpool.reposSize(), 3 );
  checkRepoIter();

  satpool.reposErase( ":obs_virtualbox_11_1" );
  BOOST_CHECK( !satpool.reposEmpty() );
  BOOST_CHECK_EQUAL( satpool.reposSize(), 2 );
  checkRepoIter();

  satpool.reposErase( ":11.0-update" );
  test.loadRepo( TESTS_SRC_DIR "/data/openSUSE-11.1" );
  BOOST_CHECK( !satpool.reposEmpty() );
  BOOST_CHECK_EQUAL( satpool.reposSize(), 2 );
  checkRepoIter();



/*  for_( it, satpool.reposBegin(),  satpool.reposEnd() )
  {
    cout << "- " << *it << endl;
  }*/
  //test.loadRepo( TESTS_SRC_DIR "/data/openSUSE-11.1" );
}

#if 0
BOOST_AUTO_TEST_CASE(LookupAttr_)
{
  base::LogControl::TmpLineWriter shutUp( new log::FileLineWriter( "/tmp/YLOG" ) );
  MIL << "GO" << endl;
}
#endif
