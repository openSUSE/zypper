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
#include "zypp/repo/susetags/Downloader.h"


using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;

using namespace zypp::repo;

void susetags_download_test(const string &dir)
{
  Pathname p = dir + "/stable-x86-subset";
  Url url("dir:" + p.asString());
  MediaSetAccess media(url);
  susetags::Downloader downloader("/");
  filesystem::TmpDir tmp;
  
  Pathname localdir(tmp.path());
  
  downloader.download(media,localdir);
  
  const char* files[] =
  {
    "/suse",
    "/suse/setup",
    "/suse/setup/descr",
    "/suse/setup/descr/kde-10.3-71.i586.pat",
    "/suse/setup/descr/packages",
    "/suse/setup/descr/packages.DU",
    "/suse/setup/descr/packages.en",
    "/suse/setup/descr/packages.es",
    "/suse/setup/descr/patterns",
    "/content",
    "/gpg-pubkey-7e2e3b05-44748aba.asc",
    "/media.1",
//    "/media.1/products.asc",
//    "/media.1/products.key",
    "/media.1/media",
//    "/media.1/products",
//    "/media.1/info.txt",
//    "/media.1/license.zip",
    "/gpg-pubkey-a1912208-446a0899.asc",
    "/gpg-pubkey-307e3d54-44201d5d.asc",
    "/gpg-pubkey-9c800aca-40d8063e.asc",
    "/content.asc",
    "/content.key",
    "/gpg-pubkey-3d25d3d9-36e12d04.asc",
    "/gpg-pubkey-0dfb3188-41ed929b.asc",
    NULL
  };
  
  int i=0;
  while ( files[i] != NULL )
  {
    BOOST_CHECK_MESSAGE( PathInfo(localdir + files[i] ).isExist(), string(files[i]).c_str() );
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
    datadir = (Pathname(datadir) + "/repo/susetags/data").asString();
    cout << "SUSETags Downloader_test:"
      " path to directory with test data required as parameter. Using " << datadir  << endl;
    //return (test_suite *)0;
    
  }
  else
  {
    datadir = argv[1];
  }
  
  test_suite* test= BOOST_TEST_SUITE("SUSETags Downloader");
  
  std::string const params[] = { datadir };
  test->add(BOOST_PARAM_TEST_CASE(&susetags_download_test,
                                 (std::string const*)params, params+1));
  return test;
}

// vim: set ts=2 sts=2 sw=2 ai et:
