/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PublicKey.cc
 *
*/
#include <climits>

#include <iostream>
#include <vector>

#include <zypp/base/Gettext.h>
#include <zypp/base/String.h>
#include <zypp/base/Regex.h>
#include <zypp/PublicKey.h>
#include <zypp/ExternalProgram.h>
#include <zypp/TmpPath.h>
#include <zypp/PathInfo.h>
#include <zypp/base/Exception.h>
#include <zypp/base/LogTools.h>
#include <zypp/Date.h>
#include <zypp/KeyManager.h>
#include <zypp/Target.h>
#include <zypp/target/rpm/RpmDb.h>
#include <zypp/ZYppFactory.h>

#include <gpgme.h>

using std::endl;

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::gpg"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace
  {
    inline bool isExpired( const Date & expires_r )
    { return( expires_r && expires_r < Date::now() ); }

    inline int hasDaysToLive( const Date & expires_r )
    {
      if ( expires_r )
      {
        Date exp( expires_r - Date::now() );
        int ret = exp / Date::day;
        if ( exp < 0 ) ret -= 1;
        return ret;
      }
      return INT_MAX;
    }

    inline std::string expiresDetail( const Date & expires_r )
    {
      str::Str str;
      if ( ! expires_r )
      {
        // translators: an annotation to a gpg keys expiry date
        str << _("does not expire");
      }
      else if ( isExpired( expires_r ) )
      {
        // translators: an annotation to a gpg keys expiry date: "expired: 1999-04-12"
        str << ( str::Format(_("expired: %1%") ) % expires_r.printDate() );
      }
      else
      {
        // translators: an annotation to a gpg keys expiry date: "expires: 2111-04-12"
        str << ( str::Format(_("expires: %1%") ) % expires_r.printDate() );
      }
      return str;
    }

    inline std::string expiresDetailVerbose( const Date & expires_r )
    {
      if ( !expires_r )
      { // translators: an annotation to a gpg keys expiry date
        return _("(does not expire)");
      }
      std::string ret( expires_r.asString() );
      int ttl( hasDaysToLive( expires_r ) );
      if ( ttl <= 90 )
      {
        ret += " ";
        if ( ttl < 0 )
        { // translators: an annotation to a gpg keys expiry date
          ret += _("(EXPIRED)");
        }
        else if ( ttl == 0 )
        { // translators: an annotation to a gpg keys expiry date
          ret += _("(expires within 24h)");
        }
        else
        { // translators: an annotation to a gpg keys expiry date
          ret += str::form( PL_("(expires in %d day)", "(expires in %d days)", ttl ), ttl );
        }
      }
      return ret;
    }

    inline std::string keyAlgoName( const gpgme_subkey_t & key_r )
    {
      std::string ret;
      if ( const char * n = ::gpgme_pubkey_algo_name( key_r->pubkey_algo ) )
        ret = str::Str() << n << ' ' << key_r->length;
      else
        ret = "?";
      return ret;
    }

    inline bool shorterIsSuffixCI( const std::string & lhs, const std::string & rhs )
    {
      if ( lhs.size() >= rhs.size() )
        return str::endsWithCI( lhs, rhs );
      return str::endsWithCI( rhs, lhs );
    }
  } //namespace
  ///////////////////////////////////////////////////////////////////


  ///////////////////////////////////////////////////////////////////
  /// \class PublicSubkeyData::Impl
  /// \brief  PublicSubkeyData implementation.
  ///////////////////////////////////////////////////////////////////

  struct PublicSubkeyData::Impl
  {
    std::string _id;
    Date        _created;
    Date        _expires;

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl();

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const;
  };

  shared_ptr<zypp::PublicSubkeyData::Impl> PublicSubkeyData::Impl::nullimpl()
  {
    static shared_ptr<Impl> _nullimpl( new Impl );
    return _nullimpl;
  }

  zypp::PublicSubkeyData::Impl *PublicSubkeyData::Impl::clone() const
  {
    return new Impl( *this );
  }

  ///////////////////////////////////////////////////////////////////
  /// class PublicSubkeyData
  ///////////////////////////////////////////////////////////////////

  PublicSubkeyData::PublicSubkeyData()
    : _pimpl( Impl::nullimpl() )
  {}

  PublicSubkeyData::PublicSubkeyData(const _gpgme_subkey *rawSubKeyData)
    : _pimpl (new Impl)
  {
    _pimpl->_created = zypp::Date(rawSubKeyData->timestamp);
    _pimpl->_expires = zypp::Date(rawSubKeyData->expires);
    _pimpl->_id = str::asString(rawSubKeyData->keyid);
  }

  PublicSubkeyData::~PublicSubkeyData()
  {}

  PublicSubkeyData::operator bool() const
  { return !_pimpl->_id.empty(); }

  std::string PublicSubkeyData::id() const
  { return _pimpl->_id; }

  Date PublicSubkeyData::created() const
  { return _pimpl->_created; }

  Date PublicSubkeyData::expires() const
  { return _pimpl->_expires; }

  bool PublicSubkeyData::expired() const
  { return isExpired( _pimpl->_expires ); }

  int PublicSubkeyData::daysToLive() const
  { return hasDaysToLive( _pimpl->_expires ); }

  std::string PublicSubkeyData::asString() const
  {
    return str::Str() << id() << " " << created().printDate() << " [" << expiresDetail( expires() ) << "]";
  }

  ///////////////////////////////////////////////////////////////////
  /// \class PublicKeySignatureData::Impl
  /// \brief  PublicKeySignatureData implementation.
  ///////////////////////////////////////////////////////////////////

  struct PublicKeySignatureData::Impl
  {
    std::string _keyid;
    std::string _name;
    Date        _created;
    Date        _expires;

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl();

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const;
  };

  shared_ptr<zypp::PublicKeySignatureData::Impl> PublicKeySignatureData::Impl::nullimpl()
  {
    static shared_ptr<Impl> _nullimpl( new Impl );
    return _nullimpl;
  }

  zypp::PublicKeySignatureData::Impl *PublicKeySignatureData::Impl::clone() const
  {
    return new Impl( *this );
  }

  ///////////////////////////////////////////////////////////////////
  /// class PublicKeySignatureData
  ///////////////////////////////////////////////////////////////////

  PublicKeySignatureData::PublicKeySignatureData()
    : _pimpl( Impl::nullimpl() )
  {}

  PublicKeySignatureData::PublicKeySignatureData(const _gpgme_key_sig *rawKeySignatureData)
    : _pimpl (new Impl)
  {
    _pimpl->_keyid = str::asString(rawKeySignatureData->keyid);
    _pimpl->_name = str::asString(rawKeySignatureData->uid);
    _pimpl->_created = zypp::Date(rawKeySignatureData->timestamp);
    _pimpl->_expires = zypp::Date(rawKeySignatureData->expires);
  }

  PublicKeySignatureData::~PublicKeySignatureData()
  {}

  PublicKeySignatureData::operator bool() const
  { return !_pimpl->_keyid.empty(); }

  std::string PublicKeySignatureData::id() const
  { return _pimpl->_keyid; }

  std::string PublicKeySignatureData::name() const
  { return _pimpl->_name; }

  Date PublicKeySignatureData::created() const
  { return _pimpl->_created; }

  Date PublicKeySignatureData::expires() const
  { return _pimpl->_expires; }

  bool PublicKeySignatureData::expired() const
  { return isExpired( _pimpl->_expires ); }

  int PublicKeySignatureData::daysToLive() const
  { return hasDaysToLive( _pimpl->_expires ); }

  std::string PublicKeySignatureData::asString() const
  {
    std::string nameStr;
    if (!name().empty()) {
      nameStr = str::Str() << name() << " ";
    }
    else {
      nameStr = "[User ID not found] ";
    }
    return str::Str() << nameStr
                      << id() << " "
                      << created().printDate()
                      << " [" << expiresDetail( expires() ) << "]";
  }

  bool PublicKeySignatureData::inTrustedRing() const
  { return getZYpp()->keyRing()->isKeyTrusted(id()); }

  bool PublicKeySignatureData::inKnownRing() const
  { return getZYpp()->keyRing()->isKeyKnown(id()); }

  ///////////////////////////////////////////////////////////////////
  /// \class PublicKeyData::Impl
  /// \brief  PublicKeyData implementation.
  ///////////////////////////////////////////////////////////////////
  ///
  struct PublicKeyData::Impl
  {
    std::string _id;
    std::string _name;
    std::string _fingerprint;
    std::string _algoName;
    Date        _created;
    Date        _expires;

    std::vector<PublicSubkeyData> _subkeys;
    std::vector<PublicKeySignatureData> _signatures;

  public:
    bool hasSubkeyId( const std::string & id_r ) const;

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl();
    static shared_ptr<Impl> fromGpgmeKey(gpgme_key_t rawData);

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const;
  };

  bool PublicKeyData::Impl::hasSubkeyId( const std::string &id_r) const
  {
    bool ret = false;
    for ( const PublicSubkeyData & sub : _subkeys ) {
      if ( shorterIsSuffixCI( sub.id(), id_r ) ) {
        ret = true;
        break;
      }
    }
    return ret;
  }

  shared_ptr<PublicKeyData::Impl> PublicKeyData::Impl::nullimpl()
  {
    static shared_ptr<Impl> _nullimpl( new Impl );
    return _nullimpl;
  }

  shared_ptr<PublicKeyData::Impl> PublicKeyData::Impl::fromGpgmeKey(gpgme_key_t rawData)
  {
    //gpgme stores almost nothing in the top level key
    //the information we look for is stored in the subkey, where subkey[0]
    //is always the primary key
    gpgme_subkey_t sKey = rawData->subkeys;
    if (sKey) {
      shared_ptr<PublicKeyData::Impl> data(new Impl);
      //libzypp expects the date of the latest signature on the first uid
      if ( rawData->uids && rawData->uids->signatures ) {
        data->_created = zypp::Date(rawData->uids->signatures->timestamp);
        // bsc#1179222: The keyring does not order the signatures when multiple
        // versions of the same key are imported. We take the last signature here,
        // the one GPGME_EXPORT_MODE_MINIMAL will later use in export.
        for ( auto t = rawData->uids->signatures->next; t; t = t->next ) {
            if (t->keyid != nullptr) {
              data->_signatures.push_back(PublicKeySignatureData(t));
            }

          if ( t->timestamp > data->_created )
            data->_created = t->timestamp;
        }
      }
      else
        data->_created = zypp::Date(sKey->timestamp);

      data->_expires = zypp::Date(sKey->expires);
      data->_fingerprint = str::asString(sKey->fpr);
      data->_algoName = keyAlgoName( sKey );
      data->_id = str::asString(sKey->keyid);

      //get the primary user ID
      if (rawData->uids) {
        data->_name = str::asString(rawData->uids->uid);
      }

      //the rest of the keys
      sKey = sKey->next;
      while (sKey) {
        data->_subkeys.push_back( PublicSubkeyData(sKey) );
        sKey = sKey->next;
      }
      return data;
    }
    return nullimpl();
  }

  zypp::PublicKeyData::Impl *PublicKeyData::Impl::clone() const
  {
    return new Impl( *this );
  }

  ///////////////////////////////////////////////////////////////////
  /// class PublicKeyData
  ///////////////////////////////////////////////////////////////////

  PublicKeyData::PublicKeyData()
    : _pimpl( Impl::nullimpl() )
  {}

  PublicKeyData::PublicKeyData(shared_ptr<Impl> data)
    : _pimpl( data )
  {}

  PublicKeyData::~PublicKeyData()
  {}

  PublicKeyData PublicKeyData::fromGpgmeKey(_gpgme_key *data)
  { return PublicKeyData(Impl::fromGpgmeKey(data)); }

  PublicKeyData::operator bool() const
  { return !_pimpl->_fingerprint.empty(); }

  std::string PublicKeyData::id() const
  { return _pimpl->_id; }

  std::string PublicKeyData::name() const
  { return _pimpl->_name; }

  std::string PublicKeyData::fingerprint() const
  { return _pimpl->_fingerprint; }

  std::string PublicKeyData::algoName() const
  {  return _pimpl->_algoName; }

  Date PublicKeyData::created() const
  { return _pimpl->_created; }

  Date PublicKeyData::expires() const
  { return _pimpl->_expires; }

  bool PublicKeyData::expired() const
  { return isExpired( _pimpl->_expires ); }

  int PublicKeyData::daysToLive() const
  { return hasDaysToLive( _pimpl->_expires ); }

  std::string PublicKeyData::expiresAsString() const
  { return expiresDetailVerbose( _pimpl->_expires ); }

  std::string PublicKeyData::gpgPubkeyVersion() const
  { return _pimpl->_id.empty() ? _pimpl->_id : str::toLower( _pimpl->_id.substr(8,8) ); }

  std::string PublicKeyData::gpgPubkeyRelease() const
  { return _pimpl->_created ? str::hexstring( _pimpl->_created ).substr(2) : std::string(); }

  std::string PublicKeyData::rpmName() const
  { return str::Format( "gpg-pubkey-%1%-%2%" ) % gpgPubkeyVersion() % gpgPubkeyRelease();  }

  std::string PublicKeyData::asString() const
  {
    if ( not *this )
      return "[NO_KEY]";

    str::Str str;
    str << "[" << _pimpl->_id << "-" << gpgPubkeyRelease();
    for ( auto && sub : _pimpl->_subkeys )
      str << ", " << sub.id();
    return str << "] [" << _pimpl->_name.c_str() << "] [" << expiresDetail( _pimpl->_expires ) << "]";
  }

  bool PublicKeyData::hasSubkeys() const
  { return !_pimpl->_subkeys.empty(); }

  Iterable<PublicKeyData::SubkeyIterator> PublicKeyData::subkeys() const
  { return makeIterable( &(*_pimpl->_subkeys.begin()), &(*_pimpl->_subkeys.end()) ); }

  Iterable<PublicKeyData::KeySignatureIterator> PublicKeyData::signatures() const
  { return makeIterable( &(*_pimpl->_signatures.begin()), &(*_pimpl->_signatures.end()) ); }

  bool PublicKeyData::providesKey( const std::string & id_r ) const
  {
    if ( not isSafeKeyId( id_r ) )
      return( id_r.size() == 8 && str::endsWithCI( _pimpl->_id, id_r ) );

    if ( str::endsWithCI( _pimpl->_fingerprint, id_r ) )
      return true;

    return _pimpl->hasSubkeyId( id_r );
  }

  PublicKeyData::AsciiArt PublicKeyData::asciiArt() const
  { return AsciiArt( fingerprint(), algoName() ); }

  std::ostream & dumpOn( std::ostream & str, const PublicKeyData & obj )
  {
    str << "[" << obj.name() << "]" << endl;
    str << "  fpr " << obj.fingerprint() << endl;
    str << "   id " << obj.id() << endl;
    str << "  alg " << obj.algoName() << endl;
    str << "  cre " << Date::ValueType(obj.created()) << ' ' << obj.created() << endl;
    str << "  exp " << Date::ValueType(obj.expires()) << ' ' << obj.expiresAsString() << endl;
    str << "  ttl " << obj.daysToLive() << endl;
    for ( auto && sub : obj._pimpl->_subkeys )
      str << "  sub " << sub << endl;
    str << "  rpm " << obj.gpgPubkeyVersion() << "-" << obj.gpgPubkeyRelease() << endl;
    return str;
  }

  bool operator==( const PublicKeyData & lhs, const PublicKeyData & rhs )
  { return ( lhs.fingerprint() == rhs.fingerprint() && lhs.created() == rhs.created() ); }


  ///////////////////////////////////////////////////////////////////
  /// \class PublicKey::Impl
  /// \brief  PublicKey implementation.
  ///////////////////////////////////////////////////////////////////
  struct PublicKey::Impl
  {
    Impl()
    {}

    Impl( const Pathname & keyFile_r )
    : _dontUseThisPtrDirectly( new filesystem::TmpFile )
    {
      PathInfo info( keyFile_r );
      MIL << "Taking pubkey from " << keyFile_r << " of size " << info.size() << " and sha1 " << filesystem::checksum(keyFile_r, "sha1") << endl;

      if ( !info.isExist() )
        ZYPP_THROW(Exception("Can't read public key from " + keyFile_r.asString() + ", file not found"));

      if ( filesystem::hardlinkCopy( keyFile_r, path() ) != 0 )
        ZYPP_THROW(Exception("Can't copy public key data from " + keyFile_r.asString() + " to " +  path().asString() ));

      readFromFile();
    }

    Impl( const filesystem::TmpFile & sharedFile_r )
      : _dontUseThisPtrDirectly( new filesystem::TmpFile( sharedFile_r ) )
    { readFromFile(); }

    // private from keyring
    Impl( const filesystem::TmpFile & sharedFile_r, const PublicKeyData & keyData_r )
      : _dontUseThisPtrDirectly( new filesystem::TmpFile( sharedFile_r ) )
      , _keyData( keyData_r )
    {
      if ( ! keyData_r )
      {
        WAR << "Invalid PublicKeyData supplied: scanning from file" << endl;
        readFromFile();
      }
    }

    // private from keyring
    Impl( const PublicKeyData & keyData_r )
      : _keyData( keyData_r )
    {}

    public:
      const PublicKeyData & keyData() const
      { return _keyData; }

      Pathname path() const
      { return( /*the one and only intended use*/_dontUseThisPtrDirectly ? _dontUseThisPtrDirectly->path() : Pathname() ); }

      const std::list<PublicKeyData> & hiddenKeys() const
      { return _hiddenKeys; }

    protected:
      void readFromFile()
      {
        PathInfo info( path() );
        MIL << "Reading pubkey from " << info.path() << " of size " << info.size() << " and sha1 " << filesystem::checksum(info.path(), "sha1") << endl;

        std::list<PublicKeyData> keys = KeyManagerCtx::createForOpenPGP().readKeyFromFile( path() );
        switch ( keys.size() )
        {
          case 0:
            ZYPP_THROW( BadKeyException( "File " + path().asString() + " doesn't contain public key data" , path() ) );
            break;

          case 1:
            // ok.
            _keyData = keys.back();
            _hiddenKeys.clear();
            break;

          default:
            WAR << "File " << path().asString() << " contains multiple keys: " <<  keys << endl;
            _keyData = keys.back();
            keys.pop_back();
            _hiddenKeys.swap( keys );
            break;
        }

        MIL << "Read pubkey from " << info.path() << ": " << _keyData << endl;
      }

    private:
      shared_ptr<filesystem::TmpFile> _dontUseThisPtrDirectly; // shared_ptr ok because TmpFile itself is a refernce type (no COW)
      PublicKeyData		_keyData;
      std::list<PublicKeyData>  _hiddenKeys;

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
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  // class PublicKey
  ///////////////////////////////////////////////////////////////////
  PublicKey::PublicKey()
    : _pimpl( Impl::nullimpl() )
  {}

  PublicKey::PublicKey( const Pathname & file )
  : _pimpl( new Impl( file ) )
  {}

  PublicKey::PublicKey( const filesystem::TmpFile & sharedfile )
  : _pimpl( new Impl( sharedfile ) )
  {}

  PublicKey::PublicKey( const filesystem::TmpFile & sharedfile, const PublicKeyData & keyData_r )
  : _pimpl( new Impl( sharedfile, keyData_r ) )
  {}

  PublicKey::PublicKey( const PublicKeyData & keyData_r )
  : _pimpl( new Impl( keyData_r ) )
  {}

  PublicKey::~PublicKey()
  {}

  PublicKey PublicKey::noThrow( const Pathname & keyFile_r )
  try { return PublicKey( keyFile_r ); } catch(...) { return PublicKey(); }

  const PublicKeyData & PublicKey::keyData() const
  { return _pimpl->keyData(); }

  Pathname PublicKey::path() const
  { return _pimpl->path(); }

  const std::list<PublicKeyData> & PublicKey::hiddenKeys() const
  { return _pimpl->hiddenKeys(); }

  bool PublicKey::fileProvidesKey( const std::string & id_r ) const
  {
    if ( providesKey( id_r ) )
      return true;
    for ( const auto & keydata : hiddenKeys() ) {
      if ( keydata.providesKey( id_r ) )
        return true;
    }
    return false;
  }

  std::string PublicKey::id() const
  { return keyData().id(); }

  std::string PublicKey::name() const
  { return keyData().name(); }

  std::string PublicKey::fingerprint() const
  { return keyData().fingerprint(); }

  std::string PublicKey::algoName() const
  { return keyData().algoName(); }

  Date PublicKey::created() const
  { return keyData().created(); }

  Date PublicKey::expires() const
  { return keyData().expires(); }

  bool PublicKey::expired() const
  { return keyData().expired(); }

  int PublicKey::daysToLive() const
  { return keyData().daysToLive(); }

  std::string PublicKey::expiresAsString() const
  { return keyData().expiresAsString(); }

  std::string PublicKey::gpgPubkeyVersion() const
  { return keyData().gpgPubkeyVersion(); }

  std::string PublicKey::gpgPubkeyRelease() const
  { return keyData().gpgPubkeyRelease(); }

  std::string PublicKey::asString() const
  { return keyData().asString(); }

  std::string PublicKey::rpmName() const
  { return keyData().rpmName(); }

  bool PublicKey::operator==( const PublicKey & rhs ) const
  { return rhs.keyData() == keyData(); }

  bool PublicKey::operator==( const std::string & sid ) const
  { return ( isSafeKeyId( sid ) || sid.size() == 8 ) && str::endsWithCI( fingerprint(), sid ); }

  std::ostream & dumpOn( std::ostream & str, const PublicKey & obj )
  { return dumpOn( str, obj.keyData() ); }




  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
