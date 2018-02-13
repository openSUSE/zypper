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

#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"
#include "zypp/PublicKey.h"
#include "zypp/ExternalProgram.h"
#include "zypp/TmpPath.h"
#include "zypp/PathInfo.h"
#include "zypp/base/Exception.h"
#include "zypp/base/LogTools.h"
#include "zypp/Date.h"
#include "zypp/KeyManager.h"

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
  /// \class PublicKeyData::Impl
  /// \brief  PublicKeyData implementation.
  ///////////////////////////////////////////////////////////////////
  ///
  struct PublicKeyData::Impl
  {
    std::string _id;
    std::string _name;
    std::string _fingerprint;
    Date        _created;
    Date        _expires;

    std::vector<PublicSubkeyData> _subkeys;

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

  bool PublicKeyData::Impl::hasSubkeyId(const std::string &id_r) const
  {
    bool ret = false;
    for ( const PublicSubkeyData & sub : _subkeys )
    {
      if ( sub.id() == id_r )
      {
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
    //gpgpme stores almost nothing in the top level key
    //the information we look for is stored in the subkey, where subkey[0]
    //is always the primary key
    gpgme_subkey_t sKey = rawData->subkeys;
    if (sKey) {
      shared_ptr<PublicKeyData::Impl> data(new Impl);
      //libzypp expects the date of the first signature on the first uid
      if(rawData->uids && rawData->uids->signatures)
        data->_created = zypp::Date(rawData->uids->signatures->timestamp);
      else
        data->_created = zypp::Date(sKey->timestamp);

      data->_expires = zypp::Date(sKey->expires);
      data->_fingerprint = str::asString(sKey->fpr);
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

  std::string PublicKeyData::asString() const
  {
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

  bool PublicKeyData::providesKey( const std::string & id_r ) const
  { return( id_r == _pimpl->_id || _pimpl->hasSubkeyId( id_r ) ); }

  std::ostream & dumpOn( std::ostream & str, const PublicKeyData & obj )
  {
    str << "[" << obj.name() << "]" << endl;
    str << "  fpr " << obj.fingerprint() << endl;
    str << "   id " << obj.id() << endl;
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
      std::string _initHomeDir()	///< readFromFile helper to prepare the 'gpg --homedir'
      { Pathname ret( zypp::myTmpDir() / "PublicKey" ); filesystem::assert_dir( ret ); return ret.asString(); }

      void readFromFile()
      {
        PathInfo info( path() );
        MIL << "Reading pubkey from " << info.path() << " of size " << info.size() << " and sha1 " << filesystem::checksum(info.path(), "sha1") << endl;

        //@TODO is this still required? KeyManagerCtx creates a homedir on the fly
        static std::string tmppath( _initHomeDir() );

        KeyManagerCtx::Ptr ctx = KeyManagerCtx::createForOpenPGP();
        if (!ctx || !ctx->setHomedir(tmppath)) {
          ZYPP_THROW( Exception( std::string("Can't read public key data: Setting the keyring path failed!")) );
        }

        std::list<PublicKeyData> keys = ctx->readKeyFromFile(path());
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

  const PublicKeyData & PublicKey::keyData() const
  { return _pimpl->keyData(); }

  Pathname PublicKey::path() const
  { return _pimpl->path(); }

  const std::list<PublicKeyData> & PublicKey::hiddenKeys() const
  { return _pimpl->hiddenKeys(); }

  std::string PublicKey::id() const
  { return keyData().id(); }

  std::string PublicKey::name() const
  { return keyData().name(); }

  std::string PublicKey::fingerprint() const
  { return keyData().fingerprint(); }

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

  bool PublicKey::operator==( const PublicKey & rhs ) const
  { return rhs.keyData() == keyData(); }

  bool PublicKey::operator==( const std::string & sid ) const
  { return sid == id(); }

  std::ostream & dumpOn( std::ostream & str, const PublicKey & obj )
  { return dumpOn( str, obj.keyData() ); }




  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
