#include <stdio.h>
#include <iostream>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Easy.h"
#include "zypp/ZYppFactory.h"
#include "zypp/sat/WhatProvides.h"

#define BOOST_TEST_MODULE WhatProvides

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;

BOOST_AUTO_TEST_CASE(pool_query)
{
  Pathname dir(TESTS_SRC_DIR);
  dir += "/zypp/data/PoolQuery";

  ZYpp::Ptr z = getZYpp();

  sat::Pool::instance().addRepoSolv(dir + "foo.solv");

  Capability cap("amarok < 1.13");
  sat::WhatProvides q( cap );
  if ( ! q.empty() )
  {
    MIL << "Found " << q.size() << " matches for " << cap << ":" << endl;
    for_( it, q.begin(), q.end() )
    MIL << *it << endl;
  }
}
