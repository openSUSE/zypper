/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/KeyRing.h
 *
*/
#ifndef ZYPP_KEYRING_H
#define ZYPP_KEYRING_H

#include <iosfwd>
#include <map>
#include <list>
#include <set>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/Flags.h"
#include "zypp/Callback.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Locale.h"
#include "zypp/PublicKey.h"
#include "zypp/KeyContext.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(KeyRing);

  /** Callbacks from signature verification workflow.
   *
   * Per default all methods answer \c false. This may be canged
   * by calling \ref KeyRing::setDefaultAccept.
   * \code
   *  KeyRing::setDefaultAccept( KeyRing::ACCEPT_UNSIGNED_FILE | KeyRing::ACCEPT_VERIFICATION_FAILED );
   * \endcode
   * \see \ref KeyRing
  */
  struct KeyRingReport : public callback::ReportBase
  {
    /**
     * User reply options for the askUserToTrustKey callback.
     *
     * \param filedes Name of the file (repo alias) or filename if not available
     */
    enum KeyTrust
    {
      /**
       * User has chosen not to trust the key.
       */
      KEY_DONT_TRUST = 0,
      /**
       * This basically means, we knew the key, but it was not trusted. User
       * has chosen to continue, but not import the key.
       */
      KEY_TRUST_TEMPORARILY,
      /**
       * Import the key.
       * This means saving the key in the trusted database so next run it will appear as trusted.
       * Nothing to do with KEY_TRUST_TEMPORARILY, as you CAN trust a key without importing it,
       * basically you will be asked every time again.
       * There are programs who prefer to manage the trust keyring on their own and use trustKey
       * without importing it into rpm.
       */
      KEY_TRUST_AND_IMPORT
    };

    /**
     * Ask user to trust and/or import the key to trusted keyring.
     * \see KeyTrust
     */
    virtual KeyTrust askUserToAcceptKey( const PublicKey &key, const KeyContext &keycontext = KeyContext() );

    /** Informal callback showing the trusted key that will be used for verification. */
    virtual void infoVerify( const std::string & file_r, const PublicKeyData & keyData_r, const KeyContext &keycontext = KeyContext() );

    virtual bool askUserToAcceptUnsignedFile( const std::string &file, const KeyContext &keycontext = KeyContext() );

    /**
     * we DONT know the key, only its id, but we have never seen it, the difference
     * with trust key is that if you dont have it, you can't import it later.
     * The answer means continue yes or no?
     *
     */
    virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &id, const KeyContext &keycontext = KeyContext() );

    /**
     * The file \ref filedesc is signed but the verification failed
     *
     * \param filedesc Filename or its description.
     */
    virtual bool askUserToAcceptVerificationFailed( const std::string &file, const PublicKey &key, const KeyContext &keycontext = KeyContext() );

  };

  struct KeyRingSignals : public callback::ReportBase
  {
    virtual void trustedKeyAdded( const PublicKey &/*key*/ )
    {}
    virtual void trustedKeyRemoved( const PublicKey &/*key*/ )
    {}
  };

  class KeyRingException : public Exception
   {
     public:
       /** Ctor taking message.
      * Use \ref ZYPP_THROW to throw exceptions.
        */
       KeyRingException()
       : Exception( "Bad Key Exception" )
       {}
       /** Ctor taking message.
        * Use \ref ZYPP_THROW to throw exceptions.
        */
       KeyRingException( const std::string & msg_r )
       : Exception( msg_r )
       {}
       /** Dtor. */
       virtual ~KeyRingException() throw() {};
   };

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : KeyRing
  //
  /** Gpg key handling.
   *
  */
  class KeyRing : public base::ReferenceCounted, private base::NonCopyable
  {
    friend std::ostream & operator<<( std::ostream & str, const KeyRing & obj );

    public:
      /** \name Default answers in verification workflow.
       * Per default all answers are \c false.
       */
      //@{
      /** \ref DefaultAccept flags (\see \ref base::Flags) are used to
       *  define the default callback answers during signature verification.
       * \code
       *  KeyRing::setDefaultAccept( KeyRing::ACCEPT_UNSIGNED_FILE | ACCEPT_VERIFICATION_FAILED );
       * \endcode
       * \see \ref KeyRingReport.
       */
      enum DefaultAcceptBits
      {
        ACCEPT_NOTHING             = 0x0000,
        ACCEPT_UNSIGNED_FILE       = 0x0001,
        ACCEPT_UNKNOWNKEY          = 0x0002,
        TRUST_KEY_TEMPORARILY      = 0x0004,
        TRUST_AND_IMPORT_KEY       = 0x0008,
        ACCEPT_VERIFICATION_FAILED = 0x0010,
      };
      ZYPP_DECLARE_FLAGS( DefaultAccept, DefaultAcceptBits );

      /** Get the active accept bits. */
      static DefaultAccept defaultAccept();

      /** Set the active accept bits. */
      static void setDefaultAccept( DefaultAccept value_r );
     //@}

  public:
    /** Implementation  */
    class Impl;

  public:
    /** Default ctor */
    KeyRing(const Pathname &baseTmpDir);

    /**
     * imports a key from a file.
     * throw if key was not imported
     */
    void importKey( const PublicKey &key, bool trusted = false);

    /** Initial import from \ref RpmDb. */
    void multiKeyImport( const Pathname & keyfile_r, bool trusted_r = false );

    void dumpTrustedPublicKey( const std::string &id, std::ostream &stream )
    { dumpPublicKey(id, true, stream); }

    void dumpUntrustedPublicKey( const std::string &id, std::ostream &stream )
    { dumpPublicKey(id, false, stream); }

    void dumpPublicKey( const std::string &id, bool trusted, std::ostream &stream );

    /** Export a public key identified by its key data. */
    PublicKey exportPublicKey( const PublicKeyData & keyData );

    /** Export a trusted public key identified by its key data. */
    PublicKey exportTrustedPublicKey( const PublicKeyData & keyData );

    /**
     * reads the public key id from a signature
     */
    std::string readSignatureKeyId( const Pathname &signature );

    /**
     * true if the key id is trusted
     */
    bool isKeyTrusted( const std::string &id );

    /**
     * true if the key id is knows, that means
     * at least exist on the untrusted keyring
     */
    bool isKeyKnown( const std::string &id );

    /**
     * removes a key from the keyring.
     * If trusted is true, Remove it from trusted keyring too.
     */
    void deleteKey( const std::string &id, bool trusted =  false );

    /**
     * Get a list of public keys in the keyring (incl. ASCII armored keys in tmpfiles)
     */
    std::list<PublicKey> publicKeys();

    /**
     * Get a list of trusted public keys in the keyring (incl. ASCII armored keys in tmpfiles)
     */
    std::list<PublicKey> trustedPublicKeys();

    /**
     * Get a list of public key data in the keyring (key data only)
     */
    std::list<PublicKeyData> publicKeyData();

    /**
     * Get a list of trusted public key data in the keyring (key data only)
     */
    std::list<PublicKeyData> trustedPublicKeyData();

    /**
     * Follows a signature verification interacting with the user.
     * The bool returned depends on user decision to trust or not.
     *
     * To propagate user decisions, either connect to the \ref KeyRingReport
     * or use its static methods to set the desired defaults.
     *
     * \code
     * struct KeyRingReportReceive : public callback::ReceiveReport<KeyRingReport>
     * {
     *   KeyRingReportReceive() { connect(); }
     *
     *   // Overload the virtual methods to return the appropriate values.
     *   virtual bool askUserToAcceptUnsignedFile( const std::string &file );
     *   ...
     * };
     * \endcode
     *
     * \param file Path of the file to be verified
     * \param filedesc Description of the file (to give the user some context)
     * \param signature Signature to verify the file against
     *
     * \see \ref KeyRingReport
     */
    bool verifyFileSignatureWorkflow(
        const Pathname &file,
        const std::string filedesc,
        const Pathname &signature,
        const KeyContext &keycontext = KeyContext());


    /**
     * Verifies a file against a signature, with no user interaction
     *
     * \param file Path of the file to be verified
     * \param signature Signature to verify the file against
     */
    bool verifyFileSignature( const Pathname &file, const Pathname &signature );

    bool verifyFileTrustedSignature( const Pathname &file, const Pathname &signature );

    /** Dtor */
    ~KeyRing();

  private:
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates KeyRing Stream output */
  inline std::ostream & operator<<( std::ostream & str, const KeyRing & /*obj*/ )
  {
    //return str << obj.asString();
    return str;
  }

  /** \relates KeyRing::DefaultAccept  */
  ZYPP_DECLARE_OPERATORS_FOR_FLAGS( KeyRing::DefaultAccept );

  ///////////////////////////////////////////////////////////////////

  namespace target
  {
    namespace rpm
    {
      /** Internal connection to rpm database. Not for public use. */
      struct KeyRingSignals : public ::zypp::KeyRingSignals
      {};
    }
  }

 /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_KEYRING_H
