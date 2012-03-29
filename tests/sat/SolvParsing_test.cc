#include <stdio.h>
#include <iostream>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/TmpPath.h"
#include "zypp/RepoManager.h"
#include "zypp/base/Easy.h"
#include "zypp/ZYppFactory.h"
#include "zypp/Package.h"
#include "zypp/sat/Solvable.h"

// allows us to control signature
// callbacks
#include "KeyRingTestReceiver.h"

#define BOOST_TEST_MODULE ResObject

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;


/*
 * this test test that the attributes
 * from the metadata are preserved into
 * the final object
 *
 * so the test covers both libsolv-tools
 * right insertion and parsing
 * and libzypp ResObject and friends data
 * extraction from solv files
 */


// init the solv
static void init_pool_yum()
{
  Pathname dir(TESTS_SRC_DIR);
  dir += "/repo/yum/data/10.2-updates-subset";

  ZYpp::Ptr z = getZYpp();
  ZConfig::instance().setSystemArchitecture(Arch("i586"));

  filesystem::TmpDir tmp;
  RepoManagerOptions opts = RepoManagerOptions::makeTestSetup(tmp.path());
  RepoManager mgr(opts);

  KeyRingTestReceiver keyring_callbacks;
  KeyRingTestSignalReceiver receiver;

  // disable sgnature checking
  keyring_callbacks.answerAcceptKey(KeyRingReport::KEY_TRUST_TEMPORARILY);
  keyring_callbacks.answerAcceptVerFailed(true);
  keyring_callbacks.answerAcceptUnknownKey(true);

  RepoInfo info;
  info.setAlias("updates");
  info.addBaseUrl(dir.asUrl());
  mgr.buildCache(info);
  mgr.loadFromCache(info);
}

BOOST_AUTO_TEST_CASE(attributes)
{
    init_pool_yum();
    MIL << sat::Pool::instance();
    Repository r = sat::Pool::instance().reposFind("updates");

    int c = 0;

    for ( Repository::SolvableIterator it = r.solvablesBegin();
          it != r.solvablesEnd();
          ++it )
    {
        sat::Solvable s = *it;
        //MIL << s.ident() << endl;
        if ( s.ident() == "openssl-devel" )
        {
            c++;
            Package::Ptr p = asKind<Package>(makeResObject(s));
            BOOST_CHECK(p);

            //solvable 5 (6):
            //name: openssl-devel 0.9.8d-17.2 i586
            BOOST_CHECK_EQUAL(p->name(), "openssl-devel");
            //vendor: SUSE LINUX Products GmbH, Nuernberg, Germany
            BOOST_CHECK_EQUAL(p->vendor(), "SUSE LINUX Products GmbH, Nuernberg, Germany");
            //solvable:checksum: 9f6a44015ad97680e9f93d0edefa1d533940479c
            BOOST_CHECK_EQUAL(p->checksum(), CheckSum::sha1("9f6a44015ad97680e9f93d0edefa1d533940479c"));
            //solvable:summary:
            BOOST_CHECK_EQUAL(p->summary(), "Include Files and Libraries mandatory for Development.");
            //solvable:description: This package contains all necessary include files and libraries needed
            //to develop applications that require these.
            BOOST_CHECK_EQUAL(p->description(), "This package contains all necessary include files and libraries needed\nto develop applications that require these.");
            //solvable:authors: Mark J. Cox <mark@openssl.org>
            //Ralf S. Engelschall <rse@openssl.org>
            //Dr. Stephen <Henson steve@openssl.org>
            //Ben Laurie <ben@openssl.org>
            //Bodo Moeller <bodo@openssl.org>
            //Ulf Moeller <ulf@openssl.org>
            //Holger Reif <holger@openssl.org>
            //Paul C. Sutton <paul@openssl.org>
            //solvable:packager: http://bugs.opensuse.org
            //solvable:url: http://www.openssl.org/
            //solvable:buildtime: 1165493634
            //solvable:installsize: 3845
            BOOST_CHECK_EQUAL(p->installSize(), ByteCount(3937193));
            //solvable:downloadsize: 909
            BOOST_CHECK_EQUAL(p->downloadSize(), ByteCount(930588));
            //solvable:mediadir: rpm/i586
            //solvable:mediafile: openssl-devel-0.9.8d-17.2.i586.rpm
            //solvable:license: BSD License and BSD-like, Other License(s), see package
            //solvable:group: Development/Libraries/C and C++
            BOOST_CHECK_EQUAL(p->group(), "Development/Libraries/C and C++");
            //solvable:sourcearch: src
            //solvable:sourceevr: (void)
            //solvable:sourcename: openssl
            //solvable:headerend: 34861

        }
    }

    // check that we actually found all testeable
    // resolvables
    BOOST_CHECK_EQUAL(c, 1);


}
