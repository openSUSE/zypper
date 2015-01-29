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
  ZConfig::instance().setSystemArchitecture(Arch("i686"));
  ::setenv( "ZYPP_REPO_RELEASEVER", "13.2", 1 );

  RepoVariablesStringReplacer replacer1;
  BOOST_CHECK_EQUAL( replacer1(""),		"" );
  BOOST_CHECK_EQUAL( replacer1("$"),		"$" );
  BOOST_CHECK_EQUAL( replacer1("$arc"),		"$arc" );
  BOOST_CHECK_EQUAL( replacer1("$arch"),	"i686" );

  BOOST_CHECK_EQUAL( replacer1("$archit"),	"$archit" );
  BOOST_CHECK_EQUAL( replacer1("${rc}it"),	"${rc}it" );
  BOOST_CHECK_EQUAL( replacer1("$arch_it"),	"$arch_it" );

  BOOST_CHECK_EQUAL( replacer1("$arch-it"),	"i686-it" );
  BOOST_CHECK_EQUAL( replacer1("$arch it"),	"i686 it" );
  BOOST_CHECK_EQUAL( replacer1("${arch}it"),	"i686it" );

  BOOST_CHECK_EQUAL( replacer1("${arch}it$archit $arch"),	"i686it$archit i686" );
  BOOST_CHECK_EQUAL( replacer1("X${arch}it$archit $arch-it"),	"Xi686it$archit i686-it" );

  BOOST_CHECK_EQUAL( replacer1("${releasever}"),	"13.2" );
  BOOST_CHECK_EQUAL( replacer1("${releasever_major}"),	"13" );
  BOOST_CHECK_EQUAL( replacer1("${releasever_minor}"),	"2" );

  BOOST_CHECK_EQUAL(replacer1("http://foo/$arch/bar"), "http://foo/i686/bar");

  /* check RepoVariablesUrlReplacer */
  RepoVariablesUrlReplacer replacer2;

  BOOST_CHECK_EQUAL(replacer2(Url("ftp://user:secret@site.org/$arch/")).asCompleteString(),
		    "ftp://user:secret@site.org/i686/");

  BOOST_CHECK_EQUAL(replacer2(Url("http://user:my$arch@site.org/$basearch/")).asCompleteString(),
		    "http://user:my$arch@site.org/i386/");

  BOOST_CHECK_EQUAL(replacer2(Url("http://site.org/update/?arch=$arch")).asCompleteString(),
		    "http://site.org/update/?arch=i686");

  BOOST_CHECK_EQUAL(replacer2(Url("http://site.org/update/$releasever/?arch=$arch")).asCompleteString(),
		    "http://site.org/update/13.2/?arch=i686");
}

// vim: set ts=2 sts=2 sw=2 ai et:
