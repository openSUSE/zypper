#include "TestSetup.h"
#include <zypp/sat/LookupAttr.h>
#include <zypp/base/StrMatcher.h>
#include <zypp/ResObjects.h>

static TestSetup test( Arch_x86_64 );

// Must be the first test!
BOOST_AUTO_TEST_CASE(bnc_435838)
{
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
  //test.loadTarget(); // initialize and load target
  test.loadRepo( TESTS_SRC_DIR "/data/openSUSE-11.1" );
  test.loadRepo( TESTS_SRC_DIR "/data/obs_virtualbox_11_1" );
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

BOOST_AUTO_TEST_CASE(LookupAttr_existingattr_matcher)
{
  sat::LookupAttr q( sat::SolvAttr::name );

  BOOST_CHECK_THROW( q.setStrMatcher( StrMatcher("[]ypper",Match::REGEX) ), MatchInvalidRegexException );
  BOOST_CHECK( ! q.strMatcher() );
  BOOST_CHECK_NO_THROW( q.setStrMatcher( StrMatcher("[zZ]ypper",Match::REGEX) ) );
  BOOST_CHECK( q.strMatcher() );

  BOOST_CHECK_EQUAL( q.size(), 9 );
  for_(it,q.begin(),q.end())
  { cout << it << endl;}
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

BOOST_AUTO_TEST_CASE(LookupAttr_solvable_attribute_substructure)
{
  sat::LookupAttr q( sat::SolvAttr::updateReference );
  BOOST_CHECK_EQUAL( q.size(), 303 );

  for_( res, q.begin(), q.end() )
  {
    BOOST_CHECK( ! res.subEmpty() );
    BOOST_CHECK_EQUAL( res.subSize(), 4 );

    BOOST_CHECK_EQUAL( res.subFind( sat::SolvAttr::allAttr ), res.subBegin() );
    BOOST_CHECK_EQUAL( res.subFind( "" ),                     res.subBegin() );

    BOOST_CHECK_EQUAL( res.subFind( sat::SolvAttr::updateReference ), res.subEnd() );
    BOOST_CHECK_EQUAL( res.subFind( "noval" ),                        res.subEnd() );

    BOOST_CHECK_NE( res.subFind( sat::SolvAttr::updateReferenceType ),  res.subEnd() );
    BOOST_CHECK_NE( res.subFind( sat::SolvAttr::updateReferenceHref ),  res.subEnd() );
    BOOST_CHECK_NE( res.subFind( sat::SolvAttr::updateReferenceId ),    res.subEnd() );
    BOOST_CHECK_NE( res.subFind( sat::SolvAttr::updateReferenceTitle ), res.subEnd() );

    BOOST_CHECK_EQUAL( res.subFind( sat::SolvAttr::updateReferenceType ),  res.subFind( "type" ) );
    BOOST_CHECK_EQUAL( res.subFind( sat::SolvAttr::updateReferenceHref ),  res.subFind( "href" ) );
    BOOST_CHECK_EQUAL( res.subFind( sat::SolvAttr::updateReferenceId ),    res.subFind( "id" ) );
    BOOST_CHECK_EQUAL( res.subFind( sat::SolvAttr::updateReferenceTitle ), res.subFind( "title" ) );

    // repeatedly calling subBegin() is ok:
    BOOST_CHECK_EQUAL( res.subFind( sat::SolvAttr::updateReferenceType ).subBegin(),  res.subBegin() );
  }

  // search substructure id without parent-structure works for wellknown structures:
  q = sat::LookupAttr( sat::SolvAttr::updateReferenceId );
  BOOST_CHECK_EQUAL( q.size(), 303 );

  // search id in parent-structure:
  q = sat::LookupAttr( sat::SolvAttr::updateReferenceId, sat::SolvAttr::updateReference );
  BOOST_CHECK_EQUAL( q.size(), 303 );

  // search id in any parent-structure:
  q = sat::LookupAttr( sat::SolvAttr::updateReferenceId, sat::SolvAttr::allAttr );
  BOOST_CHECK_EQUAL( q.size(), 303 );

  // search any id in parent-structure: (4 ids per updateReference)
  q = sat::LookupAttr( sat::SolvAttr::allAttr, sat::SolvAttr::updateReference );
  BOOST_CHECK_EQUAL( q.size(), 1212 );

  // search any id in any parent-structure:
  q = sat::LookupAttr( sat::SolvAttr::allAttr, sat::SolvAttr::allAttr );
  BOOST_CHECK_EQUAL( q.size(), 10473 );
}

BOOST_AUTO_TEST_CASE(LookupAttr_repoattr)
{
  sat::LookupAttr q( sat::SolvAttr::repositoryAddedFileProvides, sat::LookupAttr::REPO_ATTR );
  BOOST_CHECK( ! q.empty() );
  BOOST_CHECK_EQUAL( q.size(), 264 );

  sat::LookupRepoAttr p( sat::SolvAttr::repositoryAddedFileProvides );
  BOOST_CHECK( ! p.empty() );
  BOOST_REQUIRE_EQUAL( p.size(), q.size() );

  sat::LookupRepoAttr::iterator pit( p.begin() );
  for_( qit, q.begin(), q.end() )
  {
    BOOST_CHECK_EQUAL( qit, pit );
    ++pit;
  }
}

#if 0
BOOST_AUTO_TEST_CASE(LookupAttr_)
{
  base::LogControl::TmpLineWriter shutUp( new log::FileLineWriter( "/tmp/YLOG" ) );
  MIL << "GO" << endl;
}
#endif
