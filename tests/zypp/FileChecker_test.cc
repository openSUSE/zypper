
#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/KeyRing.h"
#include "zypp/PublicKey.h"
#include "zypp/TmpPath.h"

#include "zypp/FileChecker.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "KeyRingTestReceiver.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using namespace boost::unit_test::log;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;

void keyring_test( const string &dir )
{
  Pathname file( Pathname(dir) + "hello.txt" );
  Pathname file2( Pathname(dir) + "hello2.txt" );
  Pathname pubkey( Pathname(dir) + "hello.txt.key" );
  Pathname signature( Pathname(dir) + "hello.txt.asc" );
  
  /**
   * 1st scenario, the signature does
   * match
   */
  {
    KeyRingTestReceiver keyring_callbacks;
    KeyRingTestSignalReceiver receiver;
    
    keyring_callbacks.answerTrustKey(true);
    SignatureFileChecker sigchecker( signature );
    sigchecker.addPublicKey(pubkey);
    sigchecker(file); 
  }
  
  /**
   * second scenario, the signature does not
   * match, an exception has to be thrown
   */
  {
    KeyRingTestReceiver keyring_callbacks;
    KeyRingTestSignalReceiver receiver;
    
    keyring_callbacks.answerTrustKey(true);
    SignatureFileChecker sigchecker( signature );
    sigchecker.addPublicKey(pubkey);
    
    BOOST_CHECK_THROW( sigchecker(file2), zypp::Exception );

  }
  
}

void checksum_test( const string &dir )
{
  Pathname file( Pathname(dir) + "hello.txt" );
  Pathname file2( Pathname(dir) + "hello2.txt" );
  Pathname pubkey( Pathname(dir) + "hello.txt.key" );
  Pathname signature( Pathname(dir) + "hello.txt.asc" );
  
  /**
   * 1st scenario, checksum matches
   */
  {
    ChecksumFileChecker checker( CheckSum("sha1", "f2105202a0f017ab818b670d04982a89f55f090b") );
    checker(file);
  }
  
  /**
   * 1st scenario, checksum does not matches
   */
  {
    ChecksumFileChecker checker( CheckSum("sha1", "f2105202a0f017ab818b670d04982a89f55f090b") );
    BOOST_CHECK_THROW( checker(file2), zypp::FileCheckException );
  }
}

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  string datadir;
  if (argc < 2)
  {
    datadir = TESTS_SRC_DIR;
    datadir = (Pathname(datadir) + "/zypp/data/FileChecker").asString();
    cout << "filechecker_test:"
      " path to directory with test data required as parameter. Using " << datadir  << endl;
    //return (test_suite *)0;
  }
  else
  {
    datadir = argv[1];
  }

  std::string const params[] = { datadir };
    //set_log_stream( std::cout );
  test_suite* test= BOOST_TEST_SUITE( "FileCheckerTest" );
  test->add(BOOST_PARAM_TEST_CASE( &keyring_test,
                              (std::string const*)params, params+1));
  test->add(BOOST_PARAM_TEST_CASE( &checksum_test,
                              (std::string const*)params, params+1));
  return test;
}

