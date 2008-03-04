
#include <iostream>
#include <list>
#include <string>

// Boost.Test
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

using boost::unit_test::test_case;
using namespace std;
using namespace zypp;


// most frequently you implement test cases as a free functions
BOOST_AUTO_TEST_CASE(checksum_test)
{
  BOOST_CHECK_THROW( CheckSum( "sha1", "dsdsads" ), Exception ); // wrong size
  BOOST_CHECK_THROW( CheckSum( "sha256", "dsdsads" ), Exception ); // wrong size
  BOOST_CHECK_THROW( CheckSum( "md5", "dsdsads" ), Exception ); // wrong size
}
