
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/TmpPath.h"
#include "zypp/RepoStatus.h"
#include "zypp/PathInfo.h"

#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;

BOOST_AUTO_TEST_CASE(repostatus_test)
{
  TmpDir tmpPath;
  RepoStatus status;
}
