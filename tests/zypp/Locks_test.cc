#include <stdio.h>
#include <iostream>
#include <iterator>
#include <boost/test/auto_unit_test.hpp>
#include <list>

#include "zypp/PoolQuery.h"
#include "zypp/PoolQueryUtil.tcc"
#include "zypp/TmpPath.h"
#include "zypp/Locks.h"
#include "TestSetup.h"

#define BOOST_TEST_MODULE Locks

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;

bool isLocked( const sat::Solvable & solvable )
{
  zypp::PoolItem pi( zypp::ResPool::instance().find( solvable ) );
  if( pi.status().isLocked() )
    return true;
  return false;
}

BOOST_AUTO_TEST_CASE(pool_query_init)
{
  TestSetup test( Arch_x86_64 );
  //test.loadTarget(); // initialize and load target
  test.loadRepo( TESTS_SRC_DIR "/data/openSUSE-11.1", "opensuse" );
  test.loadRepo( TESTS_SRC_DIR "/data/OBS_zypp_svn-11.1", "@System" );
}


/////////////////////////////////////////////////////////////////////////////
//  0xx basic queries
/////////////////////////////////////////////////////////////////////////////

// default query + one search string
// q.addString("foo");
// result: all resolvables having at least one attribute matching foo
BOOST_AUTO_TEST_CASE(locks_1)
{
  cout << "****001****"  << endl;
  PoolQuery q;
  q.addString("zypper");
  Locks::instance().addLock(q);
  for_(it,q.begin(),q.end())
  {
    BOOST_CHECK(isLocked(*it));
  }
  Locks::instance().removeLock(q); //clear before next test
}

BOOST_AUTO_TEST_CASE(locks_save_load)
{
  cout << "****save/load****"  << endl;
  Pathname src(TESTS_SRC_DIR);
    src += "zypp/data/Locks/locks";
  Locks::instance().readAndApply(src);
  PoolQuery q;
  q.addString("zypper");
  for_(it,q.begin(),q.end())
  {
    BOOST_CHECK(isLocked(*it));
  }
#if 1 
  filesystem::TmpFile testfile;
  //Pathname testfile(TESTS_SRC_DIR);
    //  testfile += "/zypp/data/Locks/testlocks";
  Locks::instance().removeLock(q); 
  Locks::instance().save(testfile);
  Locks::instance().readAndApply(testfile);
  //now unlocked
  for_(it,q.begin(),q.end())
  {
    BOOST_CHECK(!isLocked(*it));
  }
  BOOST_CHECK(Locks::instance().size()==0);
#endif
}

BOOST_AUTO_TEST_CASE(locks_save_without_redundancy)
{
  cout << "****save without redundancy****"  << endl;
  PoolQuery q;
  q.addString("zypper");
  Locks& locks = Locks::instance();
  locks.addLock(q);
  locks.addLock(q);
  locks.merge();
  BOOST_CHECK( locks.size()==1 );
  locks.addLock(q);
  locks.merge();
  BOOST_CHECK( locks.size()==1 );
  locks.removeLock(q);
  locks.merge();
  BOOST_CHECK( locks.size() == 0 );
}

BOOST_AUTO_TEST_CASE( locks_empty )
{
  cout << "****test and clear empty locks****"  << endl;
  PoolQuery q;
  q.addString("foo-bar-nonexist");
  Locks& locks = Locks::instance();
  locks.addLock(q);
  locks.merge(); //only need merge list
  BOOST_CHECK( locks.existEmpty() );
  locks.removeEmpty();
  BOOST_CHECK( locks.size() == 0 );
}
