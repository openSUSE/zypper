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

bool result_cb( const ResObject::Ptr &r )
{
  cout << r << endl;
  return true;
}

BOOST_AUTO_TEST_CASE(pool_query)
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
