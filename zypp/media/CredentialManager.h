/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/CredentialManager.h
 *
 */
#ifndef ZYPP_MEDIA_CREDENTIALMANAGER_H
#define ZYPP_MEDIA_CREDENTIALMANAGER_H

#include <set>

#include "zypp/Pathname.h"
#include "zypp/media/MediaUserAuth.h"

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  class Url;

  //////////////////////////////////////////////////////////////////////
  namespace media
  { ////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME : CredManagerOptions
  //
  /**
   * \todo configurable cred file locations
   */
  struct CredManagerOptions
  {
    CredManagerOptions(const Pathname & rootdir = "");

    Pathname globalCredFilePath;
    Pathname userCredFilePath;
    Pathname customCredFileDir;
  };
  //////////////////////////////////////////////////////////////////////

  // comparator for CredentialSet
  struct AuthDataComparator
  {
    static const url::ViewOption vopt;
    bool operator()(const AuthData_Ptr & lhs, const AuthData_Ptr & rhs);
  };

  //////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME : CredentialManager
  //
  /**
   * \todo better method names
   * \todo delete(AuthData) method
   */
  class CredentialManager
  {
  public:
    typedef std::set<AuthData_Ptr, AuthDataComparator> CredentialSet;
    typedef CredentialSet::size_type                   CredentialSize;
    typedef CredentialSet::const_iterator              CredentialIterator;


    CredentialManager(const CredManagerOptions & opts = CredManagerOptions());

    ~CredentialManager()
    {}

  public:
    /**
     * Get credentials for the specified \a url.
     *
     * If the URL contains also username, it will be used to find the match
     * for this user (in case mutliple are available).
     *
     * \param url URL to find credentials for.
     * \return Pointer to retrieved authentication data on success or an empty
     *         AuthData_Ptr otherwise.
     * \todo return a copy instead?
     */
    AuthData_Ptr getCred(const Url & url);

    /**
     * Read credentials from a file.
     */
    AuthData_Ptr getCredFromFile(const Pathname & file);

    /**
     * Add new global credentials.
     */
    void addGlobalCred(const AuthData & cred);

    /**
     * Add new user credentials.
     */
    void addUserCred(const AuthData & cred);

    /**
     * Add new credentials with user callbacks.
     *
     * If the cred->url() contains 'credentials' query parameter, the
     * credentials will be automatically saved to the specified file using the
     * \ref saveInFile() method.
     *
     * Otherwise a callback will be called asking whether to save to custom
     * file, or to global or user's credentials catalog.
     *
     * \todo Currently no callback is called, credentials are automatically
     *       saved to user's credentials.cat if no 'credentials' parameter
     *       has been specified
     */
    void addCred(const AuthData & cred);

    /**
     * Saves any unsaved credentials added via \ref addUserCred() or
     * \a addGlobalCred() methods.
     */
    void save();

    /**
     * Saves given \a cred to global credentials file.
     *
     * \note Use this method to add just one piece of credentials. To add
     *       multiple items at once, use addGlobalCred() followed
     *       by save()
     */
    void saveInGlobal(const AuthData & cred);

    /**
     * Saves given \a cred to user's credentials file.
     *
     * \note Use this method to add just one piece of credentials. To add
     *       multiple items at once, use addUserCred() followed
     *       by save()
     */
    void saveInUser(const AuthData & cred);

    /**
     * Saves given \a cred to user specified credentials file.
     *
     * If the credFile path is absolute, it will be saved at that precise
     * location. If \a credFile is just a filename, it will be saved
     * in \ref CredManagerOptions::customCredFileDir. Otherwise the current
     * working directory will be prepended to the file path.
     */
    void saveInFile(const AuthData &, const Pathname & credFile);

    /**
     * Remove all global or user credentials from memory and disk.
     *
     * \param global  Whether to remove global or user credentials.
     */
    void clearAll(bool global = false);


    CredentialIterator credsGlobalBegin() const;
    CredentialIterator credsGlobalEnd()   const;
    CredentialSize     credsGlobalSize()  const;
    bool               credsGlobalEmpty() const;

    CredentialIterator credsUserBegin() const;
    CredentialIterator credsUserEnd()   const;
    CredentialSize     credsUserSize()  const;
    bool               credsUserEmpty() const;

    class Impl;
  private:
    RW_pointer<Impl> _pimpl;
  };
  //////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////
  } // media
  //////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // zypp
//////////////////////////////////////////////////////////////////////

#endif /* ZYPP_MEDIA_CREDENTIALMANAGER_H */

