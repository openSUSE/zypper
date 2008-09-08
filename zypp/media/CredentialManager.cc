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
#include <iostream>

#include "zypp/base/Function.h"
#include "zypp/base/Logger.h"

#include "zypp/media/CredentialFileReader.h"

#include "zypp/media/CredentialManager.h"

using namespace std;

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
  //////////////////////////////////////////////////////////////////////

  CredManagerOptions::CredManagerOptions(const Pathname & rootdir)
    : globalCredFilePath(rootdir / "/etc/zypp/credentials")
  {
    char * homedir = getenv("HOME");
    if (homedir)
      userCredFilePath = rootdir / homedir / ".zypp/credentials"; 
  }


  //////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME : CredentialManager::Impl 
  //
  struct CredentialManager::Impl
  {
    Impl(const CredManagerOptions & options);

    ~Impl()
    {}

    
    bool processCredentials(AuthData_Ptr & cred);

    AuthData_Ptr getCred(const Url & url);

    CredManagerOptions _options;

    CredentialSet _credsGlobal;
    CredentialSet _credsUser;
    CredentialSet _credsTmp;
  };
  //////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME : CredentialManager::Impl 
  //
  //////////////////////////////////////////////////////////////////////

  CredentialManager::Impl::Impl(const CredManagerOptions & options)
    : _options(options)
  {
    CredentialFileReader(
        _options.globalCredFilePath,
        bind(&Impl::processCredentials, this, _1));
    _credsGlobal = _credsTmp; _credsTmp.clear();
    DBG << "Got " << _credsGlobal.size() << " global records." << endl;

    if (!_options.userCredFilePath.empty())
    {
      CredentialFileReader(
          _options.userCredFilePath,
          bind(&Impl::processCredentials, this, _1));
      _credsUser = _credsTmp; _credsTmp.clear();
      DBG << "Got " << _credsUser.size() << " user records." << endl;
    }
  }


  bool CredentialManager::Impl::processCredentials(AuthData_Ptr & cred)
  {
    _credsTmp.insert(cred);
    return true;
  }

  static AuthData_Ptr findIn(const CredentialManager::CredentialSet & set,
                             const Url & url,
                             url::ViewOption vopt)
  {
    for(CredentialManager::CredentialIterator it = set.begin(); it != set.end(); ++it)
    {
      if (url.asString(vopt) == (*it)->url().asString(vopt))
        return *it;
    }
    
    return AuthData_Ptr();
  }


  AuthData_Ptr CredentialManager::Impl::getCred(const Url & url)
  {
    AuthData_Ptr result;

    // compare the urls via asString(), but ignore password
    // default url::ViewOption will take care of that.
    // operator==(Url,Url) compares the whole Url

    // if the wanted URL does not contain username, ignore that, too
    url::ViewOption vopt;
    if (url.getUsername().empty())
      vopt = vopt - url::ViewOption::WITH_USERNAME;

    // search in global credentials
    result = findIn(_credsGlobal, url, vopt);

    // search in home credentials
    if (!result)
      result = findIn(_credsUser, url, vopt);

    if (result)
      DBG << "Found credentials for '" << url << "':" << endl << *result;
    else
      DBG << "No credentials for '" << url << "'" << endl;

    return result;
  }


  //////////////////////////////////////////////////////////////////////
  //
  // CLASS NAME : CredentialManager 
  //
  //////////////////////////////////////////////////////////////////////

  CredentialManager::CredentialManager(const CredManagerOptions & opts)
    : _pimpl(new Impl(opts))
  {}

  AuthData_Ptr CredentialManager::getCred(const Url & url)
  { return _pimpl->getCred(url); }


  void CredentialManager::save(const AuthData & cred, bool global)
  { global ? saveInGlobal(cred) : saveInUser(cred); }

  void CredentialManager::saveInGlobal(const AuthData & cred)
  {
    //! \todo
  }

  void CredentialManager::saveInUser(const AuthData & cred)
  {
    //! \todo
  }

  void saveIn(const AuthData &, const Pathname & credFile)
  {
    //! \todo
  }


  CredentialManager::CredentialIterator CredentialManager::credsGlobalBegin() const
  { return _pimpl->_credsGlobal.begin(); }

  CredentialManager::CredentialIterator CredentialManager::credsGlobalEnd() const
  { return _pimpl->_credsGlobal.end(); }

  CredentialManager::CredentialSize CredentialManager::credsGlobalSize() const
  { return _pimpl->_credsGlobal.size(); }

  bool CredentialManager::credsGlobalEmpty() const
  { return _pimpl->_credsGlobal.empty(); }


  CredentialManager::CredentialIterator CredentialManager::credsUserBegin() const
  { return _pimpl->_credsUser.begin(); }

  CredentialManager::CredentialIterator CredentialManager::credsUserEnd() const
  { return _pimpl->_credsUser.end(); }

  CredentialManager::CredentialSize CredentialManager::credsUserSize() const
  { return _pimpl->_credsUser.size(); }

  bool CredentialManager::credsUserEmpty() const
  { return _pimpl->_credsUser.empty(); }


    ////////////////////////////////////////////////////////////////////
  } // media
  //////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // zypp
//////////////////////////////////////////////////////////////////////
