#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <string>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/TmpPath.h"
#include "zypp/PathInfo.h"
#include "zypp/RepoManager.h"
#include "zypp/sat/Pool.h"
#include "zypp/repo/DeltaCandidates.h"
#include "zypp/repo/PackageDelta.h"
#include "KeyRingTestReceiver.h"

using boost::unit_test::test_case;

using namespace std;
using namespace zypp;
using namespace zypp::repo;
using namespace zypp::filesystem;

#define TEST_DIR TESTS_SRC_DIR "/zypp/data/Delta"

BOOST_AUTO_TEST_CASE(delta)
{
  KeyRing::setDefaultAccept( KeyRing::ACCEPT_UNKNOWNKEY | KeyRing::ACCEPT_UNSIGNED_FILE );

  TmpDir rootdir;
  RepoManager rm( RepoManagerOptions::makeTestSetup( rootdir ) );

  RepoInfo updates;
  updates.setAlias("updates");
  updates.addBaseUrl( Pathname(TEST_DIR).asUrl() );

  try
  {
    rm.buildCache(updates);
    rm.loadFromCache(updates);
  }
  catch (const Exception & e)
  {
    BOOST_FAIL( string("Problem getting the data: ")+ e.msg()) ;
  }
  sat::Pool pool(sat::Pool::instance());

  repo::DeltaCandidates dc(list<Repository>(pool.reposBegin(),pool.reposEnd()), "libzypp");

  std::list<packagedelta::DeltaRpm> deltas = dc.deltaRpms(0);
  for_ (it,deltas.begin(),deltas.end())
  {
    BOOST_CHECK(it->name() == "libzypp");
    BOOST_CHECK(it->edition() == Edition("4.21.3-2"));
    BOOST_CHECK(it->arch() == "i386");
    BOOST_CHECK(it->baseversion().edition().match(Edition("4.21.3-1"))
      ||it->baseversion().edition().match(Edition("4.21.2-3")));

    cout << it->name() << " - " << it->edition() << " - " <<  it->arch()
      << " base: " << it->baseversion().edition() << endl;

    cout << (it->edition() == "4.21.3-2") << endl;              // fine
    cout << (it->edition() == Edition("4.21.3-2")) << endl;     // fine
    cout << (it->edition().match(Edition("4.21.3-2")) == 0) << endl; // match returns -1,0,1
    cout << (it->edition().match("4.21.3-2") == 0) << endl;          // match returns -1,0,1
  }
}
