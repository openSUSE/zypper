#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

#include <string>
#include <list>
#include <iostream>
#include <unistd.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>
#include "mymediaverifier.h"

using namespace zypp;
using namespace zypp::media;

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

BOOST_AUTO_TEST_CASE(curl_params_reset)
{
  MediaManager     mm;
  media::MediaId   id;
  
  Url url("http://ftp.kernel.org/pub/");
  
  id = mm.open( url, "");
  mm.attach(id);

  Pathname dest;
  Pathname src("/README");
  mm.doesFileExist(id, src);
  mm.provideFile(id, src);
  dest = mm.localPath(id, src);
  BOOST_REQUIRE( PathInfo(dest).size() != 0 );
  mm.provideFile(id, src);
  dest = mm.localPath(id, src);
  BOOST_REQUIRE( PathInfo(dest).size() != 0 );
  mm.doesFileExist(id, src);
  BOOST_REQUIRE( PathInfo(dest).size() != 0 );
  mm.release(id);   
}

BOOST_AUTO_TEST_CASE(http_test)
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

BOOST_AUTO_TEST_CASE(ftp_test)
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

BOOST_AUTO_TEST_CASE(isotest)
{
   if ( geteuid() != 0 )
   {
     BOOST_WARN( "ISO test requires root permissions! (mount)");
     return;
   }
  
  MediaManager     mm;
  media::MediaId   id;
  
  //Url url("nfs://dist.suse.de/dist/install/openSUSE-10.2-GM/");
  Url url("dir:/mounts/dist/install/openSUSE-10.2-GM/");
  
  Url iso_url("iso:/");
  iso_url.setQueryParam("iso", "openSUSE-10.2-RC5-PromoDVD-i386.iso");
  iso_url.setQueryParam("url", url.asString());

  id = mm.open( iso_url, "");
  mm.attach(id);
  BOOST_REQUIRE( mm.doesFileExist(id, Pathname("/README")) );
  BOOST_REQUIRE( ! mm.doesFileExist(id, Pathname("/fakefile")) );
  mm.release(id); 
}

BOOST_AUTO_TEST_CASE(nfs_tst)
{
   if ( geteuid() != 0 )
   {
     BOOST_WARN( "NFS test requires root permissions! (mount)");
     return;
   }
  
  MediaManager     mm;
  media::MediaId   id;
  Url url("nfs://dist.suse.de/dist/install");
  
  id = mm.open( url, "");
  mm.attach(id);
  BOOST_REQUIRE( mm.doesFileExist(id, Pathname("/SLP/openSUSE-10.2-RM/i386/DVD1/README")) );
  BOOST_REQUIRE( ! mm.doesFileExist(id, Pathname("/fakefile")) );
  mm.release(id);
  
}

// vim: set ts=2 sts=2 sw=2 ai et:
