
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/TmpPath.h"
#include "zypp/RepoStatus.h"
#include "zypp/PathInfo.h"

#include <boost/test/auto_unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;

BOOST_AUTO_TEST_CASE(repostatus_test)
{
  TmpFile tmpPath;
  TmpFile tmpPath2;
  RepoStatus status;
  RepoStatus fstatus( tmpPath );
  RepoStatus fstatus2( tmpPath2 );
  BOOST_CHECK_EQUAL( status.empty(), true );
  BOOST_CHECK_EQUAL( (status&&status).empty(), true );

  BOOST_CHECK_EQUAL( fstatus.empty(), false );
  BOOST_CHECK_EQUAL( (fstatus&&status).empty(), false );

  BOOST_CHECK_EQUAL( (fstatus&&status), (status&&fstatus) );
  BOOST_CHECK_EQUAL( (fstatus&&fstatus2), (fstatus2&&fstatus) );

}
