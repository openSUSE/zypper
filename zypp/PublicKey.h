/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PublicKey.h
 *
*/
#ifndef ZYPP_PUBLICKEY_H
#define ZYPP_PUBLICKEY_H

#include <iosfwd>
#include <map>
#include <list>
#include <set>
#include <string>

#include "zypp/base/Iterable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"
#include "zypp/Edition.h"
#include "zypp/Date.h"

struct _gpgme_key;
struct _gpgme_subkey;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace filesystem
  {
    class TmpFile;
  }
  class PublicKeyData;
  class KeyManagerCtx;

  ///////////////////////////////////////////////////////////////////
  /// \class BadKeyException
  /// \brief Exception thrown when the supplied key is not a valid gpg key
  ///////////////////////////////////////////////////////////////////
  class BadKeyException : public Exception
  {
    public:
      /** Ctor taking message.
     * Use \ref ZYPP_THROW to throw exceptions.
       */
      BadKeyException()
      : Exception( "Bad Key Exception" )
      {}

      Pathname keyFile() const
      { return _keyfile; }

      /** Ctor taking message.
       * Use \ref ZYPP_THROW to throw exceptions.
       */
      BadKeyException( const std::string & msg_r, const Pathname &keyfile = Pathname() )
      : Exception( msg_r ), _keyfile(keyfile)
      {}
      /** Dtor. */
      virtual ~BadKeyException() throw() {};
    private:
      Pathname _keyfile;
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class PublicSubkeyData
  /// \brief Class representing a GPG Public Keys subkeys.
  /// \see \ref PublicKeyData.
  ///////////////////////////////////////////////////////////////////
  class PublicSubkeyData
  {
  public:
    /** Default constructed: empty data. */
    PublicSubkeyData();

    ~PublicSubkeyData();

    /** Whether this contains valid data (not default constructed). */
    explicit operator bool() const;

  public:
    /** Subkey ID. */
    std::string id() const;

    /** Creation date. */
    Date created() const;

    /** Expiry date, or \c Date() if the key never expires. */
    Date expires() const;

    /**  Whether the key has expired. */
    bool expired() const;

    /** Number of days (24h) until the key expires (or since it exired).
     * A value of \c 0 means the key will expire within the next 24h.
     * Negative values indicate the key has expired less than \c N days ago.
     * For keys without expiration date \c INT_MAX is returned.
     */
    int daysToLive() const;

    /** Simple string representation.
     * Encodes \ref id, \ref created and \ref expires
     * \code
     * 640DB551 2016-04-12 [expires: 2019-04-12]
     * \endcode
     */
    std::string asString() const;

  private:
    class Impl;
    RWCOW_pointer<Impl> _pimpl;
    friend class PublicKeyData;
    friend std::ostream & dumpOn( std::ostream & str, const PublicKeyData & obj );
    PublicSubkeyData(const _gpgme_subkey *rawSubKeyData);
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PublicSubkeyData Stream output */
  inline std::ostream & operator<<( std::ostream & str, const PublicSubkeyData & obj )
  { return str << obj.asString(); }

  ///////////////////////////////////////////////////////////////////
  /// \class PublicKeyData
  /// \brief Class representing one GPG Public Keys data.
  /// \ref PublicKeyData are provided e.g. by a \ref PublicKey or
  /// a \ref KeyRing. \ref PublicKeyData are usually easier to
  /// retrieve and sufficient unless you actually need an ASCII
  /// armored version of the key placed in a tempfile. In this
  /// case use \ref PublicKey.
  ///////////////////////////////////////////////////////////////////
  class PublicKeyData
  {
  public:
    /** Default constructed: empty data. */
    PublicKeyData();

    ~PublicKeyData();

    /** Whether this contains valid data (not default constructed). */
    explicit operator bool() const;

  public:
    /** Key ID. */
    std::string id() const;

    /** Key name.  */
    std::string name() const;

    /** Key fingerprint.*/
    std::string fingerprint() const;

    /** Creation / last modification date (latest selfsig). */
    Date created() const;

    /** Expiry date, or \c Date() if the key never expires. */
    Date expires() const;

    /**  Whether the key has expired. */
    bool expired() const;

    /** Number of days (24h) until the key expires (or since it exired).
     * A value of \c 0 means the key will expire within the next 24h.
     * Negative values indicate the key has expired less than \c N days ago.
     * For keys without expiration date \c INT_MAX is returned.
     */
    int daysToLive() const;

    /** * Expiry info in a human readable form.
     * The exipry daye plus an annotation if the key has expired, or will
     * expire within 90 days.
     * \code
     * (does not expire)
     * Tue May 11 13:37:33 CEST 2010
     * Tue May 11 13:37:33 CEST 2010 (expires in 90 days)
     * Tue May 11 13:37:33 CEST 2010 (expires in 1 day)
     * Tue May 11 13:37:33 CEST 2010 (expires within 24h)
     * Tue May 11 13:37:33 CEST 2010 (EXPIRED)
     * \endcode
     */
    std::string expiresAsString() const;

    /** Gpg-pubkey version as computed by rpm (trailing 8 byte \ref id) */
    std::string gpgPubkeyVersion() const;

    /** Gpg-pubkey release as computed by rpm (hexencoded \ref created) */
    std::string gpgPubkeyRelease() const;

    /** Gpg-pubkey \ref Edition built from version and release.*/
    Edition gpgPubkeyEdition() const
    { return Edition( gpgPubkeyVersion(), gpgPubkeyRelease() ); }

    /** Simple string representation.
     * Encodes \ref id, \ref gpgPubkeyRelease, \ref name and \ref fingerprint.
     * \code
     * [E3A5C360307E3D54-4be01a65] [SuSE Package Signing Key <build@suse.de>] [4E98E67519D98DC7362A5990E3A5C360307E3D54]
     * \endcode
     */
    std::string asString() const;

  public:
    typedef const PublicSubkeyData * SubkeyIterator;

    /** Whether \ref subkeys is not empty. */
    bool hasSubkeys() const;

    /** Iterate any subkeys. */
    Iterable<SubkeyIterator> subkeys() const;

    /** Whether \a id_r is the \ref id of the primary key or of a subkey. */
    bool providesKey( const std::string & id_r ) const;

  private:
    class Impl;
    RWCOW_pointer<Impl> _pimpl;

    friend class KeyManagerCtx;
    static PublicKeyData fromGpgmeKey(_gpgme_key *data);

    PublicKeyData(shared_ptr<Impl> data);
    friend std::ostream & dumpOn( std::ostream & str, const PublicKeyData & obj );
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PublicKeyData Stream output */
  inline std::ostream & operator<<( std::ostream & str, const PublicKeyData & obj )
  { return str << obj.asString(); }

  /** \relates PublicKeyData Detailed stream output */
  std::ostream & dumpOn( std::ostream & str, const PublicKeyData & obj );

  /** \relates PublicKeyData Equal based on  fingerprint anf creation date. */
  bool operator==( const PublicKeyData & lhs, const PublicKeyData & rhs );

  /** \relates PublicKeyData NotEqual. */
  inline bool operator!=( const PublicKeyData & lhs, const PublicKeyData & rhs )
  { return !( lhs == rhs ); }

  ///////////////////////////////////////////////////////////////////
  /// \class PublicKey
  /// \brief Class representing one GPG Public Key (PublicKeyData + ASCII armored in a tempfile).
  ///
  /// If you don't need the ASCII armored version of the key stored in
  /// a tempfile, using \ref PublicKeyData might be sufficient.
  ///
  /// \note In case the ASCII armored blob actually contains multiple
  /// keys, the \b last keys data are made available via the API. The
  /// additional keys data are made available via \ref hiddenKeys.
  ///////////////////////////////////////////////////////////////////
  class PublicKey
  {
  public:
    /** Implementation  */
    class Impl;

  public:
    /** Default ctor. */
    PublicKey();

    /** Ctor taking the key from a file.
     *
     * This is quite expensive, as a copy of the file is created and
     * used. If you can construct PublicKey from a \ref filesystem::TmpFile,
     * this prevents copying.
     *
     * \throws when data does not make a key
     */
    explicit PublicKey( const Pathname & keyFile_r );

    /** Ctor reading the key from a \ref TmpFile.
     *
     * PublicKey holds a reference on the TmpFile providing the key.
     *
     * \throws when data does not make a key
     */
    explicit PublicKey( const filesystem::TmpFile & sharedFile_r );

    ~PublicKey();

  public:
    /** The public keys data (\see \ref PublicKeyData).*/
    const PublicKeyData & keyData() const;

    typedef PublicKeyData::SubkeyIterator SubkeyIterator;

    bool isValid() const
    { return ! ( id().empty() || fingerprint().empty() ); }

    std::string id() const;			//!< \see \ref PublicKeyData
    std::string name() const;			//!< \see \ref PublicKeyData
    std::string fingerprint() const;		//!< \see \ref PublicKeyData
    Date created() const;			//!< \see \ref PublicKeyData
    Date expires() const;			//!< \see \ref PublicKeyData
    std::string expiresAsString() const;	//!< \see \ref PublicKeyData
    bool expired() const;			//!< \see \ref PublicKeyData
    int daysToLive() const;			//!< \see \ref PublicKeyData
    std::string gpgPubkeyVersion() const;	//!< \see \ref PublicKeyData
    std::string gpgPubkeyRelease() const;	//!< \see \ref PublicKeyData
    std::string asString() const;		//!< \see \ref PublicKeyData

    Edition gpgPubkeyEdition() const		///!< \see \ref PublicKeyData
    { return keyData().gpgPubkeyEdition(); }

    bool hasSubkeys() const			///!< \see \ref PublicKeyData
    { return keyData().hasSubkeys(); }

    Iterable<SubkeyIterator> subkeys() const	///!< \see \ref PublicKeyData
    { return keyData().subkeys(); }

    bool providesKey( const std::string & id_r ) const	///!< \see \ref PublicKeyData
    { return keyData().providesKey( id_r ); }

  public:
    /** File containig the ASCII armored key. */
    Pathname path() const;

    /** Additional keys data in case the ASCII armored blob containes multiple keys. */
    const std::list<PublicKeyData> & hiddenKeys() const;

  public:
    bool operator==( const PublicKey & rhs ) const;
    bool operator==( const std::string & sid ) const;

  private:
    friend class KeyRing;
    /** KeyRing ctor: No need to parse file if KeyRing already had valid KeyData. */
    PublicKey( const filesystem::TmpFile & sharedFile_r, const PublicKeyData & keyData_r );
    /** KeyRing ctor: Legacy callback APIs take PublicKey, but just need the PublicKeyData No need to export to file. */
    explicit PublicKey( const PublicKeyData & keyData_r );

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PublicKey Stream output */
  inline std::ostream & operator<<( std::ostream & str, const PublicKey & obj )
  { return str << obj.asString(); }

  /** \relates PublicKey Detailed stream output */
  std::ostream & dumpOn( std::ostream & str, const PublicKey & obj );

 /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PUBLICKEY_H
