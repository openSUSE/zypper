
#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/base/Exception.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;

/**
 * Test case for
 * bool is_checksum( const Pathname & file, const CheckSum &checksum );
 * std::string checksum( const Pathname & file, const std::string &algorithm );
 */
BOOST_AUTO_TEST_CASE(pathinfo_checksum_test)
{
  const char *buffer = "I will test the checksum of this";
  TmpFile file;
  ofstream str(file.path().asString().c_str(),ofstream::out);

  if (!str.good())
    ZYPP_THROW(Exception("cant open file"));

  str << buffer;
  str.flush();
  str.close();

  CheckSum file_sha1("sha1", "142df4277c326f3549520478c188cab6e3b5d042");
  CheckSum file_md5("md5", "f139a810b84d82d1f29fc53c5e59beae");

  BOOST_CHECK_EQUAL( checksum( file.path(), "sha1"), "142df4277c326f3549520478c188cab6e3b5d042" );
  BOOST_CHECK_EQUAL( checksum( file.path(), "md5"), "f139a810b84d82d1f29fc53c5e59beae" );

  BOOST_REQUIRE( is_checksum( file.path(), file_sha1 ) );
  BOOST_REQUIRE( is_checksum( file.path(), file_md5 ) );
}

BOOST_AUTO_TEST_CASE(pathinfo_is_exist_test)
{
  TmpDir dir;
  Pathname subdir("text with spaces");
  // create a fake file
  BOOST_CHECK_EQUAL( filesystem::mkdir(dir.path() + subdir), 0 );

  Pathname filepath = (dir.path() + subdir+ "filename");
  ofstream str(filepath.asString().c_str(),ofstream::out);
  str << "foo bar" << endl;
  str.flush();
  str.close();

  BOOST_CHECK( PathInfo(filepath).isExist() );
}

BOOST_AUTO_TEST_CASE(pathipathinfo_misc_test)
{
  TmpDir dir;

  PathInfo info(dir.path());
  BOOST_CHECK(info.isDir());
}

BOOST_AUTO_TEST_CASE(pathinfo_expandlink_test)
{
  TmpDir dir;

  // ---- not a link

  // create a file
  Pathname file(dir / "file");
  ofstream str(file.asString().c_str(),ofstream::out);
  str << "foo bar" << endl;
  str.flush();
  str.close();

  // expandlink should return the original Pathname if it does not point to a link
  BOOST_CHECK_EQUAL( file, filesystem::expandlink(file) );

  // ---- valid link

  // create a (relative!) link to that file
  Pathname link1(dir / "link1");
  BOOST_CHECK_EQUAL( filesystem::symlink(file.basename(), link1), 0);

  // does the link expand to the file?
  BOOST_CHECK_EQUAL( file, filesystem::expandlink(link1) );

  // ---- broken link

  // create a link to a non-existent file
  Pathname brokenlink(dir / "brokenlink");
  Pathname non_existent(dir / "non-existent");
  BOOST_CHECK_EQUAL( filesystem::symlink(non_existent, brokenlink), 0);
  PathInfo info(brokenlink, PathInfo::LSTAT);
  BOOST_CHECK(info.isLink());

  // expandlink should return an empty Pathname for a broken link
  BOOST_CHECK_EQUAL( Pathname(), filesystem::expandlink(brokenlink) );

  // ---- cyclic link

  // make the 'non-existent' a link to 'brokenlink' :O)
  BOOST_CHECK_EQUAL( filesystem::symlink(brokenlink, non_existent), 0);
  // expandlink should return an empty Pathname for such a cyclic link
  BOOST_CHECK_EQUAL( Pathname(), filesystem::expandlink(brokenlink) );
  BOOST_CHECK_EQUAL( Pathname(), filesystem::expandlink(non_existent) );

  cout << brokenlink << " -> " << filesystem::expandlink(brokenlink) << endl;
}

BOOST_AUTO_TEST_CASE(test_assert_dir_file)
{
  TmpDir   root;

  Pathname rfile( root/"file" );
  BOOST_CHECK_EQUAL( filesystem::assert_file( rfile ), 0 );
  BOOST_CHECK( PathInfo(rfile).isFile() );

  Pathname rdir ( root/"dir" );
  BOOST_CHECK_EQUAL( filesystem::assert_dir ( rdir ),  0 );
  BOOST_CHECK( PathInfo(rdir).isDir() );

  // empty path
  Pathname path;
  BOOST_CHECK_EQUAL( filesystem::assert_file( path ), ENOENT );
  BOOST_CHECK_EQUAL( filesystem::assert_dir ( path ), ENOENT );

  // for dirs:
  // existing dir
  path = rdir;
  BOOST_CHECK_EQUAL( filesystem::assert_dir( path ), 0 );
  BOOST_CHECK( PathInfo(path).isDir() );
  // new dirs
  path = rdir/"sub/subsub";
  BOOST_CHECK_EQUAL( filesystem::assert_dir( path ), 0 );
  BOOST_CHECK( PathInfo(path).isDir() );
  // file in path
  path = rfile/"sub";
  BOOST_CHECK_EQUAL( filesystem::assert_dir( path ), ENOTDIR );
  BOOST_CHECK( !PathInfo(path).isDir() );
  // path is file
  path = rfile;
  BOOST_CHECK_EQUAL( filesystem::assert_dir( path ), EEXIST );
  BOOST_CHECK( !PathInfo(path).isDir() );

  // for files:
  // existing file
  path = rfile;
  BOOST_CHECK_EQUAL( filesystem::assert_file( path ), 0 );
  BOOST_CHECK( PathInfo(path).isFile() );
  // new file
  path = rdir/"sub/file";
  BOOST_CHECK_EQUAL( filesystem::assert_file( path ), 0 );
  BOOST_CHECK( PathInfo(path).isFile() );
  // file in path
  path = rfile/"sub/file";
  BOOST_CHECK_EQUAL( filesystem::assert_file( path ), ENOTDIR );
  BOOST_CHECK( ! PathInfo(path).isFile() );
  // path is dir
  path = rdir;
  BOOST_CHECK_EQUAL( filesystem::assert_file( path ), EEXIST );
  BOOST_CHECK( ! PathInfo(path).isFile() );
}

BOOST_AUTO_TEST_CASE(test_exchange)
{
  TmpDir root;
  Pathname a;
  Pathname b;
  // paths must not be epmty:
  BOOST_CHECK_EQUAL( filesystem::exchange( a, b ), EINVAL );
  a = root/"a/p";
  BOOST_CHECK_EQUAL( filesystem::exchange( a, b ), EINVAL );
  b = root/"b/p";
  BOOST_CHECK_EQUAL( filesystem::exchange( a, b ), 0 ); // ok if both don't exist

  // one path not existing:
  filesystem::assert_file( a );
  BOOST_CHECK( PathInfo(a).isFile() );
  BOOST_CHECK( !PathInfo(b).isFile() );

  BOOST_CHECK_EQUAL( filesystem::exchange( a, b ), 0 );
  BOOST_CHECK( !PathInfo(a).isFile() );
  BOOST_CHECK( PathInfo(b).isFile() );

  BOOST_CHECK_EQUAL( filesystem::exchange( a, b ), 0 );
  BOOST_CHECK( PathInfo(a).isFile() );
  BOOST_CHECK( !PathInfo(b).isFile() );

  // both paths exist:
  filesystem::assert_dir( b );
  BOOST_CHECK( PathInfo(b).isDir() );

  BOOST_CHECK_EQUAL( filesystem::exchange( a, b ), 0 );
  BOOST_CHECK( PathInfo(a).isDir() );
  BOOST_CHECK( PathInfo(b).isFile() );

  BOOST_CHECK_EQUAL( filesystem::exchange( a, b ), 0 );
  BOOST_CHECK( PathInfo(a).isFile() );
  BOOST_CHECK( PathInfo(b).isDir() );
}
