/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp-media/auth/authdata.cc
 *
 */

#include "authdata.h"
#include <zypp-core/base/String.h>

using std::endl;

namespace zypp {
  namespace media {


AuthData::AuthData(const Url & url)
      : _url(url), _lastChange(0)
{
  _username = url.getUsername();
  _password = url.getPassword();
}


bool AuthData::valid() const
{
  return username().size() && password().size();
}

time_t AuthData::lastDatabaseUpdate() const
{
  return _lastChange;
}

void AuthData::setLastDatabaseUpdate( time_t time )
{
  _lastChange = time;
}

std::ostream & AuthData::dumpOn( std::ostream & str ) const
{
  if (_url.isValid())
    str << "[" << _url.asString( url::ViewOptions() - url::ViewOptions::WITH_USERNAME - url::ViewOptions::WITH_PASSWORD ) << "]" << endl;
  else
    str << "[<no-url>]" << endl;
  str << "username: '" << _username << "'" << std::endl
      << "password: " << (_password.empty() ? "<empty>" : "<non-empty>");
  return str;
}

std::ostream & AuthData::dumpAsIniOn( std::ostream & str ) const
{
  if (_url.isValid())
    str
      << "[" << _url.asString(
        url::ViewOptions()
        - url::ViewOptions::WITH_USERNAME
        - url::ViewOptions::WITH_PASSWORD)
      << "]" << endl;

  str
    << "username = " << _username << endl
    << "password = " << _password << endl;

  return str;
}

std::ostream & operator << (std::ostream & str, const AuthData & auth_data)
{
  auth_data.dumpOn(str);
  return str;
}

  } // namespace media
} // namespace zypp
