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

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace filesystem
  {
    class TmpFile;
  }
  class Date;


  /**
   * Exception thrown when the supplied key is
   * not a valid gpg key
   */
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
  /// \class PublicKey
  /// \brief Class representing one GPG Public Key.
  /// If a key file actually contains multiple keys, the last one
  /// is taken.
  ///////////////////////////////////////////////////////////////////
  class PublicKey
  {
    friend std::ostream & operator<<( std::ostream & str, const PublicKey & obj );

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
    explicit
    PublicKey( const Pathname & file );

    /** Ctor reading the key from a \ref TmpFile.
     *
     * PublicKey holds a reference on the TmpFile providing the key.
     *
     * \throws when data does not make a key
     */
    explicit
    PublicKey( const filesystem::TmpFile & sharedfile );

    ~PublicKey();

    bool isValid() const
    { return ( ! id().empty() && ! fingerprint().empty() && !path().empty() ); }

    std::string asString() const;
    std::string id() const;
    std::string name() const;
    std::string fingerprint() const;

    /** Version rpm would assign to this key if imported into the rpm database.
     * Rpm uses the lowercased trailing 8 byte from \ref id as \c version, and the
     * creations dates lowercased hexadecimal representation as \c release.
     * \see \ref gpgPubkeyRelease
     * \code
     * [zypp OBS Project <zypp@build.opensuse.org>]
     *   fpr 47D7CE1DD600935B3B90365733D38EBC7FB7F464
     *    id 33D38EBC7FB7F464           <-- trailing 8 byte
     *   cre Thu Mar 13 19:15:40 2008   <-- converted to hex
     *   exp Sat May 22 20:15:40 2010
     * ]
     *
     * Converting the creation date to its hexadecimal representation:
     * $ bc <<<"obase=16;$(date -d 'Thu Mar 13 19:15:40 2008' +%s)"
     * 47D96F4C
     *
     * Rpms name for this key: gpg-pubkey-7fb7f464-47d96f4c
     * \endcode
     */
    std::string gpgPubkeyVersion() const;

    /** Release rpm would assign to this key if imported into the rpm database.
     * This is the creations dates hexadecimal representation as \c release lowercased.
     * \see \ref gpgPubkeyVersion
     */
    std::string gpgPubkeyRelease() const;

    /**
     * Date when the key was created.
     */
    Date created() const;

    /**
     * Date when the key expires.
     * If the key never expires the date is Date() (i.e. 0 seconds since the epoch (1.1.1970))
     */
    Date expires() const;

    /**
     * Expiry info in a human readable form.
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

    /** Whether the key has expired. */
    bool expired() const;

    /** Number of days (24h) until the key expires (or since it exired).
     * A value of \c 0 means the key will expire within the next 24h.
     * Negative values indicate the key has expired less than \c N days ago.
     * For keys without expiration date \c INT_MAX is returned.
     */
    int daysToLive() const;

    Pathname path() const;

    bool operator==( PublicKey b ) const;
    bool operator==( std::string sid ) const;

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
