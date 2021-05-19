/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/KeyRingContexts.h
 */
#ifndef ZYPP_KEYRINGCONTEXTS_H
#define ZYPP_KEYRINGCONTEXTS_H

#include <iosfwd>
#include <string>
#include <set>

#include <zypp/base/PtrTypes.h>

#include <zypp/Pathname.h>
#include <zypp/KeyContext.h>

///////////////////////////////////////////////////////////////////
namespace zypp::keyring
{
  ///////////////////////////////////////////////////////////////////
  /// I/O context for KeyRing::verifyFileSignatureWorkflow.
  ///////////////////////////////////////////////////////////////////
  class VerifyFileContext
  {
  public:
    /** Ctor. */
    VerifyFileContext();
    /** Ctor may take file to verify and detatched signature. */
    explicit VerifyFileContext( Pathname file_r );
    /** Ctor may take file to verify and detatched signature. */
    VerifyFileContext( Pathname file_r, Pathname signature_r );

    ~VerifyFileContext();

    /** File to verify. */
    const Pathname & file() const;
    void file( Pathname file_r );

    /** Detached signature or empty. */
    const Pathname & signature() const;
    void signature( Pathname signature_r );

    /** Short name for file (default: basename). */
    std::string shortFile() const;
    void shortFile( std::string shortFile_r );

    /** \ref KeyContext passed to callbacks */
    const KeyContext & keyContext() const;
    void keyContext( KeyContext keyContext_r );

    /** List of key safe key ids to import IFF \ref fileValidated. */
    typedef std::set<std::string> BuddyKeys;
    const BuddyKeys & buddyKeys() const;
    void addBuddyKey( std::string sid_r );

    /** \name Results provided by \ref KeyRing::verifyFileSignatureWorkflow.
     */
    //@{
    /** Reset all result values to safe defaults. */
    void resetResults();

    /** May return \c true due to user interaction or global defaults even if the signature was not actually verified. */
    bool fileAccepted() const;
    void fileAccepted( bool yesno_r) ;

    /** Whether the signature was actually successfully verified. */
    bool fileValidated() const;
    void fileValidated( bool yesno_r );

    /** The id of the gpg key which signed the file. */
    const std::string & signatureId() const;
    void signatureId( std::string signatureId_r );

    /** Whether the \ref SignatureId is in the trusted keyring (not temp. trusted). */
    bool signatureIdTrusted() const;
    void signatureIdTrusted( bool yesno_r );
    //@}
  public:
    struct Impl;	///< Implementation
  private:
    RWCOW_pointer<Impl> _pimpl;	///< Pointer to implementation
  };

  /** \relates VerifyFileContext Stream output */
  std::ostream & operator<<( std::ostream & str, const VerifyFileContext & obj );

} // namespace zypp::keyring
///////////////////////////////////////////////////////////////////
#endif // ZYPP_KEYRINGCONTEXTS_H
