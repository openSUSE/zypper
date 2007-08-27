
#include <iostream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/Arch.h"
#include "zypp/TmpPath.h"
#include "zypp/ZConfig.h"
#include "zypp/cache/CacheInitializer.h"
#include "zypp/cache/sqlite3x/sqlite3x.hpp"

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
  filesystem::TmpDir tmpdir;
  {
    //unit_test_log::instance().set_log_threshold_level(log_messages);
    cache::CacheInitializer initializer(tmpdir.path(), "test.db");
    
    sqlite3_connection con( (tmpdir.path() + "test.db").asString().c_str());
    //con.executenonquery(SOURCES_TABLE_SCHEMA);
    int count = con.executeint("select count(*) from sqlite_master where type='table';");
    BOOST_CHECK( initializer.justInitialized() );
    BOOST_CHECK( !initializer.justReinitialized() );
    // 14 tables need to be created
    BOOST_CHECK( count > 0);
    int version = con.executeint("select version from db_info;");
    BOOST_CHECK_EQUAL( version, ZYPP_CACHE_SCHEMA_VERSION );
  }
  
  // now screw up the versioning and check if schema is rebuilt
  {
    sqlite3_connection con( (tmpdir.path() + "test.db").asString().c_str());
    con.executenonquery("update db_info set version=9999 where 1;");
    cache::CacheInitializer initializer(tmpdir.path(), "test.db");
    BOOST_CHECK( !initializer.justInitialized() );
    BOOST_CHECK( initializer.justReinitialized() );
    
  }
  
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    ZConfig::instance().overrideSystemArchitecture( Arch( "i686" ) );
    test_suite* test= BOOST_TEST_SUITE( "CacheInit" );
    test->add( BOOST_TEST_CASE( &cacheinit_test ), 0 /* expected zero error */ );
    return test;
}
