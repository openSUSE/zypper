
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

#include <boost/test/auto_unit_test.hpp>

#include "KeyRingTestReceiver.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "/zypp/data/FileChecker")

BOOST_AUTO_TEST_CASE(keyring_test)
{
  Pathname file( Pathname(DATADIR) + "hello.txt" );
  Pathname file2( Pathname(DATADIR) + "hello2.txt" );
  Pathname pubkey( Pathname(DATADIR) + "hello.txt.key" );
  Pathname signature( Pathname(DATADIR) + "hello.txt.asc" );
  
  /**
   * 1st scenario, the signature does
   * match
   */
  {
    KeyRingTestReceiver keyring_callbacks;
    KeyRingTestSignalReceiver receiver;
    
    keyring_callbacks.answerAcceptKey(KeyRingReport::KEY_TRUST_TEMPORARILY);
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
    
    keyring_callbacks.answerAcceptKey(KeyRingReport::KEY_TRUST_TEMPORARILY);
    SignatureFileChecker sigchecker( signature );
    sigchecker.addPublicKey(pubkey);
    
    BOOST_CHECK_THROW( sigchecker(file2), zypp::Exception );

  }
  
}

BOOST_AUTO_TEST_CASE(checksum_test)
{
  Pathname file( Pathname(DATADIR) + "hello.txt" );
  Pathname file2( Pathname(DATADIR) + "hello2.txt" );
  Pathname pubkey( Pathname(DATADIR) + "hello.txt.key" );
  Pathname signature( Pathname(DATADIR) + "hello.txt.asc" );
  
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

