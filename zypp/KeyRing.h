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
#include "zypp/Callback.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Locale.h"
#include "zypp/PublicKey.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(KeyRing);

  struct KeyRingReport : public callback::ReportBase
  {
    virtual bool askUserToAcceptUnsignedFile( const std::string &file );
    virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &id );
    virtual bool askUserToTrustKey( const PublicKey &key);
    virtual bool askUserToImportKey( const PublicKey &key);
    virtual bool askUserToAcceptVerificationFailed( const std::string &file, const PublicKey &key );
  };
  
  struct KeyRingSignals : public callback::ReportBase
  {
    virtual void trustedKeyAdded( const KeyRing &keyring, const PublicKey &key )
    {}
    virtual void trustedKeyRemoved( const KeyRing &keyring, const PublicKey &key )
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
  /** Class that represent a text and multiple translations.
  */
  class KeyRing  : public base::ReferenceCounted, private base::NonCopyable
  {
    friend std::ostream & operator<<( std::ostream & str, const KeyRing & obj );

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
     * removes a key from the keyring.
     * If trusted is true, Remove it from trusted keyring too.
     */
    void deleteKey( const std::string &id, bool trusted =  false);

    std::list<PublicKey> publicKeys();
    std::list<PublicKey> trustedPublicKeys();

    /**
     * Follows a signature verification interacting with the user.
     * The boolr eturned depends on user desicion to trust or not.
     */
    bool verifyFileSignatureWorkflow( const Pathname &file, const std::string filedesc, const Pathname &signature);
    bool verifyFileSignature( const Pathname &file, const Pathname &signature);
    bool verifyFileTrustedSignature( const Pathname &file, const Pathname &signature);

/** Dtor */
    ~KeyRing();

  public:

    /** Synonym for \ref text */
    //std::string asString() const
    //{}

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates KeyRing Stream output */
  inline std::ostream & operator<<( std::ostream & str, const KeyRing & obj )
  {
    //return str << obj.asString();
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_KEYRING_H
