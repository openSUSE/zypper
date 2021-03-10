#include <stdio.h>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include <zypp/MediaSetAccess.h>
#include <zypp/media/MediaHandler.h>
#include <zypp/Url.h>
#include <zypp/PathInfo.h>

#include "WebServer.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;
using namespace zypp::filesystem;

class SimpleVerifier : public media::MediaVerifierBase
{
public:

  SimpleVerifier( const std::string &id )
  {
    _media_id = id;
  }

  bool isDesiredMedia(const media::MediaHandler &ref) const override
  {
    return ref.doesFileExist(Pathname("/x." + _media_id ));
  }

private:
  std::string _media_id;
};

bool check_file_exists(const Pathname &path)
{
  FILE *file;

  if ((file = fopen(path.asString().c_str(), "r")) == NULL) return false;

  fclose(file);
  return true;
}

/*
 * Check how MediaSetAccess::rewriteUrl() works.
 */
BOOST_AUTO_TEST_CASE(msa_url_rewrite)
{
  BOOST_CHECK_EQUAL(
    MediaSetAccess::rewriteUrl(Url("iso:/?iso=/path/to/CD1.iso"), 1).asString(),
    Url("iso:/?iso=/path/to/CD1.iso").asString());

  BOOST_CHECK_EQUAL(
    MediaSetAccess::rewriteUrl(Url("iso:/?iso=/path/to/CD1.iso"), 2).asString(),
    Url("iso:/?iso=/path/to/CD2.iso").asString());

  BOOST_CHECK_EQUAL(
    MediaSetAccess::rewriteUrl(Url("iso:/?iso=/path/to/CD1.iso"), 13).asString(),
    Url("iso:/?iso=/path/to/CD13.iso").asString());

  BOOST_CHECK_EQUAL(
    MediaSetAccess::rewriteUrl(Url("iso:/?iso=/path/to/cd1.iso"), 2).asString(),
    Url("iso:/?iso=/path/to/cd2.iso").asString());

  BOOST_CHECK_EQUAL(
    MediaSetAccess::rewriteUrl(Url("iso:/?iso=/path/to/cd2.iso"), 1).asString(),
    Url("iso:/?iso=/path/to/cd1.iso").asString());

  BOOST_CHECK_EQUAL(
    MediaSetAccess::rewriteUrl(Url("iso:/?iso=/path/to/dvd1.iso"), 2).asString(),
    Url("iso:/?iso=/path/to/dvd2.iso").asString());

  BOOST_CHECK_EQUAL(
    MediaSetAccess::rewriteUrl(Url("dir:/path/to/CD1"), 2).asString(),
    Url("dir:/path/to/CD2").asString());

  // trailing slash check
  BOOST_CHECK_EQUAL(
    MediaSetAccess::rewriteUrl(Url("dir:/path/to/CD1/"), 2).asString(),
    Url("dir:/path/to/CD2/").asString());

  BOOST_CHECK_EQUAL(
    MediaSetAccess::rewriteUrl(Url("nfs://nfs-server/exported/path/to/dvd1"), 2).asString(),
    Url("nfs://nfs-server/exported/path/to/dvd2").asString());

  // single media check  shouldn't this fail somehow??
  BOOST_CHECK_EQUAL(
    MediaSetAccess::rewriteUrl(Url("http://ftp.opensuse.org/pub/opensuse/distribution/SL-OSS-factory/inst-source"), 2).asString(),
    Url("http://ftp.opensuse.org/pub/opensuse/distribution/SL-OSS-factory/inst-source").asString());
}

#define DATADIR (Pathname(TESTS_SRC_DIR) / "/zypp/data/mediasetaccess")

/*
 *
 * test data dir structure:
 *
 * .
 * |-- src1
 * |   |-- cd1
 * |   |   |-- dir
 * |   |   |   |-- file1
 * |   |   |   |-- file2
 * |   |   |   `-- subdir
 * |   |   |       `-- file
 * |   |   `-- test.txt
 * |   |-- cd2
 * |   |   `-- test.txt
 * |   `-- cd3
 * |       `-- test.txt
 * `-- src2
 *     `-- test.txt
 *
 */

/*
 * Provide files from set without verifiers.
 */
BOOST_AUTO_TEST_CASE(msa_provide_files_set)
{
  Url url = (DATADIR + "/src1/cd1").asUrl();
  MediaSetAccess setaccess(url);

  Pathname file1 = setaccess.provideFile("/test.txt", 1);
  BOOST_CHECK(check_file_exists(file1) == true);

  Pathname file2 = setaccess.provideFile("/test.txt", 2);
  BOOST_CHECK(check_file_exists(file2) == true);

  Pathname file3 = setaccess.provideFile("/test.txt", 3);
  BOOST_CHECK(check_file_exists(file3) == true);
}

/*
 * Provide files from set with verifiers.
 */
BOOST_AUTO_TEST_CASE(msa_provide_files_set_verified)
{
  Url url = (DATADIR + "/src1/cd1").asUrl();
  MediaSetAccess setaccess(url);

  setaccess.setVerifier(1, media::MediaVerifierRef(new SimpleVerifier("media1")));
  setaccess.setVerifier(2, media::MediaVerifierRef(new SimpleVerifier("media2")));
  setaccess.setVerifier(3, media::MediaVerifierRef(new SimpleVerifier("media3")));

  // provide file from media1
  Pathname file1 = setaccess.provideFile("/test.txt", 1);
  BOOST_CHECK(check_file_exists(file1) == true);

  // provide file from invalid media
  BOOST_CHECK_THROW(setaccess.provideFile("/test.txt", 2),
                    media::MediaNotDesiredException);

  // provide file from media3
  Pathname file3 = setaccess.provideFile("/test.txt", 3);
  BOOST_CHECK(check_file_exists(file3) == true);
}

/*
 * Provide file from single media with verifier.
 */
BOOST_AUTO_TEST_CASE(msa_provide_files_single)
{
  Url url = (DATADIR + "/src2").asUrl();
  MediaSetAccess setaccess(url);
  setaccess.setVerifier(1, media::MediaVerifierRef(new SimpleVerifier("media")));

  // provide file from media
  Pathname file = setaccess.provideFile("/test.txt", 1);
  BOOST_CHECK(check_file_exists(file) == true);

  // provide non-existent file
  // (default answer from callback should be ABORT)
  BOOST_CHECK_THROW(setaccess.provideFile("/imnothere", 2),
                    media::MediaFileNotFoundException);
}

/*
 * Provide directory from src/cd1.
 */
BOOST_AUTO_TEST_CASE(msa_provide_dir)
{
  Url url = (DATADIR + "/src1/cd1").asUrl();

  MediaSetAccess setaccess(url);

  Pathname dir = setaccess.provideDir("/dir", false, 1);

  Pathname file1 = dir + "/file1";
  BOOST_CHECK(check_file_exists(file1) == true);

  Pathname file2 = dir + "/file2";
  BOOST_CHECK(check_file_exists(file2) == true);

  // provide non-existent dir
  // (default answer from callback should be ABORT)
  BOOST_CHECK_THROW(setaccess.provideDir("/imnothere", 2),
                    media::MediaFileNotFoundException);

  // This can't be properly tested with 'dir' schema, probably only curl
  // schemas (http, ftp) where download is actually needed.
  // Other schemas just get mounted onto a local dir and the whole subtree
  // is automatically available that way.
  // BOOST_CHECK(check_file_exists(dir + "/subdir/file") == false);
  // BOOST_CHECK(check_file_exists(dir + "/subdir") == false);
}


/*
 * Provide directory from src/cd1 (recursively).
 */
BOOST_AUTO_TEST_CASE(msa_provide_dirtree)
{
  Url url = (DATADIR + "/src1/cd1").asUrl();
  MediaSetAccess setaccess(url);

  Pathname dir = setaccess.provideDir("/dir", true, 1);

  Pathname file1 = dir + "/file1";
  BOOST_CHECK(check_file_exists(file1) == true);

  Pathname file2 = dir + "/file2";
  BOOST_CHECK(check_file_exists(file2) == true);

  Pathname file3 = dir + "/subdir/file";
  BOOST_CHECK(check_file_exists(file3) == true);
}

/*
 * file exists local
 */
BOOST_AUTO_TEST_CASE(msa_file_exist_local)
{
  Url url = (DATADIR + "/src1/cd1").asUrl();
  MediaSetAccess setaccess(url);

  BOOST_CHECK(setaccess.doesFileExist("/test.txt"));
  BOOST_CHECK(!setaccess.doesFileExist("/testBADNAME.txt"));
}

/*
 * file exists remote
 */
BOOST_AUTO_TEST_CASE(msa_remote_tests)
{
  WebServer web( DATADIR / "/src1/cd1", 10002 );
  BOOST_REQUIRE( web.start() );
  MediaSetAccess setaccess( web.url(), "/" );

  BOOST_CHECK(!setaccess.doesFileExist("/testBADNAME.txt"));
  BOOST_CHECK(setaccess.doesFileExist("/test.txt"));

  // check providing a file via http works
  Pathname local = setaccess.provideFile("/test.txt");
  BOOST_CHECK(CheckSum::sha1(sha1sum(local)) == CheckSum::sha1("2616e23301d7fcf7ac3324142f8c748cd0b6692b"));

  // providing a file which does not exist should throw
  BOOST_CHECK_THROW(setaccess.provideFile("/testBADNAME.txt"), media::MediaFileNotFoundException);

  Pathname fPath;
  {
    Url url = web.url();
    url.setPathName("/testBADNAME.txt");

    // providing a file which does not exist should throw
    BOOST_CHECK_THROW(MediaSetAccess::provideFileFromUrl(url), media::MediaFileNotFoundException);

    url.setPathName("/test.txt");

    //providing a file by static method, file should exist after method call
    ManagedFile file = MediaSetAccess::provideFileFromUrl( url );
    fPath = file;
    BOOST_CHECK(check_file_exists(fPath) == true);
  }
  //file should be removed once the ManagedFile goes out of scope
  BOOST_CHECK(check_file_exists(fPath) == false);

  BOOST_CHECK(setaccess.doesFileExist("/test-big.txt"));
  BOOST_CHECK(setaccess.doesFileExist("dir/test-big.txt"));

  {
    // providing a file with wrong filesize should throw
    OnMediaLocation locPlain("dir/test-big.txt");
    locPlain.setDownloadSize( zypp::ByteCount(500, zypp::ByteCount::B) );
    BOOST_CHECK_THROW(setaccess.provideFile(locPlain), media::MediaFileSizeExceededException);

    // using the correct file size should NOT throw
    locPlain.setDownloadSize( zypp::ByteCount(7135, zypp::ByteCount::B) );
    Pathname file = setaccess.provideFile( locPlain );
    BOOST_CHECK(check_file_exists(file) == true);
  }

  {
    // test the maximum filesize again with metalink downloads
    // providing a file with wrong filesize should throw
    OnMediaLocation locMeta("/test-big.txt");
    locMeta.setDownloadSize( zypp::ByteCount(500, zypp::ByteCount::B) );
    BOOST_CHECK_THROW(setaccess.provideFile(locMeta), media::MediaFileSizeExceededException);

    // using the correct file size should NOT throw
    locMeta.setDownloadSize( zypp::ByteCount(7135, zypp::ByteCount::B) );
    Pathname file = setaccess.provideFile( locMeta );
    BOOST_CHECK(check_file_exists(file) == true);
  }

  web.stop();
}


// vim: set ts=2 sts=2 sw=2 ai et:
