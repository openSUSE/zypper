#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

#include <string>
#include <list>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include "mymediaverifier.h"

using namespace zypp;
using namespace zypp::media;

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

void verifier_test()
{
  MediaVerifierRef verifier(
      new MyMediaVerifier(/* "SUSE-Linux-CORE-i386 9" */)
                           );
  MediaManager     mm;
  media::MediaId   id;

  //id = mm.open(zypp::Url("cd:/"), "");
  id = mm.open(zypp::Url("ftp://machcd2/CDs/suse102-cd-oss-i386/CD1"), "");
  mm.addVerifier( id, verifier);
  mm.attach(id);
  mm.provideFile(id, Pathname("/suse/setup/descr/EXTRA_PROV"));
  mm.release(id);
  mm.attach(id);
  mm.provideFile(id, Pathname("/suse/setup/descr/EXTRA_PROV"));  
}

test_suite*
init_unit_test_suite( int, char* [] )
{
  test_suite* test= BOOST_TEST_SUITE( "MediaVerifierTest" );
  test->add( BOOST_TEST_CASE( &verifier_test ), 0 /* expected zero error */ );
  return test;
}



// vim: set ts=2 sts=2 sw=2 ai et:
