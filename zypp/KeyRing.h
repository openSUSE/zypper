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
     * The file \ref filedesc is unsigned
     * \param filedesc Name of the file (repo alias) or filename if not available
     */
    virtual bool askUserToAcceptUnsignedFile( const std::string &filedesc );

    /**
     * we DONT know the key, only its id, but we have never seen it, the difference
     * with trust key is that if you dont have it, you can't import it later.
     * The answer means continue yes or no?
     *
     * \param filedes Name of the file (repo alias) or filename if not available
     */
    virtual bool askUserToAcceptUnknownKey( const std::string &filedesc, const std::string &id );

    /**
     * This basically means, we know the key, but it is not trusted, Continue
     * yes or no?. Nothing else is performed (import, etc)
     */
    virtual bool askUserToTrustKey( const PublicKey &key);


    /**
     * Import the key.
     * This means saving the key in the trusted database so next run it will appear as trusted.
     * Nothing to do with trustKey, as you CAN trust a key without importing it,
     * basically you will be asked every time again.
     * There are programs who prefer to manage the trust keyring on their own and use trustKey
     * without importing it into rpm.
     *
     */
    virtual bool askUserToImportKey( const PublicKey &key);

    /**
     * The file \ref filedesc is signed but the verification failed
     *
     * \param filedesc Name of the file (repo alias) or filename if not available
     */
    virtual bool askUserToAcceptVerificationFailed( const std::string &filedesc, const PublicKey &key );

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
       *  KeyRingReport::setDefaultAccept( KeyRing::ACCEPT_UNSIGNED_FILE | ACCEPT_VERIFICATION_FAILED );
       * \endcode
       * \see \ref KeyRingReport.
       */
      enum DefaultAcceptBits
      {
        ACCEPT_NOTHING             = 0x0000,
        ACCEPT_UNSIGNED_FILE       = 0x0001,
        ACCEPT_UNKNOWNKEY          = 0x0002,
        TRUST_KEY                  = 0x0004,
        IMPORT_KEY                 = 0x0008,
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
    //explicit
    //KeyRing(const Pathname &general_kr, const Pathname &trusted_kr);

    /**
     * imports a key from a file.
     * throw if key was not imported
     */
    void importKey( const PublicKey &key, bool trusted = false);

    void dumpTrustedPublicKey( const std::string &id, std::ostream &stream )
    { dumpPublicKey(id, true, stream); }

    void dumpUntrustedPublicKey( const std::string &id, std::ostream &stream )
    { dumpPublicKey(id, false, stream); }

    void dumpPublicKey( const std::string &id, bool trusted, std::ostream &stream );

    /**
     * reads the public key id from a signature
     */
    std::string readSignatureKeyId( const Pathname &signature );

    /**
     * true if the key id is trusted
     */
    bool isKeyTrusted( const std::string &id);

    /**
     * true if the key id is knows, that means
     * at least exist on the untrusted keyring
     */
    bool isKeyKnown( const std::string &id );

    /**
     * removes a key from the keyring.
     * If trusted is true, Remove it from trusted keyring too.
     */
    void deleteKey( const std::string &id, bool trusted =  false);

    /**
     * Get a list of public keys in the keyring
     */
    std::list<PublicKey> publicKeys();

    /**
     * Get a list of trusted public keys in the keyring
     */
    std::list<PublicKey> trustedPublicKeys();

    /**
     * Get a list of public key ids in the keyring
     */
    std::list<std::string> publicKeyIds();

    /**
     * Get a list of trusted public key ids in the keyring
     */
    std::list<std::string> trustedPublicKeyIds();

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
    bool verifyFileSignatureWorkflow( const Pathname &file, const std::string filedesc, const Pathname &signature);

    /**
     * Verifies a file against a signature, with no user interaction
     *
     * \param file Path of the file to be verified
     * \param signature Signature to verify the file against
     */
    bool verifyFileSignature( const Pathname &file, const Pathname &signature);

    bool verifyFileTrustedSignature( const Pathname &file, const Pathname &signature);

    /** Dtor */
    ~KeyRing();

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
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
