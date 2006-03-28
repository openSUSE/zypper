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
//#include "zypp/base/Logger.h"

#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include <boost/regex.hpp>

#include "zypp/base/String.h"
#include "zypp/KeyRing.h"
#include "zypp/ExternalProgram.h"

using std::endl;
using namespace boost;

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
  private:
    //mutable std::map<Locale, std::string> translations;
    
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
    
    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
    std::string line;
    int count = 0;
    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      MIL << line << std::endl;
    }
    prog.close();
  }
  
  PublicKey KeyRing::Impl::importKey( const Pathname &keyfile, const Pathname &keyring)
  {
    const char* argv[] =
    {
      "gpg",
      "--batch",
      "--homedir",
      keyring.asString().c_str(),
      "--import",
      keyfile.asString().c_str(),
      NULL
    };
    
    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);

    //if(!prog)
    //  return 2;
    //"gpg: key 9C800ACA: public key "SuSE Package Signing Key <build@suse.de>" imported"
    //TODO parse output and return key id
    MIL << std::endl;
    boost::regex rxImported("^gpg: key [[[:digit:]][[:word:]]]+ \"(.+)\" imported$");
    std::string line;
    int count = 0;
    for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      MIL << line << std::endl;
      boost::smatch what;
      if(boost::regex_match(line, what, rxImported, boost::match_extra))
      {
        MIL << std::endl;
        dumpRegexpResults(what);
        prog.close();
        return PublicKey();
      }
    }
    prog.close();
    throw Exception("failed to import key");
  }
  
  void KeyRing::Impl::deleteKey( const std::string &id, const Pathname &keyring )
  {
    const char* argv[] =
    {
      "gpg",
      "--batch",
      "--yes",
      "--delete-keys",
      "--homedir",
      keyring.asString().c_str(),
      id.c_str(),
      NULL
    };
    
    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
    prog.close();
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
  
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
