#include "TestSetup.h"
#include <zypp/sat/LookupAttr.h>
#include <zypp/ResObjects.h>

#include <zypp/sat/detail/PoolImpl.h>

#define LABELED(V) #V << ":\t" << V

static TestSetup test( "/tmp/x", Arch_x86_64 );

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

BOOST_AUTO_TEST_CASE(LookupAttr_iterate_solvables)
{
  // sat::SolvAttr::name query should visit each solvable once.
  // So query size and containersize are to be equal if we query
  // pool/repo/solvable. Quick check whether the iterators
  // position info matches the result.

  sat::Pool satpool( test.satpool() );

  {
    // iterate all:
    sat::LookupAttr q( sat::SolvAttr::name );
    BOOST_CHECK_EQUAL( q.size(), satpool.solvablesSize() );
    // quick test whether iterator positions actually matches the result:
    for_( res, q.begin(), q.end() )
    {
      BOOST_CHECK_EQUAL( res.inRepo(), res.inSolvable().repository() );
      BOOST_CHECK_EQUAL( res.inSolvAttr(), sat::SolvAttr::name );
    }
  }
  {
    unsigned total = 0;
    for_( it, satpool.reposBegin(), satpool.reposEnd() )
    {
      // iterate one repo:
      sat::LookupAttr q( sat::SolvAttr::name, *it );
      BOOST_CHECK_EQUAL( q.size(), it->solvablesSize() );
      total += q.size();
      // test result actually matches the repo:
      for_( res, q.begin(), q.end() )
      {
        BOOST_CHECK_EQUAL( res.inRepo(), *it );
        BOOST_CHECK_EQUAL( res.inRepo(), res.inSolvable().repository() );
        BOOST_CHECK_EQUAL( res.inSolvAttr(), sat::SolvAttr::name );
      }
    }
    BOOST_CHECK_EQUAL( total, satpool.solvablesSize() );
  }
  {
    unsigned total = 0;
    for_( it, satpool.solvablesBegin(), satpool.solvablesEnd() )
    {
      // iterate one solvable:
      sat::LookupAttr q( sat::SolvAttr::name, *it );
      BOOST_CHECK_EQUAL( q.size(), 1 );
      total += q.size();
      // test result actually matches the solvable:
      for_( res, q.begin(), q.end() )
      {
        BOOST_CHECK_EQUAL( res.inSolvable(), *it );
        BOOST_CHECK_EQUAL( res.inRepo(), res.inSolvable().repository() );
        BOOST_CHECK_EQUAL( res.inSolvAttr(), sat::SolvAttr::name );
     }
    }
    BOOST_CHECK_EQUAL( total, satpool.solvablesSize() );
  }
}

BOOST_AUTO_TEST_CASE(LookupAttr_itetate_all_attributes)
{
  sat::Pool satpool( test.satpool() );

  // iterate all:
  sat::LookupAttr all( sat::SolvAttr::allAttr );

  {
    unsigned total = 0;
    for_( it, satpool.reposBegin(), satpool.reposEnd() )
    {
      // iterate one repo:
      sat::LookupAttr q( sat::SolvAttr::allAttr, *it );
      total += q.size();
    }
    BOOST_CHECK_EQUAL( total, all.size() );
  }
  {
    unsigned total = 0;
    for_( it, satpool.solvablesBegin(), satpool.solvablesEnd() )
    {
      // iterate one solvable:
      sat::LookupAttr q( sat::SolvAttr::allAttr, *it );
      total += q.size();
    }
    BOOST_CHECK_EQUAL( total, all.size() );
 }
}

BOOST_AUTO_TEST_CASE(LookupAttr_solvable_attribute_types)
{
  base::LogControl::TmpLineWriter shutUp( new log::FileLineWriter( "/tmp/YLOG" ) );
  MIL << "GO" << endl;

  ResPool pool( test.pool() );
  for_( it, pool.byKindBegin<Patch>(), pool.byKindEnd<Patch>() )
  {
    Patch::constPtr p( (*it)->asKind<Patch>() );
    USR << p << endl;

    sat::LookupAttr q( sat::SolvAttr::allAttr, p->satSolvable() );
    //sat::LookupAttr q( sat::SolvAttr("update:reference") );
    for_( res, q.begin(), q.end() )
    {
      if ( //res.inSolvAttr() == sat::SolvAttr("update:reference") &&
           res.solvAttrType() == IdString("repokey:type:flexarray").id() )
      {
        MIL << res << endl;
        DBG << *res << endl;
        ::_Dataiterator * dip = res.get();
        INT << dip << endl;

        ::dataiterator_setpos( dip );

        ::Dataiterator di2;
        ::dataiterator_init( &di2
            , sat::Pool::instance().get()
            , 0
            , SOLVID_POS
            , 0
            , 0
            , 0 );

        while ( ::dataiterator_step( &di2 ) )
        {
          DBG << di2 << endl;
        }
      }
    }
    break;
  }

  {
    sat::LookupAttr q( sat::SolvAttr("update:reference") );
    USR << q << " " << q.size() << endl;
  }
  {
    sat::LookupAttr q( sat::SolvAttr("update:reference:href") );
    USR << q << " " << q.size() << endl;
  }


}


BOOST_AUTO_TEST_CASE(LookupAttr_)
{
  base::LogControl::TmpLineWriter shutUp( new log::FileLineWriter( "/tmp/YLOG" ) );
  MIL << "GO" << endl;
}







