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
#ifndef ZYPP_MEDIA_CURLHELPER_H_INCLUDED
#define ZYPP_MEDIA_CURLHELPER_H_INCLUDED

#include <curl/curl.h>
#include <zypp/Url.h>
#include <zypp/media/TransferSettings.h>
#include <zypp/ZYppCallbacks.h>

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

struct ProgressData
{
  ProgressData( CURL *_curl, time_t _timeout = 0, const zypp::Url & _url = zypp::Url(),
    zypp::ByteCount expectedFileSize_r = 0,
    zypp::callback::SendReport<zypp::media::DownloadProgressReport> *_report = nullptr );

  CURL	*curl;
  zypp::Url	url;
  time_t	timeout;
  bool	reached;
  bool      fileSizeExceeded;
  zypp::callback::SendReport<zypp::media::DownloadProgressReport> *report;
  zypp::ByteCount _expectedFileSize;

  time_t _timeStart	= 0;	///< Start total stats
  time_t _timeLast	= 0;	///< Start last period(~1sec)
  time_t _timeRcv	= 0;	///< Start of no-data timeout
  time_t _timeNow	= 0;	///< Now

  double _dnlTotal	= 0.0;	///< Bytes to download or 0 if unknown
  double _dnlLast	= 0.0;	///< Bytes downloaded at period start
  double _dnlNow	= 0.0;	///< Bytes downloaded now

  int    _dnlPercent= 0;	///< Percent completed or 0 if _dnlTotal is unknown

  double _drateTotal= 0.0;	///< Download rate so far
  double _drateLast	= 0.0;	///< Download rate in last period

  void updateStats( double dltotal = 0.0, double dlnow = 0.0 );

  int reportProgress() const;


  // download rate of the last period (cca 1 sec)
  double                                        drate_period;
  // bytes downloaded at the start of the last period
  double                                        dload_period;
  // seconds from the start of the download
  long                                          secs;
  // average download rate
  double                                        drate_avg;
  // last time the progress was reported
  time_t                                        ltime;
  // bytes downloaded at the moment the progress was last reported
  double                                        dload;
  // bytes uploaded at the moment the progress was last reported
  double                                        uload;
};

void globalInitCurlOnce();
int  log_curl(CURL *curl, curl_infotype info,  char *ptr, size_t len, void *max_lvl);
size_t log_redirects_curl( char *ptr, size_t size, size_t nmemb, void *userdata);


void fillSettingsFromUrl( const zypp::Url &url, zypp::media::TransferSettings &s );
void fillSettingsSystemProxy( const zypp::Url& url, zypp::media::TransferSettings &s );

/**
 * initialized only once, this gets the anonymous id
 * from the target, which we pass in the http header
 */
const char * anonymousIdHeader();

/**
 * initialized only once, this gets the distribution flavor
 * from the target, which we pass in the http header
 */
const char * distributionFlavorHeader();

/**
 * initialized only once, this gets the agent string
 * which also includes the curl version
 */
const char * agentString();

void curlEscape( std::string & str_r,  const char char_r, const std::string & escaped_r );
std::string curlEscapedPath( std::string path_r );
std::string curlUnEscape( std::string text_r );

zypp::Url clearQueryString(const zypp::Url &url);
zypp::Url propagateQueryParams( zypp::Url url_r, const zypp::Url & template_r );

}




#endif
