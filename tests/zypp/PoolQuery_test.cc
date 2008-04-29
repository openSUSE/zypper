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

struct PrintAndCount
{
  PrintAndCount() : _count(0) {}

  bool operator()( const sat::Solvable & solvable )
  {
    zypp::PoolItem pi( zypp::ResPool::instance().find( solvable ) );
    cout << pi.resolvable() << endl;
    // name: yast2-sound 2.16.2-9 i586
    ++_count;
    return true;
  }

  unsigned _count;
};

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

#if 0
BOOST_AUTO_TEST_CASE(pool_query_exp)
{
  cout << "****exp****"  << endl;

  PoolQuery q;
  q.addString("zypper");
  q.addAttribute(sat::SolvAttr::name);

  // should list 1 selectable?
  cout << "****selectables****"  << endl;
  for (PoolQuery::Selectable_iterator it = q.selectableBegin();
       it != q.selectableEnd(); ++it)
  {
    ui::Selectable::Ptr s = *it;
    cout << s->kind() << ":" << s->name() << " hasinstalled: " << s->installedEmpty() << endl;
  }
  cout << "****solvables****" << endl;
  PrintAndCount cb;
  std::for_each(q.begin(), q.end(), cb);
}
#endif


/////////////////////////////////////////////////////////////////////////////
//  0xx basic queries
/////////////////////////////////////////////////////////////////////////////

// no conditions, default query
// result: all available resolvables
BOOST_AUTO_TEST_CASE(pool_query_000)
{
  cout << "****000****"  << endl;
  PoolQuery q;
  //cout << q.size() << endl;
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 16);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 6);

  cout << endl;

  PoolQuery q1;
  q1.addAttribute(sat::SolvAttr::name, "zypper");

  BOOST_CHECK(std::for_each(q1.begin(), q1.end(), PrintAndCount())._count == 6);
}

// kind filter
BOOST_AUTO_TEST_CASE(pool_query_003)
{
  cout << "****003****"  << endl;
  PoolQuery q;
  q.addString("zypper");
  q.addAttribute(sat::SolvAttr::name);
  q.addKind(ResKind::package);

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 3);
}

// match exact
BOOST_AUTO_TEST_CASE(pool_query_004)
{
  cout << "****004****"  << endl;
  PoolQuery q;
  q.addString("vim");
  q.addAttribute(sat::SolvAttr::name);
  q.setMatchExact();

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 3);

  PoolQuery q1;
  q1.addString("zypp");
  q1.addAttribute(sat::SolvAttr::name);
  q1.setMatchExact();

  std::for_each(q1.begin(), q1.end(), PrintAndCount());
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 11);

  cout << "****005.2****"  << endl;

  PoolQuery q1;
  q1.addString("*zypp*");
  q1.addAttribute(sat::SolvAttr::name);
  q1.setMatchGlob();

  BOOST_CHECK(std::for_each(q1.begin(), q1.end(), PrintAndCount())._count == 28);

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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 11);

  cout << "****006.2***"  << endl;

  PoolQuery q1;
  q1.addString("zypper|smart");
  q1.addAttribute(sat::SolvAttr::name);
  q1.setMatchRegex();

  BOOST_CHECK(std::for_each(q1.begin(), q1.end(), PrintAndCount())._count == 21);

  cout << "****006.3***"  << endl;

  // invalid regex
  PoolQuery q2;
  q2.addString("zypp\\");
  q2.setMatchRegex();
  BOOST_CHECK_THROW(q2.begin(), Exception);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 1);

  cout << endl;

  PoolQuery q1;
  q1.addString("zypper");
  q1.addAttribute(sat::SolvAttr::name);
  q1.setMatchExact();
  q1.setUninstalledOnly();

  BOOST_CHECK(std::for_each(q1.begin(), q1.end(), PrintAndCount())._count == 5);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 74);

  cout << endl;

  PoolQuery q1;
  q1.addString("mp3");
  q1.addAttribute(sat::SolvAttr::name);

  BOOST_CHECK(std::for_each(q1.begin(), q1.end(), PrintAndCount())._count == 7);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 30);

  cout << endl;

  PoolQuery q2;
  q2.addString("ZYpp");
  q2.addAttribute(sat::SolvAttr::name);
  q2.addAttribute(sat::SolvAttr::summary);
  q2.addAttribute(sat::SolvAttr::description);
  q2.setCaseSensitive();

  BOOST_CHECK(std::for_each(q2.begin(), q2.end(), PrintAndCount())._count == 2);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 35);
}


// multi attr (same value via addAttribute())
BOOST_AUTO_TEST_CASE(pool_query_103)
{
  cout << "****103.1****"  << endl;
  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, "novell");
  q.addAttribute(sat::SolvAttr::summary, "novell");

  //std::for_each(q.begin(), q.end(), cb);
  BOOST_CHECK(q.size() == 42);

  cout << "****103.2****"  << endl;

  PoolQuery q1;
  q1.addString("novell");
  q1.addAttribute(sat::SolvAttr::name);
  q1.addAttribute(sat::SolvAttr::summary);

//  std::for_each(q1.begin(), q1.end(), cb);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 22);
}

// multiple attributes, different search strings (one string per attrbute), regex matching
BOOST_AUTO_TEST_CASE(pool_query_105)
{
  cout << "****105****"  << endl;
  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, "no.ell");
  q.addAttribute(sat::SolvAttr::summary, "package management");
  q.setMatchRegex();

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 22);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 3);
}

// default query + one repo
BOOST_AUTO_TEST_CASE(pool_query_301)
{
  cout << "****301****"  << endl;
  PoolQuery q;
  q.addRepo("zypp_svn");

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 21);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 8);
}

/////////////////////////////////////////////////////////////////////////////
//  4xx repo kind queries (addKind(ResKind))
/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(pool_query_400)
{
  cout << "****400****"  << endl;
  PoolQuery q;
  q.addString("lamp_server");
  q.addAttribute(sat::SolvAttr::name);
  q.addKind(ResKind::pattern);
  q.setMatchExact();

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 1);
}

// should find packages and patterns
BOOST_AUTO_TEST_CASE(pool_query_401)
{
  cout << "****401****"  << endl;
  PoolQuery q;
  q.addString("mail*");
  q.addAttribute(sat::SolvAttr::name);
  q.setMatchGlob();

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 8);
}

/*
BOOST_AUTO_TEST_CASE(pool_query_X)
{
  cout << "****X****"  << endl;
  PoolQuery q;
  q.addString("pack*");
  q.addAttribute(sat::SolvAttr::name);

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 28);
}
*/


#if 1
BOOST_AUTO_TEST_CASE(pool_query_recovery)
{
  Pathname testfile(TESTS_SRC_DIR);
    testfile += "/zypp/data/PoolQuery/savedqueries";
  cout << "****recovery****"  << endl;
  std::vector<PoolQuery> queries;
  std::insert_iterator<std::vector<PoolQuery> > ii( queries,queries.begin());
  readPoolQueriesFromFile(testfile,ii);
  BOOST_REQUIRE_MESSAGE(queries.size()==2,"Bad count of read queries.");
  BOOST_CHECK(queries[0].size() == 8);
  PoolQuery q;
  q.addString("ma*");
  q.addRepo("factory");
  q.addKind(ResKind::patch);
  q.setMatchRegex();
  q.setRequireAll();
  q.setCaseSensitive();
  q.setUninstalledOnly();
  BOOST_CHECK(q==queries[1]);
}

#endif

BOOST_AUTO_TEST_CASE(pool_query_serialize)
{
  PoolQuery q;
  q.addString("ma");
  q.addAttribute(sat::SolvAttr::name);
  q.addRepo("factory-nonoss");
  q.addRepo("zypp_svn");
  PoolQuery q2;
  q2.addAttribute(sat::SolvAttr::name,"ma");
  q2.addRepo("factory-nonoss");
  q2.addRepo("zypp_svn");


//  Pathname testfile(TESTS_SRC_DIR);
  //  testfile += "/zypp/data/PoolQuery/testqueries";
  filesystem::TmpFile testfile;
  cout << "****serialize****"  << endl;
  std::vector<PoolQuery> queries;
  queries.push_back(q);
  queries.push_back(q2);
  writePoolQueriesToFile(testfile,queries.begin(),queries.end());
  BOOST_REQUIRE_MESSAGE(queries.size()==2,"Bad count of added queries.");

  std::insert_iterator<std::vector<PoolQuery> > ii( queries,queries.end());
  readPoolQueriesFromFile(testfile,ii);
  BOOST_REQUIRE_MESSAGE(queries.size()==4,"Bad count of writed/readed queries.");
  BOOST_CHECK(queries[2] == queries[0]);
  BOOST_CHECK(queries[3] == queries[1]);
}


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

  BOOST_CHECK(q==q2);
  BOOST_CHECK(q!=q3);
  BOOST_CHECK(q==q4);
  BOOST_CHECK(q4!=q3);
}
