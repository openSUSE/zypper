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

BOOST_AUTO_TEST_CASE(http_test)
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

// vim: set ts=2 sts=2 sw=2 ai et:
