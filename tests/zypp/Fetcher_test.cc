#include <stdio.h>
#include <iostream>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/base/Logger.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/Fetcher.h"
#include "zypp/Url.h"
#include "zypp/TmpPath.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace zypp::media;
using namespace boost::unit_test;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "/zypp/data/Fetcher/remote-site")

BOOST_AUTO_TEST_CASE(fetcher)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  Fetcher fetcher;

  filesystem::TmpDir dest;

  fetcher.enqueueDir(OnMediaLocation().setFilename("/complexdir"), true);
  fetcher.start( dest.path(), media );

  BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir2").isExist() );
  BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir2/subdir2-file1.txt").isExist() );
  BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir1/subdir1-file1.txt").isExist() );
  BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir1/subdir1-file2.txt").isExist() );
  
  fetcher.reset();
  
  fetcher.enqueue(OnMediaLocation().setFilename("/file-1.txt"));
  fetcher.start( dest.path(), media );  

  BOOST_CHECK( PathInfo(dest.path() + "/file-1.txt").isExist() );

  MIL << fetcher;
}



// vim: set ts=2 sts=2 sw=2 ai et:
