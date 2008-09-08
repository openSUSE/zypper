#include <iostream>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/Url.h"

#include "zypp/media/CredentialManager.h"

using std::cout;
using std::endl;
using namespace zypp;
using namespace zypp::media;


BOOST_AUTO_TEST_CASE(read_cred_for_url)
{
  CredManagerOptions opts;
  opts.globalCredFilePath = TESTS_SRC_DIR "/media/data/credentials";
  opts.userCredFilePath = Pathname();

  CredentialManager cm(opts);
  Url url("https://drink.it/repo/roots");

  AuthData_Ptr credentials = cm.getCred(url);

/*
  cout << "credentials:";
  if (credentials)
    cout << *credentials;
  else
    cout << "(null)";
  cout << endl;
*/

  BOOST_CHECK(credentials->username() == "ginger");
  BOOST_CHECK(credentials->password() == "ale");
  
  Url url2("ftp://maria@weprovidesoft.fr/download/opensuse/110");
  credentials = cm.getCred(url2);

  BOOST_CHECK(credentials->username() == "maria");
  BOOST_CHECK(credentials->password() == "antoin");
}
