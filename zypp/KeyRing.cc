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
//#include "zypp/base/Logger.h"
#include <sys/file.h>
#include <cstdio>
#include <unistd.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include <boost/regex.hpp>

#include "zypp/base/String.h"
#include "zypp/base/IOStream.h"
#include "zypp/KeyRing.h"
#include "zypp/ExternalProgram.h"
#include "zypp/TmpPath.h"

using std::endl;
using namespace boost;
using namespace zypp::filesystem;
using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(KeyRing);
  
  static void dumpRegexpResults( const boost::smatch &what )
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
  
  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : KeyRing::Impl
  //
  /** KeyRing implementation. */
  struct KeyRing::Impl
  {
    Impl()
    {}

    Impl( const Pathname &general_kr, const Pathname &trusted_kr )
    {
      filesystem::assert_dir(general_kr);
      filesystem::assert_dir(trusted_kr);
      
      _general_kr = general_kr;
      _trusted_kr = trusted_kr;
    }

    void importKey( const Pathname &keyfile, bool trusted = false);
    PublicKey readPublicKey( const Pathname &keyfile );
    std::string readSignatureKeyId(  const Pathname &data, const Pathname &keyfile );
    
    void deleteKey( const std::string &id, bool trusted );
    std::list<PublicKey> trustedPublicKeys();
    std::list<PublicKey> publicKeys();
    
    void dumpPublicKey( const std::string &id, bool trusted, std::ostream &stream );
    
    bool verifyFileSignatureWorkflow( const Pathname &file, const Pathname &signature);
    
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
    
    Pathname _general_kr;
    Pathname _trusted_kr;
  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  
  void KeyRing::Impl::importKey( const Pathname &keyfile, bool trusted)
  {
    importKey( keyfile, trusted ? _trusted_kr : _general_kr );
  }
  
  void KeyRing::Impl::deleteKey( const std::string &id, bool trusted)
  {
    deleteKey( id, trusted ? _trusted_kr : _general_kr );
  }
  
  std::list<PublicKey> KeyRing::Impl::publicKeys()
  {
    return publicKeys( _general_kr );
  }
  
  std::list<PublicKey> KeyRing::Impl::trustedPublicKeys()
  {
    return publicKeys( _trusted_kr );
  }
  
  bool KeyRing::Impl::verifyFileTrustedSignature( const Pathname &file, const Pathname &signature)
  {
    return verifyFile( file, signature, _trusted_kr );
  }
  
  bool KeyRing::Impl::verifyFileSignature( const Pathname &file, const Pathname &signature)
  {
    return verifyFile( file, signature, _general_kr );
  }
  
  bool KeyRing::Impl::publicKeyExists( std::string id, const Pathname &keyring)
  {
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
    Pathname keyring = trusted ? _trusted_kr : _general_kr;
    const char* argv[] =
    {
      "gpg",
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
  
  
  bool KeyRing::Impl::verifyFileSignatureWorkflow( const Pathname &file, const Pathname &signature)
  {
    callback::SendReport<KeyRingReport> report;
    callback::SendReport<KeyRingSignals> emitSignal;
    MIL << "Going to verify signature for " << file << " with " << signature << std::endl; 

    // get the id of the signature
    std::string id = readSignatureKeyId(file, signature);
    
    // doeskey exists in trusted keyring
    if ( publicKeyExists( id, _trusted_kr ) )
    {
      TmpFile trustedKey;
      exportKey( id, trustedKey.path(), true);
      PublicKey key = readPublicKey(trustedKey.path());
      MIL << "Key " << id << " " << key.name << " is trusted" << std::endl;
      // it exists, is trusted, does it validates?
      if ( verifyFile( file, signature, _trusted_kr ) )
        return true;  
      else
        return report->askUserToAcceptVerificationFailed( file, key.id, key.name );
    }
    else
    {
      if ( publicKeyExists( id, _general_kr ) )
      {
        TmpFile unKey;
        exportKey( id, unKey.path(), false);
        MIL << "Exported key " << id << " to " << unKey << std::endl;
        
        PublicKey key = readPublicKey(unKey.path());
        MIL << "Key " << id << " " << key.name << " is not trusted" << std::endl;
        // ok the key is not trusted, ask the user to trust it or not
#warning We need the key details passed to the callback
        if ( report->askUserToTrustKey(key.id, key.name, "") )
        {
          MIL << "User wants to trust key " << id << " " << key.name << std::endl;
          //dumpFile(unKey.path());
          
          importKey( unKey.path(), _trusted_kr );
          emitSignal->trustedKeyAdded( (const KeyRing &)(*this), id, key.name );
          
          // emit key added
          if ( verifyFile( file, signature, _trusted_kr ) )
          {
            MIL << "File signature is verified" << std::endl;
            return true;  
          }
          else
          {
            MIL << "File signature check fails" << std::endl;
            if ( report->askUserToAcceptVerificationFailed( file, key.id, key.name ) )
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
        if ( report->askUserToAcceptUnknownKey( file, id, "Unknown Key" ) )
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
    const char* argv[] =
    {
      "gpg",
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
    
    boost::regex rxColons("^([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):\n$");
    
    // pub:-:1024:17:A84EDAE89C800ACA:2000-10-19:2008-06-21::-:SuSE Package Signing Key <build@suse.de>:
    
    PublicKey key;
    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      //MIL << "[" << line << "]" << std::endl;
      boost::smatch what;
      if(boost::regex_match(line, what, rxColons, boost::match_extra))
      {
        if ( what[1] == "pub" )
        {
          key.id = what[5];
          key.name = what[10];
          return key;
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
      "--quiet",
      "--list-public-keys",
      "--with-colons",
      "--with-fingerprint",
      "--homedir",
      keyring.asString().c_str(),
      NULL
    };
    std::list<PublicKey> keys;
    
    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
    std::string line;
    int count = 0;
    
    boost::regex rxColons("^([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):([^:]*):\n$");
    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      //MIL << line << std::endl;
      boost::smatch what;
      if(boost::regex_match(line, what, rxColons, boost::match_extra))
      {
        if ( what[1] == "pub" )
        {
          PublicKey key;
          key.id = what[5];
          key.name = what[10];
          MIL << "Found key " << key.id << " [" << key.name << "]" << std::endl;
          keys.push_back(key);
        }
        dumpRegexpResults(what);
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
  
  
  std::string KeyRing::Impl::readSignatureKeyId( const Pathname &data, const Pathname &keyfile )
  {  
    // HACK create a tmp keyring with no keys
    TmpDir dir;
    
    const char* argv[] =
    {
      "gpg",
      "--quiet",
      "--no-tty",
      "--no-greeting",
      "--batch",
      "--status-fd",
      "1",
      "--homedir",
      dir.path().asString().c_str(),
      "--verify",
      keyfile.asString().c_str(),
      data.asString().c_str(),
      NULL
    };
    
    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
    
    std::string line;
    int count = 0;
    
    boost::regex rxNoKey("^\\[GNUPG:\\] NO_PUBKEY (.+)\n$");
    std::string id;
    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      //MIL << "[" << line << "]" << std::endl;
      boost::smatch what;
      if(boost::regex_match(line, what, rxNoKey, boost::match_extra))
      {
        if ( what.size() > 1 )
          id = what[1];
        //dumpRegexpResults(what);
      }
    }
    prog.close();
    return id;
  }    
      
  bool KeyRing::Impl::verifyFile( const Pathname &file, const Pathname &signature, const Pathname &keyring)
  {
    const char* argv[] =
    {
      "gpg",
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
  KeyRing::KeyRing()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : KeyRing::KeyRing
  //	METHOD TYPE : Ctor
  //
  KeyRing::KeyRing( const Pathname &general_kr, const Pathname &trusted_kr )
  : _pimpl( new Impl(general_kr, trusted_kr) )
  {}

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
  
  std::string KeyRing::readSignatureKeyId(  const Pathname &data, const Pathname &keyfile )
  {
    return _pimpl->readSignatureKeyId(data, keyfile);
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
  
  bool KeyRing::verifyFileSignatureWorkflow( const Pathname &file, const Pathname &signature)
  {
    return _pimpl->verifyFileSignatureWorkflow(file, signature);
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
