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
  
  Url url("http://ftp.kernel.org/pub/");
  
//   iso_url = "iso:/";
//   iso_url.setQueryParam("iso", "SUSE-10.1-Beta5/SUSE-Linux-10.1-beta5-i386-CD1.iso");
//   iso_url.setQueryParam("url", src_url.asString());

  id = mm.open( url, "");
  //mm.addVerifier( id, verifier);
  mm.attach(id);
  BOOST_REQUIRE( mm.doesFileExist(id, Pathname("/README")) );
  BOOST_REQUIRE( ! mm.doesFileExist(id, Pathname("/fakefile")) );
  mm.release(id); 
}

void ftp_test()
{
  //MediaVerifierRef verifier( new MyMediaVerifier() );
  MediaManager     mm;
  media::MediaId   id;
  
  Url url("ftp://ftp.kernel.org/pub/");
  
//   iso_url = "iso:/";
//   iso_url.setQueryParam("iso", "SUSE-10.1-Beta5/SUSE-Linux-10.1-beta5-i386-CD1.iso");
//   iso_url.setQueryParam("url", src_url.asString());

  id = mm.open( url, "");
  //mm.addVerifier( id, verifier);
  mm.attach(id);
  BOOST_REQUIRE( mm.doesFileExist(id, Pathname("/README")) );
  BOOST_REQUIRE( ! mm.doesFileExist(id, Pathname("/fakefile")) );
  mm.release(id); 
}

void iso_test()
{
   if ( geteuid() != 0 )
   {
     BOOST_ERROR( "ISO test requires root permissions! (mount)");
     return;
   }
  
  MediaManager     mm;
  media::MediaId   id;
  
  //Url url("nfs://dist.suse.de/dist/install");
  Url url("dir:/mounts/dist/install/SUSE-10.1-RC5/");
  
  Url iso_url("iso:/");
  iso_url.setQueryParam("iso", "SUSE-Linux-10.1-RC5-i386-CD1.iso");
  iso_url.setQueryParam("url", url.asString());

  id = mm.open( iso_url, "");
  mm.attach(id);
  BOOST_REQUIRE( mm.doesFileExist(id, Pathname("/README")) );
  BOOST_REQUIRE( ! mm.doesFileExist(id, Pathname("/fakefile")) );
  mm.release(id); 
}

void nfs_test()
{
   if ( geteuid() != 0 )
   {
     BOOST_ERROR( "NFS test requires root permissions! (mount)");
     return;
   }
  
  MediaManager     mm;
  media::MediaId   id;
  Url url("nfs://dist.suse.de/dist/install");
  
  id = mm.open( url, "");
  mm.attach(id);
  BOOST_REQUIRE( mm.doesFileExist(id, Pathname("/SLP/SUSE-10.1-RC5/i386/CD1/README")) );
  BOOST_REQUIRE( ! mm.doesFileExist(id, Pathname("/fakefile")) );
  mm.release(id);
  
}

test_suite*
init_unit_test_suite( int, char* [] )
{
  test_suite* test= BOOST_TEST_SUITE( "MediaFileExistTest" );
  //test->add( BOOST_TEST_CASE( &nfs_test ), 0 /* expected zero error */ );
  test->add( BOOST_TEST_CASE( &http_test ), 0 /* expected zero error */ );
  test->add( BOOST_TEST_CASE( &ftp_test ), 0 /* expected zero error */ );
  test->add( BOOST_TEST_CASE( &iso_test ), 0 /* expected zero error */ );
  test->add( BOOST_TEST_CASE( &nfs_test ), 0 /* expected zero error */ );
  return test;
}



// vim: set ts=2 sts=2 sw=2 ai et:
