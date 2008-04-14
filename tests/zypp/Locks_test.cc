#include <stdio.h>
#include <iostream>
#include <iterator>
#include <boost/test/auto_unit_test.hpp>
#include <list>

#include "zypp/ZYppFactory.h"
#include "zypp/PoolQuery.h"
#include "zypp/PoolQueryUtil.tcc"
#include "zypp/TmpPath.h"
#include "zypp/Locks.h"

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
  cout << "non-locked resolvable" << pi.resolvable() << endl;
  // name: yast2-sound 2.16.2-9 i586
  return false;
}

static void init_pool()
{
  Pathname dir(TESTS_SRC_DIR);
  dir += "/zypp/data/Locks";

  ZYpp::Ptr z = getZYpp();
  ZConfig::instance().setSystemArchitecture(Arch("i586"));

  RepoInfo i1; i1.setAlias("factory");
  sat::Pool::instance().addRepoSolv(dir / "factory.solv", i1);
  RepoInfo i2; i2.setAlias("@System");
  sat::Pool::instance().addRepoSolv(dir / "@System.solv", i2);
}

BOOST_AUTO_TEST_CASE(pool_query_init)
{
  init_pool();
}

/////////////////////////////////////////////////////////////////////////////
//  0xx basic queries
/////////////////////////////////////////////////////////////////////////////

// no conditions, default query
// result: all available resolvables
BOOST_AUTO_TEST_CASE(locks_1)
{
  cout << "****000****"  << endl;
  PoolQuery q;
  Locks::instance().addLock(q);
  for_(it,q.begin(),q.end())
  {
    BOOST_CHECK(isLocked(*it));
  }
}

// default query + one search string
// q.addString("foo");
// result: all resolvables having at least one attribute matching foo
BOOST_AUTO_TEST_CASE(locks_2)
{
  cout << "****001****"  << endl;
  PoolQuery q;
  q.addString("zypper");
  Locks::instance().addLock(q);
  for_(it,q.begin(),q.end())
  {
    BOOST_CHECK(isLocked(*it));
  }
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
  Locks::instance().removeLock(q);
  for_(it,q.begin(),q.end())
  {
    BOOST_CHECK(!isLocked(*it));
  }
#if 1 
  filesystem::TmpFile testfile;
  //Pathname testfile(TESTS_SRC_DIR);
    //  testfile += "/zypp/data/Locks/testlocks";
  Locks::instance().save(testfile);
  Locks::instance().readAndApply(testfile);
  //still locked
  for_(it,q.begin(),q.end())
  {
    BOOST_CHECK(isLocked(*it));
  }
  Locks::instance().removeLock(q); //need twice because finded from previous test
  Locks::instance().save(testfile);
  Locks::instance().readAndApply(testfile);
  //now unlocked - first unlock remove indetical lock from previous test
  //and next unlock remove lock from lockfile
  for_(it,q.begin(),q.end())
  {
    BOOST_CHECK(!isLocked(*it));
  }
#endif
}
