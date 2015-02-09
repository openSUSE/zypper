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
#include "zypp/base/ValueTransform.h"
#include "zypp/repo/RepoVariables.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;
using namespace zypp::repo;

#define DATADIR (Pathname(TESTS_SRC_DIR) +  "/repo/yum/data")

typedef std::list<std::string> ListType;

namespace std {
  std::ostream & operator<<( std::ostream & str, const ListType & obj )
  {
    str << "[";
    for ( const auto & el : obj )
      str << " " << el;
    return str << " ]";
  }
}

// plain functor
struct PlainTransformator
{
  std::string operator()( const std::string & value_r ) const
  { return "{"+value_r+"}"; }
};

// plain functor + std::unary_function typedefs
struct FncTransformator : public PlainTransformator, public std::unary_function<const std::string &, std::string>
{};


BOOST_AUTO_TEST_CASE(value_transform)
{
  using zypp::base::ValueTransform;
  using zypp::base::ContainerTransform;

  typedef ValueTransform<std::string, FncTransformator> ReplacedString;
  typedef ContainerTransform<ListType, FncTransformator> ReplacedStringList;

  ReplacedString r( "val" );
  BOOST_CHECK_EQUAL( r.raw(), "val" );
  BOOST_CHECK_EQUAL( r.transformed(),	"{val}" );

  r.raw() = "new";
  BOOST_CHECK_EQUAL( r.raw(), "new" );
  BOOST_CHECK_EQUAL( r.transformed(),	"{new}" );

  ReplacedStringList rl;
  BOOST_CHECK_EQUAL( rl.empty(), true );
  BOOST_CHECK_EQUAL( rl.size(), 0 );
  BOOST_CHECK_EQUAL( rl.raw(), ListType() );
  BOOST_CHECK_EQUAL( rl.transformed(), ListType() );

  rl.raw().push_back("a");
  rl.raw().push_back("b");
  rl.raw().push_back("c");

  BOOST_CHECK_EQUAL( rl.empty(), false );
  BOOST_CHECK_EQUAL( rl.size(), 3 );
  BOOST_CHECK_EQUAL( rl.raw(), ListType({ "a","b","c" }) );
  BOOST_CHECK_EQUAL( rl.transformed(), ListType({ "{a}", "{b}", "{c}" }) );

  BOOST_CHECK_EQUAL( rl.transformed( rl.rawBegin() ), "{a}" );
}

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
