
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/TmpPath.h"
#include "zypp/RepoStatus.h"
#include "zypp/PathInfo.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using namespace boost::unit_test::log;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;
using namespace zypp::repo;

void repostatus_test()
{
  TmpDir tmpPath;
  RepoStatus status;
}

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  test_suite* test= BOOST_TEST_SUITE( "RepoStatus" );
  test->add( BOOST_TEST_CASE( &repostatus_test ), 0 /* expected zero error */ );
  return test;
}

