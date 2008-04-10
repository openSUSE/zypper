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
  // name: yast2-sound 2.16.2-9 i586
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
  RepoInfo i4; i4.setAlias("@System");
  sat::Pool::instance().addRepoSolv(dir / "@System.solv", i4);
}

BOOST_AUTO_TEST_CASE(pool_query_init)
{
  init_pool();
}

BOOST_AUTO_TEST_CASE(pool_query_exp)
{
  cout << "****exp****"  << endl;
  
  PoolQuery q;
  q.addString("zypper");
  q.addAttribute(sat::SolvAttr::name);

  // should list 2 selectables
  for (PoolQuery::Selectable_iterator it = q.selectableBegin();
       it != q.selectableEnd(); ++it)
  {
    ui::Selectable::Ptr s = *it;
    cout << s->kind() << ":" << s->name() << " hasinstalled: " << s->installedEmpty() << endl;
  }

  std::for_each(q.begin(), q.end(), &result_cb);
}

/////////////////////////////////////////////////////////////////////////////
//  0xx basic queries
/////////////////////////////////////////////////////////////////////////////

// no conditions, default query
// result: all available resolvables
BOOST_AUTO_TEST_CASE(pool_query_000)
{
  cout << "****000****"  << endl;
  PoolQuery q;
  cout << q.size() << endl;
  BOOST_CHECK(q.size() == 11451);
  /**!\todo should be 11453 probably according to:
   * dumpsolv factory.solv factory-nonoss.solv zypp_svn.solv \@System.solv | \
   * grep '^name:.*\(noarch\|i386\|i586\|i686\|src\)$' | wc -l
   */
}

// default query + one search string
// q.addString("foo");
// result: all resolvables having at least one attribute matching foo
BOOST_AUTO_TEST_CASE(pool_query_001)
{
  cout << "****001****"  << endl;
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
BOOST_AUTO_TEST_CASE(pool_query_002)
{
  cout << "****002****"  << endl;
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

// kind filter
BOOST_AUTO_TEST_CASE(pool_query_003)
{
  cout << "****003****"  << endl;
  PoolQuery q;
  q.addString("zypper");
  q.addAttribute(sat::SolvAttr::name);
  q.addKind(ResTraits<Package>::kind);

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 3);
}

// match exact
BOOST_AUTO_TEST_CASE(pool_query_004)
{
  cout << "****004****"  << endl;
  PoolQuery q;
  q.addString("vim");
  q.addAttribute(sat::SolvAttr::name);
  q.setMatchExact();

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 3);

  PoolQuery q1;
  q1.addString("zypp");
  q1.addAttribute(sat::SolvAttr::name);
  q1.setMatchExact();

  std::for_each(q1.begin(), q1.end(), &result_cb);
  BOOST_CHECK(q1.empty());
}

// use globs
BOOST_AUTO_TEST_CASE(pool_query_005)
{
  cout << "****005.1****"  << endl;
  PoolQuery q;
  q.addString("z?p*");
  q.addAttribute(sat::SolvAttr::name);
  q.setMatchGlob();

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 11);

  cout << "****005.2****"  << endl;

  PoolQuery q1;
  q1.addString("*zypp*");
  q1.addAttribute(sat::SolvAttr::name);
  q1.setMatchGlob();

  std::for_each(q1.begin(), q1.end(), &result_cb);
  BOOST_CHECK(q1.size() == 28);

  cout << "****005.3****"  << endl;

  // should be the same as above
  PoolQuery q2;
  q2.addString("zypp");
  q2.addAttribute(sat::SolvAttr::name);

  BOOST_CHECK(q2.size() == 28);
}

// use regex
BOOST_AUTO_TEST_CASE(pool_query_006)
{
  cout << "****006.1***"  << endl;

  // should be the same as 005 1
  PoolQuery q;
  q.addString("^z.p.*");
  q.addAttribute(sat::SolvAttr::name);
  q.setMatchRegex();

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 11);

  cout << "****006.2***"  << endl;

  PoolQuery q1;
  q1.addString("zypper|smart");
  q1.addAttribute(sat::SolvAttr::name);
  q1.setMatchRegex();

  std::for_each(q1.begin(), q1.end(), &result_cb);
  BOOST_CHECK(q1.size() == 21);

  cout << "****006.3***"  << endl;

  // invalid regex
  PoolQuery q2;
  q2.addString("zypp\\");
  q2.setMatchRegex();
  BOOST_CHECK_THROW(q2.size(), Exception);
}


// match by installed status (basically by system vs. repo)
BOOST_AUTO_TEST_CASE(pool_query_050)
{
  cout << "****050****"  << endl;
  PoolQuery q;
  q.addString("zypper");
  q.addAttribute(sat::SolvAttr::name);
  q.setMatchExact();
  q.setInstalledOnly();

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 1);

  cout << endl;

  PoolQuery q1;
  q1.addString("zypper");
  q1.addAttribute(sat::SolvAttr::name);
  q1.setMatchExact();
  q1.setUninstalledOnly();

  std::for_each(q1.begin(), q1.end(), &result_cb);
  BOOST_CHECK(q1.size() == 5);
}


/////////////////////////////////////////////////////////////////////////////
//  1xx multiple attribute queries
/////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE(pool_query_100)
{
  cout << "****100****"  << endl;
  PoolQuery q;
  /* This string is found sometimes only in solvable names (e.g. novell-lum),
     sometimes only in summary (e.g. yast2-casa-ats) and sometimes only
     in descriptions (e.g. beagle-quickfinder).  novell-lum doesn't exist
     in our test solv file, but let's ignore this.  I didn't find a string
     with the same characteristics giving fewer matches :-/  */
  q.addString("novell");
  q.addAttribute(sat::SolvAttr::name);
  q.addAttribute(sat::SolvAttr::summary);
  q.addAttribute(sat::SolvAttr::description);

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 74);

  cout << endl;

  PoolQuery q1;
  q1.addString("mp3");
  q1.addAttribute(sat::SolvAttr::name);

  std::for_each(q1.begin(), q1.end(), &result_cb);
  BOOST_CHECK(q1.size() == 7);
}


// multi attr (same value) substring matching (case sensitive and insensitive)
BOOST_AUTO_TEST_CASE(pool_query_101)
{
  cout << "****101****"  << endl;

  PoolQuery q;
  q.addString("ZYpp");
  q.addAttribute(sat::SolvAttr::name);
  q.addAttribute(sat::SolvAttr::summary);
  q.addAttribute(sat::SolvAttr::description);

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 30);

  cout << endl;

  PoolQuery q2;
  q2.addString("ZYpp");
  q2.addAttribute(sat::SolvAttr::name);
  q2.addAttribute(sat::SolvAttr::summary);
  q2.addAttribute(sat::SolvAttr::description);
  q2.setCaseSensitive();

  std::for_each(q2.begin(), q2.end(), &result_cb);
  BOOST_CHECK(q2.size() == 2);
}


// multi attr (same value) glob matching (case sensitive and insensitive)
BOOST_AUTO_TEST_CASE(pool_query_102)
{
  cout << "****102****"  << endl;
  PoolQuery q;
  q.addString("pack*");
  q.addAttribute(sat::SolvAttr::name);
  q.addAttribute(sat::SolvAttr::summary);
  q.setMatchGlob();

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 35);
}


// multi attr (same value via addAttribute())
BOOST_AUTO_TEST_CASE(pool_query_103)
{
  cout << "****103.1****"  << endl;
  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, "novell");
  q.addAttribute(sat::SolvAttr::summary, "novell");

//  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 42);

  cout << "****103.2****"  << endl;

  PoolQuery q1;
  q1.addString("novell");
  q1.addAttribute(sat::SolvAttr::name);
  q1.addAttribute(sat::SolvAttr::summary);

//  std::for_each(q1.begin(), q1.end(), &result_cb);
  BOOST_CHECK(q1.size() == 42);

  cout << endl;
}


// multiple attributes, different search strings (one string per attrbute)
BOOST_AUTO_TEST_CASE(pool_query_104)
{
  cout << "****104****"  << endl;
  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, "novell");
  q.addAttribute(sat::SolvAttr::summary, "package management");

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 22);
}


/////////////////////////////////////////////////////////////////////////////
//  3xx repo filter queries (addRepo(alias_str))
/////////////////////////////////////////////////////////////////////////////

// default query + one attribute(one string) + one repo
BOOST_AUTO_TEST_CASE(pool_query_300)
{
  cout << "****300****"  << endl;
  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, "zypper");
  q.addRepo("zypp_svn");

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 3);
}

// default query + one repo
BOOST_AUTO_TEST_CASE(pool_query_301)
{
  cout << "****301****"  << endl;
  PoolQuery q;
  q.addRepo("zypp_svn");

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 21);
}

// multiple repos + one attribute
BOOST_AUTO_TEST_CASE(pool_query_302)
{
  cout << "****302****"  << endl;
  PoolQuery q;
  q.addString("ma");
  q.addAttribute(sat::SolvAttr::name);
  q.addRepo("factory-nonoss");
  q.addRepo("zypp_svn");

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 8);
}

/*
BOOST_AUTO_TEST_CASE(pool_query_X)
{
  cout << "****X****"  << endl;
  PoolQuery q;
  q.addString("pack*");
  q.addAttribute(sat::SolvAttr::name);

  std::for_each(q.begin(), q.end(), &result_cb);
  BOOST_CHECK(q.size() == 28);
}
*/

// test matching
BOOST_AUTO_TEST_CASE(pool_query_equal)
{
  cout << "****equal****"  << endl;
  PoolQuery q;
  q.addString("zypp");
  q.addAttribute(sat::SolvAttr::name);
  q.setMatchGlob();
  PoolQuery q2;
  q2.addString("zypp");
  q2.addAttribute(sat::SolvAttr::name);
  q2.setMatchGlob();
  PoolQuery q3;
  q3.addString("zypp");
  q3.addAttribute(sat::SolvAttr::name);
  q3.setMatchGlob();
  q3.setRequireAll(true);
  PoolQuery q4;
  q4.addAttribute(sat::SolvAttr::name,"zypp");
  q4.setMatchGlob();

  BOOST_CHECK(equal(q,q2));
  BOOST_CHECK(!equal(q,q3));
  //only exact equal! \TODO maybe change
  BOOST_CHECK(!equal(q,q4));
  BOOST_CHECK(!equal(q4,q3));
}

// save/load query
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
