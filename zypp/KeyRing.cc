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

#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include "zypp/base/Logger.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/String.h"
#include "zypp/Pathname.h"
#include "zypp/KeyRing.h"
#include "zypp/ExternalProgram.h"
#include "zypp/TmpPath.h"

using std::endl;
using namespace zypp::filesystem;
using namespace std;

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::KeyRing"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(KeyRing);

  static void dumpRegexpResults( const str::smatch &what )
  {
    for ( unsigned int k=0; k < what.size(); k++)
    {
      XXX << "[match "<< k << "] [" << what[k] << "]" << std::endl;
    }
  }

  static bool printLine( const std::string &line )
  {
    MIL <<  line << std::endl;
  }

  static void dumpFile(const Pathname &file)
  {
    std::ifstream is(file.asString().c_str());
    iostr::forEachLine( is, printLine);
  }

  namespace
  {
    bool _keyRingDefaultAccept( getenv("ZYPP_KEYRING_DEFAULT_ACCEPT_ALL") );
  }

  bool KeyRingReport::askUserToAcceptUnsignedFile( const std::string &file )
  { return _keyRingDefaultAccept; }

  bool KeyRingReport::askUserToAcceptUnknownKey( const std::string &file, const std::string &keyid, const std::string &keyname, const std::string &fingerprint )
  { return _keyRingDefaultAccept; }

  bool KeyRingReport::askUserToTrustKey( const std::string &keyid, const std::string &keyname, const std::string &fingerprint )
  { return _keyRingDefaultAccept; }

  bool KeyRingReport::askUserToAcceptVerificationFailed( const std::string &file, const std::string &keyid, const std::string &keyname, const std::string &fingerprint )
  { return _keyRingDefaultAccept; }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : KeyRing::Impl
  //
  /** KeyRing implementation. */
  struct KeyRing::Impl
  {
    Impl(const Pathname &baseTmpDir)
    : _trusted_tmp_dir(baseTmpDir)
    ,  _general_tmp_dir(baseTmpDir)

    {
      _base_dir = baseTmpDir;
    }

    /*
    Impl( const Pathname &general_kr, const Pathname &trusted_kr )
    {
      filesystem::assert_dir(general_kr);
      filesystem::assert_dir(trusted_kr);

      generalKeyRing() = general_kr;
      trustedKeyRing() = trusted_kr;
    }
    */

    void importKey( const Pathname &keyfile, bool trusted = false);
    PublicKey readPublicKey( const Pathname &keyfile );
    std::string readSignatureKeyId( const Pathname &signature );

    void deleteKey( const std::string &id, bool trusted );
    std::list<PublicKey> trustedPublicKeys();
    std::list<PublicKey> publicKeys();

    void dumpPublicKey( const std::string &id, bool trusted, std::ostream &stream );

    bool verifyFileSignatureWorkflow( const Pathname &file, const std::string filedesc, const Pathname &signature);

    bool verifyFileSignature( const Pathname &file, const Pathname &signature);
    bool verifyFileTrustedSignature( const Pathname &file, const Pathname &signature);
  private:
    //mutable std::map<Locale, std::string> translations;
    bool verifyFile( const Pathname &file, const Pathname &signature, const Pathname &keyring);
    void importKey( const Pathname &keyfile, const Pathname &keyring);

    void exportKey( std::string id, const Pathname &keyfile, bool trusted);

    void deleteKey( const std::string &id, const Pathname &keyring );
    std::list<PublicKey> publicKeys(const Pathname &keyring);

    bool publicKeyExists( std::string id, const Pathname &keyring);

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
      static shared_ptr<Impl> _nullimpl( new Impl( Pathname("/var/tmp") ) );
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

  void KeyRing::Impl::importKey( const Pathname &keyfile, bool trusted)
  {
    importKey( keyfile, trusted ? trustedKeyRing() : generalKeyRing() );
  }

  void KeyRing::Impl::deleteKey( const std::string &id, bool trusted)
  {
    deleteKey( id, trusted ? trustedKeyRing() : generalKeyRing() );
  }

  std::list<PublicKey> KeyRing::Impl::publicKeys()
  {
    return publicKeys( generalKeyRing() );
  }

  std::list<PublicKey> KeyRing::Impl::trustedPublicKeys()
  {
    return publicKeys( trustedKeyRing() );
  }

  bool KeyRing::Impl::verifyFileTrustedSignature( const Pathname &file, const Pathname &signature)
  {
    return verifyFile( file, signature, trustedKeyRing() );
  }

  bool KeyRing::Impl::verifyFileSignature( const Pathname &file, const Pathname &signature)
  {
    return verifyFile( file, signature, generalKeyRing() );
  }

  bool KeyRing::Impl::publicKeyExists( std::string id, const Pathname &keyring)
  {
    MIL << "Searching key [" << id << "] in keyring " << keyring << std::endl;
    std::list<PublicKey> keys = publicKeys(keyring);
    for (std::list<PublicKey>::const_iterator it = keys.begin(); it != keys.end(); it++)
    {
      if ( id == (*it).id )
        return true;
    }
    return false;
  }

  void KeyRing::Impl::exportKey( std::string id, const Pathname &keyfile, bool trusted)
  {
    try {
      std::ofstream os(keyfile.asString().c_str());
      dumpPublicKey( id, trusted, os );
      os.close();
    }
    catch (std::exception &e)
    {
      ERR << "Cannot export key " << id << " from " << (trusted ? "trusted" : "untrusted ") << " keyring  to file " << keyfile << std::endl;
    }
  }

  void KeyRing::Impl::dumpPublicKey( const std::string &id, bool trusted, std::ostream &stream )
  {
    Pathname keyring = trusted ? trustedKeyRing() : generalKeyRing();
    const char* argv[] =
    {
      "gpg",
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
    std::string line;
    int count;
    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      stream << line;
    }
    prog.close();
  }


  bool KeyRing::Impl::verifyFileSignatureWorkflow( const Pathname &file, const std::string filedesc, const Pathname &signature)
  {
    callback::SendReport<KeyRingReport> report;
    callback::SendReport<KeyRingSignals> emitSignal;
    MIL << "Going to verify signature for " << file << " with " << signature << std::endl;

    // if signature does not exists, ask user if he wants to accept unsigned file.
    if( signature.empty() || (!PathInfo(signature).isExist()) )
    {
	bool res = report->askUserToAcceptUnsignedFile( filedesc );
	MIL << "User decision on unsigned file: " << res << endl;
	return res;
    }

    // get the id of the signature
    std::string id = readSignatureKeyId(signature);

    // doeskey exists in trusted keyring
    if ( publicKeyExists( id, trustedKeyRing() ) )
    {
      TmpFile trustedKey(_base_dir);
      exportKey( id, trustedKey.path(), true);
      PublicKey key = readPublicKey(trustedKey.path());
      MIL << "Key " << id << " " << key.name << " is trusted" << std::endl;
      // it exists, is trusted, does it validates?
      if ( verifyFile( file, signature, trustedKeyRing() ) )
        return true;
      else
        return report->askUserToAcceptVerificationFailed( filedesc, key.id, key.name, key.fingerprint );
    }
    else
    {
      if ( publicKeyExists( id, generalKeyRing() ) )
      {
        TmpFile unKey(_base_dir);
        exportKey( id, unKey.path(), false);
        MIL << "Exported key " << id << " to " << unKey << std::endl;

        PublicKey key = readPublicKey(unKey.path());
        MIL << "Key " << id << " " << key.name << " is not trusted" << std::endl;
        // ok the key is not trusted, ask the user to trust it or not
#warning We need the key details passed to the callback
        if ( report->askUserToTrustKey(key.id, key.name, key.fingerprint) )
        {
          MIL << "User wants to trust key " << id << " " << key.name << std::endl;
          //dumpFile(unKey.path());

          importKey( unKey.path(), trustedKeyRing() );
          emitSignal->trustedKeyAdded( (const KeyRing &)(*this), id, key.name, key.fingerprint );

          // emit key added
          if ( verifyFile( file, signature, trustedKeyRing() ) )
          {
            MIL << "File signature is verified" << std::endl;
            return true;
          }
          else
          {
            MIL << "File signature check fails" << std::endl;
            if ( report->askUserToAcceptVerificationFailed( filedesc, key.id, key.name, key.fingerprint ) )
            {
              MIL << "User continues anyway." << std::endl;
              return true;
            }
            else
            {
              MIL << "User does not want to continue" << std::endl;
              return false;
            }
          }
        }
        else
        {
          MIL << "User does not want to trust key " << id << " " << key.name << std::endl;
          return false;
        }
      }
      else
      {
        // unknown key...
        MIL << "File [" << file << "] ( " << filedesc << " ) signed with unknown key [" << id << "]" << std::endl;
        if ( report->askUserToAcceptUnknownKey( filedesc, id, "", "" ) )
        {
          MIL << "User wants to accept unknown key " << id << std::endl;
          return true;
        }
        else
        {
          MIL << "User does not want to accept unknown key " << id << std::endl;
          return false;
        }
      }
    }
    return false;
  }


  PublicKey KeyRing::Impl::readPublicKey( const Pathname &keyfile )
  {
    TmpDir dir(_base_dir);

    const char* argv[] =
    {
      "gpg",
      "--no-default-keyring",
      "--homedir",
      dir.path().asString().c_str(),
      "--with-fingerprint",
      "--with-colons",
      "--quiet",
      "--no-tty",
      "--no-greeting",
      "--batch",
      "--status-fd",
      "1",
      keyfile.asString().c_str(),
      NULL
    };

    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);

    std::string line;
    int count = 0;

    str::regex rxColons("^([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):\n$");

    // pub:-:1024:17:A84EDAE89C800ACA:2000-10-19:2008-06-21::-:SuSE Package Signing Key <build@suse.de>:

    PublicKey key;
    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      //MIL << "[" << line << "]" << std::endl;
      str::smatch what;
      if(str::regex_match(line, what, rxColons, str::match_extra))
      {
        if ( what[1] == "pub" )
        {
          key.id = what[5];
          key.name = what[10];
          //return key;
        }
        else if ( what[1] == "fpr" )
        {
          key.fingerprint = what[10];
        }
        //dumpRegexpResults(what);
      }
    }
    prog.close();
    return key;
  }

  std::list<PublicKey> KeyRing::Impl::publicKeys(const Pathname &keyring)
  {
    const char* argv[] =
    {
      "gpg",
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
    std::list<PublicKey> keys;

    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
    std::string line;
    int count = 0;

    str::regex rxColons("^([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):\n$");
    str::regex rxColonsFpr("^([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):\n$");

    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      //MIL << line << std::endl;
      str::smatch what;
      if(str::regex_match(line, what, rxColons, str::match_extra))
      {
        PublicKey key;
        if ( what[1] == "pub" )
        {
          key.id = what[5];
          key.name = what[10];

          std::string line2;
          for(line2 = prog.receiveLine(); !line2.empty(); line2 = prog.receiveLine(), count++ )
          {
            str::smatch what2;
            if (str::regex_match(line2, what2, rxColonsFpr, str::match_extra))
            {
              if ( (what2[1] == "fpr") && (what2[1] != "pub") && (what2[1] !="sub"))
              {
                key.fingerprint = what2[10];
                break;
              }
            }
          }
          keys.push_back(key);
          MIL << "Found key " << "[" << key.id << "]" << " [" << key.name << "]" << " [" << key.fingerprint << "]" << std::endl;
        }
        //dumpRegexpResults(what);
      }
    }
    prog.close();
    return keys;
  }

  void KeyRing::Impl::importKey( const Pathname &keyfile, const Pathname &keyring)
  {
    const char* argv[] =
    {
      "gpg",
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

    int code;
    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
    code = prog.close();

    //if ( code != 0 )
    //  ZYPP_THROW(Exception("failed to import key"));
  }

  void KeyRing::Impl::deleteKey( const std::string &id, const Pathname &keyring )
  {
    const char* argv[] =
    {
      "gpg",
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
      ZYPP_THROW(Exception("Failed to delete key."));
    else
      MIL << "Deleted key " << id << " from keyring " << keyring << std::endl;
  }


  std::string KeyRing::Impl::readSignatureKeyId(const Pathname &signature )
  {
    MIL << "Deetermining key id if signature " << signature << std::endl;
    // HACK create a tmp keyring with no keys
    TmpDir dir(_base_dir);
    TmpFile fakeData(_base_dir);

    const char* argv[] =
    {
      "gpg",
      "--no-default-keyring",
      "--quiet",
      "--no-tty",
      "--no-greeting",
      "--batch",
      "--status-fd",
      "1",
      "--homedir",
      dir.path().asString().c_str(),
      "--verify",
      signature.asString().c_str(),
      fakeData.path().asString().c_str(),
      NULL
    };

    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);

    std::string line;
    int count = 0;

    str::regex rxNoKey("^\\[GNUPG:\\] NO_PUBKEY (.+)\n$");
    std::string id;
    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      //MIL << "[" << line << "]" << std::endl;
      str::smatch what;
      if(str::regex_match(line, what, rxNoKey, str::match_extra))
      {
        if ( what.size() > 1 )
          id = what[1];
        //dumpRegexpResults(what);
      }
    }
    MIL << "Determined key id [" << id << "] for signature " << signature << std::endl;
    prog.close();
    return id;
  }

  bool KeyRing::Impl::verifyFile( const Pathname &file, const Pathname &signature, const Pathname &keyring)
  {
    const char* argv[] =
    {
      "gpg",
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

  void KeyRing::importKey( const Pathname &keyfile, bool trusted)
  {
    _pimpl->importKey(keyfile, trusted);
  }

  PublicKey KeyRing::readPublicKey( const Pathname &keyfile )
  {
    return _pimpl->readPublicKey(keyfile);
  }

  std::string KeyRing::readSignatureKeyId( const Pathname &signature )
  {
    return _pimpl->readSignatureKeyId(signature);
  }

  void KeyRing::deleteKey( const std::string &id, bool trusted )
  {
    _pimpl->deleteKey(id, trusted);
  }

  std::list<PublicKey> KeyRing::publicKeys()
  {
    return _pimpl->publicKeys();
  }

  std::list<PublicKey> KeyRing::trustedPublicKeys()
  {
    return _pimpl->trustedPublicKeys();
  }

  bool KeyRing::verifyFileSignatureWorkflow( const Pathname &file, const std::string filedesc, const Pathname &signature)
  {
    return _pimpl->verifyFileSignatureWorkflow(file, filedesc, signature);
  }

  bool KeyRing::verifyFileSignature( const Pathname &file, const Pathname &signature)
  {
    return _pimpl->verifyFileSignature(file, signature);
  }

  bool KeyRing::verifyFileTrustedSignature( const Pathname &file, const Pathname &signature)
  {
    return _pimpl->verifyFileTrustedSignature(file, signature);
  }

  void KeyRing::dumpPublicKey( const std::string &id, bool trusted, std::ostream &stream )
  {
    _pimpl->dumpPublicKey( id, trusted, stream);
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
