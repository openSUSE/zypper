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

  replacer1.resetVarCache();

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

  // no target activated yet, there should be no replacement of
  // $distver
  BOOST_CHECK_EQUAL(replacer2(Url("http://site.org/update/$distver/?arch=$arch")).asCompleteString(),
		    "http://site.org/update/$distver/?arch=i686");
    
  // now we initialize the target
  filesystem::TmpDir tmp;
    
  ZYpp::Ptr z = getZYpp();

  // create the products.d directory
  assert_dir(tmp.path() / "/etc/products.d" );
  BOOST_CHECK( copy( Pathname(TESTS_SRC_DIR) / "/zypp/data/Target/product.prod",  tmp.path() / "/etc/products.d/product.prod") == 0 );
  // make it the base product
  BOOST_CHECK( symlink(tmp.path() / "/etc/products.d/product.prod", tmp.path() / "/etc/products.d/baseproduct" ) == 0 );

  replacer2.resetVarCache();

  z->initializeTarget( tmp.path() );
  // target activated, there should be replacement of
  // $distver
  BOOST_CHECK_EQUAL(replacer2(Url("http://site.org/update/$releasever/?arch=$arch")).asCompleteString(),
		    "http://site.org/update/10/?arch=i686");

}

// vim: set ts=2 sts=2 sw=2 ai et:
