
#include <iostream>
#include <fstream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/KeyRing.h"
#include "zypp/PublicKey.h"
#include "zypp/TmpPath.h"

#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using namespace boost::unit_test::log;

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;

/**
 * Keyring Callback Receiver with some features
 * Allows to simulate and configure user answer
 * Can record which callbacks were called
 */
struct KeyRingReceive : public zypp::callback::ReceiveReport<zypp::KeyRingReport>
{
  KeyRingReceive()
  {
    reset();
    connect();
  }

  void reset()
  {
    _answer_accept_unknown_key = false;
    _answer_trust_key = false;
    _answer_import_key = false;
    _answer_ver_failed = false;
    _answer_accept_unsigned_file = false;
    _asked_user_to_accept_unknown_key = false;
    _asked_user_to_trust_key = false;
    _asked_user_to_import_key = false;
    _asked_user_to_accept_ver_failed = false;
    _asked_user_to_accept_unsigned_file = false;
  }
  
  ~KeyRingReceive()
  {
    disconnect();
  }
  
  void answerAcceptVerFailed( bool answer )
  { _answer_ver_failed = answer; }
  
  bool askedAcceptVerFailed() const
  { return _asked_user_to_accept_ver_failed; }
  
  void answerAcceptUnknownKey( bool answer )
  { _answer_accept_unknown_key = answer; }
  
  bool askedAcceptUnknownKey() const
  { return _asked_user_to_accept_unknown_key; }
  
  void answerTrustKey( bool answer )
  { _answer_trust_key = answer; }
  
  bool askedTrustKey() const
  { return _asked_user_to_trust_key; }
  
  void answerImportKey( bool answer )
  { _answer_import_key = answer; }
  
  bool askedImportKey() const
  { return _asked_user_to_import_key; }
  
  void answerAcceptUnsignedFile( bool answer )
  { _answer_accept_unsigned_file = answer; }
  
  bool askedAcceptUnsignedFile() const
  { return _asked_user_to_accept_unsigned_file; }
    
  virtual bool askUserToAcceptUnsignedFile( const std::string &file )
  {
    MIL << std::endl;
    _asked_user_to_accept_unsigned_file = true;
    return _answer_accept_unsigned_file;
  }
  
  virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &id )
  {
    MIL << std::endl;
    _asked_user_to_accept_unknown_key = true;
    return _answer_accept_unknown_key;
  }

  virtual bool askUserToImportKey( const PublicKey &key )
  {
    MIL << std::endl;
    _asked_user_to_import_key = true;
    return _answer_import_key;
  }

  virtual bool askUserToTrustKey(  const PublicKey &key  )
  {
    MIL << std::endl;
    _asked_user_to_trust_key = true;
    return _answer_trust_key;
  }
  virtual bool askUserToAcceptVerificationFailed( const std::string &file,  const PublicKey &key  )
  {
    MIL << std::endl;
    _asked_user_to_accept_ver_failed = true;
    return _answer_ver_failed;
  }
  
  // how to answer
  bool _answer_accept_unknown_key;
  bool _answer_trust_key;
  bool _answer_import_key;
  bool _answer_ver_failed;
  bool _answer_accept_unsigned_file;
  
  // we use this variables to check that the
  // callbacks were called
  bool _asked_user_to_accept_unknown_key;
  bool _asked_user_to_trust_key;
  bool _asked_user_to_import_key;
  bool _asked_user_to_accept_ver_failed;
  bool _asked_user_to_accept_unsigned_file;
};

/**
 * Keyring Signal Receiver with some features
 * Allows to simulate and configure user answer
 * Can record which callbacks were called
 */
struct KeyRingSignalReceiver : callback::ReceiveReport<KeyRingSignals>
{
  KeyRingSignalReceiver(/*RpmDb &rpmdb*/)
  : _trusted_key_added_called(false)
  {
    MIL << "KeyRing signals enabled" << endl;
    connect();
  }

  ~KeyRingSignalReceiver()
  {
    disconnect();
  }

  virtual void trustedKeyAdded( const KeyRing &keyring, const PublicKey &key )
  {
    MIL << "TEST: trusted key added to zypp Keyring. Syncronizing keys with fake rpm keyring" << std::endl;
    _trusted_key_added_called = true;
    //std::cout << "trusted key added to zypp Keyring. Syncronizing keys with rpm keyring" << std::endl;
    //_rpmdb.importZyppKeyRingTrustedKeys();
    //_rpmdb.exportTrustedKeysInZyppKeyRing();
  }

  virtual void trustedKeyRemoved( const KeyRing &keyring, const PublicKey &key  )
  {
  }
  
  bool _trusted_key_added_called;
  
};

void keyring_test( const string &dir )
{
  PublicKey key( Pathname(dir) + "public.asc" );
  
 /** 
  * scenario #1
  * import a not trusted key
  * ask for trust, answer yes
  * ask for import, answer no
  */
  {
    KeyRingReceive keyring_callbacks;
    KeyRingSignalReceiver receiver;
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
    
    keyring_callbacks.answerTrustKey(true);
    bool to_continue = keyring.verifyFileSignatureWorkflow( Pathname(dir) + "repomd.xml", "Blah Blah", Pathname(dir) + "repomd.xml.asc");
  
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnknownKey(), "Should not ask for unknown key, it was known");
    BOOST_CHECK_MESSAGE( keyring_callbacks.askedTrustKey(), "Verify Signature Workflow with only 1 untrusted key should ask user wether to trust");
    BOOST_CHECK_MESSAGE( keyring_callbacks.askedImportKey(), "Trusting a key should ask for import");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptVerFailed(), "The signature validates");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnsignedFile(), "It is a signed file, so dont ask the opposite");
    
    BOOST_CHECK_MESSAGE( to_continue, "We did not import, but we trusted and signature validates.");
  }
  
  /** 
  * scenario #1.1
  * import a not trusted key
  * ask for trust, answer yes
  * ask for import, answer no
  * vorrupt the file and check
  */
  {
    KeyRingReceive keyring_callbacks;
    KeyRingSignalReceiver receiver;
    // base sandbox for playing
    TmpDir tmp_dir;
    KeyRing keyring( tmp_dir.path() );
    
    BOOST_CHECK_EQUAL( keyring.publicKeys().size(), (unsigned) 0 );
    BOOST_CHECK_EQUAL( keyring.trustedPublicKeys().size(), (unsigned) 0 );
  
    keyring.importKey( key, false );
    
    keyring_callbacks.answerTrustKey(true);
    
    // now we will recheck with a corrupted file
    bool to_continue = keyring.verifyFileSignatureWorkflow( Pathname(dir) + "repomd.xml.corrupted", "Blah Blah", Pathname(dir) + "repomd.xml.asc");
    
    // check wether the user got the right questions
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnknownKey(), "Should not ask for unknown key, it was known");
    BOOST_CHECK_MESSAGE( keyring_callbacks.askedTrustKey(), "Verify Signature Workflow with only 1 untrusted key should ask user wether to trust");
    BOOST_CHECK_MESSAGE( keyring_callbacks.askedImportKey(), "Trusting a key should ask for import");
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
    KeyRingReceive keyring_callbacks;
    KeyRingSignalReceiver receiver;
    // base sandbox for playing
    TmpDir tmp_dir;
    KeyRing keyring( tmp_dir.path() );
    
    keyring.importKey( key, false );
    
    keyring_callbacks.answerTrustKey(true);
    // now we will recheck with a unsigned file
    bool to_continue = keyring.verifyFileSignatureWorkflow( Pathname(dir) + "repomd.xml", "Blah Blah", Pathname() );
    
    // check wether the user got the right questions
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnknownKey(), "Should not ask for unknown key, it was known");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedTrustKey(), "No signature, no key to trust");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedImportKey(), "No signature, no key to import");
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
    KeyRingReceive keyring_callbacks;
    KeyRingSignalReceiver receiver;
    // base sandbox for playing
    TmpDir tmp_dir;
    KeyRing keyring( tmp_dir.path() );
    
    BOOST_CHECK_MESSAGE( ! keyring.isKeyKnown( key.id() ), "empty keyring has not known keys");
    
    //keyring_callbacks.answerAcceptUnknownKey(true);
    bool to_continue = keyring.verifyFileSignatureWorkflow( Pathname(dir) + "repomd.xml", "Blah Blah", Pathname(dir) + "repomd.xml.asc");
    BOOST_CHECK_MESSAGE(keyring_callbacks.askedAcceptUnknownKey(), "Should ask to accept unknown key, empty keyring");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedTrustKey(), "Unknown key cant be trusted");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedImportKey(), "Unknown key cant be imported");
    
    BOOST_CHECK_MESSAGE( ! to_continue, "We answered no to accept unknown key");
  }
  
  /** scenario #3
  * import trusted key
  * should ask nothing
  * should emit signal
  */
  {
    KeyRingReceive keyring_callbacks;
    KeyRingSignalReceiver receiver;
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
    
    bool to_continue = keyring.verifyFileSignatureWorkflow( Pathname(dir) + "repomd.xml", "Blah Blah", Pathname(dir) + "repomd.xml.asc");
  
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnknownKey(), "Should not ask for unknown key, it was known");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedTrustKey(), "Verify Signature Workflow with only 1 untrusted key should ask user wether to trust");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedImportKey(), "Trusting a key should ask for import");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptVerFailed(), "The signature validates");
    BOOST_CHECK_MESSAGE( ! keyring_callbacks.askedAcceptUnsignedFile(), "It is a signed file, so dont ask the opposite");
    
    BOOST_CHECK_MESSAGE( to_continue, "We did not import, but we trusted and signature validates.");
  }
  //keyring.importKey( key, true );
  //BOOST_CHECK_EQUAL( receiver._trusted_key_added_called, true );
  //BOOST_CHECK_EQUAL( keyring.trustedPublicKeys().size(), 1 );
}

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  string datadir;
  if (argc < 2)
  {
    datadir = TESTS_SRC_DIR;
    datadir = (Pathname(datadir) + "/zypp/data/KeyRing").asString();
    cout << "keyring_test:"
      " path to directory with test data required as parameter. Using " << datadir  << endl;
    //return (test_suite *)0;
  }
  else
  {
    datadir = argv[1];
  }

  std::string const params[] = { datadir };
    //set_log_stream( std::cout );
  test_suite* test= BOOST_TEST_SUITE( "PublicKeyTest" );
  test->add(BOOST_PARAM_TEST_CASE( &keyring_test,
                              (std::string const*)params, params+1));
  return test;
}

