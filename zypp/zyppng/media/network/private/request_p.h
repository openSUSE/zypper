#ifndef ZYPP_NG_MEDIA_CURL_PRIVATE_REQUEST_P_H_INCLUDED
#define ZYPP_NG_MEDIA_CURL_PRIVATE_REQUEST_P_H_INCLUDED

#include <zypp/zyppng/base/private/base_p.h>
#include <zypp/zyppng/media/network/request.h>
#include <zypp/media/MediaException.h>
#include <zypp/zyppng/base/Timer>
#include <curl/curl.h>
#include <array>
#include <memory>
#include <zypp/Digest.h>

namespace zyppng {

  class NetworkRequestPrivate : public BasePrivate
  {
  public:
    ZYPP_DECLARE_PUBLIC(NetworkRequest)

    NetworkRequestPrivate( Url &&url, zypp::Pathname &&targetFile, off_t &&start, off_t &&len, NetworkRequest::FileMode fMode );
    virtual ~NetworkRequestPrivate();

    bool initialize(std::string &errBuf );
    void aboutToStart ();
    void setResult ( NetworkRequestError &&err );
    void reset ();
    void onActivityTimeout (Timer &);

    template<typename T>
    void setCurlOption ( CURLoption opt, T data )
    {
      auto ret = curl_easy_setopt( _easyHandle, opt, data );
      if ( ret != 0 ) {
        ZYPP_THROW( zypp::media::MediaCurlSetOptException( _url, _errorBuf.data() ) );
      }
    }

    Url   _url;        //file URL
    zypp::Pathname _targetFile; //target file
    TransferSettings _settings;
    NetworkRequest::Options _options;

    long _curlDebug = 0L;
    std::string _lastRedirect;	///< to log/report redirections
    std::string _currentCookieFile = "/var/lib/YaST2/cookies";

    off_t _start = -1;  //start offset of block to request
    off_t _len   = 0;  //len of block to request ( 0 if full length
    off_t _downloaded = -1; //downloaded bytes
    off_t _reportedSize = 0; //size reported by the curl backend
    bool  _expectRangeStatus = false;
    NetworkRequest::FileMode _fMode = NetworkRequest::WriteExclusive;
    NetworkRequest::Priority _priority = NetworkRequest::Normal;

    std::shared_ptr<zypp::Digest> _digest; //digest to be used to calculate checksum
    std::vector<unsigned char> _expectedChecksum; //checksum to be expected after download is finished

    NetworkRequest::State _state = NetworkRequest::Pending;
    NetworkRequestError _result;
    std::array<char, CURL_ERROR_SIZE+1> _errorBuf; //provide a buffer for a nicely formatted error

    FILE *_outFile = nullptr;
    void *_easyHandle = nullptr; // the easy handle that controlling this request
    NetworkRequestDispatcher *_dispatcher = nullptr; // the parent downloader owning this request

    Timer::Ptr _activityTimer;

    //signals
    signal<void ( NetworkRequest &req )> _sigStarted;
    signal<void ( NetworkRequest &req, off_t dltotal, off_t dlnow, off_t ultotal, off_t ulnow )> _sigProgress;
    signal<void ( NetworkRequest &req, const NetworkRequestError &err )> _sigFinished;

    static int curlProgressCallback ( void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow );
    static size_t writeCallback ( char *ptr, size_t size, size_t nmemb, void *userdata );

    std::unique_ptr< curl_slist, decltype (&curl_slist_free_all) > _headers;
  };

  std::vector<char> peek_data_fd ( FILE *fd, off_t offset, size_t count );
}

#endif
