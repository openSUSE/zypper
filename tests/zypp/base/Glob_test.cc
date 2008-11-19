#include "TestSetup.h"
#include "zypp/Pathname.h"
#include "zypp/Glob.h"

#define BOOST_TEST_MODULE Glob

static Pathname TEST_ROOT( TESTS_SRC_DIR"/zypp/base/Glob_test.dat" );

using filesystem::Glob;

BOOST_AUTO_TEST_CASE(Glob_default)
{
  // enable loging for the scope of this block:
  // base::LogControl::TmpLineWriter shutUp( new log::FileLineWriter( "-" ) );
  Glob q;
  BOOST_CHECK( q.empty() );
  BOOST_CHECK( q.size() == 0 );
  BOOST_CHECK_EQUAL( q.begin(), q.end() );
  BOOST_CHECK( q.defaultFlags() == Glob::Flags() );

  q.add( TEST_ROOT/"file" );
  BOOST_CHECK( ! q.empty() );
  BOOST_CHECK( q.size() == 1 );
  BOOST_CHECK_NE( q.begin(), q.end() );
  BOOST_CHECK_EQUAL( *q.begin(), TEST_ROOT/"file" );

  q.reset( Glob::_BRACE );
  BOOST_CHECK( q.empty() );
  BOOST_CHECK( q.size() == 0 );
  BOOST_CHECK_EQUAL( q.begin(), q.end() );
  BOOST_CHECK( q.defaultFlags() == Glob::_BRACE );

  q.add( TEST_ROOT/"file*" );
  BOOST_CHECK( q.size() == 3 );

  q.add( TEST_ROOT/"*{.xml,.xml.gz}" );
  BOOST_CHECK( q.size() == 5 );

  q.clear(); // no flags reset: Glob::_BRACE active
  BOOST_CHECK( q.size() == 0 );

  q.add( TEST_ROOT/"*{.xml,.xml.gz}" );
  BOOST_CHECK( q.size() == 2 );

  q.reset(); // flags reset: Glob::_BRACE off
  BOOST_CHECK( q.size() == 0 );

  q.add( TEST_ROOT/"*{.xml,.xml.gz}" );
  BOOST_CHECK( q.size() == 0 );
}

BOOST_AUTO_TEST_CASE(Glob_static)
{
  std::set<Pathname> q;
  Glob::collect( TEST_ROOT/"*{.xml,.xml.gz}", Glob::_BRACE, std::inserter( q, q.begin() ) );
  BOOST_REQUIRE( q.size() == 2 );
  BOOST_CHECK_EQUAL( *q.begin(), TEST_ROOT/"file.xml" );
  BOOST_CHECK_EQUAL( *++q.begin(), TEST_ROOT/"file.xml.gz" );
}
