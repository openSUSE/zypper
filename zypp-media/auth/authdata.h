/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp-media/auth/AuthData
 * Convenience interface for handling authentication data of media user.
 */
#ifndef ZYPP_MEDIA_AUTH_DATA_H
#define ZYPP_MEDIA_AUTH_DATA_H

#include <zypp-core/Url.h>
#include <zypp-core/base/PtrTypes.h>
#include <zypp-media/ng/HeaderValueMap>

namespace zypp {
  namespace media {

///////////////////////////////////////////////////////////////////


/**
 * Class for handling media authentication data. This is the most generic
 * class containing only username and password members.
 */
class AuthData
{
public:
  AuthData()
  {}

  AuthData(const Url & url);

  AuthData(const std::string & username, const std::string & password)
    : _username(username), _password(password), _lastChange(0)
  {}

  virtual ~AuthData() {};

  /**
   * Checks validity of authentication data.
   * \return true if the object contains non-empty username and
   *  non-empty password, false otherwise.
   */
  virtual bool valid() const;

  void setUrl(const Url & url) { _url = url; }
  void setUsername(const std::string & username) { _username = username; }
  void setPassword(const std::string & password) { _password = password; }

  Url url() const { return _url; }
  std::string username() const { return _username; }
  std::string password() const { return _password; }

  /*!
   * Returns the timestamp of the last change to the database this
   * credential is stored in, or 0 if it is not known.
   */
  time_t lastDatabaseUpdate () const;
  void setLastDatabaseUpdate ( time_t time );

  const std::map<std::string, std::string> &extraValues() const;
  std::map<std::string, std::string> &extraValues();

  virtual std::ostream & dumpOn( std::ostream & str ) const;

  virtual std::ostream & dumpAsIniOn( std::ostream & str ) const;

private:
  Url _url;
  std::string _username;
  std::string _password;
  time_t _lastChange; //< timestamp of the last change to the database this credential is stored in
  std::map<std::string, std::string> _extraValues;
};

typedef shared_ptr<AuthData> AuthData_Ptr;
std::ostream & operator << (std::ostream & str, const AuthData & auth_data);

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_AUTH_DATA_H
