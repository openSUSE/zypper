#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/ZYppFactory.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"
#include "zypp/ZConfig.h"
#include "zypp/repo/RepoVariables.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;

using namespace zypp::repo;

void replace_test(const string &dir)
{
  /* check RepoVariablesStringReplacer */

  RepoVariablesStringReplacer replacer1;

  BOOST_CHECK_EQUAL(replacer1("http://foo/$arch/bar"),
                    "http://foo/"+ ZConfig::instance().systemArchitecture().asString() + "/bar");

  getZYpp()->setArchitecture(Arch("i686"));
  BOOST_CHECK_EQUAL(replacer1("http://foo/$arch/bar/$basearch"),
                    "http://foo/i686/bar/i386");

  /* check RepoVariablesUrlReplacer */

  RepoVariablesUrlReplacer replacer2;

  BOOST_CHECK_EQUAL(replacer2(Url("ftp://user:secret@site.org/$arch/")).asCompleteString(),
		    "ftp://user:secret@site.org/i686/");

  BOOST_CHECK_EQUAL(replacer2(Url("http://user:my$arch@site.org/$basearch/")).asCompleteString(),
		    "http://user:my$arch@site.org/i386/");

  BOOST_CHECK_EQUAL(replacer2(Url("http://site.org/update/?arch=$arch")).asCompleteString(),
		    "http://site.org/update/?arch=i686");
}

test_suite*
init_unit_test_suite( int argc, char *argv[] )
{
  string datadir;
  if (argc < 2)
  {
    datadir = TESTS_SRC_DIR;
    datadir = (Pathname(datadir) + "/repo/yum/data").asString();
    cout << "RepoVariables_test:"
      " path to directory with test data required as parameter. Using " << datadir  << endl;
    //return (test_suite *)0;

  }
  else
  {
    datadir = argv[1];
  }

  test_suite* test= BOOST_TEST_SUITE("RepoVariables");

  std::string const params[] = { datadir };
  test->add(BOOST_PARAM_TEST_CASE(&replace_test,
                                 (std::string const*)params, params+1));
  return test;
}

// vim: set ts=2 sts=2 sw=2 ai et:
