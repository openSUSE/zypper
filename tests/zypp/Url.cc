/*
** Check if the url by scheme repository works, e.g.
** if there are some initialization order problems
** (ViewOption) causing asString to format its string
** differently than configured.
*/

#include "zypp/base/Exception.h"
#include "zypp/Url.h"
#include <stdexcept>
#include <iostream>
#include <cassert>

// Boost.Test
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using boost::test_tools::close_at_tolerance;

using namespace zypp;

void test_url1(void)
{
    std::string str, one, two;
    zypp::Url   url;


    // asString & asCompleteString should not print "mailto://"
    str = "mailto:feedback@example.com?subject=hello";
    url = str;
    BOOST_CHECK_EQUAL( str, url.asString() );
    BOOST_CHECK_EQUAL( str, url.asCompleteString() );

    // asString & asCompleteString should add empty authority
    // "dvd://...", except we request to avoid it.
    str = "dvd:/srv/ftp";
    one = "dvd:///srv/ftp";
    two = "dvd:///srv/ftp";
    url = str;

    BOOST_CHECK_EQUAL( one, url.asString() );
    BOOST_CHECK_EQUAL( two, url.asCompleteString() );
    BOOST_CHECK_EQUAL( str, url.asString(zypp::url::ViewOptions() -
                                 zypp::url::ViewOption::EMPTY_AUTHORITY));

    // asString shouldn't print the password, asCompleteString should
    // further, the "//" at the begin of the path should become "/%2F"
    str = "ftp://user:pass@localhost//srv/ftp";
    one = "ftp://user@localhost/%2Fsrv/ftp";
    two = "ftp://user:pass@localhost/%2Fsrv/ftp";
    url = str;

    BOOST_CHECK_EQUAL( one, url.asString() );
    BOOST_CHECK_EQUAL( two, url.asCompleteString() );

    // asString shouldn't print the password, asCompleteString should.
    // further, the "//" at the begin of the path should be keept.
    str = "http://user:pass@localhost//srv/ftp";
    one = "http://user@localhost//srv/ftp";
    two = str;
    url = str;

    BOOST_CHECK_EQUAL( one, url.asString() );
    BOOST_CHECK_EQUAL( two, url.asCompleteString() );

    str = "file:./srv/ftp";
    BOOST_CHECK_EQUAL( zypp::Url(str).asString(), str );

    str = "ftp://foo//srv/ftp";
    BOOST_CHECK_EQUAL( zypp::Url(str).asString(), "ftp://foo/%2Fsrv/ftp" );

    str = "FTP://user@local%68ost/%2f/srv/ftp";
    BOOST_CHECK_EQUAL( zypp::Url(str).asString(), "ftp://user@localhost/%2f/srv/ftp" );

    str = "http://[::1]/foo/bar";
    BOOST_CHECK_EQUAL( str, zypp::Url(str).asString() );

    str = "http://:@just-localhost.example.net:8080/";
    BOOST_CHECK_EQUAL( zypp::Url(str).asString(), "http://just-localhost.example.net:8080/" );

    str = "mailto:feedback@example.com?subject=hello";
    BOOST_CHECK_EQUAL( str, zypp::Url(str).asString() );

    str = "nfs://nfs-server/foo/bar/trala";
    BOOST_CHECK_EQUAL( str, zypp::Url(str).asString() );

    str = "ldap://example.net/dc=example,dc=net?cn,sn?sub?(cn=*)#x";
    BOOST_CHECK_THROW( zypp::Url(str).asString(), url::UrlNotAllowedException );
  
    str = "ldap://example.net/dc=example,dc=net?cn,sn?sub?(cn=*)";
    BOOST_CHECK_EQUAL( str, zypp::Url(str).asString() );

    // parseable but invalid, since no host avaliable
    str = "ldap:///dc=foo,dc=bar";
    BOOST_CHECK_EQUAL( str, zypp::Url(str).asString());
    BOOST_CHECK( !zypp::Url(str).isValid());

    // throws:  host is mandatory
    str = "ftp:///foo/bar";
    BOOST_CHECK_THROW(zypp::Url(str).asString(), url::UrlNotAllowedException );

    // throws:  host is mandatory
    str = "http:///%2f/srv/ftp";
    BOOST_CHECK_THROW(zypp::Url(str).asString(), url::UrlNotAllowedException );

    // OK, host allowed in file-url
    str = "file://localhost/some/path";
    BOOST_CHECK_EQUAL( str, zypp::Url(str).asString());

    // throws:  host not allowed
    str = "cd://localhost/some/path";
    BOOST_CHECK_THROW(zypp::Url(str).asString(), url::UrlNotAllowedException );

    // throws: no path (email)
    str = "mailto:";
    BOOST_CHECK_THROW(zypp::Url(str).asString(), url::UrlNotAllowedException );

    // throws:  no path
    str = "cd:";
    BOOST_CHECK_THROW(zypp::Url(str).asString(), url::UrlNotAllowedException );

    // OK, valid (no host, path is there)
    str = "cd:///some/path";
    BOOST_CHECK_EQUAL( str, zypp::Url(str).asString());
    BOOST_CHECK( zypp::Url(str).isValid());
}

void test_url2(void)
{
  zypp::Url url("http://user:pass@localhost:/path/to;version=1.1?arg=val#frag");

  BOOST_CHECK_EQUAL( url.asString(),
  "http://user@localhost/path/to?arg=val#frag" );

  BOOST_CHECK_EQUAL( url.asString(zypp::url::ViewOptions() +
                     zypp::url::ViewOptions::WITH_PASSWORD),
  "http://user:pass@localhost/path/to?arg=val#frag");

  BOOST_CHECK_EQUAL( url.asString(zypp::url::ViewOptions() +
                     zypp::url::ViewOptions::WITH_PATH_PARAMS),
  "http://user@localhost/path/to;version=1.1?arg=val#frag");

  BOOST_CHECK_EQUAL( url.asCompleteString(),
  "http://user:pass@localhost/path/to;version=1.1?arg=val#frag");
}

void test_url3()
{
  zypp::Url   url("http://localhost/path/to#frag");
  std::string key;
  std::string val;

  // will be encoded as "hoho=ha%20ha"
  key = "hoho";
  val = "ha ha";
  url.setQueryParam(key, val);
  BOOST_CHECK_EQUAL( url.asString(),
  "http://localhost/path/to?hoho=ha%20ha#frag");

  // will be encoded as "foo%3Dbar%26key=foo%26bar%3Dvalue"
  key = "foo=bar&key";
  val = "foo&bar=value";
  url.setQueryParam(key, val);
  BOOST_CHECK_EQUAL( url.asString(),
  "http://localhost/path/to?foo%3Dbar%26key=foo%26bar%3Dvalue&hoho=ha%20ha#frag");

  // will be encoded as "foo%25bar=is%25de%25ad"
  key = "foo%bar";
  val = "is%de%ad";
  url.setQueryParam(key, val);
  BOOST_CHECK_EQUAL( url.asString(),
  "http://localhost/path/to?foo%25bar=is%25de%25ad&foo%3Dbar%26key=foo%26bar%3Dvalue&hoho=ha%20ha#frag");

  // get encoded query parameters and compare with results:
  zypp::url::ParamVec params( url.getQueryStringVec());
  const char * const  result[] = {
    "foo%25bar=is%25de%25ad",
    "foo%3Dbar%26key=foo%26bar%3Dvalue",
    "hoho=ha%20ha"
  };
  BOOST_CHECK( params.size() == (sizeof(result)/sizeof(result[0])));
  for( size_t i=0; i<params.size(); i++)
  {
      BOOST_CHECK_EQUAL( params[i], result[i]);
  }
}

void test_url4()
{
  try
  {
    zypp::Url url("ldap://example.net/dc=example,dc=net?cn,sn?sub?(cn=*)");

    // fetch query params as vector
    zypp::url::ParamVec pvec( url.getQueryStringVec());
    BOOST_CHECK( pvec.size() == 3);
    BOOST_CHECK_EQUAL( pvec[0], "cn,sn");
    BOOST_CHECK_EQUAL( pvec[1], "sub");
    BOOST_CHECK_EQUAL( pvec[2], "(cn=*)");

    // fetch the query params map
    // with its special ldap names/keys
    zypp::url::ParamMap pmap( url.getQueryStringMap());
    zypp::url::ParamMap::const_iterator m;
    for(m=pmap.begin(); m!=pmap.end(); ++m)
    {
      if("attrs"  == m->first)
      {
        BOOST_CHECK_EQUAL( m->second, "cn,sn");
      }
      else
      if("filter" == m->first)
      {
        BOOST_CHECK_EQUAL( m->second, "(cn=*)");
      }
      else
      if("scope"  == m->first)
      {
        BOOST_CHECK_EQUAL( m->second, "sub");
      }
      else
      {
        BOOST_FAIL("Unexpected LDAP query parameter name in the map!");
      }
    }

    url.setQueryParam("attrs", "cn,sn,uid");
    url.setQueryParam("filter", "(|(sn=foo)(cn=bar))");

    BOOST_CHECK_EQUAL(url.getQueryParam("attrs"),  "cn,sn,uid");
    BOOST_CHECK_EQUAL(url.getQueryParam("filter"), "(|(sn=foo)(cn=bar))");

  }
  catch(const zypp::url::UrlException &e)
  {
    ZYPP_CAUGHT(e);
  }
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "Url" );
    test->add( BOOST_TEST_CASE( &test_url1 ), 0 /* expected zero error */ );
    test->add( BOOST_TEST_CASE( &test_url2 ), 0 );
    test->add( BOOST_TEST_CASE( &test_url3 ), 0 );
    test->add( BOOST_TEST_CASE( &test_url4 ), 0 );
    return test;
}


// vim: set ts=2 sts=2 sw=2 ai et:
