#include <iostream>
#include <boost/test/auto_unit_test.hpp>
#include <set>

#include "zypp/Url.h"
#include "zypp/PathInfo.h"
#include "zypp/base/Easy.h"
#include "zypp/media/MediaUserAuth.h"

#include "zypp/media/CredentialFileReader.h"

using namespace std;
using namespace zypp;
using namespace zypp::media;

typedef std::set<AuthData_Ptr> CredentialSet;

struct CredCollector
{
  bool collect(AuthData_Ptr & cred)
  {
    cout << "got: " << endl << *cred << endl;
    creds.insert(cred);
    return true;
  }

  CredentialSet creds;
};

BOOST_AUTO_TEST_CASE(read_cred)
{
  CredCollector collector;
  Pathname credfile = TESTS_SRC_DIR "/media/data/credentials.cat";
  CredentialFileReader reader(credfile,
      bind( &CredCollector::collect, &collector, _1 ));

  BOOST_CHECK_EQUAL(collector.creds.size(), 3);
}
