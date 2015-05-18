/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/FileChecker.h
 *
*/
#ifndef ZYPP_FILECHECKER_H
#define ZYPP_FILECHECKER_H

#include <iosfwd>
#include <list>
#include "zypp/base/DefaultIntegral.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Function.h"
#include "zypp/PathInfo.h"
#include "zypp/CheckSum.h"
#include "zypp/KeyContext.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class PublicKey;

  /**
   * Functor signature used to check files.
   * \param file File to check.
   *
   * \throws FileCheckException when the file does not
   * validate and the user don't want to continue.
   */
  typedef function<void ( const Pathname &file )> FileChecker;

  class FileCheckException : public Exception
  {
  public:
    FileCheckException(const std::string &msg)
      : Exception(msg)
    {}
  };

  class CheckSumCheckException : public FileCheckException
  {
  public:
    CheckSumCheckException(const std::string &msg)
      : FileCheckException(msg)
    {}
  };

  class SignatureCheckException : public FileCheckException
  {
  public:
    SignatureCheckException(const std::string &msg)
      : FileCheckException(msg)
    {}
  };

  /**
   * Built in file checkers
   */

  /**
   * \short Checks for a valid checksum and interacts with the user.
   */
   class ChecksumFileChecker
   {
   public:
     typedef CheckSumCheckException ExceptionType;
     /**
      * Constructor.
      * \param checksum Checksum that validates the file
      */
     ChecksumFileChecker( const CheckSum &checksum );
     /**
      * \short Try to validate the file
      * \param file File to validate.
      *
      * \throws CheckSumCheckException if validation fails
      */
     void operator()( const Pathname &file ) const;
   private:
     CheckSum _checksum;
   };

   /**
    * \short Checks for the validity of a signature
    */
   class SignatureFileChecker
   {
     public:
       typedef SignatureCheckException ExceptionType;
       typedef function<void ( const SignatureFileChecker & checker,  const Pathname & file )> OnSigValidated;

     public:
      /**
      * Constructor.
      * \param signature Signature that validates the file
      */
      SignatureFileChecker( const Pathname &signature );

      /**
      * Default Constructor.
      * \short Signature for unsigned files
      * Use it when you dont have a signature but you want
      * to check the user to accept an unsigned file.
      */
      SignatureFileChecker();

      /**
       * Set context for this checker.
       *
       * Use this method if you're not adding the key (with context) via
       * one of the addPublicKey methods. The addPublicKey method overwrites
       * the context.
       */
      void setKeyContext(const KeyContext & keycontext);

      /** Return the current context */
      const KeyContext & keyContext() const
      { return _context; }

      /** Return whether the last file passed to \ref operator() was accepted.
       * If this is \ref false \ref operator() was not invoked or threw a
       * \ref SignatureCheckException.
       */
      bool fileAccepted() const
      { return _fileAccepted; }

      /** Return whether the last file passed to \ref operator() was actually sucessfully verified.
       * If this is \c false but \ref fileAccepted, the file was accepted due to user interaction or
       * global settings, but the signature was not verified.
       */
      bool fileValidated() const
      { return _fileValidated; }

      /**
       * add a public key to the list of known keys
       */
      void addPublicKey( const PublicKey & publickey, const KeyContext & keycontext = KeyContext());
      /** \overload Convenience taking the public keys pathname. */
      void addPublicKey( const Pathname & publickey, const KeyContext & keycontext = KeyContext());

      /**
       * Calls \ref KeyRing::verifyFileSignatureWorkflow to verify the file.
       *
       * Keep in mind the the workflow may return \c true (file accepted) due to user interaction
       * or global defaults even if a signature was not actually sucessfully verified. Whether a
       * signature was actually sucessfully verified can be determined by checking \ref fileValidated
       * which is invokes IFF a signature for this file actually validated.
       *
       * \param file File to validate.fileValidated
       *
       * \throws SignatureCheckException if validation fails
       */
      void operator()( const Pathname &file ) const;

     protected:
      Pathname _signature;
      KeyContext _context;
      mutable DefaultIntegral<bool,false> _fileAccepted;
      mutable DefaultIntegral<bool,false> _fileValidated;
   };

   /**
   * \short Checks for nothing
   * Used as the default checker
   */
   class NullFileChecker
   {
   public:
     void operator()( const Pathname &file )  const;
   };

   /**
    * \short Checker composed of more checkers.
    *
    * Allows to create a checker composed of various
    * checkers altothether. It will only
    * validate if all the checkers validate.
    *
    * \code
    * CompositeFileChecker com;
    * com.add(checker1);
    * com.add(checker2);
    * fetcher.enqueue(location, com);
    * \endcode
    */
   class CompositeFileChecker
   {
   public:
     void add( const FileChecker &checker );
    /**
     * \throws FileCheckException if validation fails
     */
     void operator()( const Pathname &file ) const;

     int checkersSize() const { return _checkers.size(); }
   private:
     std::list<FileChecker> _checkers;
   };

  /** \relates FileChecker Stream output */
  std::ostream & operator<<( std::ostream & str, const FileChecker & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_FILECHECKER_H
