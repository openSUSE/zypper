#include <stdio.h>
#include <iostream>
#include <iterator>
#include <boost/test/auto_unit_test.hpp>
#include <list>

#include "zypp/ZYppFactory.h"
#include "zypp/PoolQuery.h"
#include "zypp/PoolQueryUtil.tcc"
#include "zypp/TmpPath.h"

#define BOOST_TEST_MODULE PoolQuery

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;

bool result_cb( const sat::Solvable & solvable )
{
  zypp::PoolItem pi( zypp::ResPool::instance().find( solvable ) );
  cout << pi.resolvable() << endl;
  return true;
}

static void init_pool()
{
  Pathname dir(TESTS_SRC_DIR);
  dir += "/zypp/data/PoolQuery";

  ZYpp::Ptr z = getZYpp();
  ZConfig::instance().setSystemArchitecture(Arch("i586"));

  RepoInfo i1; i1.setAlias("factory");
  sat::Pool::instance().addRepoSolv(dir / "factory.solv", i1);
  RepoInfo i2; i2.setAlias("factory-nonoss");
  sat::Pool::instance().addRepoSolv(dir / "factory-nonoss.solv", i2);
  RepoInfo i3; i3.setAlias("zypp_svn");
  sat::Pool::instance().addRepoSolv(dir / "zypp_svn.solv", i3);
  sat::Pool::instance().addRepoSolv(dir / "@System.solv");
}

BOOST_AUTO_TEST_CASE(pool_query_init)
{
  init_pool();
}

// no conditions, default query
// result: all available resolvables
BOOST_AUTO_TEST_CASE(pool_query_1)
{
  cout << "****1****"  << endl;
  PoolQuery q;
  cout << q.size() << endl;
  BOOST_CHECK(q.size() == 11449);
}

// default query + one search string
// q.addString("foo");
// result: all resolvables having at least one attribute matching foo
BOOST_AUTO_TEST_CASE(pool_query_2)
{
  cout << "****2****"  << endl;
  PoolQuery q;
  q.addString("zypper");

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 16);
}

// default query + one attribute + one string
// q.addAttribute(foo, bar);
// should be the same as
// q.addAttribute(foo); q.addString(bar);
// result: resolvables with foo containing bar
BOOST_AUTO_TEST_CASE(pool_query_3)
{
  cout << "****3****"  << endl;
  PoolQuery q;
  q.addString("zypper");
  q.addAttribute(sat::SolvAttr::name);

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 6);

  cout << endl;

  PoolQuery q1;
  q1.addAttribute(sat::SolvAttr::name, "zypper");

  std::for_each(q1.begin(), q1.end(), &result_cb);
  BOOST_CHECK(q1.size() == 6);
}


// default query + one attribute(one string) + one repo
// q.addRepo(foorepo);
// q.addAttribute(solvable:name, foo);
// FAILS
BOOST_AUTO_TEST_CASE(pool_query_4)
{
  cout << "****4****"  << endl;
  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, "zypper");
  q.addRepo("zypp_svn");

  PoolQuery::ResultIterator it = q.begin();
  std::for_each(
    it,
    q.end(),
    &result_cb);
  BOOST_CHECK(q.size() == 3);
}

BOOST_AUTO_TEST_CASE(pool_query_5)
{
  cout << "****5****"  << endl;
  PoolQuery q;
  q.addRepo("zypp_svn");

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 21);
}

BOOST_AUTO_TEST_CASE(pool_query_6)
{
  cout << "****6****"  << endl;
  PoolQuery q;
  q.addString("browser");
  q.addAttribute(sat::SolvAttr::name);
  q.addAttribute(sat::SolvAttr::summary);
  q.addAttribute(sat::SolvAttr::description);

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 15);

  cout << endl;

  PoolQuery q1;
  q1.addString("browser");
  q1.addAttribute(sat::SolvAttr::name);

  std::for_each(q1.begin(), q1.end(), &result_cb);
  BOOST_CHECK(q1.size() == 5);
}

BOOST_AUTO_TEST_CASE(pool_query_7)
{
}

BOOST_AUTO_TEST_CASE(pool_query_8)
{
}

BOOST_AUTO_TEST_CASE(pool_query_save_restore)
{
#warning CAN NOT USE A FIX SOLV FILE
// must store some raw metadata and generate the solv file
// otherwise trestcases break whenever the solv format changes
#if 0
  Pathname dir(TESTS_SRC_DIR);
  dir += "/zypp/data/PoolQuery";

  ZYpp::Ptr z = getZYpp();

  sat::Pool::instance().addRepoSolv(dir + "foo.solv");

  PoolQuery query;
  //query.setInstalledOnly();
  query.execute("kde", &result_cb);

  cout << "search done." << endl;

  query.setMatchExact(true);
  query.execute("kde", &result_cb);

  cout << "search done." << endl;
#endif

//test recovery from file
  Pathname pathToQueries(TESTS_SRC_DIR);
  pathToQueries += "/zypp/data/PoolQuery/savedqueries";

  std::list<PoolQuery> savedQueries;

  std::insert_iterator<std::list<PoolQuery> > ii(savedQueries, savedQueries.end());
  readPoolQueriesFromFile(pathToQueries,ii);
  BOOST_CHECK( savedQueries.size() == 2 );

  filesystem::TmpFile tmp;
  Pathname tmpPath = tmp.path();

  savedQueries.clear();

  PoolQuery q1;
  PoolQuery q2;

  q1.addKind( Resolvable::Kind::patch );
  q2.addKind( Resolvable::Kind::patch );
  q2.addKind( Resolvable::Kind::pattern );

  savedQueries.push_front( q1 );
  savedQueries.push_front( q2 );

  writePoolQueriesToFile ( tmpPath, savedQueries.begin(), savedQueries.end() );
  std::insert_iterator<std::list<PoolQuery> > ii2(savedQueries,
      savedQueries.end());
  //reread writed queries
  readPoolQueriesFromFile( tmpPath, ii2);
  //TODO test if 0==2 and 1==3
  BOOST_CHECK( savedQueries.size() == 4 );

}
