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

inline void testGetCreds( CredentialManager & cm_r, const std::string & url_r,
		      const std::string & user_r = "",
		      const std::string & pass_r = "" )
{
  Url url( url_r );
  AuthData_Ptr cred = cm_r.getCred( url );
  //cout << "FOR: " << url << endl;
  //cout << "GOT: " << cred << endl;
  if ( user_r.empty() && pass_r.empty() )
  {
    BOOST_CHECK_EQUAL( cred, AuthData_Ptr() );
  }
  else
  {
    BOOST_CHECK_EQUAL( cred->username(), user_r );
    BOOST_CHECK_EQUAL( cred->password(), pass_r );
  }
}

BOOST_AUTO_TEST_CASE(read_cred_for_url)
{
  CredManagerOptions opts;
  opts.globalCredFilePath = TESTS_SRC_DIR "/media/data/credentials.cat";
  opts.userCredFilePath = Pathname();
  CredentialManager cm( opts );

  BOOST_CHECK_EQUAL( cm.credsGlobalSize(), 3 );

  testGetCreds( cm, "https://drink.it/repo/roots",				"ginger", "ale" );
  testGetCreds( cm, "ftp://weprovidesoft.fr/download/opensuse/110",		"agda", "ichard" );
  testGetCreds( cm, "ftp://magda@weprovidesoft.fr/download/opensuse/110",	"magda", "richard" );
  testGetCreds( cm, "ftp://agda@weprovidesoft.fr/download/opensuse/110",	"agda", "ichard" );
  testGetCreds( cm, "ftp://unknown@weprovidesoft.fr/download/opensuse/110" );	// NULL
  testGetCreds( cm, "http://url.ok/but/not/creds" );				// NULL
}

struct CredCollector
{
  bool collect(AuthData_Ptr & cred)
  {
    //cout << "got: " << endl << *cred << endl;
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
  CredentialFileReader( opts.globalCredFilePath, bind( &CredCollector::collect, &collector, _1 ) );
  BOOST_CHECK_EQUAL( collector.creds.size(), 1 );

  collector.creds.clear();
  cm1.saveInGlobal(cr2);
  CredentialFileReader( opts.globalCredFilePath, bind( &CredCollector::collect, &collector, _1 ) );
  BOOST_CHECK_EQUAL(collector.creds.size(), 2 );
  
  collector.creds.clear();
  // save the same creds again
  cm1.saveInGlobal(cr2);
  CredentialFileReader( opts.globalCredFilePath, bind( &CredCollector::collect, &collector, _1 ) );
  BOOST_CHECK_EQUAL(collector.creds.size(), 2 );

  // todo check created file permissions
}

BOOST_AUTO_TEST_CASE(service_base_url)
{
  filesystem::TmpDir tmp;
  CredManagerOptions opts;
  opts.globalCredFilePath = tmp / "fooha";
  CredentialManager cm( opts );

  AuthData cred( "benson","absolute" );
  cred.setUrl( Url( "http://joooha.com/service/path" ) );
  cm.addGlobalCred( cred );

  testGetCreds( cm, "http://joooha.com/service/path/repo/repofoo",		"benson", "absolute" );
  testGetCreds( cm, "http://benson@joooha.com/service/path/repo/repofoo",	"benson", "absolute" );
  testGetCreds( cm, "http://nobody@joooha.com/service/path/repo/repofoo" );	// NULL
}
