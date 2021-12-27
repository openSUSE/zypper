/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/KeyManager.h
 *
*/
#ifndef ZYPP_KEYMANAGER_H
#define ZYPP_KEYMANAGER_H

#include <zypp/base/PtrTypes.h>
#include <zypp/Pathname.h>
#include <zypp/PublicKey.h>
#include <zypp-core/ByteArray.h>

#include <memory>

namespace zypp
{

 ///////////////////////////////////////////////////////////////////
 /// \class KeyManagerCtx::KeyManagerCtx
 /// \brief Wrapper for GPGME
 ///
 /// Encapsulates all calls to the gpgme library, each instance
 /// represents a context of operations on the specified keyring.
 ///////////////////////////////////////////////////////////////////
class KeyManagerCtx
{
    public:
        /** Creates a new KeyManagerCtx for PGP using a volatile temp. homedir/keyring.
         *
         * Mainly used with methods, which need a context but do not need a keyring
         * (like \ref readKeyFromFile or \ref readSignatureFingerprints).
         *
         * \note The underlying keyring is intentionally NOT the users keyring.
         * Think of it as a volatile keyring whose content may get cleared anytime.
         *
         * \throws KeyRingException if context can not be created or set up
         */
        static KeyManagerCtx createForOpenPGP();

        /** Creates a new KeyManagerCtx for PGP using a custom homedir/keyring.
         *
         * \note If you explicitly pass an empty \c Pathname, no homedir/keyring
         * will be set and GPGME will use its defaults.
         *
         * \throws KeyRingException if context can not be created or set up
         */
        static KeyManagerCtx createForOpenPGP( const Pathname & keyring_r );

        /** Return the homedir/keyring. */
        Pathname homedir() const;

        /**  Returns a list of all public keys found in the current keyring */
        std::list<PublicKeyData> listKeys();

        /** Returns a list of all \a PublicKeyData found in \a file */
        std::list<PublicKeyData> readKeyFromFile(const Pathname & file);

        /** Tries to verify \a file using \a signature, returns true on success */
        bool verify(const Pathname & file, const Pathname & signature);

        /** Exports the key with \a id into the given \a stream, returns true on success */
        bool exportKey(const std::string & id, std::ostream & stream);

        /** Tries to import a key from \a keyfile, returns true on success */
        bool importKey(const Pathname & keyfile);

        /** Tries to import a key from \a buffer, returns true on success */
        bool importKey(const ByteArray & keydata);

        /** Tries to delete a key specified by \a id, returns true on success */
        bool deleteKey (const std::string & id);

        /** Reads all fingerprints from the \a signature file , returns a list of all found fingerprints */
        std::list<std::string> readSignatureFingerprints(const Pathname & signature);

        /** Reads all fingerprints from the \a buffer, returns a list of all found fingerprints */
        std::list<std::string> readSignatureFingerprints( const ByteArray & keyData );

    private:
      KeyManagerCtx();

      class Impl;
      RW_pointer<Impl> _pimpl; ///< Pointer to implementation
};

}


#endif
