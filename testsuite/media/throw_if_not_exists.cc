#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

#include <string>
#include <list>
#include <iostream>
#include <unistd.h>

#include <boost/test/unit_test.hpp>

#include "mymediaverifier.h"

using namespace zypp;
using namespace zypp::media;

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

void http_test()
{
  //MediaVerifierRef verifier( new MyMediaVerifier() );
  MediaManager     mm;
  media::MediaId   id;
  
  Url url("http://www.google.com");
  
  id = mm.open( url, "");
  //mm.addVerifier( id, verifier);
  mm.attach(id);
  BOOST_CHECK_THROW( mm.provideFile(id, Pathname("/file-not-exists")), Exception );
  mm.release(id); 
}

test_suite*
init_unit_test_suite( int, char* [] )
{
  test_suite* test= BOOST_TEST_SUITE( "throw_if_not_exists" );
  //test->add( BOOST_TEST_CASE( &nfs_test ), 0 /* expected zero error */ );
  test->add( BOOST_TEST_CASE( &http_test ), 0 /* expected zero error */ );
  //test->add( BOOST_TEST_CASE( &ftp_test ), 0 /* expected zero error */ );
  //test->add( BOOST_TEST_CASE( &iso_test ), 0 /* expected zero error */ );
  //test->add( BOOST_TEST_CASE( &nfs_test ), 0 /* expected zero error */ );
  return test;
}



// vim: set ts=2 sts=2 sw=2 ai et:
