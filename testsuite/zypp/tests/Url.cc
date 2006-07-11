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

void test_url(void)
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

    // asString shouldn't print the password, asCompleteString should
    // further, the "//" at the begin of the path should be keept.
    str = "http://user:pass@localhost//srv/ftp";
    one = "http://user@localhost//srv/ftp";
    two = str;
    url = str;

    BOOST_CHECK_EQUAL( one, url.asString() );
    BOOST_CHECK_EQUAL( two, url.asCompleteString() );
}

void test_url1()
{
  std::string str, out;

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
}

void test_url2(void)
{
  std::string str;

  str = "http://user:pass@localhost:/path/to;version=1.1?arg=val#frag";
  std::cout << "STR:  " << str << std::endl << std::endl;

  zypp::Url   url;
  url = str;

  str = url.asString();
  std::cout << "URL1: " << str << std::endl << std::endl;

  str = url.asString(zypp::url::ViewOptions() +
                     zypp::url::ViewOptions::WITH_PASSWORD);
  std::cout << "URL2: " << str << std::endl << std::endl;

  str = url.asString(zypp::url::ViewOptions() +
                     zypp::url::ViewOptions::WITH_PATH_PARAMS);
  std::cout << "URL3: " << str << std::endl << std::endl;

  str = url.asCompleteString();
  std::cout << "URL4: " << str << std::endl << std::endl;
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
  std::cout << "ADD1: '" << key << "' = '" << val << "'" << std::endl;
  std::cout << "URL1: " << url.asString() << std::endl << std::endl;

  // will be encoded as "foo%3Dbar%26key=foo%26bar%3Dvalue"
  key = "foo=bar&key";
  val = "foo&bar=value";
  url.setQueryParam(key, val);
  std::cout << "ADD2: '" << key << "' = '" << val << "'" << std::endl;
  std::cout << "URL2: " << url.asString() << std::endl << std::endl;

  // will be encoded as "foo%25bar=is%25de%25ad"
  key = "foo%bar";
  val = "is%de%ad";
  url.setQueryParam(key, val);
  std::cout << "ADD3: '" << key << "' = '" << val << "'" << std::endl;
  std::cout << "URL3: " << url.asString() << std::endl << std::endl;

  // get encoded query parameters:
  std::cout << "QUERY PARAMS:" << std::endl;
  zypp::url::ParamVec params( url.getQueryStringVec());
  for( size_t i=0; i<params.size(); i++)
  {
    std::cout << "\t" << params[i] << std::endl;
  }
  std::cout << std::endl;
}

void test_url4()
{
  try
  {
    std::string str;
    zypp::Url   url;

    str = "ldap://example.net/dc=example,dc=net?cn,sn?sub?(cn=*)";
    std::cout << "STR: " << str << std::endl;

    url = str;
    std::cout << "URL: " << url.asString() << std::endl;

    zypp::url::ParamVec pvec( url.getQueryStringVec());
    zypp::url::ParamVec::const_iterator v;
    for(v=pvec.begin(); v!=pvec.end(); ++v)
    {
      std::cout << "PARAM: " << *v << std::endl;
    }
    std::cout << std::endl;

    zypp::url::ParamMap pmap( url.getQueryStringMap());
    zypp::url::ParamMap::const_iterator m;
    for(m=pmap.begin(); m!=pmap.end(); ++m)
    {
      std::cout << "KEY: " << m->first  << std::endl;
      std::cout << "VAL: " << m->second << std::endl;
      std::cout << std::endl;
    }

    url.setQueryParam("attrs", "cn,sn,uid");
    std::cout << "OUT: " << url.asString() << std::endl;
  }
  catch(const zypp::url::UrlException &e)
  {
    ZYPP_CAUGHT(e);
  }
}

void test_url5()
{
  struct Test {
    char *str;
    char *inf;
    int  exp;
  };
  struct Test tests[] = {
    {"ldap:///dc=foo,dc=bar",      "invalid: no host is ok for ldap", 1},
    {"ftp:///foo/bar",             "throws:  host is mandatory",      2},
    {"http:///%2f/srv/ftp",        "throws:  host is mandatory",      2},
    {"file://localhost/some/path", "valid:   host is allowed",        0},
    {"cd://localhost/some/path",   "throws:  host not allowed",       2},
    {"mailto:",                    "throws:  no path (email)",        2},
    {"cd:",                        "throws:  no path",                2},
    {"cd:///some/path",            "valid:   no host, path is there", 0},
    {NULL}
  };

  try
  {
    zypp::Url url;
    for(struct Test *test=tests; test && test->str; test++)
    {
      std::cout << "STR: " << test->str << std::endl;
      std::cout << "INF: " << test->inf << std::endl;
      try
      {
        url = test->str;

        std::cout << "URL: " << url.asString() << std::endl;

        bool valid = url.isValid();
        std::cout << "OK?: " << (valid ? "valid" : "invalid")
                             << std::endl;

        if( valid && test->exp != 0)
            ZYPP_THROW(zypp::Exception("Unexpected result: exp != 0"));
        else
        if( !valid && test->exp != 1)
            ZYPP_THROW(zypp::Exception("Unexpected result: exp == 1"));
      }
      catch(const zypp::url::UrlException &)
      {
        std::cout << "ERR: exception caught" << std::endl;
        if(test->exp != 2)
          ZYPP_THROW(zypp::Exception("Unexpected result exp != 2"));
      }
      std::cout << std::endl;
    }
  }
  catch(const zypp::Exception &e)
  {
    ZYPP_CAUGHT(e);
  }
}

test_suite*
init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "Url" );
    test->add( BOOST_TEST_CASE( &test_url ), 0 /* expected zero error */ );
    test->add( BOOST_TEST_CASE( &test_url2 ), 0 );
    test->add( BOOST_TEST_CASE( &test_url3 ), 0 );
    test->add( BOOST_TEST_CASE( &test_url4 ), 0 );
    test->add( BOOST_TEST_CASE( &test_url5 ), 0 );
    return test;
}


// vim: set ts=2 sts=2 sw=2 ai et:
