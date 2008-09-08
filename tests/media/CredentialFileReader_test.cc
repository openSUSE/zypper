#include <iostream>
#include <boost/test/auto_unit_test.hpp>
#include <set>

#include "zypp/Url.h"
#include "zypp/media/MediaUserAuth.h"

#include "zypp/media/CredentialFileReader.h"

using std::cout;
using std::endl;
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
  CredentialFileReader reader(TESTS_SRC_DIR "/media/data/credentials",
    bind( &CredCollector::collect, &collector, _1 ));

  BOOST_CHECK(collector.creds.size() == 2);
}
