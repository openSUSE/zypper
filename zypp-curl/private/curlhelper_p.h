/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPP_MEDIA_PRIVATE_CURLHELPER_P_H_INCLUDED
#define ZYPP_MEDIA_PRIVATE_CURLHELPER_P_H_INCLUDED

#include <curl/curl.h>
#include <zypp-core/Url.h>
#include <zypp-curl/TransferSettings>

#define  CONNECT_TIMEOUT        60
#define  TRANSFER_TIMEOUT_MAX   60 * 60
#define  DETECT_DIR_INDEX       0

#define EXPLICITLY_NO_PROXY "_none_"

#undef CURLVERSION_AT_LEAST
#define CURLVERSION_AT_LEAST(M,N,O) LIBCURL_VERSION_NUM >= ((((M)<<8)+(N))<<8)+(O)

namespace zypp
{
  namespace env
  {
    /** Long number for setting CURLOPT_DEBUGDATA */
    inline long ZYPP_MEDIA_CURL_DEBUG()
    {
      long ret = 0L;
      if ( char *ptr = ::getenv("ZYPP_MEDIA_CURL_DEBUG"); ptr && *ptr )
        str::strtonum( ptr, ret );
      return ret;
    }

    /** 4/6 to force IPv4/v6 */
    int ZYPP_MEDIA_CURL_IPRESOLVE();
  } // namespace env
} //namespace zypp

//do not export
namespace internal {

void globalInitCurlOnce();
int  log_curl(CURL *curl, curl_infotype info,  char *ptr, size_t len, void *max_lvl);
size_t log_redirects_curl( char *ptr, size_t size, size_t nmemb, void *userdata);


void fillSettingsFromUrl( const zypp::Url &url, zypp::media::TransferSettings &s );
void fillSettingsSystemProxy( const zypp::Url& url, zypp::media::TransferSettings &s );

void curlEscape( std::string & str_r,  const char char_r, const std::string & escaped_r );
std::string curlEscapedPath( std::string path_r );
std::string curlUnEscape( std::string text_r );

zypp::Url clearQueryString(const zypp::Url &url);
zypp::Url propagateQueryParams( zypp::Url url_r, const zypp::Url & template_r );

}

#endif //ZYPP_MEDIA_PRIVATE_CURLHELPER_P_H_INCLUDED
