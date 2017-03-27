#include <boost/test/auto_unit_test.hpp>

#include <iostream>

#include "TestSetup.h"
#include <zypp/base/LogTools.h>

#include <zypp/ResObjects.h>
#include <zypp/ResPool.h>

using boost::unit_test::test_case;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using namespace zypp;

static TestSetup test;

void testcase_init()
{
//   cout << "+++[repoinit]=======================" << endl;
  test.loadTestcaseRepos( TESTS_SRC_DIR"/data/PoolReuseIds/SeqA" );
//   for ( auto && pi : ResPool::instance() )
//     cout << pi << " " << pi.resolvable() << endl;
//   cout << "---[repoinit]=======================" << endl;
}

void testcase_init2()
{
//   cout << "+++[repoinit2]=======================" << endl;
  sat::Pool::instance().reposEraseAll();
  test.loadTestcaseRepos( TESTS_SRC_DIR"/data/PoolReuseIds/SeqB" );
//   for ( auto && pi : ResPool::instance() )
//     cout << pi << " " << pi.resolvable() << endl;
//   cout << "---[repoinit2]=======================" << endl;
}

void checkpi( const PoolItem & pi )
{
  BOOST_CHECK( pi.resolvable() );
  BOOST_CHECK_EQUAL( pi.id(),	pi.resolvable()->id() );
  BOOST_CHECK_EQUAL( bool(asKind<Package>( pi.resolvable() )),	    isKind<Package>(pi) );
  BOOST_CHECK_EQUAL( bool(asKind<Patch>( pi.resolvable() )),	    isKind<Patch>(pi) );
  BOOST_CHECK_EQUAL( bool(asKind<Pattern>( pi.resolvable() )),	    isKind<Pattern>(pi) );
  BOOST_CHECK_EQUAL( bool(asKind<Product>( pi.resolvable() )),	    isKind<Product>(pi) );
  BOOST_CHECK_EQUAL( bool(asKind<SrcPackage>( pi.resolvable() )),   isKind<SrcPackage>(pi) );
  BOOST_CHECK_EQUAL( bool(asKind<Application>( pi.resolvable() )),  isKind<Application>(pi) );
}

void repocheck()
{
//   cout << "+++[repocheck]======================" << endl;
  for ( auto && pi : ResPool::instance() )
  {
//     cout << "??? " << pi << endl;
    checkpi( pi );
  }
//   cout << "---[repocheck]======================" << endl;
}

///////////////////////////////////////////////////////////////////
// Check that after ERASING ALL REPOS and loading a new one, ResPool
// actually creates new PoolItems rather than reusing already existing
// ones.
//
// Adding/removing repos will not reuse poolIDs unless actually all
// repos are removed from the pool. In this case ResPool must invalidate
// ALL existing PoolItems (especially the Resolvable Pointers inside).
//
//  SeqA		  SeqB
//  (1)package		- (1)application
//  (2)pattern		- (2)package
//  ...			  ...
//
// The two test repos have Resolvables of different kind in different
// order. If ResPool fails to recreate the PoolItem, we'll experience
// cast errors. PoolItem(1) will claim to be an application, but the
// Resolvable is still the original one created for a package...
///////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(t_1)	{ testcase_init(); }
BOOST_AUTO_TEST_CASE(t_2)	{ repocheck(); }
BOOST_AUTO_TEST_CASE(t_4)	{ testcase_init2(); }
BOOST_AUTO_TEST_CASE(t_5)	{ repocheck(); }
