#include <zypp/zyppng/media/network/private/request_p.h>
#include <zypp/zyppng/media/network/private/networkrequesterror_p.h>
#include <zypp/zyppng/media/network/networkrequestdispatcher.h>
#include <zypp/media/CurlHelper.h>
#include <zypp/media/CurlConfig.h>
#include <zypp/media/MediaUserAuth.h>
#include <zypp/ZConfig.h>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/Pathname.h>
#include <stdio.h>
#include <fcntl.h>
#include <sstream>


namespace zyppng {

  std::vector<char> peek_data_fd( FILE *fd, off_t offset, size_t count )
  {
    if ( !fd )
      return {};

    fflush( fd );

    std::vector<char> data( count + 1 , '\0' );

    ssize_t l = -1;
    while ((l = pread( fileno( fd ), data.data(), count, offset ) ) == -1 && errno == EINTR)
      ;
    if (l == -1)
      return {};

    return data;
  }

  NetworkRequestPrivate::NetworkRequestPrivate(Url &&url, zypp::Pathname &&targetFile, off_t &&start, off_t &&len, NetworkRequest::FileMode fMode )
    : _url ( std::move(url) )
    , _targetFile ( std::move( targetFile) )
    , _start ( std::move(start) )
    , _len ( std::move(len) )
    , _fMode ( std::move(fMode) )
    , _activityTimer ( Timer::create() )
    , _headers( std::unique_ptr< curl_slist, decltype (&curl_slist_free_all) >( nullptr, &curl_slist_free_all ) )
  {
    _activityTimer->sigExpired().connect( sigc::mem_fun( this, &NetworkRequestPrivate::onActivityTimeout ));
  }

  NetworkRequestPrivate::~NetworkRequestPrivate()
  {
    if ( _easyHandle ) {
      //clean up for now, later we might reuse handles
      curl_easy_cleanup( _easyHandle );
      //reset in request but make sure the request was not enqueued again and got a new handle
      _easyHandle = nullptr;
    }
  }

  bool NetworkRequestPrivate::initialize( std::string &errBuf )
  {
    reset();

    if ( _easyHandle )
    //will reset to defaults but keep live connections, session ID and DNS caches
      curl_easy_reset( _easyHandle );
    else
      _easyHandle = curl_easy_init();

    _errorBuf.fill( '\0' );
    curl_easy_setopt( _easyHandle, CURLOPT_ERRORBUFFER, this->_errorBuf.data() );

    try {

      setCurlOption( CURLOPT_PRIVATE, this );
      setCurlOption( CURLOPT_XFERINFOFUNCTION, NetworkRequestPrivate::curlProgressCallback );
      setCurlOption( CURLOPT_XFERINFODATA, this  );
      setCurlOption( CURLOPT_NOPROGRESS, 0L);
      setCurlOption( CURLOPT_FAILONERROR, 1L);
      setCurlOption( CURLOPT_NOSIGNAL, 1L);

      std::string urlBuffer( _url.asString() );
      setCurlOption( CURLOPT_URL, urlBuffer.c_str() );

      setCurlOption( CURLOPT_WRITEFUNCTION, NetworkRequestPrivate::writeCallback );
      setCurlOption( CURLOPT_WRITEDATA, this );

      if ( _options & NetworkRequest::HeadRequest ) {
        // instead of returning no data with NOBODY, we return
        // little data, that works with broken servers, and
        // works for ftp as well, because retrieving only headers
        // ftp will return always OK code ?
        // See http://curl.haxx.se/docs/knownbugs.html #58
        if (  (_url.getScheme() == "http" ||  _url.getScheme() == "https") && _settings.headRequestsAllowed() )
          setCurlOption( CURLOPT_NOBODY, 1L );
        else
          setCurlOption( CURLOPT_RANGE, "0-1" );
      } else {
        std::string rangeDesc;
        if ( _start >= 0) {
          _expectRangeStatus = true;
          rangeDesc = zypp::str::form("%llu-", static_cast<unsigned long long>( _start ));
          if( _len > 0 ) {
            rangeDesc.append( zypp::str::form( "%llu", static_cast<unsigned long long>(_start + _len - 1) ) );
          }
          if ( setCurlOption( CURLOPT_RANGE, rangeDesc.c_str() ), CURLE_OK ) {
            strncpy( _errorBuf.data(), "curl_easy_setopt range failed", CURL_ERROR_SIZE);
            return false;
          }
        } else {
          _expectRangeStatus = false;
        }

      }

      //make a local copy of the settings, so headers are not added multiple times
      TransferSettings locSet = _settings;

      // add custom headers for download.opensuse.org (bsc#955801)
      if ( _url.getHost() == "download.opensuse.org" )
      {
        locSet.addHeader( internal::anonymousIdHeader() );
        locSet.addHeader( internal::distributionFlavorHeader() );
      }

      locSet.addHeader("Pragma:");

      locSet.setTimeout( zypp::ZConfig::instance().download_transfer_timeout() );
      locSet.setConnectTimeout( CONNECT_TIMEOUT );

      locSet.setUserAgentString( internal::agentString() );

      {
        char *ptr = getenv("ZYPP_MEDIA_CURL_DEBUG");
        _curlDebug = (ptr && *ptr) ? zypp::str::strtonum<long>( ptr) : 0L;
        if( _curlDebug > 0)
        {
          setCurlOption( CURLOPT_VERBOSE, 1L);
          setCurlOption( CURLOPT_DEBUGFUNCTION, internal::log_curl);
          setCurlOption( CURLOPT_DEBUGDATA, &_curlDebug);
        }
      }

      /** Force IPv4/v6 */
      switch ( internal::env::ZYPP_MEDIA_CURL_IPRESOLVE() )
      {
        case 4: setCurlOption( CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4 ); break;
        case 6: setCurlOption( CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6 ); break;
        default: break;
      }

      setCurlOption( CURLOPT_HEADERFUNCTION, internal::log_redirects_curl );
      setCurlOption( CURLOPT_HEADERDATA, &_lastRedirect );

      /**
        * Connect timeout
        */
      setCurlOption( CURLOPT_CONNECTTIMEOUT, locSet.connectTimeout() );
      // If a transfer timeout is set, also set CURLOPT_TIMEOUT to an upper limit
      // just in case curl does not trigger its progress callback frequently
      // enough.
      if ( locSet.timeout() )
      {
        setCurlOption( CURLOPT_TIMEOUT, 3600L );
      }

      if ( _url.getScheme() == "https" )
      {
#if CURLVERSION_AT_LEAST(7,19,4)
        // restrict following of redirections from https to https only
	if ( _url.getHost() == "download.opensuse.org" )
	  setCurlOption( CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS );
	else
	  setCurlOption( CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS );
#endif

#if CURLVERSION_AT_LEAST(7,60,0)	// SLE15+
        setCurlOption( CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS );
#endif

        if( locSet.verifyPeerEnabled() ||
             locSet.verifyHostEnabled() )
        {
          setCurlOption(CURLOPT_CAPATH, locSet.certificateAuthoritiesPath().c_str());
        }

        if( ! locSet.clientCertificatePath().empty() )
        {
          setCurlOption(CURLOPT_SSLCERT, locSet.clientCertificatePath().c_str());
        }
        if( ! locSet.clientKeyPath().empty() )
        {
          setCurlOption(CURLOPT_SSLKEY, locSet.clientKeyPath().c_str());
        }

#ifdef CURLSSLOPT_ALLOW_BEAST
        // see bnc#779177
        setCurlOption( CURLOPT_SSL_OPTIONS, CURLSSLOPT_ALLOW_BEAST );
#endif
        setCurlOption(CURLOPT_SSL_VERIFYPEER, locSet.verifyPeerEnabled() ? 1L : 0L);
        setCurlOption(CURLOPT_SSL_VERIFYHOST, locSet.verifyHostEnabled() ? 2L : 0L);
        // bnc#903405 - POODLE: libzypp should only talk TLS
        setCurlOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
      }

      // follow any Location: header that the server sends as part of
      // an HTTP header (#113275)
      setCurlOption( CURLOPT_FOLLOWLOCATION, 1L);
      // 3 redirects seem to be too few in some cases (bnc #465532)
      setCurlOption( CURLOPT_MAXREDIRS, 6L );

      //set the user agent
      setCurlOption(CURLOPT_USERAGENT, locSet.userAgentString().c_str() );


      /*---------------------------------------------------------------*
        CURLOPT_USERPWD: [user name]:[password]
        Url::username/password -> CURLOPT_USERPWD
        If not provided, anonymous FTP identification
      *---------------------------------------------------------------*/
      if ( locSet.userPassword().size() )
      {
        setCurlOption(CURLOPT_USERPWD, locSet.userPassword().c_str());
        std::string use_auth = _settings.authType();
        if (use_auth.empty())
          use_auth = "digest,basic";	// our default
        long auth = zypp::media::CurlAuthData::auth_type_str2long(use_auth);
        if( auth != CURLAUTH_NONE)
        {
          DBG << "Enabling HTTP authentication methods: " << use_auth
              << " (CURLOPT_HTTPAUTH=" << auth << ")" << std::endl;
          setCurlOption(CURLOPT_HTTPAUTH, auth);
        }
      }

      if ( locSet.proxyEnabled() && ! locSet.proxy().empty() )
      {
        DBG << "Proxy: '" << locSet.proxy() << "'" << std::endl;
        setCurlOption(CURLOPT_PROXY, locSet.proxy().c_str());
        setCurlOption(CURLOPT_PROXYAUTH, CURLAUTH_BASIC|CURLAUTH_DIGEST|CURLAUTH_NTLM );

        /*---------------------------------------------------------------*
         *    CURLOPT_PROXYUSERPWD: [user name]:[password]
         *
         * Url::option(proxyuser and proxypassword) -> CURLOPT_PROXYUSERPWD
         *  If not provided, $HOME/.curlrc is evaluated
         *---------------------------------------------------------------*/

        std::string proxyuserpwd = locSet.proxyUserPassword();

        if ( proxyuserpwd.empty() )
        {
          zypp::media::CurlConfig curlconf;
          zypp::media::CurlConfig::parseConfig(curlconf); // parse ~/.curlrc
          if ( curlconf.proxyuserpwd.empty() )
            DBG << "Proxy: ~/.curlrc does not contain the proxy-user option" << std::endl;
          else
          {
            proxyuserpwd = curlconf.proxyuserpwd;
            DBG << "Proxy: using proxy-user from ~/.curlrc" << std::endl;
          }
        }
        else
        {
          DBG << "Proxy: using provided proxy-user '" << _settings.proxyUsername() << "'" << std::endl;
        }

        if ( ! proxyuserpwd.empty() )
        {
          setCurlOption(CURLOPT_PROXYUSERPWD, internal::curlUnEscape( proxyuserpwd ).c_str());
        }
      }
#if CURLVERSION_AT_LEAST(7,19,4)
      else if ( locSet.proxy() == EXPLICITLY_NO_PROXY )
      {
        // Explicitly disabled in URL (see fillSettingsFromUrl()).
        // This should also prevent libcurl from looking into the environment.
        DBG << "Proxy: explicitly NOPROXY" << std::endl;
        setCurlOption(CURLOPT_NOPROXY, "*");
      }

#endif
      else
      {
        DBG << "Proxy: not explicitly set" << std::endl;
        DBG << "Proxy: libcurl may look into the environment" << std::endl;
      }

      /** Speed limits */
      if ( locSet.minDownloadSpeed() != 0 )
      {
        setCurlOption(CURLOPT_LOW_SPEED_LIMIT, locSet.minDownloadSpeed());
        // default to 10 seconds at low speed
        setCurlOption(CURLOPT_LOW_SPEED_TIME, 60L);
      }

#if CURLVERSION_AT_LEAST(7,15,5)
      if ( locSet.maxDownloadSpeed() != 0 )
        setCurlOption(CURLOPT_MAX_RECV_SPEED_LARGE, locSet.maxDownloadSpeed());
#endif

      if ( zypp::str::strToBool( _url.getQueryParam( "cookies" ), true ) )
        setCurlOption( CURLOPT_COOKIEFILE, _currentCookieFile.c_str() );
      else
        MIL << "No cookies requested" << std::endl;
      setCurlOption(CURLOPT_COOKIEJAR, _currentCookieFile.c_str() );

#if CURLVERSION_AT_LEAST(7,18,0)
      // bnc #306272
      setCurlOption(CURLOPT_PROXY_TRANSFER_MODE, 1L );
#endif

      // append settings custom headers to curl
      for ( TransferSettings::Headers::const_iterator it = locSet.headersBegin();
            it != locSet.headersEnd();
            ++it ) {
        if ( !z_func()->addRequestHeader( it->c_str() ) )
          ZYPP_THROW(zypp::media::MediaCurlInitException(_url));
      }

      if ( _headers )
        setCurlOption( CURLOPT_HTTPHEADER, _headers.get() );

      return true;

    } catch ( const zypp::Exception &excp ) {
      ZYPP_CAUGHT(excp);
      errBuf = excp.asString();
    }
    return false;
  }

  void NetworkRequestPrivate::aboutToStart()
  {
    if ( _activityTimer )
      _activityTimer->start( static_cast<uint64_t>( _settings.timeout() ) * 1000 );

    _state = NetworkRequest::Running;
    _sigStarted.emit( *z_func() );
  }

  void NetworkRequestPrivate::setResult( NetworkRequestError &&err )
  {
    if ( _outFile )
      fclose( _outFile );
    _outFile = nullptr;

    _result = std::move(err);

    if ( _activityTimer )
      _activityTimer->stop();

    if ( _result.type() == NetworkRequestError::NoError ) {
      //we have a successful download, lets see if the checksum is fine IF we have one
      _state = NetworkRequest::Finished;
      if ( _expectedChecksum.size() && _digest ) {
        if ( _digest->digestVector() != _expectedChecksum ) {
          _state = NetworkRequest::Error;

          auto hexToStr = []( const std::vector<u_char> &hex ) {
            std::string res;
            for (std::vector<u_char>::size_type j = 0; j < hex.size(); j++)
              res += zypp::str::form("%02hhx", hex[j]);
            return res;
          };

          _result = NetworkRequestErrorPrivate::customError( NetworkRequestError::InvalidChecksum, (zypp::str::Format("Invalid checksum %1%, expected checksum %2%") % _digest->digest() % hexToStr( _expectedChecksum ) ) );
        }
      }
    } else
      _state = NetworkRequest::Error;

    _sigFinished.emit( *z_func(), _result );
  }

  void NetworkRequestPrivate::reset()
  {
    if ( _outFile )
      fclose( _outFile );

    if ( _digest )
      _digest->reset();

    _outFile = nullptr;
    _easyHandle = nullptr;
    _result = NetworkRequestError();
    _state = NetworkRequest::Pending;
    _downloaded = -1;
    _reportedSize = 0;
    _errorBuf.fill( 0 );
    _headers.reset( nullptr );
  }

  void NetworkRequestPrivate::onActivityTimeout( Timer & )
  {
    std::map<std::string, boost::any> extraInfo;
    extraInfo.insert( {"requestUrl", _url } );
    extraInfo.insert( {"filepath", _targetFile } );
    _dispatcher->cancel( *z_func(), NetworkRequestErrorPrivate::customError( NetworkRequestError::Timeout, "Download timed out", std::move(extraInfo) ) );
  }

  int NetworkRequestPrivate::curlProgressCallback( void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow )
  {
    if ( !clientp )
      return 0;
    NetworkRequestPrivate *that = reinterpret_cast<NetworkRequestPrivate *>( clientp );

    //reset the timer
    if ( that->_activityTimer && that->_activityTimer->isRunning() )
      that->_activityTimer->start();

    //keep signals to a minimum
    if ( that->_downloaded == dlnow )
      return 0;

    that->_downloaded   = dlnow;
    that->_reportedSize = dltotal;

    if ( that->_len > 0 && that->_len < dlnow ) {
      that->_dispatcher->cancel( *that->z_func(), NetworkRequestErrorPrivate::customError( NetworkRequestError::ExceededMaxLen ) );
      return 0;
    }

    that->_sigProgress.emit( *that->z_func(), dltotal, dlnow, ultotal, ulnow );
    return 0;
  }

  size_t NetworkRequestPrivate::writeCallback( char *ptr, size_t size, size_t nmemb, void *userdata )
  {
    if ( !userdata )
      return 0;

    //it is valid to call this function with no data to write, just return OK
    if ( size * nmemb == 0)
      return 0;

    NetworkRequestPrivate *that = reinterpret_cast<NetworkRequestPrivate *>( userdata );

    //in case of a HEAD request, we do not write anything
    if ( that->_options & NetworkRequest::HeadRequest ) {
      return ( size * nmemb );
    }

    //If we expect a file range we better double check that we got the status code for it
    if ( that->_expectRangeStatus ) {
      char *effurl;
      (void)curl_easy_getinfo( that->_easyHandle, CURLINFO_EFFECTIVE_URL, &effurl);
      if (effurl && !strncasecmp(effurl, "http", 4))
      {
        long statuscode = 0;
        (void)curl_easy_getinfo( that->_easyHandle, CURLINFO_RESPONSE_CODE, &statuscode);
        if (statuscode != 206) {
          strncpy( that->_errorBuf.data(), "Expected range status code 206, but got none.", CURL_ERROR_SIZE);
          return 0;
        }
      }
    }

    if ( !that->_outFile ) {
      std::string openMode = "w+b";
      if ( that->_fMode == NetworkRequest::WriteShared )
        openMode = "r+b";

      that->_outFile = fopen( that->_targetFile.asString().c_str() , openMode.c_str() );

      //if the file does not exist create a new one
      if ( !that->_outFile && that->_fMode == NetworkRequest::WriteShared ) {
        that->_outFile = fopen( that->_targetFile.asString().c_str() , "w+b" );
      }

      if ( !that->_outFile ) {
        strncpy( that->_errorBuf.data(), "Unable to open target file.", CURL_ERROR_SIZE);
        return 0;
      }

      if ( that->_start > 0 )
        if ( fseek( that->_outFile, that->_start, SEEK_SET ) != 0 ) {
          strncpy( that->_errorBuf.data(), "Unable to set output file pointer.", CURL_ERROR_SIZE);
          return 0;
        }
    }

     size_t written = fwrite( ptr, size, nmemb, that->_outFile );
     if ( that->_digest ) {
       that->_digest->update( ptr, written );
     }

     return written;
  }

  NetworkRequest::NetworkRequest(zyppng::Url url, zypp::filesystem::Pathname targetFile, off_t start, off_t len, zyppng::NetworkRequest::FileMode fMode)
    : Base ( *new NetworkRequestPrivate( std::move(url), std::move(targetFile), std::move(start), std::move(len), std::move(fMode) ) )
  {
  }

  NetworkRequest::~NetworkRequest()
  {
    Z_D();

    if ( d->_dispatcher )
      d->_dispatcher->cancel( *this, "Request destroyed while still running" );

    if ( d->_outFile )
      fclose( d->_outFile );
  }

  void NetworkRequest::setPriority(NetworkRequest::Priority prio)
  {
    d_func()->_priority = prio;
  }

  NetworkRequest::Priority NetworkRequest::priority() const
  {
    return d_func()->_priority;
  }

  void NetworkRequest::setOptions( Options opt )
  {
    d_func()->_options = opt;
  }

  NetworkRequest::Options NetworkRequest::options() const
  {
    return d_func()->_options;
  }

  void NetworkRequest::setRequestRange(off_t start, off_t len)
  {
    Z_D();
    if ( d->_state == Running )
      return;
    d->_start = start;
    d->_len = len;
  }

  const std::string &NetworkRequest::lastRedirectInfo() const
  {
    return d_func()->_lastRedirect;
  }

  void *NetworkRequest::nativeHandle() const
  {
    return d_func()->_easyHandle;
  }

  std::vector<char> NetworkRequest::peekData( off_t offset, size_t count ) const
  {
    Z_D();
    return peek_data_fd( d->_outFile, offset, count );
  }

  Url NetworkRequest::url() const
  {
    return d_func()->_url;
  }

  void NetworkRequest::setUrl(const Url &url)
  {
    Z_D();
    if ( d->_state == NetworkRequest::Running )
      return;

    d->_url = url;
  }

  const zypp::filesystem::Pathname &NetworkRequest::targetFilePath() const
  {
    return d_func()->_targetFile;
  }

  std::string NetworkRequest::contentType() const
  {
    char *ptr = NULL;
    if ( curl_easy_getinfo( d_func()->_easyHandle, CURLINFO_CONTENT_TYPE, &ptr ) == CURLE_OK && ptr )
      return std::string(ptr);
    return std::string();
  }

  off_t NetworkRequest::downloadOffset() const
  {
    return d_func()->_start;
  }

  off_t NetworkRequest::reportedByteCount() const
  {
    return d_func()->_reportedSize;
  }

  off_t NetworkRequest::expectedByteCount() const
  {
    return d_func()->_len;
  }

  off_t NetworkRequest::downloadedByteCount() const
  {
    Z_D();
    if ( d->_downloaded == -1 )
      return 0;
    return d->_downloaded;
  }

  void NetworkRequest::setDigest( std::shared_ptr<zypp::Digest> dig )
  {
    d_func()->_digest = dig;
  }

  void NetworkRequest::setExpectedChecksum(std::vector<unsigned char> checksum )
  {
    d_func()->_expectedChecksum = std::move(checksum);
  }

  TransferSettings &NetworkRequest::transferSettings()
  {
    return d_func()->_settings;
  }

  std::shared_ptr<zypp::Digest> NetworkRequest::digest( ) const
  {
    return d_func()->_digest;
  }

  NetworkRequest::State NetworkRequest::state() const
  {
    return d_func()->_state;
  }

  const NetworkRequestError &NetworkRequest::error() const
  {
    return d_func()->_result;
  }

  std::string NetworkRequest::extendedErrorString() const
  {
    Z_D();
    if ( !hasError() )
      return std::string();

    return d->_result.nativeErrorString();
  }

  bool NetworkRequest::hasError() const
  {
    return error().isError();
  }

  bool NetworkRequest::addRequestHeader( const std::string &header )
  {
    Z_D();

    curl_slist *res = curl_slist_append( d->_headers ? d->_headers.get() : nullptr, header.c_str() );
    if ( !res )
      return false;

    if ( !d->_headers )
      d->_headers = std::unique_ptr< curl_slist, decltype (&curl_slist_free_all) >( res, &curl_slist_free_all );

    return true;
  }

  SignalProxy<void (NetworkRequest &req)> NetworkRequest::sigStarted()
  {
    return d_func()->_sigStarted;
  }

  SignalProxy<void (NetworkRequest &req, off_t dltotal, off_t dlnow, off_t ultotal, off_t ulnow)> NetworkRequest::sigProgress()
  {
    return d_func()->_sigProgress;
  }

  SignalProxy<void (zyppng::NetworkRequest &req, const zyppng::NetworkRequestError &err)> NetworkRequest::sigFinished()
  {
    return d_func()->_sigFinished;
  }


}
