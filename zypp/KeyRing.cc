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

#include "zypp/base/LogTools.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/WatchFile.h"
#include "zypp/PathInfo.h"
#include "zypp/KeyRing.h"
#include "zypp/ExternalProgram.h"
#include "zypp/TmpPath.h"

using std::endl;

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

  bool KeyRingReport::askUserToAcceptUnsignedFile( const std::string & file, const KeyContext & keycontext )
  { return _keyRingDefaultAccept.testFlag( KeyRing::ACCEPT_UNSIGNED_FILE ); }

  KeyRingReport::KeyTrust
  KeyRingReport::askUserToAcceptKey( const PublicKey & key, const KeyContext & keycontext )
  {
    if ( _keyRingDefaultAccept.testFlag( KeyRing::TRUST_KEY_TEMPORARILY ) )
      return KEY_TRUST_TEMPORARILY;
    if ( _keyRingDefaultAccept.testFlag( KeyRing::TRUST_AND_IMPORT_KEY ) )
      return KEY_TRUST_AND_IMPORT;
    return KEY_DONT_TRUST;
  }

  bool KeyRingReport::askUserToAcceptUnknownKey( const std::string & file, const std::string & id, const KeyContext & keycontext )
  { return _keyRingDefaultAccept.testFlag( KeyRing::ACCEPT_UNKNOWNKEY ); }

  bool KeyRingReport::askUserToAcceptVerificationFailed( const std::string & file, const PublicKey & key, const KeyContext & keycontext )
  { return _keyRingDefaultAccept.testFlag( KeyRing::ACCEPT_VERIFICATION_FAILED ); }

  namespace
  {
    ///////////////////////////////////////////////////////////////////
    /// \class CachedPublicKeyData
    /// \brief Functor returning the keyrings data (cached).
    /// \code
    ///   const std::list<PublicKeyData> & cachedPublicKeyData( const Pathname & keyring );
    /// \endcode
    ///////////////////////////////////////////////////////////////////
    struct CachedPublicKeyData // : private base::NonCopyable - but KeyRing uses RWCOW though also NonCopyable :(
    {
      const std::list<PublicKeyData> & operator()( const Pathname & keyring_r ) const
      { return getData( keyring_r ); }

    private:
      struct Cache
      {
	scoped_ptr<WatchFile> _keyringP;
	std::list<PublicKeyData> _data;

	// Empty copy ctor to allow insert into std::map as
	// scoped_ptr is noncopyable.
	Cache() {}
	Cache( const Cache & rhs ) {}
      };

      typedef std::map<Pathname,Cache> CacheMap;

      const std::list<PublicKeyData> & getData( const Pathname & keyring_r ) const
      {
	Cache & cache( _cacheMap[keyring_r] );
	if ( ! cache._keyringP )
	{
	  // init new cache entry
	  cache._keyringP.reset( new WatchFile( keyring_r/"pubring.gpg", WatchFile::NO_INIT ) );
	}
	return getData( keyring_r, cache );
      }

      const std::list<PublicKeyData> & getData( const Pathname & keyring_r, Cache & cache_r ) const
      {
	if ( cache_r._keyringP->hasChanged() )
	{
	  const char* argv[] =
	  {
	    GPG_BINARY,
	    "--list-public-keys",
	    "--homedir", keyring_r.c_str(),
	    "--no-default-keyring",
	    "--quiet",
	    "--with-colons",
	    "--fixed-list-mode",
	    "--with-fingerprint",
	    "--with-sig-list",
	    "--no-tty",
	    "--no-greeting",
	    "--batch",
	    "--status-fd", "1",
	    NULL
	  };

	  PublicKeyScanner scanner;
	  ExternalProgram prog( argv ,ExternalProgram::Discard_Stderr, false, -1, true );
	  for( std::string line = prog.receiveLine(); !line.empty(); line = prog.receiveLine() )
	  {
	    scanner.scan( line );
	  }
	  prog.close();

	  cache_r._data.swap( scanner._keys );
	  MIL << "Found keys: " << cache_r._data  << endl;
	}
	return cache_r._data;
      }

      mutable CacheMap _cacheMap;
    };
    ///////////////////////////////////////////////////////////////////
  }

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

    void importKey( const PublicKey & key, bool trusted = false );
    void multiKeyImport( const Pathname & keyfile_r, bool trusted_r = false );
    void deleteKey( const std::string & id, bool trusted );

    std::string readSignatureKeyId( const Pathname & signature );

    bool isKeyTrusted( const std::string & id )
    { return bool(publicKeyExists( id, trustedKeyRing() )); }
    bool isKeyKnown( const std::string & id )
    { return publicKeyExists( id, trustedKeyRing() ) || publicKeyExists( id, generalKeyRing() ); }

    std::list<PublicKey> trustedPublicKeys()
    { return publicKeys( trustedKeyRing() ); }
    std::list<PublicKey> publicKeys()
    { return publicKeys( generalKeyRing() ); }

    const std::list<PublicKeyData> & trustedPublicKeyData()
    { return publicKeyData( trustedKeyRing() ); }
    const std::list<PublicKeyData> & publicKeyData()
    { return publicKeyData( generalKeyRing() ); }

    void dumpPublicKey( const std::string & id, bool trusted, std::ostream & stream )
    { dumpPublicKey( id, ( trusted ? trustedKeyRing() : generalKeyRing() ), stream ); }

    PublicKey exportPublicKey( const PublicKeyData & keyData )
    { return exportKey( keyData, generalKeyRing() ); }
    PublicKey exportTrustedPublicKey( const PublicKeyData & keyData )
    { return exportKey( keyData, trustedKeyRing() ); }

    bool verifyFileSignatureWorkflow(
        const Pathname & file,
        const std::string & filedesc,
        const Pathname & signature,
        const KeyContext & keycontext = KeyContext());

    bool verifyFileSignature( const Pathname & file, const Pathname & signature )
    { return verifyFile( file, signature, generalKeyRing() ); }
    bool verifyFileTrustedSignature( const Pathname & file, const Pathname & signature )
    { return verifyFile( file, signature, trustedKeyRing() ); }

  private:
    bool verifyFile( const Pathname & file, const Pathname & signature, const Pathname & keyring );
    void importKey( const Pathname & keyfile, const Pathname & keyring );

    PublicKey exportKey( const std::string & id, const Pathname & keyring );
    PublicKey exportKey( const PublicKeyData & keyData, const Pathname & keyring );

    void dumpPublicKey( const std::string & id, const Pathname & keyring, std::ostream & stream );
    filesystem::TmpFile dumpPublicKeyToTmp( const std::string & id, const Pathname & keyring );

    void deleteKey( const std::string & id, const Pathname & keyring );

    std::list<PublicKey> publicKeys( const Pathname & keyring);
    const std::list<PublicKeyData> & publicKeyData( const Pathname & keyring )
    { return cachedPublicKeyData( keyring ); }

    /** Get \ref PublicKeyData for ID (\c false if ID is not found). */
    PublicKeyData publicKeyExists( const std::string & id, const Pathname & keyring );

    const Pathname generalKeyRing() const
    { return _general_tmp_dir.path(); }
    const Pathname trustedKeyRing() const
    { return _trusted_tmp_dir.path(); }

    // Used for trusted and untrusted keyrings
    filesystem::TmpDir _trusted_tmp_dir;
    filesystem::TmpDir _general_tmp_dir;
    Pathname _base_dir;

  private:
    /** Functor returning the keyrings data (cached).
     * \code
     *  const std::list<PublicKeyData> & cachedPublicKeyData( const Pathname & keyring );
     * \endcode
     */
    CachedPublicKeyData cachedPublicKeyData;

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl( filesystem::TmpPath::defaultLocation() ) );
      return _nullimpl;
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////


  void KeyRing::Impl::importKey( const PublicKey & key, bool trusted )
  {
    importKey( key.path(), trusted ? trustedKeyRing() : generalKeyRing() );

    if ( trusted )
    {
      callback::SendReport<target::rpm::KeyRingSignals> rpmdbEmitSignal;
      callback::SendReport<KeyRingSignals> emitSignal;

      rpmdbEmitSignal->trustedKeyAdded( key );
      emitSignal->trustedKeyAdded( key );
    }
  }

  void KeyRing::Impl::multiKeyImport( const Pathname & keyfile_r, bool trusted_r )
  {
    importKey( keyfile_r, trusted_r ? trustedKeyRing() : generalKeyRing() );
  }

  void KeyRing::Impl::deleteKey( const std::string & id, bool trusted )
  {
    PublicKey key;

    if ( trusted )
    {
	key = exportKey( id, trustedKeyRing() );
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

  PublicKeyData KeyRing::Impl::publicKeyExists( const std::string & id, const Pathname & keyring )
  {
    MIL << "Searching key [" << id << "] in keyring " << keyring << endl;
    const std::list<PublicKeyData> & keys( publicKeyData( keyring ) );
    for_( it, keys.begin(), keys.end() )
    {
      if ( id == (*it).id() )
      {
        return *it;
      }
    }
    return PublicKeyData();
  }

  PublicKey KeyRing::Impl::exportKey( const PublicKeyData & keyData, const Pathname & keyring )
  {
    return PublicKey( dumpPublicKeyToTmp( keyData.id(), keyring ), keyData );
  }

  PublicKey KeyRing::Impl::exportKey( const std::string & id, const Pathname & keyring )
  {
    PublicKeyData keyData( publicKeyExists( id, keyring ) );
    if ( keyData )
      return PublicKey( dumpPublicKeyToTmp( keyData.id(), keyring ), keyData );

    // Here: key not found
    WAR << "No key " << id << " to export from " << keyring << endl;
    return PublicKey();
  }


  void KeyRing::Impl::dumpPublicKey( const std::string & id, const Pathname & keyring, std::ostream & stream )
  {
    const char* argv[] =
    {
      GPG_BINARY,
      "-a",
      "--export",
      "--homedir", keyring.asString().c_str(),
      "--no-default-keyring",
      "--quiet",
      "--no-tty",
      "--no-greeting",
      "--no-permission-warning",
      "--batch",
      id.c_str(),
      NULL
    };
    ExternalProgram prog( argv,ExternalProgram::Discard_Stderr, false, -1, true );
    for ( std::string line = prog.receiveLine(); !line.empty(); line = prog.receiveLine() )
    {
      stream << line;
    }
    prog.close();
  }

  filesystem::TmpFile KeyRing::Impl::dumpPublicKeyToTmp( const std::string & id, const Pathname & keyring )
  {
    filesystem::TmpFile tmpFile( _base_dir, "pubkey-"+id+"-" );
    MIL << "Going to export key " << id << " from " << keyring << " to " << tmpFile.path() << endl;

    std::ofstream os( tmpFile.path().c_str() );
    dumpPublicKey( id, keyring, os );
    os.close();
    return tmpFile;
  }

  bool KeyRing::Impl::verifyFileSignatureWorkflow(
      const Pathname & file,
      const std::string & filedesc,
      const Pathname & signature,
      const KeyContext & context )
  {
    callback::SendReport<KeyRingReport> report;
    MIL << "Going to verify signature for " << filedesc << " ( " << file << " ) with " << signature << endl;

    // if signature does not exists, ask user if he wants to accept unsigned file.
    if( signature.empty() || (!PathInfo( signature ).isExist()) )
    {
      bool res = report->askUserToAcceptUnsignedFile( filedesc, context );
      MIL << "User decision on unsigned file: " << res << endl;
      return res;
    }

    // get the id of the signature
    std::string id = readSignatureKeyId( signature );

    // doeskey exists in trusted keyring
    PublicKeyData trustedKeyData( publicKeyExists( id, trustedKeyRing() ) );
    if ( trustedKeyData )
    {
      MIL << "Key is trusted: " << trustedKeyData << endl;

      // lets look if there is an updated key in the
      // general keyring
      PublicKeyData generalKeyData( publicKeyExists( id, generalKeyRing() ) );
      if ( generalKeyData )
      {
        // bnc #393160: Comment #30: Compare at least the fingerprint
        // in case an attacker created a key the the same id.
        if ( trustedKeyData.fingerprint() == generalKeyData.fingerprint()
	   && trustedKeyData.created() < generalKeyData.created() )
        {
          MIL << "Key was updated. Saving new version into trusted keyring: " << generalKeyData << endl;
          importKey( exportKey( generalKeyData, generalKeyRing() ), true );
	  trustedKeyData = generalKeyData = PublicKeyData(); // invalidated by import.
	}
      }

      // it exists, is trusted, does it validates?
      if ( verifyFile( file, signature, trustedKeyRing() ) )
        return true;
      else
      {
	if ( ! trustedKeyData )	// invalidated by previous import
	  trustedKeyData = publicKeyExists( id, trustedKeyRing() );
        return report->askUserToAcceptVerificationFailed( filedesc, exportKey( trustedKeyData, trustedKeyRing() ), context );
      }
    }
    else
    {
      PublicKeyData generalKeyData( publicKeyExists( id, generalKeyRing() ) );
      if ( generalKeyData )
      {
        PublicKey key( exportKey( generalKeyData, generalKeyRing() ) );
        MIL << "Exported key " << id << " to " << key.path() << endl;
        MIL << "Key " << id << " " << key.name() << " is not trusted" << endl;

        // ok the key is not trusted, ask the user to trust it or not
        KeyRingReport::KeyTrust reply = report->askUserToAcceptKey( key, context );
        if ( reply == KeyRingReport::KEY_TRUST_TEMPORARILY ||
            reply == KeyRingReport::KEY_TRUST_AND_IMPORT )
        {
          MIL << "User wants to trust key " << id << " " << key.name() << endl;
          //dumpFile( unKey.path() );

          Pathname whichKeyring;
          if ( reply == KeyRingReport::KEY_TRUST_AND_IMPORT )
          {
            MIL << "User wants to import key " << id << " " << key.name() << endl;
            importKey( key, true );
            whichKeyring = trustedKeyRing();
          }
          else
            whichKeyring = generalKeyRing();

          // emit key added
          if ( verifyFile( file, signature, whichKeyring ) )
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

  std::list<PublicKey> KeyRing::Impl::publicKeys( const Pathname & keyring )
  {
    const std::list<PublicKeyData> & keys( publicKeyData( keyring ) );
    std::list<PublicKey> ret;

    for_( it, keys.begin(), keys.end() )
    {
      PublicKey key( exportKey( *it, keyring ) );
      ret.push_back( key );
      MIL << "Found key " << key << endl;
    }
    return ret;
  }

  void KeyRing::Impl::importKey( const Pathname & keyfile, const Pathname & keyring )
  {
    if ( ! PathInfo( keyfile ).isExist() )
      // TranslatorExplanation first %s is key name, second is keyring name
      ZYPP_THROW(KeyRingException(boost::str(boost::format(
          _("Tried to import not existent key %s into keyring %s"))
          % keyfile.asString() % keyring.asString())));

    const char* argv[] =
    {
      GPG_BINARY,
      "--import",
      "--homedir", keyring.asString().c_str(),
      "--no-default-keyring",
      "--quiet",
      "--no-tty",
      "--no-greeting",
      "--no-permission-warning",
      "--status-fd", "1",
      keyfile.asString().c_str(),
      NULL
    };

    ExternalProgram prog( argv,ExternalProgram::Discard_Stderr, false, -1, true );
    prog.close();
  }

  void KeyRing::Impl::deleteKey( const std::string & id, const Pathname & keyring )
  {
    const char* argv[] =
    {
      GPG_BINARY,
      "--delete-keys",
      "--homedir", keyring.asString().c_str(),
      "--no-default-keyring",
      "--yes",
      "--quiet",
      "--no-tty",
      "--batch",
      "--status-fd", "1",
      id.c_str(),
      NULL
    };

    ExternalProgram prog( argv,ExternalProgram::Discard_Stderr, false, -1, true );

    int code = prog.close();
    if ( code )
      ZYPP_THROW(Exception(_("Failed to delete key.")));
    else
      MIL << "Deleted key " << id << " from keyring " << keyring << endl;
  }


  std::string KeyRing::Impl::readSignatureKeyId( const Pathname & signature )
  {
    if ( ! PathInfo( signature ).isFile() )
      ZYPP_THROW(Exception(boost::str(boost::format(
          _("Signature file %s not found"))% signature.asString())));

    MIL << "Determining key id if signature " << signature << endl;
    // HACK create a tmp keyring with no keys
    filesystem::TmpDir dir( _base_dir, "fake-keyring" );

    const char* argv[] =
    {
      GPG_BINARY,
      "--homedir", dir.path().asString().c_str(),
      "--no-default-keyring",
      "--quiet",
      "--no-tty",
      "--no-greeting",
      "--batch",
      "--status-fd", "1",
      signature.asString().c_str(),
      NULL
    };

    ExternalProgram prog( argv,ExternalProgram::Discard_Stderr, false, -1, true );

    std::string line;
    int count = 0;

    str::regex rxNoKey( "^\\[GNUPG:\\] NO_PUBKEY (.+)\n$" );
    std::string id;
    for( line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
    {
      //MIL << "[" << line << "]" << endl;
      str::smatch what;
      if( str::regex_match( line, what, rxNoKey ) )
      {
        if ( what.size() >= 1 )
	{
          id = what[1];
	  break;
	}
        //dumpRegexpResults( what );
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

  bool KeyRing::Impl::verifyFile( const Pathname & file, const Pathname & signature, const Pathname & keyring )
  {
    const char* argv[] =
    {
      GPG_BINARY,
      "--verify",
      "--homedir", keyring.asString().c_str(),
      "--no-default-keyring",
      "--quiet",
      "--no-tty",
      "--batch",
      "--no-greeting",
      "--status-fd", "1",
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

    ExternalProgram prog( argv,ExternalProgram::Discard_Stderr, false, -1, true );

    return ( prog.close() == 0 ) ? true : false;
  }

  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : KeyRing
  //
  ///////////////////////////////////////////////////////////////////

  KeyRing::KeyRing( const Pathname & baseTmpDir )
  : _pimpl( new Impl( baseTmpDir ) )
  {}

  KeyRing::~KeyRing()
  {}


  void KeyRing::importKey( const PublicKey & key, bool trusted )
  { _pimpl->importKey( key, trusted ); }

  void KeyRing::multiKeyImport( const Pathname & keyfile_r, bool trusted_r )
  { _pimpl->multiKeyImport( keyfile_r, trusted_r ); }

  std::string KeyRing::readSignatureKeyId( const Pathname & signature )
  { return _pimpl->readSignatureKeyId( signature ); }

  void KeyRing::deleteKey( const std::string & id, bool trusted )
  { _pimpl->deleteKey( id, trusted ); }

  std::list<PublicKey> KeyRing::publicKeys()
  { return _pimpl->publicKeys(); }

  std:: list<PublicKey> KeyRing::trustedPublicKeys()
  { return _pimpl->trustedPublicKeys(); }

  std::list<PublicKeyData> KeyRing::publicKeyData()
  { return _pimpl->publicKeyData(); }

  std::list<PublicKeyData> KeyRing::trustedPublicKeyData()
  { return _pimpl->trustedPublicKeyData(); }

  bool KeyRing::verifyFileSignatureWorkflow(
      const Pathname & file,
      const std::string filedesc,
      const Pathname & signature,
      const KeyContext & keycontext )
  { return _pimpl->verifyFileSignatureWorkflow( file, filedesc, signature, keycontext ); }

  bool KeyRing::verifyFileSignature( const Pathname & file, const Pathname & signature )
  { return _pimpl->verifyFileSignature( file, signature ); }

  bool KeyRing::verifyFileTrustedSignature( const Pathname & file, const Pathname & signature )
  { return _pimpl->verifyFileTrustedSignature( file, signature ); }

  void KeyRing::dumpPublicKey( const std::string & id, bool trusted, std::ostream & stream )
  { _pimpl->dumpPublicKey( id, trusted, stream ); }

  PublicKey KeyRing::exportPublicKey( const PublicKeyData & keyData )
  { return _pimpl->exportPublicKey( keyData ); }

  PublicKey KeyRing::exportTrustedPublicKey( const PublicKeyData & keyData )
  { return _pimpl->exportTrustedPublicKey( keyData ); }

  bool KeyRing::isKeyTrusted( const std::string & id )
  { return _pimpl->isKeyTrusted( id ); }

  bool KeyRing::isKeyKnown( const std::string & id )
  { return _pimpl->isKeyKnown( id ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
