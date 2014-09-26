
#ifndef _ZYPP_KEYRING_TEST_RECEIVER_H
#define _ZYPP_KEYRING_TEST_RECEIVER_H

#include "zypp/Callback.h"
#include "zypp/KeyRing.h"
#include "zypp/PublicKey.h"
#include "zypp/KeyContext.h"

/**
 * Keyring Callback Receiver with some features
 * Allows to simulate and configure user answer
 * Can record which callbacks were called
 */
struct KeyRingTestReceiver : public zypp::callback::ReceiveReport<zypp::KeyRingReport>
{
  KeyRingTestReceiver()
  {
    reset();
    connect();
  }

  void reset()
  {
    _answer_accept_unknown_key = false;
    _answer_accept_key = KeyRingReport::KEY_DONT_TRUST;
    _answer_ver_failed = false;
    _answer_accept_unsigned_file = false;
    _asked_user_to_accept_unknown_key = false;
    _asked_user_to_accept_key = false;
    _asked_user_to_accept_ver_failed = false;
    _asked_user_to_accept_unsigned_file = false;
  }
  
  ~KeyRingTestReceiver()
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
  
  void answerAcceptKey( KeyRingReport::KeyTrust answer )
  { _answer_accept_key = answer; }
  
  bool askedAcceptKey() const
  { return _asked_user_to_accept_key; }

  void answerAcceptUnsignedFile( bool answer )
  { _answer_accept_unsigned_file = answer; }
  
  bool askedAcceptUnsignedFile() const
  { return _asked_user_to_accept_unsigned_file; }
    
  virtual bool askUserToAcceptUnsignedFile( const std::string &file, const zypp::KeyContext &keycontext )
  {
    MIL << std::endl;
    _asked_user_to_accept_unsigned_file = true;
    return _answer_accept_unsigned_file;
  }
  
  virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &id, const zypp::KeyContext &keycontext )
  {
    MIL << std::endl;
    _asked_user_to_accept_unknown_key = true;
    return _answer_accept_unknown_key;
  }

  virtual KeyRingReport::KeyTrust askUserToAcceptKey( const zypp::PublicKey &key, const zypp::KeyContext &keycontext )
  {
    MIL << std::endl;
    _asked_user_to_accept_key = true;
    return _answer_accept_key;
  }

  virtual bool askUserToAcceptVerificationFailed( const std::string &file,  const zypp::PublicKey &key, const zypp::KeyContext &keycontext  )
  {
    MIL << std::endl;
    _asked_user_to_accept_ver_failed = true;
    return _answer_ver_failed;
  }
  
  // how to answer
  bool _answer_accept_unknown_key;
  KeyRingReport::KeyTrust _answer_accept_key;
  bool _answer_ver_failed;
  bool _answer_accept_unsigned_file;
  
  // we use this variables to check that the
  // callbacks were called
  bool _asked_user_to_accept_unknown_key;
  bool _asked_user_to_accept_key;
  bool _asked_user_to_accept_ver_failed;
  bool _asked_user_to_accept_unsigned_file;
};

/**
 * Keyring Signal Receiver with some features
 * Allows to simulate and configure user answer
 * Can record which callbacks were called
 */
struct KeyRingTestSignalReceiver : zypp::callback::ReceiveReport<zypp::KeyRingSignals>
{
  KeyRingTestSignalReceiver(/*RpmDb &rpmdb*/)
  : _trusted_key_added_called(false)
  {
    MIL << "KeyRing signals enabled" << std::endl;
    connect();
  }

  ~KeyRingTestSignalReceiver()
  {
    disconnect();
  }

  virtual void trustedKeyAdded( const zypp::PublicKey &key )
  {
    MIL << "TEST: trusted key added to zypp Keyring. Synchronizing keys with fake rpm keyring" << std::endl;
    _trusted_key_added_called = true;
    //std::cout << "trusted key added to zypp Keyring. Synchronizing keys with rpm keyring" << std::endl;
    //_rpmdb.importZyppKeyRingTrustedKeys();
    //_rpmdb.exportTrustedKeysInZyppKeyRing();
  }

  virtual void trustedKeyRemoved( const zypp::PublicKey &key  )
  {
  }
  
  bool _trusted_key_added_called;
  
};

#endif
