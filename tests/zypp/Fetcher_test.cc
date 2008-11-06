#include <stdio.h>
#include <iostream>
#define BOOST_TEST_MODULE fetcher_test
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

BOOST_AUTO_TEST_SUITE( fetcher_test );

BOOST_AUTO_TEST_CASE(fetcher_enqueuedir_noindex)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  // Now test that without trusting it, it should throw
  // do the test by trusting the SHA1SUMS file signature key
  {
      filesystem::TmpDir dest;
      Fetcher fetcher;
      fetcher.enqueueDir(OnMediaLocation("/complexdir"), true);
      fetcher.start( dest.path(), media );
      fetcher.reset();

      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir2").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir2/subdir2-file1.txt").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir1/subdir1-file1.txt").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir1/subdir1-file2.txt").isExist() );
  }
}

BOOST_AUTO_TEST_CASE(fetcher_enqueuedir_autoindex)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  // do the test by trusting the SHA1SUMS file signature key
  {
      filesystem::TmpDir dest;

      // add the key as trusted
      getZYpp()->keyRing()->importKey(PublicKey(DATADIR + "/complexdir/subdir1/SHA1SUMS.key"), true);
      Fetcher fetcher;
      fetcher.setOptions( Fetcher::AutoAddIndexes );
      fetcher.enqueueDir(OnMediaLocation("/complexdir"), true);
      fetcher.start( dest.path(), media );

      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir2").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir2/subdir2-file1.txt").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir1/subdir1-file1.txt").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir1/subdir1-file2.txt").isExist() );

      fetcher.reset();
  }
}

BOOST_AUTO_TEST_CASE(fetcher_enqueue_digested_dir_autoindex)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  // do the test by trusting the SHA1SUMS file signature key but with a broken file
  {
      filesystem::TmpDir dest;
      Fetcher fetcher;
      // add the key as trusted
      getZYpp()->keyRing()->importKey(PublicKey(DATADIR + "/complexdir-broken/subdir1/SHA1SUMS.key"), true);
      fetcher.setOptions( Fetcher::AutoAddIndexes );
      fetcher.enqueueDigestedDir(OnMediaLocation("/complexdir-broken"), true);
      BOOST_CHECK_THROW( fetcher.start( dest.path(), media ), FileCheckException);
      fetcher.reset();
  }
}

BOOST_AUTO_TEST_CASE(fetcher_enqueuebrokendir_noindex)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  // do the test by trusting the SHA1SUMS file signature key but with a broken file
  {
      filesystem::TmpDir dest;
      Fetcher fetcher;
      // add the key as trusted
      getZYpp()->keyRing()->importKey(PublicKey(DATADIR + "/complexdir-broken/subdir1/SHA1SUMS.key"), true);
      fetcher.enqueueDir(OnMediaLocation("/complexdir-broken"), true);
      // this should not throw as we provided no indexes and the
      // enqueue is not digested
      fetcher.start( dest.path(), media );
      fetcher.reset();

      BOOST_CHECK( PathInfo(dest.path() + "/complexdir-broken/subdir2").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir-broken/subdir2/subdir2-file1.txt").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir-broken/subdir1/subdir1-file1.txt").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir-broken/subdir1/subdir1-file2.txt").isExist() );

  }
}

BOOST_AUTO_TEST_CASE(fetcher_enqueuebrokendir_index)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  // do the test by trusting the SHA1SUMS file signature key but with a broken file
  {
      filesystem::TmpDir dest;
      Fetcher fetcher;
      // add the key as trusted
      getZYpp()->keyRing()->importKey(PublicKey(DATADIR + "/complexdir-broken/subdir1/SHA1SUMS.key"), true);
      fetcher.setOptions( Fetcher::AutoAddIndexes );
      fetcher.enqueueDir(OnMediaLocation("/complexdir-broken"), true);
      BOOST_CHECK_THROW( fetcher.start( dest.path(), media ), Exception);
      fetcher.reset();
  }
}


BOOST_AUTO_TEST_CASE(fetcher_enqueue_digesteddir_brokendir_with_index)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  // do the test by trusting the SHA1SUMS file signature key but with a broken file
  {
      filesystem::TmpDir dest;
      Fetcher fetcher;
      // add the key as trusted
      getZYpp()->keyRing()->importKey(PublicKey(DATADIR + "/complexdir-broken/subdir1/SHA1SUMS.key"), true);
      fetcher.setOptions( Fetcher::AutoAddIndexes );
      fetcher.enqueueDigestedDir(OnMediaLocation("/complexdir-broken"), true);
      BOOST_CHECK_THROW( fetcher.start( dest.path(), media ), Exception);
      fetcher.reset();
  }
}

BOOST_AUTO_TEST_CASE(fetcher_enqueue_digested_broken_with_autoindex)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  // do the test by trusting the SHA1SUMS file signature key but with a broken file
  {
      filesystem::TmpDir dest;
      Fetcher fetcher;
      // add the key as trusted
      getZYpp()->keyRing()->importKey(PublicKey(DATADIR + "/complexdir-broken/subdir1/SHA1SUMS.key"), true);
      fetcher.setOptions( Fetcher::AutoAddIndexes );
      fetcher.enqueueDigested(OnMediaLocation("/complexdir-broken/subdir1/subdir1-file1.txt"));
      BOOST_CHECK_THROW( fetcher.start( dest.path(), media ), Exception);
      fetcher.reset();
  }
}

BOOST_AUTO_TEST_CASE(fetcher_enqueue_digested_with_autoindex)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  // do the test by trusting the SHA1SUMS file signature key with a good file
  // checksum in auto discovered index
  {
      filesystem::TmpDir dest;
      Fetcher fetcher;
      // add the key as trusted
      getZYpp()->keyRing()->importKey(PublicKey(DATADIR + "/complexdir/subdir1/SHA1SUMS.key"), true);
      fetcher.setOptions( Fetcher::AutoAddIndexes );
      fetcher.enqueueDigested(OnMediaLocation("/complexdir/subdir1/subdir1-file1.txt"));
      fetcher.start( dest.path(), media );
      fetcher.reset();
  }
}


BOOST_AUTO_TEST_CASE(fetcher_enqueuefile_noindex)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  {
      filesystem::TmpDir dest;
      Fetcher fetcher;
      fetcher.enqueue(OnMediaLocation("/file-1.txt"));
      fetcher.start( dest.path(), media );
      BOOST_CHECK( PathInfo(dest.path() + "/file-1.txt").isExist() );
  }

  //MIL << fetcher;
}

BOOST_AUTO_TEST_CASE(fetcher_simple)
{
    MediaSetAccess media( (DATADIR).asUrl(), "/" );
    Fetcher fetcher;
    
    {
        filesystem::TmpDir dest;
        OnMediaLocation loc("/complexdir/subdir1/subdir1-file1.txt");
        loc.setChecksum(CheckSum::sha1("f1d2d2f924e986ac86fdf7b36c94bcdf32beec15"));
        fetcher.enqueueDigested(loc);
        fetcher.start(dest.path(), media);
        fetcher.reset();
        // now we break the checksum and it should fail
        loc.setChecksum(CheckSum::sha1("f1d2d2f924e986ac86fdf7b36c94bcdf32beec16"));
        fetcher.enqueueDigested(loc);
        BOOST_CHECK_THROW( fetcher.start( dest.path(), media ), Exception);
        fetcher.reset();

    }
}

BOOST_AUTO_TEST_CASE(content_index)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  Fetcher fetcher;

  // test transfering one file by setting the index
  {
        filesystem::TmpDir dest;
        OnMediaLocation loc("/contentindex/subdir1/subdir1-file1.txt");
        // trust the key manually
        getZYpp()->keyRing()->importKey(PublicKey(DATADIR + "/contentindex/content.key"), true);
        fetcher.addIndex(OnMediaLocation("/contentindex/content", 1));       
        fetcher.enqueue(loc);
        fetcher.start(dest.path(), media);
        fetcher.reset();
        BOOST_CHECK( PathInfo(dest.path() + "/contentindex/subdir1/subdir1-file1.txt").isExist() );
  }

}

BOOST_AUTO_TEST_CASE(enqueue_broken_content_index)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  Fetcher fetcher;
  filesystem::TmpDir dest;
  {
        OnMediaLocation loc("/contentindex-broken-digest/subdir1/subdir1-file1.txt",1);
        // key was already imported as trusted
        fetcher.addIndex(OnMediaLocation("/contentindex-broken-digest/content", 1));
        fetcher.enqueue(loc);
        fetcher.start(dest.path(), media);
        fetcher.reset();
        BOOST_CHECK( PathInfo(dest.path() + "/contentindex-broken-digest/subdir1/subdir1-file1.txt").isExist() );

        // now retrieve a file that is modified, so the checksum has to fail
        loc = OnMediaLocation("/contentindex-broken-digest/subdir1/subdir1-file2.txt",1);
        fetcher.addIndex(OnMediaLocation("/contentindex-broken-digest/content", 1));
        fetcher.enqueue(loc);
        BOOST_CHECK_THROW( fetcher.start( dest.path(), media ), Exception);
        fetcher.reset();
  }
}

BOOST_AUTO_TEST_CASE(enqueue_digested_images_file_content_autoindex)
{
  MediaSetAccess media( ( DATADIR + "/images-file").asUrl(), "/" );
  Fetcher fetcher;
  filesystem::TmpDir dest;
  {
        OnMediaLocation loc("/images/images.xml",1);
        fetcher.setOptions( Fetcher::AutoAddIndexes );
        fetcher.enqueueDigested(loc);
        fetcher.start(dest.path(), media);
        fetcher.reset();
        BOOST_CHECK( PathInfo(dest.path() + "/images/images.xml").isExist() );
        fetcher.reset();
  }
}

BOOST_AUTO_TEST_CASE(enqueue_digested_images_file_content_autoindex_unsigned)
{
  MediaSetAccess media( ( DATADIR + "/images-file-unsigned").asUrl(), "/" );
  Fetcher fetcher;
  filesystem::TmpDir dest;
  {
        OnMediaLocation loc("/images/images.xml",1);
        fetcher.setOptions( Fetcher::AutoAddIndexes );
        fetcher.enqueueDigested(loc);
        // it should throw because unsigned file throws
        BOOST_CHECK_THROW( fetcher.start( dest.path(), media ), FileCheckException);
        fetcher.reset();
        // the target file was NOT transfered
        BOOST_CHECK( ! PathInfo(dest.path() + "/images/images.xml").isExist() );
        fetcher.reset();
  }
}

BOOST_AUTO_TEST_CASE(enqueue_broken_content_noindex)
{
  MediaSetAccess media( ( DATADIR).asUrl(), "/" );
  Fetcher fetcher;

  {
        filesystem::TmpDir dest;
        OnMediaLocation loc("/contentindex-broken-digest/subdir1/subdir1-file1.txt",1);
        // key was already imported as trusted
        fetcher.enqueue(loc);
        fetcher.start(dest.path(), media);
        fetcher.reset();
        // now retrieve a file that is modified, so the checksum has to fail
        loc = OnMediaLocation("/contentindex-broken-digest/subdir1/subdir1-file2.txt",1);
        fetcher.enqueue(loc);
        fetcher.start( dest.path(), media );
        fetcher.reset();
        BOOST_CHECK( PathInfo(dest.path() + "/contentindex-broken-digest/subdir1/subdir1-file2.txt").isExist() );

  }
}


BOOST_AUTO_TEST_CASE(enqueuedir_http)
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

      // auto add the SHA1SUMS
      fetcher.setOptions( Fetcher::AutoAddIndexes );
      fetcher.enqueueDir(OnMediaLocation("/complexdir"), true);
      fetcher.start( dest.path(), media );

      fetcher.reset();

      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir2").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir2/subdir2-file1.txt").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir1/subdir1-file1.txt").isExist() );
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir/subdir1/subdir1-file2.txt").isExist() );

      web.stop();
  }
}

BOOST_AUTO_TEST_CASE(enqueuedir_http_broken)
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

      // auto add the SHA1SUMS
      fetcher.setOptions( Fetcher::AutoAddIndexes );
      fetcher.enqueueDir(OnMediaLocation("/complexdir-broken"), true);
      // should throw because wrong checksum
      BOOST_CHECK_THROW( fetcher.start( dest.path(), media ), FileCheckException);
      fetcher.reset();

      BOOST_CHECK( PathInfo(dest.path() + "/complexdir-broken/subdir2").isExist() );

      BOOST_CHECK( ! PathInfo(dest.path() + "/complexdir-broken/subdir2/subdir2-file1.txt").isExist() );

      // this one got transfered before the failure, so it is there
      BOOST_CHECK( PathInfo(dest.path() + "/complexdir-broken/subdir1/subdir1-file1.txt").isExist() );
      BOOST_CHECK( ! PathInfo(dest.path() + "/complexdir-broken/subdir1/subdir1-file2.txt").isExist() );

      fetcher.reset();

      web.stop();
  }
}


BOOST_AUTO_TEST_SUITE_END();

// vim: set ts=2 sts=2 sw=2 ai et:
