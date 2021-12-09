/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp-curl/auth/CurlAuthData
 *
*/
#ifndef ZYPP_CURL_AUTH_CURLAUTHDATA_H_INCLUDED
#define ZYPP_CURL_AUTH_CURLAUTHDATA_H_INCLUDED

#include <zypp-media/auth/AuthData>

namespace zypp {
  namespace media {
    /**
     * Curl HTTP authentication data.
     */
    class CurlAuthData : public AuthData {
    public:
      /**
       * Default constructor. Initializes username and password to empty strings
       * and authetication type to CURLAUTH_NONE.
       */
      CurlAuthData();

      CurlAuthData(const AuthData & authData);

      CurlAuthData(std::string & username, std::string & password, std::string & auth_type)
        : AuthData(username,password), _auth_type_str(auth_type)
      {
        _auth_type = auth_type_str2long(auth_type);
      }

      CurlAuthData(std::string & username, std::string & password, long auth_type)
        : AuthData(username,password), _auth_type(auth_type)
      {
        _auth_type_str = auth_type_long2str(auth_type);
      }

      /**
       * Checks validity of authentication data.
       * \return true if the object contains non-empty username,
       *  non-empty password, and specifies authentication type; false otherwise.
       */
      virtual bool valid() const;

      /**
       * Set HTTP authentication type(s) to use.
       * \param comma separated list of HTTP authentication type names
       */
      void setAuthType(std::string auth_type)
      {
        _auth_type_str = auth_type; _auth_type = auth_type_str2long(auth_type);
      }

      /**
       * Set HTTP authentication type(s) to use.
       * \param HTTP authentication type as in long ORed form.
       * \see curl.h for available auth types
       */
      void setAuthType(long auth_type)
      {
        _auth_type = auth_type;
        _auth_type_str = auth_type_long2str(auth_type);
      }

      long authType() const { return _auth_type; }
      std::string authTypeAsString() const { return _auth_type_str; }

      std::string getUserPwd() const { return username() + ":" + password(); }


      /**
       * Converts a string of comma separated list of authetication type names
       * into a long of ORed CURLAUTH_* identifiers.
       * The method also automatically leaves out any auth types declared
       * not supported by curl_version_info().
       *
       * \throws MediaException if an invalid authentication type name is
       *         encountered.
       */
      static long auth_type_str2long( std::string & auth_type_str );
      static long auth_type_str2long( const std::string &auth_type_str );

      /**
       * Converts a long of ORed CURLAUTH_* identifiers into a string of comma
       * separated list of authentication type names.
       */
      static std::string auth_type_long2str(long auth_type);

      virtual std::ostream & dumpOn( std::ostream & str ) const;

    private:
      std::string _auth_type_str;
      long _auth_type;
    };

    typedef shared_ptr<CurlAuthData> CurlAuthData_Ptr;
    std::ostream & operator << (std::ostream & str, const CurlAuthData & auth_data);
  }
}

#endif
