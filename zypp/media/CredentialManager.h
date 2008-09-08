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

#include "zypp/Url.h"
#include "zypp/Pathname.h"
#include "zypp/media/MediaUserAuth.h"

//////////////////////////////////////////////////////////////////////
namespace zypp 
{ ////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  namespace media
  { ////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME : CredManagerOptions 
  //
  struct CredManagerOptions
  {
    CredManagerOptions(const Pathname & rootdir = "");

    Pathname globalCredFilePath;
    Pathname userCredFilePath;
  };
  //////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME : CredentialManager
  //
  /**
   * 
   */
  class CredentialManager
  {
  public:
    typedef std::set<AuthData_Ptr>        CredentialSet;
    typedef CredentialSet::size_type      CredentialSize;
    typedef CredentialSet::const_iterator CredentialIterator;


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
     */
    AuthData_Ptr getCred(const Url & url);


    void save(const AuthData &, bool global = false);

    /**
     * 
     */
    void saveInGlobal(const AuthData & cred);

    /**
     * 
     */
    void saveInUser(const AuthData & cred);

    /**
     * 
     */
    void saveIn(const AuthData &, const Pathname & credFile);

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

