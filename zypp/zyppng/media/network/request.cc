#include <zypp/zyppng/media/network/private/request_p.h>
#include <zypp/zyppng/media/network/private/networkrequesterror_p.h>
#include <zypp/zyppng/media/network/networkrequestdispatcher.h>
#include <zypp/zyppng/base/EventDispatcher>
#include <zypp/zyppng/core/String>
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
#include <utility>

#include <iostream>
#include <boost/variant.hpp>
#include <boost/variant/polymorphic_get.hpp>


namespace zyppng {

  namespace  {
    static size_t nwr_headerCallback (  char *ptr, size_t size, size_t nmemb, void *userdata  ) {
      if ( !userdata )
        return 0;

      NetworkRequestPrivate *that = reinterpret_cast<NetworkRequestPrivate *>( userdata );
      return that->headerCallback( ptr, size, nmemb );
    }
    static size_t nwr_writeCallback ( char *ptr, size_t size, size_t nmemb, void *userdata ) {
      if ( !userdata )
        return 0;

      NetworkRequestPrivate *that = reinterpret_cast<NetworkRequestPrivate *>( userdata );
      return that->writeCallback( ptr, size, nmemb );
    }

    //helper for std::visit
    template<class T> struct always_false : std::false_type {};
  }

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

  NetworkRequestPrivate::running_t::running_t( pending_t &&prevState )
    : _requireStatusPartial( prevState._requireStatusPartial )
  {
  }

  NetworkRequestPrivate::NetworkRequestPrivate(Url &&url, zypp::Pathname &&targetFile, NetworkRequest::FileMode fMode )
    : _url ( std::move(url) )
    , _targetFile ( std::move( targetFile) )
    , _fMode ( std::move(fMode) )
    , _headers( std::unique_ptr< curl_slist, decltype (&curl_slist_free_all) >( nullptr, &curl_slist_free_all ) )
  { }

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

    const std::string urlScheme = _url.getScheme();
    if ( urlScheme == "http" ||  urlScheme == "https" )
      _protocolMode = ProtocolMode::HTTP;

    try {

      setCurlOption( CURLOPT_PRIVATE, this );
      setCurlOption( CURLOPT_XFERINFOFUNCTION, NetworkRequestPrivate::curlProgressCallback );
      setCurlOption( CURLOPT_XFERINFODATA, this  );
      setCurlOption( CURLOPT_NOPROGRESS, 0L);
      setCurlOption( CURLOPT_FAILONERROR, 1L);
      setCurlOption( CURLOPT_NOSIGNAL, 1L);

      std::string urlBuffer( _url.asString() );
      setCurlOption( CURLOPT_URL, urlBuffer.c_str() );

      setCurlOption( CURLOPT_WRITEFUNCTION, nwr_writeCallback );
      setCurlOption( CURLOPT_WRITEDATA, this );

      if ( _options & NetworkRequest::HeadRequest ) {
        // instead of returning no data with NOBODY, we return
        // little data, that works with broken servers, and
        // works for ftp as well, because retrieving only headers
        // ftp will return always OK code ?
        // See http://curl.haxx.se/docs/knownbugs.html #58
        if (  _protocolMode == ProtocolMode::HTTP && _settings.headRequestsAllowed() )
          setCurlOption( CURLOPT_NOBODY, 1L );
        else
          setCurlOption( CURLOPT_RANGE, "0-1" );
      } else {
        std::string rangeDesc;

        if ( _requestedRanges.size() ) {

          if ( _requestedRanges.size() > 1 && _protocolMode != ProtocolMode::HTTP ) {
            errBuf = "Using more than one range is not supported with protocols other than HTTP/HTTPS";
            return false;
          }

          // check if we have one big range convering the whole file
          if ( _requestedRanges.size() == 1 && _requestedRanges.front().start == 0 && _requestedRanges.front().len == 0 ) {
            std::get<pending_t>( _runningMode )._requireStatusPartial = false;

          } else {
            std::sort( _requestedRanges.begin(), _requestedRanges.end(), []( const auto &elem1, const auto &elem2 ){
              return ( elem1.start < elem2.start );
            });

            std::get<pending_t>( _runningMode )._requireStatusPartial = true;

            // helper function to build up the request string for the range
            auto addRangeString = [ &rangeDesc ]( const std::pair<size_t, size_t> &range ) {
              std::string rangeD = zypp::str::form("%llu-", static_cast<unsigned long long>( range.first ) );
              if( range.second > 0 )
                rangeD.append( zypp::str::form( "%llu", static_cast<unsigned long long>( range.second ) ) );

              if ( rangeDesc.size() )
                rangeDesc.append(",").append( rangeD );
              else
                rangeDesc = std::move( rangeD );
            };

            std::optional<std::pair<size_t, size_t>> currentZippedRange;
            bool closedRange = true;
            for ( auto &range : _requestedRanges ) {

              //reset the download results
              range._valid = false;
              range.bytesWritten = 0;

              //when we have a open range in the list of ranges we will get from start of range to end of file,
              //all following ranges would never be marked as valid, so we have to fail early
              if ( !closedRange ) {
                errBuf = "It is not supported to request more ranges after a open range.";
                return false;
              }

              const auto rangeEnd = range.len > 0 ? range.start + range.len - 1 : 0;
              closedRange = (rangeEnd > 0);

              // we try to compress the requested ranges into as big chunks as possible for the request,
              // when receiving we still track the original ranges so we can collect and test their checksums
              if ( !currentZippedRange ) {
                currentZippedRange = std::make_pair( range.start, rangeEnd );
              } else {
                //range is directly consecutive to the previous range
                if ( currentZippedRange->second == range.start+1 ) {
                  currentZippedRange->second = rangeEnd;
                } else {
                  //this range does not directly follow the previous one, we build the string and start a new one
                  addRangeString( *currentZippedRange );
                  currentZippedRange = std::make_pair( range.start, rangeEnd );
                }
              }
            }

            // add the last range too
            if ( currentZippedRange )
              addRangeString( *currentZippedRange );

            setCurlOption( CURLOPT_RANGE, rangeDesc.c_str() );
          }
        } else {
          std::get<pending_t>( _runningMode )._requireStatusPartial = false;
          _requestedRanges.push_back( NetworkRequest::Range() );
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

      setCurlOption( CURLOPT_HEADERFUNCTION, &nwr_headerCallback );
      setCurlOption( CURLOPT_HEADERDATA, this );

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

      if ( urlScheme == "https" )
      {
#if CURLVERSION_AT_LEAST(7,19,4)
        // restrict following of redirections from https to https only
	if ( _url.getHost() == "download.opensuse.org" )
	  setCurlOption( CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS );
	else
	  setCurlOption( CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS );
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
    _runningMode = running_t( std::move(std::get<pending_t>( _runningMode )) );

    auto &m = std::get<running_t>( _runningMode );

    if ( m._activityTimer ) {
      m._activityTimer->sigExpired().connect( sigc::mem_fun( this, &NetworkRequestPrivate::onActivityTimeout ));
      m._activityTimer->start( static_cast<uint64_t>( _settings.timeout() ) * 1000 );
    }

    _sigStarted.emit( *z_func() );
  }

  void NetworkRequestPrivate::setResult( NetworkRequestError &&err )
  {
    if ( !std::holds_alternative<running_t>(_runningMode) ) {
      DBG << "Set result called in state " << z_func()->state() << std::endl;
      return;
    }

    auto &rmode = std::get<running_t>( _runningMode );
    rmode._outFile.reset();

    finished_t resState;
    resState._result = std::move(err);
    resState._downloaded = rmode._downloaded;
    resState._contentLenght = rmode._contentLenght;

    if ( resState._result.type() == NetworkRequestError::NoError ) {

      // check if the current range is a open range, if it is set it valid by checking the checksum
      if ( rmode._currentRange >= 0 ) {
        auto &currR = _requestedRanges[rmode._currentRange];
        rmode._currentRange = -1;
        validateRange( currR );
      }

      //we have a successful download lets see if we got everything we needed
      auto _state = NetworkRequest::Finished;
      for ( const auto &r : _requestedRanges ) {
        if ( !r._valid ) {
          _state = NetworkRequest::Error;
          if ( r.len > 0 && r.bytesWritten != r.len )
            resState._result = NetworkRequestErrorPrivate::customError( NetworkRequestError::MissingData, (zypp::str::Format("Did not receive all requested data from the server.") ) );
          else if ( r._digest && r._checksum != r._digest->digestVector() )  {
            resState._result = NetworkRequestErrorPrivate::customError( NetworkRequestError::InvalidChecksum, (zypp::str::Format("Invalid checksum %1%, expected checksum %2%") % r._digest->digest() % zypp::Digest::digestVectorToString( r._checksum ) ) );
          } else {
            resState._result = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, (zypp::str::Format("Download of block failed.") ) );
          }
          //we only report the first error
          break;
        }
      }
    }

    _runningMode = std::move( resState );
    _sigFinished.emit( *z_func(), std::get<finished_t>(_runningMode)._result );
  }

  void NetworkRequestPrivate::reset()
  {
    _protocolMode = ProtocolMode::Default;
    _headers.reset( nullptr );
    _errorBuf.fill( 0 );
    _runningMode = pending_t();
  }

  void NetworkRequestPrivate::onActivityTimeout( Timer & )
  {
    std::map<std::string, boost::any> extraInfo;
    extraInfo.insert( {"requestUrl", _url } );
    extraInfo.insert( {"filepath", _targetFile } );
    _dispatcher->cancel( *z_func(), NetworkRequestErrorPrivate::customError( NetworkRequestError::Timeout, "Download timed out", std::move(extraInfo) ) );
  }

  void NetworkRequestPrivate::validateRange( NetworkRequest::Range &rng )
  {
    if ( rng._digest && rng._checksum.size() ) {
      rng._valid = (rng._digest->digestVector() == rng._checksum);
    } else {
      rng._valid = rng.len == 0 ? true : rng.bytesWritten == rng.len;
    }
  }

  bool NetworkRequestPrivate::parseContentRangeHeader(const boost::string_view &line, size_t &start, size_t &len )
  {
    static const zypp::str::regex regex("^Content-Range:[[:space:]]+bytes[[:space:]]+([0-9]+)-([0-9]+)\\/([0-9]+)$");

    zypp::str::smatch what;
    if( !zypp::str::regex_match( line.to_string(), what, regex ) || what.size() != 4 ) {
      DBG << "Invalid Content-Range Header format: '" << line.to_string() << std::endl;
      strncpy( _errorBuf.data(), "Invalid Content-Range header format.", CURL_ERROR_SIZE);
      return false;
    }

    size_t s = zypp::str::strtonum<size_t>( what[1]);
    size_t e = zypp::str::strtonum<size_t>( what[2]);
    start = std::move(s);
    len   = ( e - s ) + 1;
    return true;
  }

  bool NetworkRequestPrivate::parseContentTypeMultiRangeHeader(const boost::string_view &line, std::string &boundary)
  {
    static const zypp::str::regex regex("^Content-Type:[[:space:]]+multipart\\/byteranges;[[:space:]]+boundary=(.*)$");

    zypp::str::smatch what;
    if( zypp::str::regex_match( line.to_string(), what, regex )  ) {
      if ( what.size() >= 2 ) {
        boundary = what[1];
        return true;
      }
    }
    return false;
  }

  int NetworkRequestPrivate::curlProgressCallback( void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow )
  {
    if ( !clientp )
      return CURLE_OK;
    NetworkRequestPrivate *that = reinterpret_cast<NetworkRequestPrivate *>( clientp );

    if ( !std::holds_alternative<running_t>(that->_runningMode) ){
      DBG << "Curl progress callback was called in invalid state "<< that->z_func()->state() << std::endl;
      return -1;
    }

    auto &rmode = std::get<running_t>( that->_runningMode );

    //reset the timer
    if ( rmode._activityTimer && rmode._activityTimer->isRunning() )
      rmode._activityTimer->start();

    rmode._isInCallback = true;
    that->_sigProgress.emit( *that->z_func(), dltotal, dlnow, ultotal, ulnow );
    rmode._isInCallback = false;

    return rmode._cachedResult ? CURLE_ABORTED_BY_CALLBACK : CURLE_OK;
  }

  size_t NetworkRequestPrivate::headerCallback(char *ptr, size_t size, size_t nmemb)
  {
    //it is valid to call this function with no data to write, just return OK
    if ( size * nmemb == 0)
      return 0;

    if ( _protocolMode == ProtocolMode::HTTP ) {

      boost::string_view hdr( ptr, size*nmemb );

      hdr.remove_prefix( std::min( hdr.find_first_not_of(" \t\r\n"), hdr.size() ) );
      const auto lastNonWhitespace = hdr.find_last_not_of(" \t\r\n");
      if ( lastNonWhitespace != hdr.npos )
        hdr.remove_suffix( hdr.size() - (lastNonWhitespace + 1) );
      else
        hdr.clear();

      XXX << "Received header: " << hdr << std::endl;

      auto &rmode = std::get<running_t>( _runningMode );
      if ( hdr.starts_with("HTTP/") ) {

        long statuscode = 0;
        (void)curl_easy_getinfo( _easyHandle, CURLINFO_RESPONSE_CODE, &statuscode);

        // ignore other status codes, maybe we are redirected etc.
        if ( statuscode >= 200 && statuscode <= 299 ) {
          if ( rmode._requireStatusPartial && statuscode != 206 ) {
            strncpy( _errorBuf.data(), "Expected range status code 206, but got none.", CURL_ERROR_SIZE);
            return 0;
          }
        }
      } else if ( hdr.starts_with( "Location:" ) ) {
        _lastRedirect = hdr.substr( 9 ).to_string();
        DBG << "redirecting to " << _lastRedirect << std::endl;

      } else if ( hdr.starts_with("Content-Type:") ) {
        std::string sep;
        if ( parseContentTypeMultiRangeHeader( hdr, sep ) ) {
          rmode._gotMultiRangeHeader = true;
          rmode._seperatorString = "--"+sep;
        }
      } else if ( hdr.starts_with("Content-Range:") ) {
        NetworkRequest::Range r;
        if ( !parseContentRangeHeader( hdr, r.start, r.len) )
          return 0;
        DBG << "Got content range :" << r.start << " len " << r.len << std::endl;
        rmode._gotContentRangeHeader = true;
        rmode._currentSrvRange = r;

      } else if ( hdr.starts_with("Content-Length:") )  {
        auto lenStr = str::trim( hdr.substr( 15 ), zypp::str::TRIM );
        auto str = std::string ( lenStr.data(), lenStr.length() );
        auto len = zypp::str::strtonum<typename zypp::ByteCount::SizeType>( str.data() );
        if ( len > 0 ) {
          DBG << "Got Content-Length Header: " << len << std::endl;
          rmode._contentLenght = zypp::ByteCount(len, zypp::ByteCount::B);
        }
      }
    }

    return ( size * nmemb );
  }

  size_t NetworkRequestPrivate::writeCallback(char *ptr, size_t size, size_t nmemb)
  {
    const auto max = ( size * nmemb );

    //it is valid to call this function with no data to write, just return OK
    if ( max == 0)
      return 0;

    //in case of a HEAD request, we do not write anything
    if ( _options & NetworkRequest::HeadRequest ) {
      return ( size * nmemb );
    }

    auto &rmode = std::get<running_t>( _runningMode );

    auto writeDataToFile = [ this, &rmode ]( off_t offset, const char *data, size_t len ) -> off_t {

      if ( rmode._currentRange < 0 )
        return 0;

      // if we have no open file create or open it
      if ( !rmode._outFile ) {
        std::string openMode = "w+b";
        if ( _fMode == NetworkRequest::WriteShared )
          openMode = "r+b";

        rmode._outFile = fopen( _targetFile.asString().c_str() , openMode.c_str() );

        //if the file does not exist create a new one
        if ( !rmode._outFile && _fMode == NetworkRequest::WriteShared ) {
          rmode._outFile = fopen( _targetFile.asString().c_str() , "w+b" );
        }

        if ( !rmode._outFile ) {
          strncpy( _errorBuf.data(), "Unable to open target file.", CURL_ERROR_SIZE);
          return 0;
        }
      }

      // seek to the given offset
      if ( offset >= 0 ) {
        if ( fseek( rmode._outFile, offset, SEEK_SET ) != 0 ) {
          strncpy( _errorBuf.data(), "Unable to set output file pointer.", CURL_ERROR_SIZE);
          return 0;
        }
      }

      auto &rng = _requestedRanges[ rmode._currentRange ];
      const auto bytesToWrite = rng.len > 0 ? std::min( rng.len - rng.bytesWritten, len ) : len;

      //make sure we do not write after the expected file size
      if ( _expectedFileSize && _expectedFileSize <= static_cast<zypp::ByteCount::SizeType>(rng.start + rng.bytesWritten + bytesToWrite) ) {
        strncpy( _errorBuf.data(), "Downloaded data exceeds expected length.", CURL_ERROR_SIZE);
        return 0;
      }

      auto written = fwrite( data, 1, bytesToWrite, rmode._outFile );
      if ( written == 0 )
        return 0;

      if ( rng._digest && rng._checksum.size() ) {
        if ( !rng._digest->update( data, written ) )
          return 0;
      }

      rng.bytesWritten += written;
      if ( rmode._currentSrvRange ) rmode._currentSrvRange->bytesWritten += written;

      if ( rng.len > 0 && rng.bytesWritten >= rng.len ) {
        rmode._currentRange = -1;
        validateRange( rng );
      }

      if ( rmode._currentSrvRange && rmode._currentSrvRange->len > 0 && rmode._currentSrvRange->bytesWritten >= rmode._currentSrvRange->len ) {
        rmode._currentSrvRange.reset();
        // we ran out of data in the current chunk, reset the target range as well because next data will be
        // a chunk header again
        rmode._currentRange = -1;
      }

      // count the number of real bytes we have downloaded so far
      rmode._downloaded += written;
      _sigBytesDownloaded.emit( *z_func(), rmode._downloaded );

      return written;
    };

    // we are currenty writing a range, continue until we hit the end of the requested chunk, or if we hit end of data
    size_t bytesWrittenSoFar = 0;

    while ( bytesWrittenSoFar != max ) {

      off_t seekTo = -1;

      // this is called after all headers have been processed
      if ( !rmode._allHeadersReceived ) {
        rmode._allHeadersReceived = true;

        // no ranges at all, must be a normal download
        if ( !rmode._gotMultiRangeHeader && !rmode._gotContentRangeHeader ) {

          if ( rmode._requireStatusPartial ) {
            //we got a invalid response, the status code pointed to being partial but we got no range definition
            strncpy( _errorBuf.data(), "Invalid data from server, range respone was announced but there was no range definiton.", CURL_ERROR_SIZE);
            return 0;
          }

          //we always download a range even if its not explicitely requested
          if ( _requestedRanges.empty() ) {
            _requestedRanges.push_back( NetworkRequest::Range() );
          }

          rmode._currentRange = 0;
          seekTo = _requestedRanges[0].start;
        }
      }

      if ( rmode._currentSrvRange && rmode._currentRange == -1  ) {
        //if we enter this branch, we just have finished writing a requested chunk but
        //are still inside a chunk that was sent by the server, due to the std the server can coalesce requested ranges
        //to optimize downloads we need to find the best match ( because the current offset might not even be in our requested ranges )
        //Or we just parsed a Content-Lenght header and start a new block

        std::optional<uint> foundRange;
        const size_t beginRange = rmode._currentSrvRange->start + rmode._currentSrvRange->bytesWritten;
        const size_t endRange = beginRange + (rmode._currentSrvRange->len - rmode._currentSrvRange->bytesWritten);
        auto currDist  = ULONG_MAX;
        for ( uint i = 0; i < _requestedRanges.size(); i++ ) {
          const auto &currR = _requestedRanges[i];

          //do not allow double ranges
          if ( currR._valid )
            continue;

          const auto currRBegin = currR.start + currR.bytesWritten;
          if ( !( beginRange <= currRBegin && endRange >= currRBegin ) )
            continue;

          // calculate the distance of the current ranges offset+data written to the range we got back from the server
          const auto newDist   = currRBegin - beginRange;

          if ( !foundRange ) {
            foundRange = i;
            currDist = newDist;
          } else {
            //pick the range with the closest distance
            if ( newDist < currDist ) {
              foundRange = i;
              currDist = newDist;
            }
          }
        }
        if ( !foundRange ) {
          strncpy( _errorBuf.data(), "Unable to find a matching range for data returned by the server.", CURL_ERROR_SIZE);
          return 0;
        }

        //set the found range as the current one
        rmode._currentRange = *foundRange;

        //continue writing where we stopped
        seekTo = _requestedRanges[*foundRange].start + _requestedRanges[*foundRange].bytesWritten;

        //if we skip bytes we need to advance our written bytecount
        const auto skipBytes = seekTo - beginRange;
        bytesWrittenSoFar += skipBytes;
        rmode._currentSrvRange->bytesWritten += skipBytes;
      }

      if ( rmode._currentRange >= 0 ) {
        auto availableData = max - bytesWrittenSoFar;
        if ( rmode._currentSrvRange ) {
          availableData = std::min( availableData, rmode._currentSrvRange->len - rmode._currentSrvRange->bytesWritten );
        }
        auto bw = writeDataToFile( seekTo, ptr + bytesWrittenSoFar, availableData );
        if ( bw <= 0 )
          return 0;

        bytesWrittenSoFar += bw;
      }

      if ( bytesWrittenSoFar == max )
        return max;

      if ( rmode._currentRange == -1 ) {

        // we still are inside the current range from the server
        if ( rmode._currentSrvRange )
          continue;

        boost::string_view incoming( ptr + bytesWrittenSoFar, max - bytesWrittenSoFar );
        auto hdrEnd = incoming.find("\r\n\r\n");
        if ( hdrEnd == incoming.npos ) {
          //no header end in the data yet, push to buffer and return
           rmode._rangePrefaceBuffer.insert( rmode._rangePrefaceBuffer.end(), incoming.begin(), incoming.end() );
           return max;
        }

        //append the data of the current header to the buffer and parse it
        rmode._rangePrefaceBuffer.insert( rmode._rangePrefaceBuffer.end(), incoming.begin(), incoming.begin() + ( hdrEnd + 4 )  );
        bytesWrittenSoFar += ( hdrEnd + 4 ); //header data plus header end

        boost::string_view data( rmode._rangePrefaceBuffer.data(), rmode._rangePrefaceBuffer.size() );
        auto sepStrIndex = data.find( rmode._seperatorString );
        if ( sepStrIndex == data.npos ) {
          strncpy( _errorBuf.data(), "Invalid multirange header format, seperator string missing.", CURL_ERROR_SIZE);
          return 0;
        }

        auto startOfHeader = sepStrIndex + rmode._seperatorString.length();

        std::vector<boost::string_view> lines;
        str::split( data.substr( startOfHeader ), std::back_inserter(lines), "\r\n", zypp::str::TRIM );
        for ( const auto &hdrLine : lines ) {
          if ( hdrLine.starts_with("Content-Range:") ) {
            NetworkRequest::Range r;
            //if we can not parse the header the message must be broken
            if(! parseContentRangeHeader( hdrLine, r.start, r.len ) ) {
              WAR << "Broken header for Network Request to " << _url << std::endl;
              return 0;
            }
            rmode._currentSrvRange = r;
            break;
          }
        }
        //clear the buffer again
        rmode._rangePrefaceBuffer.clear();
      }
    }
    return bytesWrittenSoFar;
  }

  NetworkRequest::NetworkRequest(zyppng::Url url, zypp::filesystem::Pathname targetFile, zyppng::NetworkRequest::FileMode fMode)
    : Base ( *new NetworkRequestPrivate( std::move(url), std::move(targetFile), std::move(fMode) ) )
  {
  }

  NetworkRequest::~NetworkRequest()
  {
    Z_D();

    if ( d->_dispatcher )
      d->_dispatcher->cancel( *this, "Request destroyed while still running" );
  }

  void NetworkRequest::setExpectedFileSize( zypp::ByteCount expectedFileSize )
  {
    d_func()->_expectedFileSize = std::move( expectedFileSize );
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

  void NetworkRequest::addRequestRange(size_t start, size_t len, DigestPtr digest, CheckSum expectedChkSum , std::any userData)
  {
    Z_D();
    if ( state() == Running )
      return;

    NetworkRequest::Range r;
    r.start = start;
    r.len = len;
    r._digest   = std::move( digest );
    r._checksum = std::move( expectedChkSum );
    r.userData  = std::move( userData );

    d->_requestedRanges.push_back( std::move(r) );
  }

  void NetworkRequest::resetRequestRanges()
  {
    Z_D();
    if ( state() == Running )
      return;
    d->_requestedRanges.clear();
  }

  std::vector<NetworkRequest::Range> NetworkRequest::failedRanges() const
  {
    const auto mystate = state();
    if ( mystate != Finished && mystate != Error )
      return {};

    Z_D();

    std::vector<Range> failed;
    for ( const auto &r : d->_requestedRanges ) {
      if ( !r._valid )
        failed.push_back( r );
    }
    return failed;
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

    const auto &rmode = std::get<NetworkRequestPrivate::running_t>( d->_runningMode );
    return peek_data_fd( rmode._outFile, offset, count );
  }

  Url NetworkRequest::url() const
  {
    return d_func()->_url;
  }

  void NetworkRequest::setUrl(const Url &url)
  {
    Z_D();
    if ( state() == NetworkRequest::Running )
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

  zypp::ByteCount NetworkRequest::reportedByteCount() const
  {
    return std::visit([](auto&& arg) -> zypp::ByteCount {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, NetworkRequestPrivate::pending_t>)
        return zypp::ByteCount(0);
      else if constexpr (std::is_same_v<T, NetworkRequestPrivate::running_t>
                         || std::is_same_v<T, NetworkRequestPrivate::finished_t>)
        return arg._contentLenght;
      else
        static_assert(always_false<T>::value, "Unhandled state type");
    }, d_func()->_runningMode);
  }

  zypp::ByteCount NetworkRequest::downloadedByteCount() const
  {
    return std::visit([](auto&& arg) -> zypp::ByteCount {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, NetworkRequestPrivate::pending_t>)
        return zypp::ByteCount();
      else if constexpr (std::is_same_v<T, NetworkRequestPrivate::running_t>
                          || std::is_same_v<T, NetworkRequestPrivate::finished_t>)
        return arg._downloaded;
      else
        static_assert(always_false<T>::value, "Unhandled state type");
    }, d_func()->_runningMode);
  }

  TransferSettings &NetworkRequest::transferSettings()
  {
    return d_func()->_settings;
  }

  NetworkRequest::State NetworkRequest::state() const
  {
    return std::visit([this](auto&& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, NetworkRequestPrivate::pending_t>)
        return Pending;
      else if constexpr (std::is_same_v<T, NetworkRequestPrivate::running_t>)
        return Running;
      else if constexpr (std::is_same_v<T, NetworkRequestPrivate::finished_t>) {
        if ( std::get<NetworkRequestPrivate::finished_t>( d_func()->_runningMode )._result.isError() )
          return Error;
        else
          return Finished;
      }
      else
        static_assert(always_false<T>::value, "Unhandled state type");
    }, d_func()->_runningMode);
  }

  NetworkRequestError NetworkRequest::error() const
  {
    const auto s = state();
    if ( s != Error && s != Finished )
      return NetworkRequestError();
    return std::get<NetworkRequestPrivate::finished_t>( d_func()->_runningMode)._result;
  }

  std::string NetworkRequest::extendedErrorString() const
  {
    if ( !hasError() )
      return std::string();

    return error().nativeErrorString();
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

  SignalProxy<void (NetworkRequest &req, zypp::ByteCount count)> NetworkRequest::sigBytesDownloaded()
  {
    return d_func()->_sigBytesDownloaded;
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
