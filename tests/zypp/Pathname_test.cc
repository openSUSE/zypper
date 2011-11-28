#include <iostream>
#include <string>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/LogTools.h"
#include "zypp/Pathname.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;

BOOST_AUTO_TEST_CASE(pathname_default_ctor)
{
  Pathname p;

  BOOST_CHECK_EQUAL(p.empty(),		true );
  BOOST_CHECK_EQUAL(p.absolute(),	false );
  BOOST_CHECK_EQUAL(p.relative(),	false );
  BOOST_CHECK_EQUAL(p.dirname(),	"" );
  BOOST_CHECK_EQUAL(p.basename(),	"" );
  BOOST_CHECK_EQUAL(p.extension(),	"" );
  BOOST_CHECK_EQUAL(p.absolutename(),	"" );
  BOOST_CHECK_EQUAL(p.relativename(),	"" );
}

BOOST_AUTO_TEST_CASE(pathname_root)
{
  Pathname p("/");

  BOOST_CHECK_EQUAL(p.empty(),		false );
  BOOST_CHECK_EQUAL(p.absolute(),	true );
  BOOST_CHECK_EQUAL(p.relative(),	false );
  BOOST_CHECK_EQUAL(p.dirname(),	"/" );
  BOOST_CHECK_EQUAL(p.basename(),	"/" );
  BOOST_CHECK_EQUAL(p.extension(),	"" );
  BOOST_CHECK_EQUAL(p.absolutename(),	"/" );
  BOOST_CHECK_EQUAL(p.relativename(),	"./" );
}

BOOST_AUTO_TEST_CASE(pathname_this)
{
  Pathname p(".");

  BOOST_CHECK_EQUAL(p.empty(),		false );
  BOOST_CHECK_EQUAL(p.absolute(),	false );
  BOOST_CHECK_EQUAL(p.relative(),	true );
  BOOST_CHECK_EQUAL(p.dirname(),	"." );
  BOOST_CHECK_EQUAL(p.basename(),	"." );
  BOOST_CHECK_EQUAL(p.extension(),	"" );
  BOOST_CHECK_EQUAL(p.absolutename(),	"/" );
  BOOST_CHECK_EQUAL(p.relativename(),	"." );
}

BOOST_AUTO_TEST_CASE(pathname_up)
{
  Pathname p("..");

  BOOST_CHECK_EQUAL(p.empty(),		false );
  BOOST_CHECK_EQUAL(p.absolute(),	false );
  BOOST_CHECK_EQUAL(p.relative(),	true );
  BOOST_CHECK_EQUAL(p.dirname(),	"." );
  BOOST_CHECK_EQUAL(p.basename(),	".." );
  BOOST_CHECK_EQUAL(p.extension(),	"" );
  BOOST_CHECK_EQUAL(p.absolutename(),	"/" );
  BOOST_CHECK_EQUAL(p.relativename(),	".." );
}

BOOST_AUTO_TEST_CASE(pathname_abs)
{
  Pathname p("/foo/baa.ka");

  BOOST_CHECK_EQUAL(p.empty(),		false );
  BOOST_CHECK_EQUAL(p.absolute(),	true );
  BOOST_CHECK_EQUAL(p.relative(),	false );
  BOOST_CHECK_EQUAL(p.dirname(),	"/foo" );
  BOOST_CHECK_EQUAL(p.basename(),	"baa.ka" );
  BOOST_CHECK_EQUAL(p.extension(),	".ka" );
  BOOST_CHECK_EQUAL(p.absolutename(),	"/foo/baa.ka" );
  BOOST_CHECK_EQUAL(p.relativename(),	"./foo/baa.ka" );
}

BOOST_AUTO_TEST_CASE(pathname_rel)
{
  Pathname p("./foo/./../baa.ka");

  BOOST_CHECK_EQUAL(p.empty(),		false );
  BOOST_CHECK_EQUAL(p.absolute(),	false );
  BOOST_CHECK_EQUAL(p.relative(),	true );
  BOOST_CHECK_EQUAL(p.dirname(),	"." );
  BOOST_CHECK_EQUAL(p.basename(),	"baa.ka" );
  BOOST_CHECK_EQUAL(p.extension(),	".ka" );
  BOOST_CHECK_EQUAL(p.absolutename(),	"/baa.ka" );
  BOOST_CHECK_EQUAL(p.relativename(),	"./baa.ka" );
}

BOOST_AUTO_TEST_CASE(pathname_relup)
{
  Pathname p("./../foo/./../baa");

  BOOST_CHECK_EQUAL(p.empty(),		false );
  BOOST_CHECK_EQUAL(p.absolute(),	false );
  BOOST_CHECK_EQUAL(p.relative(),	true );
  BOOST_CHECK_EQUAL(p.dirname(),	".." );
  BOOST_CHECK_EQUAL(p.basename(),	"baa" );
  BOOST_CHECK_EQUAL(p.extension(),	"" );
  BOOST_CHECK_EQUAL(p.absolutename(),	"/baa" );
  BOOST_CHECK_EQUAL(p.relativename(),	"../baa" );
}

BOOST_AUTO_TEST_CASE(pathname_strval)
{
  BOOST_CHECK_EQUAL(Pathname("").asString(),		"" );
  BOOST_CHECK_EQUAL(Pathname("/////./").asString(),	"/" );
  BOOST_CHECK_EQUAL(Pathname("./").asString(),		"." );
  BOOST_CHECK_EQUAL(Pathname("/.").asString(),		"/" );
  BOOST_CHECK_EQUAL(Pathname("./..").asString(),	"./.." );	// ? ..
  BOOST_CHECK_EQUAL(Pathname("../").asString(),		"./.." );	// ? ..
  BOOST_CHECK_EQUAL(Pathname(".././..").asString(),	"./../.." );	// ? ../..


  BOOST_CHECK_EQUAL(Pathname("//baa").asString(),	"/baa" );
  BOOST_CHECK_EQUAL(Pathname("/./baa").asString(),	"/baa" );
  BOOST_CHECK_EQUAL(Pathname("/baa/..").asString(),	"/" );
  BOOST_CHECK_EQUAL(Pathname("/baa/../baa").asString(),	"/baa" );
  BOOST_CHECK_EQUAL(Pathname("/./../foo/./../baa").asString(),	"/baa" );

  BOOST_CHECK_EQUAL(Pathname("/").asString(),		"/" );
  BOOST_CHECK_EQUAL(Pathname(".").asString(),		"." );
  BOOST_CHECK_EQUAL(Pathname("..").asString(),		"./.." );
  BOOST_CHECK_EQUAL(Pathname("/.").asString(),		"/" );
  BOOST_CHECK_EQUAL(Pathname("/..").asString(),		"/" );
  BOOST_CHECK_EQUAL(Pathname("/./.").asString(),	"/" );
  BOOST_CHECK_EQUAL(Pathname("/./..").asString(),	"/" );
  BOOST_CHECK_EQUAL(Pathname("/../.").asString(),	"/" );
  BOOST_CHECK_EQUAL(Pathname("/../..").asString(),	"/" );
  BOOST_CHECK_EQUAL(Pathname("/././").asString(),	"/" );
  BOOST_CHECK_EQUAL(Pathname("/./../").asString(),	"/" );
  BOOST_CHECK_EQUAL(Pathname("/.././").asString(),	"/" );
  BOOST_CHECK_EQUAL(Pathname("/../../").asString(),	"/" );

  BOOST_CHECK_EQUAL(Pathname("a\\b").asString(),	"./a\\b" );
  BOOST_CHECK_EQUAL(Pathname("a/b").asString(),		"./a/b" );
  BOOST_CHECK_EQUAL(Pathname("c:a\\b").asString(),	"./c:a\\b" );
  BOOST_CHECK_EQUAL(Pathname("c:a/b").asString(),	"./c:a/b" );
  BOOST_CHECK_EQUAL(Pathname("cc:a\\b").asString(),	"./cc:a\\b" );
  BOOST_CHECK_EQUAL(Pathname("cc:a/b").asString(),	"./cc:a/b" );
}

