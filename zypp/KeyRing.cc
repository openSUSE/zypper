/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/KeyRing.cc
 *
*/
#include <iostream>
#include <fstream>
#include <sys/file.h>
#include <cstdio>
#include <unistd.h>

#include <boost/format.hpp>

#include "zypp/TmpPath.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include "zypp/base/Logger.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"
#include "zypp/base/Gettext.h"
#include "zypp/PathInfo.h"
#include "zypp/KeyRing.h"
#include "zypp/ExternalProgram.h"
#include "zypp/TmpPath.h"

using namespace std;
using namespace zypp::filesystem;

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::KeyRing"

#define GPG_BINARY "/usr/bin/gpg2"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(KeyRing);

  namespace
  {
    KeyRing::DefaultAccept _keyRingDefaultAccept( KeyRing::ACCEPT_NOTHING );
  }

  KeyRing::DefaultAccept KeyRing::defaultAccept()
  { return _keyRingDefaultAccept; }

  void KeyRing::setDefaultAccept( DefaultAccept value_r )
  {
    MIL << "Set new KeyRing::DefaultAccept: " << value_r << endl;
    _keyRingDefaultAccept = value_r;
  }

  bool KeyRingReport::askUserToAcceptUnsignedFile( const string &file, const KeyContext &keycontext )
  { return _keyRingDefaultAccept.testFlag( KeyRing::ACCEPT_UNSIGNED_FILE ); }

  KeyRingReport::KeyTrust
  KeyRingReport::askUserToAcceptKey( const PublicKey &key, const KeyContext &keycontext )
  {
    if ( _keyRingDefaultAccept.testFlag( KeyRing::TRUST_KEY_TEMPORARILY ) )
      return KEY_TRUST_TEMPORARILY;
    if ( _keyRingDefaultAccept.testFlag( KeyRing::TRUST_AND_IMPORT_KEY ) )
      return KEY_TRUST_AND_IMPORT;
    return KEY_DONT_TRUST;
  }

  bool KeyRingReport::askUserToAcceptUnknownKey( const string &file, const string &id, const KeyContext &keycontext )
  { return _keyRingDefaultAccept.testFlag( KeyRing::ACCEPT_UNKNOWNKEY ); }

  bool KeyRingReport::askUserToAcceptVerificationFailed( const string &file, const PublicKey &key, const KeyContext &keycontext )
  { return _keyRingDefaultAccept.testFlag( KeyRing::ACCEPT_VERIFICATION_FAILED ); }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : KeyRing::Impl
  //
  /** KeyRing implementation. */
  struct KeyRing::Impl
  {
    Impl( const Pathname & baseTmpDir )
    : _trusted_tmp_dir( baseTmpDir, "zypp-trusted-kr" )
    , _general_tmp_dir( baseTmpDir, "zypp-general-kr" )
    , _base_dir( baseTmpDir )
    {
      MIL << "Current KeyRing::DefaultAccept: " << _keyRingDefaultAccept << endl;
    }

    void importKey( const PublicKey &key, bool trusted = false);
    void multiKeyImport( const Pathname & keyfile_r, bool trusted_r = false);
    void deleteKey( const string &id, bool trusted );

    string readSignatureKeyId( const Pathname &signature );

    bool isKeyTrusted( const string &id);
    bool isKeyKnown( const string &id );

    list<PublicKey> trustedPublicKeys();
    list<PublicKey> publicKeys();

    list<string> trustedPublicKeyIds();
    list<string> publicKeyIds();

    void dumpPublicKey( const string &id, bool trusted, ostream &stream );

    bool verifyFileSignatureWorkflow(
        const Pathname &file,
        const string filedesc,
        const Pathname &signature,
        const KeyContext &keycontext = KeyContext());

    bool verifyFileSignature( const Pathname &file, const Pathname &signature);
    bool verifyFileTrustedSignature( const Pathname &file, const Pathname &signature);
  private:
    //mutable map<Locale, string> translations;
    bool verifyFile( const Pathname &file, const Pathname &signature, const Pathname &keyring);
    void importKey( const Pathname &keyfile, const Pathname &keyring);
    PublicKey exportKey( string id, const Pathname &keyring);
    void dumpPublicKey( const string &id, const Pathname &keyring, ostream &stream );
    void deleteKey( const string &id, const Pathname &keyring );

    list<PublicKey> publicKeys(const Pathname &keyring);
    list<string> publicKeyIds(const Pathname &keyring);

    bool publicKeyExists( string id, const Pathname &keyring);

    const Pathname generalKeyRing() const;
    const Pathname trustedKeyRing() const;

    // Used for trusted and untrusted keyrings
    TmpDir _trusted_tmp_dir;
    TmpDir _general_tmp_dir;
    Pathname _base_dir;
  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl( TmpPath::defaultLocation() ) );
      return _nullimpl;
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };


  const Pathname KeyRing::Impl::generalKeyRing() const
  {
    return _general_tmp_dir.path();
  }

  const Pathname KeyRing::Impl::trustedKeyRing() const
  {
    return _trusted_tmp_dir.path();
  }

  void KeyRing::Impl::importKey( const PublicKey &key, bool trusted)
  {
    callback::SendReport<target::rpm::KeyRingSignals> rpmdbEmitSignal;
    callback::SendReport<KeyRingSignals> emitSignal;

    importKey( key.path(), trusted ? trustedKeyRing() : generalKeyRing() );

    if ( trusted )
    {
      rpmdbEmitSignal->trustedKeyAdded( key );
      emitSignal->trustedKeyAdded( key );
    }
  }

  void KeyRing::Impl::multiKeyImport( const Pathname & keyfile_r, bool trusted_r )
  {
    importKey( keyfile_r, trusted_r ? trustedKeyRing() : generalKeyRing() );
  }

  void KeyRing::Impl::deleteKey( const string &id, bool trusted)
  {
    PublicKey key;

    if (trusted)
    {
	key = exportKey(id, trustedKeyRing());
    }

    deleteKey( id, trusted ? trustedKeyRing() : generalKeyRing() );

    if ( trusted )
    {
      callback::SendReport<target::rpm::KeyRingSignals> rpmdbEmitSignal;
      callback::SendReport<KeyRingSignals> emitSignal;

      rpmdbEmitSignal->trustedKeyRemoved( key );
      emitSignal->trustedKeyRemoved( key );
    }
  }

  list<PublicKey> KeyRing::Impl::publicKeys()
  {
    return publicKeys( generalKeyRing() );
  }

  list<PublicKey> KeyRing::Impl::trustedPublicKeys()
  {
    return publicKeys( trustedKeyRing() );
  }

  list<string> KeyRing::Impl::publicKeyIds()
  {
    return publicKeyIds( generalKeyRing() );
  }

  list<string> KeyRing::Impl::trustedPublicKeyIds()
  {
    return publicKeyIds( trustedKeyRing() );
  }

  bool KeyRing::Impl::verifyFileTrustedSignature( const Pathname &file, const Pathname &signature)
  {
    return verifyFile( file, signature, trustedKeyRing() );
  }

  bool KeyRing::Impl::verifyFileSignature( const Pathname &file, const Pathname &signature)
  {
    return verifyFile( file, signature, generalKeyRing() );
  }

  bool KeyRing::Impl::isKeyTrusted( const string &id)
  {
    return publicKeyExists( id, trustedKeyRing() );
  }

  bool KeyRing::Impl::isKeyKnown( const string &id )
  {
    MIL << endl;
    if ( publicKeyExists( id, trustedKeyRing() ) )
      return true;
    else
      return publicKeyExists( id, generalKeyRing() );
  }

  bool KeyRing::Impl::publicKeyExists( string id, const Pathname &keyring)
  {
    MIL << "Searching key [" << id << "] in keyring " << keyring << endl;
    list<PublicKey> keys = publicKeys(keyring);
    for (list<PublicKey>::const_iterator it = keys.begin(); it != keys.end(); it++)
    {
      if ( id == (*it).id() )

        return true;
    }
    return false;
  }

  PublicKey KeyRing::Impl::exportKey( string id, const Pathname &keyring)
  {
    TmpFile tmp_file( _base_dir, "pubkey-"+id+"-" );
    MIL << "Going to export key " << id << " from " << keyring << " to " << tmp_file.path() << endl;

    try {
      ofstream os(tmp_file.path().c_str());
      dumpPublicKey( id, keyring, os );
      os.close();
      return PublicKey( tmp_file );
    }
    catch (BadKeyException &e)
    {
      ERR << "Cannot create public key " << id << " from " << keyring << " keyring  to file " << e.keyFile() << endl;
      // TranslatorExplanation first %s is key name, second is keyring name
      // and third is keyfile name
      ZYPP_THROW(Exception(boost::str(boost::format(
          _("Cannot create public key %s from %s keyring to file %s"))
          % id % keyring.asString() % e.keyFile().asString())));
    }
    catch (exception &e)
    {
      ERR << "Cannot export key " << id << " from " << keyring << " keyring  to file " << tmp_file.path() << endl;
    }
    return PublicKey();
  }

  void KeyRing::Impl::dumpPublicKey( const string &id, bool trusted, ostream &stream )
  {
     dumpPublicKey( id, ( trusted ? trustedKeyRing() : generalKeyRing() ), stream );
  }

  void KeyRing::Impl::dumpPublicKey( const string &id, const Pathname &keyring, ostream &stream )
  {
    const char* argv[] =
    {
      GPG_BINARY,
      "--no-default-keyring",
      "--quiet",
      "--no-tty",
      "--no-greeting",
      "--no-permission-warning",
      "--batch",
      "--homedir",
      keyring.asString().c_str(),
      "-a",
      "--export",
      id.c_str(),
      NULL
    };
    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
    string line;
    int count;
    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      stream << line;
    }
    prog.close();
  }


  bool KeyRing::Impl::verifyFileSignatureWorkflow(
      const Pathname &file,
      const string filedesc,
      const Pathname &signature,
      const KeyContext &context)
  {
    callback::SendReport<KeyRingReport> report;
    //callback::SendReport<KeyRingSignals> emitSignal;
    MIL << "Going to verify signature for " << filedesc << " ( " << file << " ) with " << signature << endl;

    // if signature does not exists, ask user if he wants to accept unsigned file.
    if( signature.empty() || (!PathInfo(signature).isExist()) )
    {
      bool res = report->askUserToAcceptUnsignedFile( filedesc, context );
      MIL << "User decision on unsigned file: " << res << endl;
      return res;
    }

    // get the id of the signature
    string id = readSignatureKeyId(signature);

    // doeskey exists in trusted keyring
    if ( publicKeyExists( id, trustedKeyRing() ) )
    {
      PublicKey key = exportKey( id, trustedKeyRing() );

      // lets look if there is an updated key in the
      // general keyring
      if ( publicKeyExists( id, generalKeyRing() ) )
      {
        // bnc #393160: Comment #30: Compare at least the fingerprint
        // in case an attacker created a key the the same id.
        PublicKey untkey = exportKey( id, generalKeyRing() );
        if ( untkey.fingerprint() == key.fingerprint()
             && untkey.created() > key.created() )
        {
          MIL << "Key " << key << " was updated. Saving new version into trusted keyring." << endl;
          importKey( untkey, true );
          key = untkey;
        }
      }

      MIL << "Key " << id << " " << key.name() << " is trusted" << endl;
      // it exists, is trusted, does it validates?
      if ( verifyFile( file, signature, trustedKeyRing() ) )
        return true;
      else
        return report->askUserToAcceptVerificationFailed( filedesc, key, context );
    }
    else
    {
      if ( publicKeyExists( id, generalKeyRing() ) )
      {
        PublicKey key =  exportKey( id, generalKeyRing());
        MIL << "Exported key " << id << " to " << key.path() << endl;
        MIL << "Key " << id << " " << key.name() << " is not trusted" << endl;

        // ok the key is not trusted, ask the user to trust it or not
        KeyRingReport::KeyTrust reply = report->askUserToAcceptKey(key, context);
        if (reply == KeyRingReport::KEY_TRUST_TEMPORARILY ||
            reply == KeyRingReport::KEY_TRUST_AND_IMPORT)
        {
          MIL << "User wants to trust key " << id << " " << key.name() << endl;
          //dumpFile(unKey.path());

          Pathname which_keyring;
          if (reply == KeyRingReport::KEY_TRUST_AND_IMPORT)
          {
            MIL << "User wants to import key " << id << " " << key.name() << endl;
            importKey( key, true );
            which_keyring = trustedKeyRing();
          }
          else
            which_keyring = generalKeyRing();

          // emit key added
          if ( verifyFile( file, signature, which_keyring ) )
          {
            MIL << "File signature is verified" << endl;
            return true;
          }
          else
          {
            MIL << "File signature check fails" << endl;
            if ( report->askUserToAcceptVerificationFailed( filedesc, key, context ) )
            {
              MIL << "User continues anyway." << endl;
              return true;
            }
            else
            {
              MIL << "User does not want to continue" << endl;
              return false;
            }
          }
        }
        else
        {
          MIL << "User does not want to trust key " << id << " " << key.name() << endl;
          return false;
        }
      }
      else
      {
        // unknown key...
        MIL << "File [" << file << "] ( " << filedesc << " ) signed with unknown key [" << id << "]" << endl;
        if ( report->askUserToAcceptUnknownKey( filedesc, id, context ) )
        {
          MIL << "User wants to accept unknown key " << id << endl;
          return true;
        }
        else
        {
          MIL << "User does not want to accept unknown key " << id << endl;
          return false;
        }
      }
    }
    return false;
  }

  list<string> KeyRing::Impl::publicKeyIds(const Pathname &keyring)
  {
    static str::regex rxColons("^([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):\n$");
    static str::regex rxColonsFpr("^([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):\n$");

    list<string> ids;

    const char* argv[] =
    {
      GPG_BINARY,
      "--no-default-keyring",
      "--quiet",
      "--list-public-keys",
      "--with-colons",
      "--with-fingerprint",
      "--no-tty",
      "--no-greeting",
      "--batch",
      "--status-fd",
      "1",
      "--homedir",
      keyring.asString().c_str(),
      NULL
    };

    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
    string line;
    int count = 0;

    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      //MIL << line << endl;
      str::smatch what;
      if(str::regex_match(line, what, rxColons))
      {
        string id;
        string fingerprint;
        if ( what[1] == "pub" )
        {
          id = what[5];

          string line2;
          for(line2 = prog.receiveLine(); !line2.empty(); line2 = prog.receiveLine(), count++ )
          {
            str::smatch what2;
            if (str::regex_match(line2, what2, rxColonsFpr))
            {
              if ( (what2[1] == "fpr") && (what2[1] != "pub") && (what2[1] !="sub"))
              {
                fingerprint = what2[10];
                break;
              }
            }
          }

          ids.push_back(id);
          MIL << "Found key " << "[" << id << "]" << endl;
        }
        //dumpRegexpResults(what);
      }
    }
    prog.close();
    return ids;
  }

  list<PublicKey> KeyRing::Impl::publicKeys(const Pathname &keyring)
  {

    list<PublicKey> keys;

    list<string> ids = publicKeyIds(keyring);

    for ( list<string>::const_iterator it = ids.begin(); it != ids.end(); ++it )
    {
      PublicKey key(exportKey( *it, keyring ));
      keys.push_back(key);
      MIL << "Found key " << "[" << key.id() << "]" << " [" << key.name() << "]" << " [" << key.fingerprint() << "]" << endl;
    }
    return keys;
  }

  void KeyRing::Impl::importKey( const Pathname &keyfile, const Pathname &keyring)
  {
    if ( ! PathInfo(keyfile).isExist() )
      // TranslatorExplanation first %s is key name, second is keyring name
      ZYPP_THROW(KeyRingException(boost::str(boost::format(
          _("Tried to import not existent key %s into keyring %s"))
          % keyfile.asString() % keyring.asString())));

    const char* argv[] =
    {
      GPG_BINARY,
      "--no-default-keyring",
      "--quiet",
      "--no-tty",
      "--no-greeting",
      "--no-permission-warning",
      "--status-fd",
      "1",
      "--homedir",
      keyring.asString().c_str(),
      "--import",
      keyfile.asString().c_str(),
      NULL
    };

    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
    prog.close();
  }

  void KeyRing::Impl::deleteKey( const string &id, const Pathname &keyring )
  {
    const char* argv[] =
    {
      GPG_BINARY,
      "--no-default-keyring",
      "--yes",
      "--quiet",
      "--no-tty",
      "--batch",
      "--status-fd",
      "1",
      "--homedir",
      keyring.asString().c_str(),
      "--delete-keys",
      id.c_str(),
      NULL
    };

    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);

    int code = prog.close();
    if ( code )
      ZYPP_THROW(Exception(_("Failed to delete key.")));
    else
      MIL << "Deleted key " << id << " from keyring " << keyring << endl;
  }


  string KeyRing::Impl::readSignatureKeyId(const Pathname &signature )
  {
    if ( ! PathInfo(signature).isFile() )
      ZYPP_THROW(Exception(boost::str(boost::format(
          _("Signature file %s not found"))% signature.asString())));

    MIL << "Determining key id if signature " << signature << endl;
    // HACK create a tmp keyring with no keys
    TmpDir dir(_base_dir, "fake-keyring");

    const char* argv[] =
    {
      GPG_BINARY,
      "--no-default-keyring",
      "--quiet",
      "--no-tty",
      "--no-greeting",
      "--batch",
      "--status-fd",
      "1",
      "--homedir",
      dir.path().asString().c_str(),
      signature.asString().c_str(),
      NULL
    };

    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);

    string line;
    int count = 0;

    str::regex rxNoKey("^\\[GNUPG:\\] NO_PUBKEY (.+)\n$");
    string id;
    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      //MIL << "[" << line << "]" << endl;
      str::smatch what;
      if(str::regex_match(line, what, rxNoKey))
      {
        if ( what.size() >= 1 )
	{
          id = what[1];
	  break;
	}
        //dumpRegexpResults(what);
      }
    }

    if ( count == 0 )
    {
      MIL << "no output" << endl;
    }

    MIL << "Determined key id [" << id << "] for signature " << signature << endl;
    prog.close();
    return id;
  }

  bool KeyRing::Impl::verifyFile( const Pathname &file, const Pathname &signature, const Pathname &keyring)
  {
    const char* argv[] =
    {
      GPG_BINARY,
      "--no-default-keyring",
      "--quiet",
      "--no-tty",
      "--batch",
      "--no-greeting",
      "--status-fd",
      "1",
      "--homedir",
      keyring.asString().c_str(),
      "--verify",
      signature.asString().c_str(),
      file.asString().c_str(),
      NULL
    };

    // no need to parse output for now
    //     [GNUPG:] SIG_ID yCc4u223XRJnLnVAIllvYbUd8mQ 2006-03-29 1143618744
    //     [GNUPG:] GOODSIG A84EDAE89C800ACA SuSE Package Signing Key <build@suse.de>
    //     gpg: Good signature from "SuSE Package Signing Key <build@suse.de>"
    //     [GNUPG:] VALIDSIG 79C179B2E1C820C1890F9994A84EDAE89C800ACA 2006-03-29 1143618744 0 3 0 17 2 00 79C179B2E1C820C1890F9994A84EDAE89C800ACA
    //     [GNUPG:] TRUST_UNDEFINED

    //     [GNUPG:] ERRSIG A84EDAE89C800ACA 17 2 00 1143618744 9
    //     [GNUPG:] NO_PUBKEY A84EDAE89C800ACA

    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);

    return (prog.close() == 0) ? true : false;
  }

  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : KeyRing
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : KeyRing::KeyRing
  //	METHOD TYPE : Ctor
  //
  KeyRing::KeyRing(const Pathname &baseTmpDir)
  : _pimpl( new Impl(baseTmpDir) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : KeyRing::KeyRing
  //	METHOD TYPE : Ctor
  //
  //KeyRing::KeyRing( const Pathname &general_kr, const Pathname &trusted_kr )
  //: _pimpl( new Impl(general_kr, trusted_kr) )
  //{}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : KeyRing::~KeyRing
  //	METHOD TYPE : Dtor
  //
  KeyRing::~KeyRing()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to implementation:
  //
  ///////////////////////////////////////////////////////////////////


  void KeyRing::importKey( const PublicKey &key, bool trusted )
  {
    _pimpl->importKey( key, trusted );
  }

  void KeyRing::multiKeyImport( const Pathname & keyfile_r, bool trusted_r )
  {
    _pimpl->multiKeyImport( keyfile_r, trusted_r );
  }

  string KeyRing::readSignatureKeyId( const Pathname &signature )
  {
    return _pimpl->readSignatureKeyId(signature);
  }

  void KeyRing::deleteKey( const string &id, bool trusted )
  {
    _pimpl->deleteKey(id, trusted);
  }

  list<PublicKey> KeyRing::publicKeys()
  {
    return _pimpl->publicKeys();
  }

  list<PublicKey> KeyRing::trustedPublicKeys()
  {
    return _pimpl->trustedPublicKeys();
  }

  list<string> KeyRing::publicKeyIds()
  {
    return _pimpl->publicKeyIds();
  }

  list<string> KeyRing::trustedPublicKeyIds()
  {
    return _pimpl->trustedPublicKeyIds();
  }

  bool KeyRing::verifyFileSignatureWorkflow(
      const Pathname &file,
      const string filedesc,
      const Pathname &signature,
      const KeyContext &keycontext)
  {
    return _pimpl->verifyFileSignatureWorkflow(file, filedesc, signature, keycontext);
  }

  bool KeyRing::verifyFileSignature( const Pathname &file, const Pathname &signature)
  {
    return _pimpl->verifyFileSignature(file, signature);
  }

  bool KeyRing::verifyFileTrustedSignature( const Pathname &file, const Pathname &signature)
  {
    return _pimpl->verifyFileTrustedSignature(file, signature);
  }

  void KeyRing::dumpPublicKey( const string &id, bool trusted, ostream &stream )
  {
    _pimpl->dumpPublicKey( id, trusted, stream);
  }

  bool KeyRing::isKeyTrusted( const string &id )
  {
    return _pimpl->isKeyTrusted(id);
  }

  bool KeyRing::isKeyKnown( const string &id )
  {
    return _pimpl->isKeyKnown(id);
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
