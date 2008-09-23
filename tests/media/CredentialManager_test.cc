#include <iostream>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/Url.h"
#include "zypp/TmpPath.h"
#include "zypp/media/CredentialFileReader.cc"
#include "zypp/media/CredentialManager.h"

#include "zypp/PathInfo.h"

using std::cout;
using std::endl;
using namespace zypp;
using namespace zypp::media;


BOOST_AUTO_TEST_CASE(read_cred_for_url)
{
  CredManagerOptions opts;
  opts.globalCredFilePath = TESTS_SRC_DIR "/media/data/credentials.cat";
  opts.userCredFilePath = Pathname();

  CredentialManager cm(opts);
  BOOST_CHECK(cm.credsGlobalSize() == 2);

  Url url("https://drink.it/repo/roots");
  AuthData_Ptr credentials = cm.getCred(url);
  BOOST_CHECK(credentials.get() != NULL);
  if (!credentials)
    return;
  BOOST_CHECK(credentials->username() == "ginger");
  BOOST_CHECK(credentials->password() == "ale");

  Url url2("ftp://magda@weprovidesoft.fr/download/opensuse/110");
  credentials = cm.getCred(url2);
  BOOST_CHECK(credentials.get() != NULL);
  if (!credentials)
    return;
  BOOST_CHECK(credentials->username() == "magda");
  BOOST_CHECK(credentials->password() == "richard");
}

struct CredCollector
{
  bool collect(AuthData_Ptr & cred)
  {
    cout << "got: " << endl << *cred << endl;
    creds.insert(cred);
    return true;
  }

  CredentialManager::CredentialSet creds;
};


BOOST_AUTO_TEST_CASE(save_creds)
{
  filesystem::TmpDir tmp;

  CredManagerOptions opts;
  opts.globalCredFilePath = tmp / "fooha";

  CredentialManager cm1(opts);
  AuthData cr1("benson","absolute");
  cr1.setUrl(Url("http://joooha.com"));
  AuthData cr2("pat","vymetheny");
  cr2.setUrl(Url("ftp://filesuck.org"));

  // should create a new file
  cm1.saveInGlobal(cr1);

  CredCollector collector;
  CredentialFileReader reader(opts.globalCredFilePath,
      bind( &CredCollector::collect, &collector, _1 ));
  BOOST_CHECK(collector.creds.size() == 1);

  cout << "----" << endl;
  collector.creds.clear();

  cm1.saveInGlobal(cr2);
  
  CredentialFileReader reader1(opts.globalCredFilePath,
      bind( &CredCollector::collect, &collector, _1 ));
  BOOST_CHECK(collector.creds.size() == 2);
  
  cout << "----" << endl;
  collector.creds.clear();

  // save the same creds again
  cm1.saveInGlobal(cr2);
  CredentialFileReader reader2(opts.globalCredFilePath,
      bind( &CredCollector::collect, &collector, _1 ));
  BOOST_CHECK(collector.creds.size() == 2);

  // todo check created file permissions
}

BOOST_AUTO_TEST_CASE(service_base_url)
{
  filesystem::TmpDir tmp;

  CredManagerOptions opts;
  opts.globalCredFilePath = tmp / "fooha";

  CredentialManager cm1(opts);
  AuthData cr1("benson","absolute");
  cr1.setUrl(Url("http://joooha.com/service/path"));
  cm1.addGlobalCred(cr1);

  AuthData_Ptr creds;
  creds = cm1.getCred(Url("http://joooha.com/service/path/repo/repofoo"));

  BOOST_CHECK(creds.get() != NULL);
  if (!creds)
    return;
  BOOST_CHECK(creds->username() == "benson");

  creds = cm1.getCred(Url("http://benson@joooha.com/service/path/repo/repofoo"));

  BOOST_CHECK(creds.get() != NULL);
  if (!creds)
    return;
  BOOST_CHECK(creds->username() == "benson");

  creds = cm1.getCred(Url("http://nobody@joooha.com/service/path/repo/repofoo"));
  BOOST_CHECK(creds.get() == NULL);
}
