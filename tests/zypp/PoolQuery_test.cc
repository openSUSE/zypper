#include "TestSetup.h"
#include "zypp/PoolQuery.h"
#include "zypp/PoolQueryUtil.tcc"

#define BOOST_TEST_MODULE PoolQuery

/////////////////////////////////////////////////////////////////////////////
static TestSetup test( Arch_x86_64 );

BOOST_AUTO_TEST_CASE(pool_query_init)
{
  // Abuse;) vbox as System repo:
  test.loadTargetRepo( TESTS_SRC_DIR "/data/obs_virtualbox_11_1" );
  test.loadRepo( TESTS_SRC_DIR "/data/openSUSE-11.1", "opensuse" );
  test.loadRepo( TESTS_SRC_DIR "/data/OBS_zypp_svn-11.1", "zyppsvn" );

  dumpRange( USR, test.pool().knownRepositoriesBegin(),
                  test.pool().knownRepositoriesEnd() );
  USR << "pool: " << test.pool() << endl;
}
/////////////////////////////////////////////////////////////////////////////

static std::ofstream devNull;
#define COUT devNull

struct PrintAndCount
{
  PrintAndCount() : _count(0) {}

  bool operator()( const sat::Solvable & solvable )
  {
    zypp::PoolItem pi( zypp::ResPool::instance().find( solvable ) );
    COUT << pi.resolvable() << endl;
    ++_count;
    return true;
  }

  unsigned _count;
};

void dumpQ( std::ostream & str, const PoolQuery & q, bool verbose = true )
{
  q.begin();
  str << q << endl;
  unsigned nc = 0;
  if ( 1 )
  {
    for_( it, q.begin(), q.end() )
    {
      ++nc;
      if ( verbose )
        str << it << endl;
    }
    str << "--> MATCHES: " << nc << endl;
  }
}


#if 0
BOOST_AUTO_TEST_CASE(pool_query_experiment)
{
  cout << "****experiment****"  << endl;

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
  cout << q.size() << endl;
  BOOST_CHECK(q.size() == 3811);

  /* dumpsolv repo1.solv repo2.solv repo3.solv | \
     grep '^name:.*\(noarch\|x86_64\|i386\|i586\|i686\|src\)$' | wc -l */
}

// default query + one search string
// q.addString("foo");
// result: all resolvables having at least one attribute matching foo
BOOST_AUTO_TEST_CASE(pool_query_001)
{
  cout << "****001****"  << endl;
  PoolQuery q;
  q.addString("zypper");

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 11);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 5);

  cout << endl;

  PoolQuery q1;
  q1.addAttribute(sat::SolvAttr::name, "zypper");

  BOOST_CHECK(std::for_each(q1.begin(), q1.end(), PrintAndCount())._count == 5);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 1);

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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 6);

  cout << "****005.2****"  << endl;

  PoolQuery q1;
  q1.addString("*zypp*");
  q1.addAttribute(sat::SolvAttr::name);
  q1.setMatchGlob();

  BOOST_CHECK(std::for_each(q1.begin(), q1.end(), PrintAndCount())._count == 26);

  cout << "****005.3****"  << endl;

  // should be the same as above
  PoolQuery q2;
  q2.addString("zypp");
  q2.addAttribute(sat::SolvAttr::name);

  BOOST_CHECK(q2.size() == 26);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 6);

  cout << "****006.2***"  << endl;

  PoolQuery q1;
  q1.addString("zypper|smart");
  q1.addAttribute(sat::SolvAttr::name);
  q1.setMatchRegex();

  BOOST_CHECK(std::for_each(q1.begin(), q1.end(), PrintAndCount())._count == 8);

  cout << "****006.3***"  << endl;

  // invalid regex
  PoolQuery q2;
  q2.addString("zypp\\");
  q2.setMatchRegex();
  BOOST_CHECK_THROW(q2.begin(), Exception);
}


// match whole words
BOOST_AUTO_TEST_CASE(pool_query_007)
{
  cout << "****007***"  << endl;

  PoolQuery q;
  q.addString("zypp");
  q.addAttribute(sat::SolvAttr::name);
  q.setMatchWord();

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 6);
}

// match by installed status (basically by system vs. repo)
BOOST_AUTO_TEST_CASE(pool_query_050)
{
  cout << "****050****"  << endl;
  PoolQuery q;
  q.addString("yasm");
  q.addAttribute(sat::SolvAttr::name);
  q.setMatchExact();
  q.setInstalledOnly();

  BOOST_CHECK_EQUAL(std::for_each(q.begin(), q.end(), PrintAndCount())._count, 4);

  cout << endl;

  PoolQuery q1;
  q1.addString("zypper");
  q1.addAttribute(sat::SolvAttr::name);
  q1.setMatchExact();
  q1.setUninstalledOnly();
  BOOST_CHECK_EQUAL(std::for_each(q1.begin(), q1.end(), PrintAndCount())._count, 5);
}

/////////////////////////////////////////////////////////////////////////////
//  1xx multiple attribute queries
/////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE(pool_query_100)
{
  cout << "****100****"  << endl;
  PoolQuery q;
  /* This string is found sometimes only in only in summary (e.g. pgcalc)
     and sometimes only in description (e.g. bc, lftp). We don't have
     any package with 'revers' only in package name, but let's ignore this.
     I didn't find a string with the same characteristics giving fewer matches
     :-/ */
  q.addString("revers");
  q.addAttribute(sat::SolvAttr::name);
  q.addAttribute(sat::SolvAttr::summary);
  q.addAttribute(sat::SolvAttr::description);

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 5);

  cout << endl;

  {
    PoolQuery q1;
    q1.addAttribute(sat::SolvAttr::name, "zypper");
    BOOST_CHECK_EQUAL(q1.size(),5);

    PoolQuery q2;
    q2.addAttribute(sat::SolvAttr::summary,"samba");
    BOOST_CHECK_EQUAL(q2.size(),13);

    // now summary and name in one go:
    q1.addAttribute(sat::SolvAttr::summary,"samba");
    BOOST_CHECK_EQUAL(q1.size(),18);
  }
}


// multi attr (same value) substring matching (case sensitive and insensitive)
BOOST_AUTO_TEST_CASE(pool_query_101)
{
  cout << "****101****"  << endl;

  PoolQuery q;
  q.addString("RELAX");
  q.addAttribute(sat::SolvAttr::name);
  q.addAttribute(sat::SolvAttr::summary);
  q.addAttribute(sat::SolvAttr::description);

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 7);

  cout << endl;

  PoolQuery q2;
  q2.addString("RELAX");
  q2.addAttribute(sat::SolvAttr::name);
  q2.addAttribute(sat::SolvAttr::summary);
  q2.addAttribute(sat::SolvAttr::description);
  q2.setCaseSensitive();

  BOOST_CHECK(std::for_each(q2.begin(), q2.end(), PrintAndCount())._count == 4);
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 23);
}


// multi attr (same value via addAttribute())
BOOST_AUTO_TEST_CASE(pool_query_103)
{
  cout << "****103.1****"  << endl;
  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, "rest");
  q.addAttribute(sat::SolvAttr::summary, "rest");

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 14);

  cout << "****103.2****"  << endl;

  PoolQuery q1;
  q1.addString("rest");
  q1.addAttribute(sat::SolvAttr::name);
  q1.addAttribute(sat::SolvAttr::summary);

  BOOST_CHECK(std::for_each(q1.begin(), q1.end(), PrintAndCount())._count == 14);
//  BOOST_CHECK(q1.size() == 42);

  cout << endl;
}

// multiple attributes, different search strings (one string per attrbute)
BOOST_AUTO_TEST_CASE(pool_query_104)
{
  cout << "****104****"  << endl;
  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, "zypper");
  q.addAttribute(sat::SolvAttr::summary, "package management");

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 8);
}

// multiple attributes, different search strings (one string per attrbute), regex matching
BOOST_AUTO_TEST_CASE(pool_query_105)
{
  cout << "****105****"  << endl;
  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, "zy..er");
  q.addAttribute(sat::SolvAttr::summary, "package management");
  q.setMatchRegex();

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 8);
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
  q.addRepo("zyppsvn");

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 4);
}

// default query + one repo
BOOST_AUTO_TEST_CASE(pool_query_301)
{
  cout << "****301****"  << endl;
  PoolQuery q;
  q.addRepo("zyppsvn");

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 42);
}

// multiple repos + one attribute
BOOST_AUTO_TEST_CASE(pool_query_302)
{
  cout << "****302****"  << endl;
  PoolQuery q;
  q.addString("zypper");
  q.addAttribute(sat::SolvAttr::name);
  q.addRepo("opensuse");
  q.addRepo("zyppsvn");

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 5);
}

/////////////////////////////////////////////////////////////////////////////
//  4xx kind queries (addKind(ResKind))
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

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 4);
}


/////////////////////////////////////////////////////////////////////////////
//  5xx multiple string/attribute queries
/////////////////////////////////////////////////////////////////////////////

// multiple strings for one attribute
BOOST_AUTO_TEST_CASE(pool_query_500)
{
  cout << "****500.1****"  << endl;
  PoolQuery q;
  q.addString("zypper");
  q.addString("yast2-packager");
  q.addAttribute(sat::SolvAttr::name);
  q.setMatchExact();
  // creates: ^(apt|zypper)$
  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 6);

  cout << "****500.2****"  << endl;
  q.addString("*bzypp");
  q.setMatchGlob();
  // creates: ^(.*zy.p|yast.*package.*|.*bzypp)$
  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 11);

  cout << "****500.3****"  << endl;
  PoolQuery q1;
  q1.addString("^libsm[a-z]*[0-9]$");
  q1.addAttribute(sat::SolvAttr::name, "bzypp$");
  q1.addKind(ResKind::package);
  q1.setMatchRegex();
  // creates: (^libsm[a-z]*[0-9]$|bzypp$)
  BOOST_CHECK(std::for_each(q1.begin(), q1.end(), PrintAndCount())._count == 5);

  cout << "****500.4****"  << endl;
  PoolQuery q2;
  q2.addString("Thunder");
  q2.addAttribute(sat::SolvAttr::name, "sun");
  q2.addKind(ResKind::package);
  q2.addRepo("opensuse");
  q2.setCaseSensitive();
  // creates: (sun|Thunder)
  BOOST_CHECK(std::for_each(q2.begin(), q2.end(), PrintAndCount())._count == 3);

  cout << "****500.5****"  << endl;
  PoolQuery q3;
  q3.addString("audio");
  q3.addAttribute(sat::SolvAttr::name, "zip");
  q3.addKind(ResKind::package);
  q3.addRepo("opensuse");
  q3.setMatchWord();
  // creates: \b(zip|audio)\b
  BOOST_CHECK(std::for_each(q3.begin(), q3.end(), PrintAndCount())._count == 3);
}

// multiple strings, multiple attributes, same strings
BOOST_AUTO_TEST_CASE(pool_query_501)
{
  cout << "****501****"  << endl;
  PoolQuery q;
  q.addString("Thunder");
  q.addString("storm");
  q.addAttribute(sat::SolvAttr::name);
  q.addAttribute(sat::SolvAttr::description);
  q.addKind(ResKind::package);
  q.addRepo("opensuse");

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 14);
}

// multiple strings, multiple attributes, same strings
BOOST_AUTO_TEST_CASE(pool_query_502)
{
  cout << "****502****"  << endl;
  PoolQuery q;
  q.addString("weather");
  q.addAttribute(sat::SolvAttr::name, "thunder");
  q.addAttribute(sat::SolvAttr::description, "storm");
  q.addKind(ResKind::package);
  q.addRepo("opensuse");

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 13);
}

/////////////////////////////////////////////////////////////////////////////
//  6xx queries with edition
/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(pool_query_X)
{
  cout << "****600.1****"  << endl;
  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, "zypper");
  q.setMatchExact();
  q.setEdition(Edition("0.12.5"), Rel::GT);

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 4);

  cout << "****600.2****"  << endl;
  q.setEdition(Edition("0.12.5"), Rel::LT);

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 0);

  cout << "****600.3****"  << endl;
  q.setEdition(Edition("0.12.5"), Rel::LE);

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 1);

  cout << "****600.4****"  << endl;
  q.setEdition(Edition("0.12.5-5"), Rel::LT);

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 1);
}

//! \todo FIXME this segfaults currently - one addString() + (version or kind or installed status condition)
/*
BOOST_AUTO_TEST_CASE(pool_query_FIXME)
{
  cout << "****FIXME****"  << endl;
  PoolQuery q;
  q.addString("zypper");
  q.setEdition(Edition("0.10.3-4"), Rel::GE);

  BOOST_CHECK(std::for_each(q.begin(), q.end(), PrintAndCount())._count == 2);
}
*/

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


BOOST_AUTO_TEST_CASE(pool_query_recovery)
{
  Pathname testfile(TESTS_SRC_DIR);
    testfile += "/zypp/data/PoolQuery/savedqueries";
  cout << "****recovery****"  << endl;

  std::vector<PoolQuery> queries;
  std::insert_iterator<std::vector<PoolQuery> > ii( queries,queries.begin());
  readPoolQueriesFromFile(testfile,ii);
  BOOST_REQUIRE_MESSAGE(queries.size() == 2, "Bad count of read queries.");

  BOOST_CHECK_EQUAL(queries[0].size(), 8);

  PoolQuery q;
  q.addString("ma*");
  q.addRepo("opensuse");
  q.addKind(ResKind::patch);
  q.setMatchRegex();
  q.setRequireAll();
  q.setCaseSensitive();
  q.setUninstalledOnly();
  q.setEdition(Edition("0.8.3"),Rel::NE);
  BOOST_CHECK(q == queries[1]);
}

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
  BOOST_REQUIRE_MESSAGE(queries.size()==4,"Bad count of written/readed queries.");
  BOOST_CHECK(queries[2] == queries[0]);
  BOOST_CHECK(queries[3] == queries[1]);
}

// test matching
BOOST_AUTO_TEST_CASE(pool_query_equal)
{
  cout << "****equal****"  << endl;
  std::vector<PoolQuery> v;
  {
    PoolQuery q;
    v.push_back( q );
  }
  {
    PoolQuery q;
    q.addAttribute( sat::SolvAttr::name, "zypper" );
    q.setMatchExact();
    q.setCaseSensitive(true);
    v.push_back( q );
  }
  {
    PoolQuery q;
    q.addAttribute( sat::SolvAttr::name, "libzypp" );	// different
    q.setMatchExact();
    q.setCaseSensitive(true);
    v.push_back( q );
  }
  {
    PoolQuery q;
    q.addAttribute( sat::SolvAttr::vendor, "zypper" );	// different
    q.setMatchExact();
    q.setCaseSensitive(true);
    v.push_back( q );
  }
  {
    PoolQuery q;
    q.addAttribute( sat::SolvAttr::name, "zypper" );
    q.setMatchExact();
    q.setCaseSensitive(false);	// different
    v.push_back( q );
  }
  {
    PoolQuery q;
    q.addAttribute( sat::SolvAttr::name, "zypper" );
    q.setMatchSubstring();	// different
    q.setCaseSensitive(true);
    v.push_back( q );
  }
  {
    PoolQuery q;
    q.addDependency( sat::SolvAttr::provides, "zypper" );
    v.push_back( q );
  }
  {
    PoolQuery q;
    q.addDependency( sat::SolvAttr::provides, "zypper", Rel::GT, Edition("1.0")  );
    v.push_back( q );
  }
  {
    PoolQuery q;
    q.addDependency( sat::SolvAttr::provides, "zypper", Rel::GT, Edition("2.0")  );
    v.push_back( q );
  }

  for_( li, 0U, v.size() )
  {
    for_( ri, 0U, v.size() )
    {
      COUT << li << " <> " << ri << endl;
      bool equal( v[li] == v[ri] );
      bool nequal( v[li] != v[ri] );
      BOOST_CHECK_EQUAL( equal, li==ri );
      BOOST_CHECK_EQUAL( equal, !nequal );
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
//  Dependency Query
/////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(addDependency)
{
  {
    cout << "****addDependency1****"  << endl;
    PoolQuery q;
    q.setCaseSensitive( false );
    q.setMatchSubstring();
    q.addString( "libzypp" );
    q.addDependency( sat::SolvAttr::provides, "FOO" ); // ! finds 'perl(CPAN::InfoObj)' 'foO'
    std::for_each(q.begin(), q.end(), PrintAndCount());
    //dumpQ( std::cout, q );
    BOOST_CHECK_EQUAL( q.size(), 13 );
  }
  {
    cout << "****addDependency2****"  << endl;
    PoolQuery q;
    q.setCaseSensitive( false );
    q.setMatchSubstring();
    q.addString( "libzypp" );
    q.addDependency( sat::SolvAttr::provides, "FOO", Rel::GT, Edition("5.0") );
    std::for_each(q.begin(), q.end(), PrintAndCount());
    //dumpQ( std::cout, q );
    BOOST_CHECK_EQUAL( q.size(), 7 );
  }
  {
    cout << "****addDependency2a****"  << endl;
    PoolQuery q;
    q.setCaseSensitive( false );
    q.setMatchSubstring();
    q.addDependency( sat::SolvAttr::provides, "libzypp", Rel::GT, Edition("5.0") );
    q.addAttribute( sat::SolvAttr::arch, Arch_i586.asString() ); // OR with arch i585
    std::for_each(q.begin(), q.end(), PrintAndCount());
    //dumpQ( std::cout, q );
    BOOST_CHECK_EQUAL( q.size(), 66 );
  }
  {
    cout << "****addDependency2b****"  << endl;
    PoolQuery q;
    q.setCaseSensitive( false );
    q.setMatchSubstring();
    // libzypp provides yast2-packagemanager...
    q.addDependency( sat::SolvAttr::provides, "yast2-packagemanager", Rel::GT, Edition("5.0"), Arch_i586 ); // AND with arch i585
    std::for_each(q.begin(), q.end(), PrintAndCount());
    //dumpQ( std::cout, q );
    BOOST_CHECK_EQUAL( q.size(), 2 );
  }
  {
    cout << "****addDependency2c****"  << endl;
    PoolQuery q;
    q.setCaseSensitive( false );
    q.setMatchSubstring();
    // but no package named yast2-packagemanager
    q.addDependency( sat::SolvAttr::name, "yast2-packagemanager", Rel::GT, Edition("5.0"), Arch_i586 ); // AND with arch i585
    std::for_each(q.begin(), q.end(), PrintAndCount());
    //dumpQ( std::cout, q );
    BOOST_CHECK_EQUAL( q.size(), 0 );
  }
  {
    cout << "****addDependency2d****"  << endl;
    PoolQuery q;
    q.setCaseSensitive( false );
    q.setMatchSubstring();
    // libzypp provides yast2-packagemanager...
    q.addDependency( sat::SolvAttr::provides, "yast2-packagemanager", Arch_i586 ); // AND with arch i585
    std::for_each(q.begin(), q.end(), PrintAndCount());
    //dumpQ( std::cout, q );
    BOOST_CHECK_EQUAL( q.size(), 2 );
  }
  {
    cout << "****addDependency2e****"  << endl;
    PoolQuery q;
    q.setCaseSensitive( false );
    q.setMatchSubstring();
    // but no package named yast2-packagemanager
    q.addDependency( sat::SolvAttr::name, "yast2-packagemanager", Arch_i586 ); // AND with arch i585
    std::for_each(q.begin(), q.end(), PrintAndCount());
    //dumpQ( std::cout, q );
    BOOST_CHECK_EQUAL( q.size(), 0 );
  }

  {
    cout << "****addDependency3****"  << endl;
    PoolQuery q;
    // includes wine
    q.addDependency( sat::SolvAttr::provides, "kernel" );
    std::for_each(q.begin(), q.end(), PrintAndCount());
    //dumpQ( std::cout, q );
    BOOST_CHECK_EQUAL( q.size(), 12 );
  }
  {
    cout << "****addDependency4****"  << endl;
    PoolQuery q;
    // no wine
    q.addDependency( sat::SolvAttr::name, "kernel" );
    std::for_each(q.begin(), q.end(), PrintAndCount());
    //dumpQ( std::cout, q );
    BOOST_CHECK_EQUAL( q.size(), 11 );
  }
  {
    cout << "****addDependency5****"  << endl;
    PoolQuery q;
    // Capability always matches exact
    q.addDependency( sat::SolvAttr::provides, Capability("kernel") );
    std::for_each(q.begin(), q.end(), PrintAndCount());
    //dumpQ( std::cout, q );
    BOOST_CHECK_EQUAL( q.size(), 2 );
  }
  {
    cout << "****addDependency6****"  << endl;
    PoolQuery q;
    // non dependecy + Capability matches solvable name!
    q.addDependency( sat::SolvAttr::summary, Capability("kernel") );
    std::for_each(q.begin(), q.end(), PrintAndCount());
    //dumpQ( std::cout, q );
    BOOST_CHECK_EQUAL( q.size(), 0 ); // non dependecy
  }
}


