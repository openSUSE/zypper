#include <stdio.h>
#include <iostream>
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/base/Logger.h"
#include "zypp/ZYppFactory.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/Fetcher.h"
#include "zypp/Url.h"
#include "zypp/TmpPath.h"

#include "WebServer.h"
#include <boost/thread.hpp>

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

  // Now test that without trusting it, it should throw
  // do the test by trusting the SHA1SUMS file signature key
  {
      filesystem::TmpDir dest;

      fetcher.enqueueDir(OnMediaLocation().setFilename("/complexdir"), true);
      BOOST_CHECK_THROW( fetcher.start( dest.path(), media ), Exception);
      fetcher.reset();
  }

  // do the test by trusting the SHA1SUMS file signature key
  {
      filesystem::TmpDir dest;

      // add the key as trusted
      getZYpp()->keyRing()->importKey(PublicKey(DATADIR + "/complexdir/subdir1/SHA1SUMS.key"), true);
      fetcher.enqueueDir(OnMediaLocation().setFilename("/complexdir"), true);
      fetcher.start( dest.path(), media );

      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir2").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir2/subdir2-file1.txt").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir1/subdir1-file1.txt").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir1/subdir1-file2.txt").isExist() );
  
      fetcher.reset();
  }

  // do the test by trusting the SHA1SUMS file signature key but with a broken file
  {
      filesystem::TmpDir dest;

      // add the key as trusted
      getZYpp()->keyRing()->importKey(PublicKey(DATADIR + "/complexdir-broken/subdir1/SHA1SUMS.key"), true);
      fetcher.enqueueDir(OnMediaLocation().setFilename("/complexdir-broken"), true);
      BOOST_CHECK_THROW( fetcher.start( dest.path(), media ), Exception);  
      fetcher.reset();
  }


  {  
      filesystem::TmpDir dest;

      fetcher.enqueue(OnMediaLocation().setFilename("/file-1.txt"));
      fetcher.start( dest.path(), media );  
      BOOST_CHECK( PathInfo(dest.path() + "/file-1.txt").isExist() );
  }
  
  //MIL << fetcher;
}

BOOST_AUTO_TEST_CASE(fetcher_remove)
{
  // at this point the key is already trusted
  {
      // add the key as trusted
      //getZYpp()->keyRing()->importKey(PublicKey(DATADIR + "/complexdir/subdir1/SHA1SUMS.key"), true);

      WebServer web((Pathname(TESTS_SRC_DIR) + "/zypp/data/Fetcher/remote-site").c_str() );
      web.start();

      MediaSetAccess media( Url("http://localhost:9099"), "/" );
      Fetcher fetcher;
      filesystem::TmpDir dest;

      fetcher.enqueueDir(OnMediaLocation().setFilename("/complexdir"), true);
      fetcher.start( dest.path(), media );

      fetcher.reset();

      fetcher.enqueueDir(OnMediaLocation().setFilename("/complexdir-broken"), true);
      BOOST_CHECK_THROW( fetcher.start( dest.path(), media ), Exception);  

      fetcher.reset();

      web.stop();
  }
}


// vim: set ts=2 sts=2 sw=2 ai et:
