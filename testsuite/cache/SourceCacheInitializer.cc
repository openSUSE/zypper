
#include <iostream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/Arch.h"
#include "zypp/TmpPath.h"
#include "zypp/cache/SourceCacheInitializer.h"

// Boost.Test
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include "zypp/cache/sqlite3x/sqlite3x.hpp"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using boost::test_tools::close_at_tolerance;

using namespace std;
using namespace zypp;
using namespace sqlite3x;

/******************************************************************
**
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
**
**      DESCRIPTION :
*/
void cacheinit_test()
{
  filesystem::TmpDir tmpdir;
  cache::SourceCacheInitializer initializer(tmpdir.path(), "test.db");
  sqlite3_connection con( (tmpdir.path() + "test.db").asString().c_str());
  //con.executenonquery(SOURCES_TABLE_SCHEMA);
  int count = con.executeint("select count(*) from sqlite_master where type='table';");
  BOOST_CHECK( initializer.justInitialized() );
  BOOST_CHECK_EQUAL( count, 3);
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "CacheInit" );
    test->add( BOOST_TEST_CASE( &cacheinit_test ), 0 /* expected zero error */ );
    return test;
}
