#include <stdio.h>
#include <iostream>
#ifdef BOOST_AUTO_TEST_MAIN
#undef BOOST_AUTO_TEST_MAIN
#endif
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/MediaSetAccess.h"
#include "zypp/Url.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;


class SimpleVerifier : public media::MediaVerifierBase
{
public:

  SimpleVerifier( const std::string &id )
  {
    _media_id = id;
  }

  virtual bool isDesiredMedia(const media::MediaAccessRef &ref)
  {
    return ref->doesFileExist(Pathname("/." + _media_id ));
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
void msa_url_rewrite()
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

/*
 * Provide files from set without verifiers.
 */
void msa_provide_files_set(const string &urlstr)
{
  Url url(urlstr);
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
void msa_provide_files_set_verified(const string &urlstr)
{
  Url url(urlstr);
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
void msa_provide_files_single(const string &urlstr)
{
  Url url(urlstr);
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
void msa_provide_dir(const string &urlstr)
{
  Url url(urlstr);
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
void msa_provide_dirtree(const string &urlstr)
{
  Url url(urlstr);
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

test_suite*
init_unit_test_suite( int argc, char *argv[] )
{
  test_suite* test= BOOST_TEST_SUITE("MediaSetAccessTest");

  // urls to test
  string datadir( TESTS_SRC_DIR );
  datadir += "/zypp/data/mediasetaccess";
  std::string const params[] = {"dir:" + datadir + "/src1/cd1"};
  std::string const params_single[] = {"dir:" + datadir + "/src2"};

  // url rewrite
  test->add(BOOST_TEST_CASE(&msa_url_rewrite));

  // provide files from media set
  test->add(BOOST_PARAM_TEST_CASE(&msa_provide_files_set,
                                  (std::string const*)params, params+1));

  // provide files from media set with verifier
  test->add(BOOST_PARAM_TEST_CASE(&msa_provide_files_set_verified,
                                  (std::string const*)params, params+1));

  // provide file from single media
  test->add(BOOST_PARAM_TEST_CASE(&msa_provide_files_single,
                                  (std::string const*)params_single, params_single+1));

  // provide directory
  test->add(BOOST_PARAM_TEST_CASE(&msa_provide_dir,
                                  (std::string const*)params, params+1));

  // provide directory tree
  test->add(BOOST_PARAM_TEST_CASE(&msa_provide_dirtree,
                                  (std::string const*)params, params+1));

  return test;
}

// vim: set ts=2 sts=2 sw=2 ai et:
