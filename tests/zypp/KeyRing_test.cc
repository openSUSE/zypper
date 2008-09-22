
#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/KeyRing.h"
#include "zypp/PublicKey.h"
#include "zypp/TmpPath.h"

#include <boost/test/auto_unit_test.hpp>

#include "KeyRingTestReceiver.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;

#define DATADIR (Pathname(TESTS_SRC_DIR) +  "/zypp/data/KeyRing")

BOOST_AUTO_TEST_CASE(keyring_test)
{
  PublicKey key( Pathname(DATADIR) + "public.asc" );

 /** 
  * scenario #1
  * import a not trusted key
  * ask for accept, answer yes 'temporarily'
  */
  {
    KeyRingTestReceiver keyring_callbacks;
    KeyRingTestSignalReceiver receiver;
    // base sandbox for playing
    TmpDir tmp_dir;
    KeyRing keyring( tmp_dir.path() );

    BOOST_CHECK_EQUAL( keyring.publicKeys().size(), (unsigned) 0 );
    BOOST_CHECK_EQUAL( keyring.trustedPublicKeys().size(), (unsigned) 0 );
  
    keyring.importKey( key, false );
    
    BOOST_CHECK_EQUAL( keyring.publicKeys().size(), (unsigned) 1 );
    BOOST_CHECK_EQUAL( keyring.trustedPublicKeys().size(), (unsigned) 0 );
    
    BOOST_CHECK_MESSAGE( keyring.isKeyKnown( key.id() ), "Imported untrusted key should be known");
    BOOST_CHECK_MESSAGE( ! keyring.isKeyTrusted( key.id() ), "Imported untrusted key should be untrusted");
    
    keyring_callbacks.answerAcceptKey(KeyRingReport::KEY_TRUST_TEMPORARILY);
    bool to_continue = keyring.verifyFileSignatureWorkflow( DATADIR + "repomd.xml", "Blah Blah", DATADIR + "repomd.xml.asc");
  
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnknownKey(), "Should not ask for unknown key, it was known");
    BOOST_CHECK_MESSAGE( keyring_callbacks.askedAcceptKey(), "Verify Signature Workflow with only 1 untrusted key should ask user wether to trust and/or import");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptVerFailed(), "The signature validates");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnsignedFile(), "It is a signed file, so dont ask the opposite");
    
    BOOST_CHECK_MESSAGE( to_continue, "We did not import, but we trusted and signature validates.");
  }

  /** 
  * scenario #1.1
  * import a not trusted key
  * ask to accept, answer yes 'temporarily'
  * vorrupt the file and check
  */
  {
    KeyRingTestReceiver keyring_callbacks;
    KeyRingTestSignalReceiver receiver;
    // base sandbox for playing
    TmpDir tmp_dir;
    KeyRing keyring( tmp_dir.path() );
    
    BOOST_CHECK_EQUAL( keyring.publicKeys().size(), (unsigned) 0 );
    BOOST_CHECK_EQUAL( keyring.trustedPublicKeys().size(), (unsigned) 0 );
  
    keyring.importKey( key, false );
    
    keyring_callbacks.answerAcceptKey(KeyRingReport::KEY_TRUST_TEMPORARILY);

    // now we will recheck with a corrupted file
    bool to_continue = keyring.verifyFileSignatureWorkflow( DATADIR + "repomd.xml.corrupted", "Blah Blah", DATADIR + "repomd.xml.asc");
    
    // check wether the user got the right questions
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnknownKey(), "Should not ask for unknown key, it was known");
    BOOST_CHECK_MESSAGE( keyring_callbacks.askedAcceptKey(), "Verify Signature Workflow with only 1 untrusted key should ask user wether to trust and/or import");
    BOOST_CHECK_MESSAGE( keyring_callbacks.askedAcceptVerFailed(), "The signature does not validates");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnsignedFile(), "It is a signed file, so dont ask the opposite");

    BOOST_CHECK_MESSAGE( ! to_continue, "We did not continue with a corrupted file");
  }
  
   /** 
  * scenario #1.2
  * import a not trusted key
  * ask for trust, answer yes
  * ask for import, answer no
  * check without signature
  */
  {
    KeyRingTestReceiver keyring_callbacks;
    KeyRingTestSignalReceiver receiver;
    // base sandbox for playing
    TmpDir tmp_dir;
    KeyRing keyring( tmp_dir.path() );
    
    keyring.importKey( key, false );
    
    keyring_callbacks.answerAcceptKey(KeyRingReport::KEY_TRUST_TEMPORARILY);
    // now we will recheck with a unsigned file
    bool to_continue = keyring.verifyFileSignatureWorkflow( DATADIR + "repomd.xml", "Blah Blah", Pathname() );
    
    // check wether the user got the right questions
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnknownKey(), "Should not ask for unknown key, it was known");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptKey(), "No signature, no key to trust");
    BOOST_CHECK_MESSAGE( keyring_callbacks.askedAcceptUnsignedFile(), "Ask the user wether to accept an unsigned file");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptVerFailed(), "There is no signature to verify");
    
    BOOST_CHECK_MESSAGE( ! to_continue, "We did not continue with a unsigned file");
  }
  
 /** scenario #2
  * empty keyring
  * should ask for unknown key
  * answer no
  */
  {
    KeyRingTestReceiver keyring_callbacks;
    KeyRingTestSignalReceiver receiver;
    // base sandbox for playing
    TmpDir tmp_dir;
    KeyRing keyring( tmp_dir.path() );

    BOOST_CHECK_MESSAGE( ! keyring.isKeyKnown( key.id() ), "empty keyring has not known keys");

    //keyring_callbacks.answerAcceptUnknownKey(true);
    bool to_continue = keyring.verifyFileSignatureWorkflow( DATADIR + "repomd.xml", "Blah Blah", DATADIR + "repomd.xml.asc");
    BOOST_CHECK_MESSAGE(keyring_callbacks.askedAcceptUnknownKey(), "Should ask to accept unknown key, empty keyring");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptKey(), "Unknown key cant be trusted");
    BOOST_CHECK_MESSAGE( ! to_continue, "We answered no to accept unknown key");
  }

  /** scenario #3
  * import trusted key
  * should ask nothing
  * should emit signal
  */
  {
    KeyRingTestReceiver keyring_callbacks;
    KeyRingTestSignalReceiver receiver;
    // base sandbox for playing
    TmpDir tmp_dir;
    KeyRing keyring( tmp_dir.path() );
    
    BOOST_CHECK_EQUAL( keyring.publicKeys().size(), (unsigned) 0 );
    BOOST_CHECK_EQUAL( keyring.trustedPublicKeys().size(), (unsigned) 0 );
  
    keyring.importKey( key, true );

    BOOST_CHECK_EQUAL( receiver._trusted_key_added_called, true );
    
    BOOST_CHECK_EQUAL( keyring.publicKeys().size(), (unsigned) 0 );
    BOOST_CHECK_EQUAL( keyring.trustedPublicKeys().size(), (unsigned) 1 );
    
    BOOST_CHECK_MESSAGE( keyring.isKeyKnown( key.id() ), "Imported trusted key should be known");
    BOOST_CHECK_MESSAGE( keyring.isKeyTrusted( key.id() ), "Imported trusted key should be trusted");
    
    bool to_continue = keyring.verifyFileSignatureWorkflow( DATADIR + "repomd.xml", "Blah Blah", DATADIR + "repomd.xml.asc");
  
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnknownKey(), "Should not ask for unknown key, it was known");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptKey(), "Verify Signature Workflow with only 1 untrusted key should ask user wether to trust and/or import");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptVerFailed(), "The signature validates");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnsignedFile(), "It is a signed file, so dont ask the opposite");
    
    BOOST_CHECK_MESSAGE( to_continue, "We did not import, but we trusted and signature validates.");
  }
  //keyring.importKey( key, true );
  //BOOST_CHECK_EQUAL( receiver._trusted_key_added_called, true );
  //BOOST_CHECK_EQUAL( keyring.trustedPublicKeys().size(), 1 );

  /* check signature id can be extracted */
  
}

BOOST_AUTO_TEST_CASE(signature_test)
{
  PublicKey key( DATADIR + "public.asc" );

  {
    KeyRingTestReceiver keyring_callbacks;
    KeyRingTestSignalReceiver receiver;
    // base sandbox for playing
    TmpDir tmp_dir;
    KeyRing keyring( tmp_dir.path() );
    
    BOOST_CHECK_EQUAL( keyring.readSignatureKeyId( DATADIR + "repomd.xml.asc" ), "BD61D89BD98821BE" );
    BOOST_CHECK_THROW( keyring.readSignatureKeyId(Pathname()), Exception );
    TmpFile tmp;
    BOOST_CHECK_EQUAL( keyring.readSignatureKeyId(tmp.path()), "" );

    keyring.importKey(key);

    BOOST_CHECK(keyring.verifyFileSignature( DATADIR + "repomd.xml", DATADIR + "repomd.xml.asc"));
    BOOST_CHECK( ! keyring.verifyFileSignature( DATADIR + "repomd.xml.corrupted", DATADIR + "repomd.xml.asc"));
  }
}


