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

  static void dumpRegexpResults( const boost::smatch &what )
  {
    for ( unsigned int k=0; k < what.size(); k++)
    {
      XXX << "[match "<< k << "] [" << what[k] << "]" << std::endl;
    }
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
      _general_kr = general_kr;
      _trusted_kr = trusted_kr;
    }

    PublicKey importKey( const Pathname &keyfile, bool trusted );
    void deleteKey( const std::string &id, bool trusted );
    std::list<PublicKey> trustedPublicKeys();
    std::list<PublicKey> publicKeys();
    bool verifyFileSignature( const Pathname &file, const Pathname &signature);
    bool verifyFileTrustedSignature( const Pathname &file, const Pathname &signature);
  private:
    //mutable std::map<Locale, std::string> translations;
    bool verifyFile( const Pathname &file, const Pathname &signature, const Pathname &keyring);
    PublicKey importKey( const Pathname &keyfile, const Pathname &keyring);
    void deleteKey( const std::string &id, const Pathname &keyring );
    std::list<PublicKey> publicKeys(const Pathname &keyring);
    
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
  
  PublicKey KeyRing::Impl::importKey( const Pathname &keyfile, bool trusted)
  {
    return importKey( keyfile, trusted ? _trusted_kr : _general_kr );
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
  
  std::list<PublicKey> KeyRing::Impl::publicKeys(const Pathname &keyring)
  {
    const char* argv[] =
    {
      "gpg",
      "--quiet",
      "--list-keys",
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
      MIL << line << std::endl;
      boost::smatch what;
      if(boost::regex_match(line, what, rxColons, boost::match_extra))
      {
        if ( what[1] == "pub" )
        {
          PublicKey key;
          key.id = what[5];
          key.name = what[10];
          keys.push_back(key);
        }
        //dumpRegexpResults(what);
      }
    }
    prog.close();
    return keys;
  }
  
  PublicKey KeyRing::Impl::importKey( const Pathname &keyfile, const Pathname &keyring)
  {
    const char* argv[] =
    {
      "gpg",
      "--quiet",
      "--no-tty",
      "--no-greeting",
      "--status-fd",
      "1",  
      "--homedir",
      keyring.asString().c_str(),
      "--import",
      keyfile.asString().c_str(),
      NULL
    };
    
    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
    
    boost::regex rxImported("^\\[GNUPG:\\] IMPORTED ([^[:space:]]+) (.+)\n$");
    std::string line;
    int count = 0;
    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      MIL << line << std::endl;
       boost::smatch what;
       if(boost::regex_match(line, what, rxImported, boost::match_extra))
       {
         MIL << std::endl;
         PublicKey key;
         key.id = what[1];
         key.name = what[2];
         return key;
       }
    }
    prog.close();
    throw Exception("failed to import key");
    return PublicKey();
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

  PublicKey KeyRing::importKey( const Pathname &keyfile, bool trusted)
  {
    return _pimpl->importKey(keyfile, trusted);
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
  
  bool KeyRing::verifyFileSignature( const Pathname &file, const Pathname &signature)
  {
    return _pimpl->verifyFileSignature(file, signature);
  }
  
  bool KeyRing::verifyFileTrustedSignature( const Pathname &file, const Pathname &signature)
  {
    return _pimpl->verifyFileTrustedSignature(file, signature);
  }
  
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
