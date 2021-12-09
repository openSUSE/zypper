/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp-curl/auth/curlauthdata.cc
 *
*/

#include "curlauthdata.h"

#include <zypp-core/base/Gettext.h>
#include <zypp-core/base/String.h>
#include <zypp-media/MediaException>

#include <curl/curl.h>


using std::endl;

namespace zypp::media {

  CurlAuthData::CurlAuthData()
    : AuthData()
    , _auth_type_str()
    , _auth_type(CURLAUTH_NONE)
  {}

  CurlAuthData::CurlAuthData(const AuthData & authData)
    : AuthData(authData)
    , _auth_type_str()
    , _auth_type(CURLAUTH_NONE)
  {}

  bool CurlAuthData::valid() const
  {
    return username().size() && password().size();
  }

  std::ostream & CurlAuthData::dumpOn( std::ostream & str ) const
  {
    AuthData::dumpOn(str) << endl
                            << " auth_type: " << _auth_type_str << " (" << _auth_type << ")";
    return str;
  }

  long CurlAuthData::auth_type_str2long( std::string & auth_type_str )
  {
    return auth_type_str2long( const_cast< const std::string &>(auth_type_str) );
  }

  long CurlAuthData::auth_type_str2long( const std::string & auth_type_str )
  {
    curl_version_info_data *curl_info = curl_version_info(CURLVERSION_NOW);

    std::vector<std::string>                  list;
    std::vector<std::string>::const_iterator  it;
    long                                      auth_type = CURLAUTH_NONE;

    zypp::str::split(auth_type_str, std::back_inserter(list), ",");

    for(it = list.begin(); it != list.end(); ++it)
    {
      if(*it == "basic")
      {
        auth_type |= CURLAUTH_BASIC;
      }
      else
        if(*it == "digest")
        {
          auth_type |= CURLAUTH_DIGEST;
        }
        else
          if((curl_info && (curl_info->features & CURL_VERSION_NTLM)) &&
               (*it == "ntlm"))
          {
            auth_type |= CURLAUTH_NTLM;
          }
          else
            if((curl_info && (curl_info->features & CURL_VERSION_SPNEGO)) &&
                 (*it == "spnego" || *it == "negotiate"))
            {
              // there is no separate spnego flag for this auth type
              auth_type |= CURLAUTH_GSSNEGOTIATE;
            }
            else
              if((curl_info && (curl_info->features & CURL_VERSION_GSSNEGOTIATE)) &&
                   (*it == "gssnego" || *it == "negotiate"))
              {
                auth_type |= CURLAUTH_GSSNEGOTIATE;
              }
              else
              {
                ZYPP_THROW(MediaException(str::Format(_("Unsupported HTTP authentication method '%s'")) % *it));
              }
    }

    return auth_type;
  }

  std::string CurlAuthData::auth_type_long2str(long auth_type)
  {
    std::list<std::string> auth_list;

    if(auth_type & CURLAUTH_GSSNEGOTIATE)
      auth_list.push_back("negotiate");

    if(auth_type & CURLAUTH_NTLM)
      auth_list.push_back("ntlm");

    if(auth_type & CURLAUTH_DIGEST)
      auth_list.push_back("digest");

    if(auth_type & CURLAUTH_BASIC)
      auth_list.push_back("basic");

    return str::join(auth_list, ",");
  }

  std::ostream & operator << (std::ostream & str, const CurlAuthData & auth_data)
  {
    auth_data.dumpOn(str);
    return str;
  }

}
