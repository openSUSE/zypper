
#include <iostream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/Arch.h"
#include "zypp/TmpPath.h"
#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

// Boost.Test
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using boost::test_tools::close_at_tolerance;
using namespace boost::unit_test;
using namespace boost::unit_test::log;
using namespace boost::unit_test_framework;
using namespace std;
using namespace zypp;
using namespace sqlite3x;

// control output with BOOST_TEST_LOG_LEVEL="all"

void cacheinit_test()
{
  //unit_test_log::instance().set_log_threshold_level(log_messages);
  filesystem::TmpDir tmpdir;
  cache::CacheInitializer initializer(tmpdir.path(), "test.db");
  
  sqlite3_connection con( (tmpdir.path() + "test.db").asString().c_str());
  //con.executenonquery(SOURCES_TABLE_SCHEMA);
  int count = con.executeint("select count(*) from sqlite_master where type='table';");
  BOOST_CHECK( initializer.justInitialized() );
  // 14 tables need to be created
  BOOST_CHECK_EQUAL( count, 14);
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "CacheInit" );
    test->add( BOOST_TEST_CASE( &cacheinit_test ), 0 /* expected zero error */ );
    return test;
}
