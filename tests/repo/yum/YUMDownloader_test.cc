#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/Url.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"
#include "zypp/source/yum/YUMDownloader.h"


using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;

using namespace zypp::source::yum;

void yum_download_test(const string &dir)
{
  Pathname p = dir + "/10.2-updates-subset";
  Url url("dir:" + p.asString());
  YUMDownloader yum(url, "/");
  filesystem::TmpDir tmp;
  
  Pathname localdir(tmp.path());
  
  yum.download(localdir);
  
  const char* files[] =
  {
    "filelists.xml.gz",
//    "other.xml.gz",
    "patches.xml",
    "patch-fetchmsttfonts.sh-2333.xml",
    "patch-flash-player-2359.xml",
    "patch-glabels-2348.xml",
    "patch-gv-2350.xml",
    "patch-openssl-2349.xml",
    "patch-tar-2351.xml",
    "primary.xml.gz",
    "repomd.xml",
    "repomd.xml.asc",
    "repomd.xml.key",
    NULL
  };
  
  int i=0;
  while ( files[i] != NULL )
  {
    BOOST_CHECK_MESSAGE( PathInfo(localdir + "/repodata/" + files[i] ).isExist(), (string("/repodata/") + files[i]).c_str() );
    i++;
  }

}

test_suite*
init_unit_test_suite( int argc, char *argv[] )
{
  string datadir;
  if (argc < 2)
  {
    datadir = TESTS_SRC_DIR;
    datadir = (Pathname(datadir) + "/repo/yum/data").asString();
    cout << "YUMDownloader_test:"
      " path to directory with test data required as parameter. Using " << datadir  << endl;
    //return (test_suite *)0;
    
  }
  else
  {
    datadir = argv[1];
  }
  
  test_suite* test= BOOST_TEST_SUITE("YUMDownloader");
  
  std::string const params[] = { datadir };
  test->add(BOOST_PARAM_TEST_CASE(&yum_download_test,
                                 (std::string const*)params, params+1));
  return test;
}

// vim: set ts=2 sts=2 sw=2 ai et:
