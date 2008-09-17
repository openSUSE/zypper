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
#include <fstream>

#include "zypp/base/Function.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Easy.h"
#include "zypp/PathInfo.h"

#include "zypp/media/CredentialFileReader.h"

#include "zypp/media/CredentialManager.h"

#define CUSTOM_CREDENTIALS_FILE_DIR "/etc/zypp/credentials.d"
#define GLOBAL_CREDENTIALS_FILE "/etc/zypp/credentials.cat" 
#define USER_CREDENTIALS_FILE   ".zypp/credentials.cat"

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
    : globalCredFilePath(rootdir / GLOBAL_CREDENTIALS_FILE)
    , customCredFileDir(rootdir / CUSTOM_CREDENTIALS_FILE_DIR)
  {
    char * homedir = getenv("HOME");
    if (homedir)
      userCredFilePath = rootdir / homedir / USER_CREDENTIALS_FILE;
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
    
    void init_globalCredentials();
    void init_userCredentials();

    bool processCredentials(AuthData_Ptr & cred);

    AuthData_Ptr getCred(const Url & url) const;
    AuthData_Ptr getCredFromFile(const Pathname & file);
    void saveGlobalCredentials();
    void saveUserCredentials();


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
    init_globalCredentials();
    init_userCredentials();
  }


  void CredentialManager::Impl::init_globalCredentials()
  {
    if (_options.globalCredFilePath.empty())
      DBG << "global cred file not known";
    else if (PathInfo(_options.globalCredFilePath).isExist())
    {
    /*  list<Pathname> entries;
      if (filesystem::readdir(entries, _options.globalCredFilePath, false) != 0)
        ZYPP_THROW(Exception("failed to read directory"));

      for_(it, entries.begin(), entries.end())*/

      CredentialFileReader(_options.globalCredFilePath,
          bind(&Impl::processCredentials, this, _1));
    }
    else
      DBG << "global cred file does not exist";

    _credsGlobal = _credsTmp; _credsTmp.clear();
    DBG << "Got " << _credsGlobal.size() << " global records." << endl;
  }


  void CredentialManager::Impl::init_userCredentials()
  {
    if (_options.userCredFilePath.empty())
      DBG << "user cred file not known";
    else if (PathInfo(_options.userCredFilePath).isExist())
    {
    /*  list<Pathname> entries;
      if (filesystem::readdir(entries, _options.userCredFilePath, false ) != 0)
        ZYPP_THROW(Exception("failed to read directory"));

      for_(it, entries.begin(), entries.end())*/
      CredentialFileReader(_options.userCredFilePath,
          bind(&Impl::processCredentials, this, _1));
    }
    else
      DBG << "user cred file does not exist";

    _credsUser = _credsTmp; _credsTmp.clear();
    DBG << "Got " << _credsUser.size() << " user records." << endl;
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


  AuthData_Ptr CredentialManager::Impl::getCred(const Url & url) const
  {
    AuthData_Ptr result;

    // compare the urls via asString(), but ignore password
    // default url::ViewOption will take care of that.
    // operator==(Url,Url) compares the whole Url

    // if the wanted URL does not contain username, ignore that, too
    url::ViewOption vopt;
//    if (url.getUsername().empty())
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


  AuthData_Ptr CredentialManager::Impl::getCredFromFile(const Pathname & file)
  {
    AuthData_Ptr result;
    
    Pathname credfile;
    if (file.absolute())
      // get from that file
      credfile = file;
    else
      // get from /etc/zypp/credentials.d
      credfile = _options.customCredFileDir / file;

    CredentialFileReader(credfile, bind(&Impl::processCredentials, this, _1));
    if (_credsTmp.empty())
      WAR << file << " does not contain valid credentials or is not readable." << endl;
    else
    {
      result = *_credsTmp.begin();
      _credsTmp.clear();
    }

    return result;
  }

  static void save_creds_in_file(
      const CredentialManager::CredentialSet creds,
      const Pathname & file/*,
       desired permissions*/)
  {
    filesystem::assert_dir(file.dirname());

    //! \todo set correct permissions
    std::ofstream fs(file.c_str());
    if (!fs)
      ZYPP_THROW(Exception("Can't open " + file.asString()));

    for_(it, creds.begin(), creds.end())
    {
      (*it)->dumpAsIniOn(fs);
      fs << endl;
    }
  }

  void  CredentialManager::Impl::saveGlobalCredentials()
  {
    save_creds_in_file(_credsGlobal, _options.globalCredFilePath);
  }
  
  void  CredentialManager::Impl::saveUserCredentials()
  {
    save_creds_in_file(_credsUser, _options.userCredFilePath);
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

  AuthData_Ptr CredentialManager::getCredFromFile(const Pathname & file)
  { return _pimpl->getCredFromFile(file); }

  void CredentialManager::save(const AuthData & cred, bool global)
  { global ? saveInGlobal(cred) : saveInUser(cred); }

  void CredentialManager::saveInGlobal(const AuthData & cred)
  {
    AuthData_Ptr c_ptr;
    c_ptr.reset(new AuthData(cred)); // FIX for child classes if needed
    _pimpl->_credsGlobal.insert(c_ptr); //! \todo avoid adding duplicates
    _pimpl->saveGlobalCredentials();
  }

  void CredentialManager::saveInUser(const AuthData & cred)
  {
    AuthData_Ptr c_ptr;
    c_ptr.reset(new AuthData(cred)); // FIX for child classes if needed
    _pimpl->_credsUser.insert(c_ptr); //! \todo avoid adding duplicates
    _pimpl->saveUserCredentials();
  }

  void saveIn(const AuthData &, const Pathname & credFile)
  {
    //! \todo save in the file or  /etc/zypp/credentials.d/credFile if not absolute 
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
