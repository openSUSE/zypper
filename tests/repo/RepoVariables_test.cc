#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/auto_unit_test.hpp>

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

#define DATADIR (Pathname(TESTS_SRC_DIR) +  "/repo/yum/data")

BOOST_AUTO_TEST_CASE(replace_text)
{
  /* check RepoVariablesStringReplacer */

  RepoVariablesStringReplacer replacer1;

  BOOST_CHECK_EQUAL(replacer1("http://foo/$arch/bar"),
                    "http://foo/"+ ZConfig::instance().systemArchitecture().asString() + "/bar");

  ZConfig::instance().setSystemArchitecture(Arch("i686"));
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

// vim: set ts=2 sts=2 sw=2 ai et:
