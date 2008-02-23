
#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <string>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/PathInfo.h"
#include "zypp/Digest.h"

using boost::unit_test::test_case;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;

/**
 * Test case for
 * static std::string digest(const std::string& name, std::istream& is, size_t bufsize = 4096);
 */
BOOST_AUTO_TEST_CASE(digest)
{
  string data("I will test the checksum of this");
  stringstream str1(data);
  stringstream str2(data);
  stringstream str3(data);

  BOOST_CHECK_EQUAL( Digest::digest( "sha1", str1 ), "142df4277c326f3549520478c188cab6e3b5d042" ); 
  BOOST_CHECK_EQUAL( Digest::digest( "md5", str2 ), "f139a810b84d82d1f29fc53c5e59beae" ); 
  // FIXME i think it should throw
  BOOST_CHECK_EQUAL( Digest::digest( "lalala", str3) , "" ); 
}
