#include <stdio.h>
#include <iostream>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/ZYppFactory.h"
#include "zypp/PoolQuery.h"

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
}
