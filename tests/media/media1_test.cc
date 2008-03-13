#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

#include <string>
#include <list>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>

#include "mymediaverifier.h"

using namespace zypp;
using namespace zypp::media;

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

BOOST_AUTO_TEST_CASE(verifier_test)
{
  MediaVerifierRef verifier(
      new MyMediaVerifier(/* "SUSE-Linux-CORE-i386 9" */)
                           );
  MediaManager     mm;
  media::MediaId   id;

  //id = mm.open(zypp::Url("cd:/"), "");
  id = mm.open(zypp::Url("ftp://machcd2/CDs/SLES-10-ISSLE-Beta1a-ppc/CD1"), "");
  mm.addVerifier( id, verifier);
  mm.attach(id);
  mm.provideFile(id, Pathname("/suse/setup/descr/EXTRA_PROV"));
  mm.release(id);
  mm.attach(id);
  mm.provideFile(id, Pathname("/suse/setup/descr/EXTRA_PROV"));  
}

// vim: set ts=2 sts=2 sw=2 ai et:
