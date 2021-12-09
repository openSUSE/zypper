// Boost.Test
#include <boost/test/unit_test.hpp>

#include <zypp-core/base/GzStream>
#include <zypp/Pathname.h>
#include <zypp-core/base/InputStream>

BOOST_AUTO_TEST_CASE(gz_simple_read_write)
{
  const zypp::Pathname file = zypp::Pathname(TESTS_BUILD_DIR) / "test.gz";
  const std::string testString("HelloWorld");

  {
    zypp::ofgzstream strOut( file.c_str() );
    BOOST_REQUIRE ( strOut.is_open() );
    BOOST_REQUIRE ( !strOut.fail() );
    BOOST_REQUIRE ( strOut.getbuf().canWrite() );
    BOOST_REQUIRE ( !strOut.getbuf().canRead() );
    strOut << testString;
    BOOST_REQUIRE ( !strOut.fail() );
  }

  {
    std::string test;
    zypp::ifgzstream str( file.c_str() );
    BOOST_REQUIRE ( str.is_open() );
    BOOST_REQUIRE ( !str.fail() );
    BOOST_REQUIRE ( !str.getbuf().canWrite() );
    BOOST_REQUIRE ( str.getbuf().canRead() );
    str >> test;
    BOOST_REQUIRE ( !str.fail() );
    BOOST_REQUIRE_EQUAL( test, testString );
  }

  {
    zypp::InputStream iStr( file );
    BOOST_REQUIRE( typeid( iStr.stream() ) == typeid( zypp::ifgzstream& ) );
  }
}

BOOST_AUTO_TEST_CASE(gz_seek)
{
  const zypp::Pathname file = zypp::Pathname(TESTS_BUILD_DIR) / "testseek.gz";
  const std::string testString("Hello World!\nLet's see if seeking works!");

  {
    zypp::ofgzstream strOut( file.c_str() );
    BOOST_REQUIRE( strOut.is_open() );
    strOut << testString;
  }

  //gzseek only supports SEEK_SET (beg) and SEEK_CUR (cur), SEEK_END (end) is not supported
  {
    std::string test;
    zypp::ifgzstream str( file.c_str() );
    str.seekg( 6, std::ios_base::beg );
    BOOST_REQUIRE ( !str.fail() );
    BOOST_REQUIRE_EQUAL( str.tellg(), 6 );
    str >> test;
    BOOST_REQUIRE_EQUAL( test, "World!" );
    BOOST_REQUIRE ( !str.fail() );
    BOOST_REQUIRE_EQUAL( str.tellg(), 12 );

    str.seekg( 7, std::ios_base::cur );
    BOOST_REQUIRE ( !str.fail() );
    BOOST_REQUIRE_EQUAL( str.tellg(), 19 );
    str >> test;
    BOOST_REQUIRE_EQUAL( test, "see" );
    BOOST_REQUIRE ( !str.fail() );
    BOOST_REQUIRE_EQUAL( str.tellg(), 22 );

    str.seekg( 0, std::ios_base::beg );
    BOOST_REQUIRE ( !str.fail() );
    BOOST_REQUIRE_EQUAL( str.tellg(), 0 );
    str >> test;
    BOOST_REQUIRE ( !str.fail() );
    BOOST_REQUIRE_EQUAL( str.tellg(), 5 );
    BOOST_REQUIRE_EQUAL( test, "Hello" );
  }
}
