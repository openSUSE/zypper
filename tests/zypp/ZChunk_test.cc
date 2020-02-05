// Boost.Test
#include <boost/test/unit_test.hpp>

#include <zypp/base/ZckStream.h>
#include <zypp/Pathname.h>
#include <zypp/base/InputStream.h>
#include <zypp/PathInfo.h>

BOOST_AUTO_TEST_CASE(zchunk_simple_read_write)
{
  const zypp::Pathname file = zypp::Pathname(TESTS_BUILD_DIR) / "test.zck";
  const std::string testString("HelloWorld");

  {
    zypp::ofzckstream strOut( file.c_str() );
    BOOST_REQUIRE( strOut.is_open() );
    strOut << testString;
  }

  BOOST_REQUIRE_EQUAL( zypp::filesystem::zipType( file ), zypp::filesystem::ZT_ZCHNK  );

  {
    std::string test;
    zypp::ifzckstream str( file.c_str() );
    str >> test;
    BOOST_REQUIRE_EQUAL( test, testString );
  }

  {
    zypp::InputStream iStr( file );
    BOOST_REQUIRE( typeid( iStr.stream() ) == typeid( zypp::ifzckstream& ) );
  }
}
