/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#include <zypp-curl/ng/network/private/request_p.h>
#include <zypp-curl/ng/network/private/networkrequesterror_p.h>
#include <zypp-curl/ng/network/networkrequestdispatcher.h>
#include <zypp-curl/ng/network/private/mediadebug_p.h>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <zypp-core/zyppng/core/String>
#include <zypp-curl/private/curlhelper_p.h>
#include <zypp-curl/CurlConfig>
#include <zypp-curl/auth/CurlAuthData>
#include <zypp-media/MediaConfig>
#include <zypp-core/base/String.h>
#include <zypp-core/base/StringV.h>
#include <zypp-core/Pathname.h>
#include <curl/curl.h>
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

  NetworkRequest::Range NetworkRequest::Range::make(size_t start, size_t len, zyppng::NetworkRequest::DigestPtr &&digest, zyppng::NetworkRequest::CheckSumBytes &&expectedChkSum, std::any &&userData, std::optional<size_t> digestCompareLen, std::optional<size_t> dataBlockPadding )
  {
    return NetworkRequest::Range {
      .start = start,
      .len   = len,
      .bytesWritten = 0,
      ._digest   = std::move( digest ),
      ._checksum = std::move( expectedChkSum ),
      ._relevantDigestLen = std::move( digestCompareLen ),
      ._chksumPad  = std::move( dataBlockPadding ),
      .userData = std::move( userData ),
      ._rangeState = State::Pending
    };
  }

  NetworkRequestPrivate::prepareNextRangeBatch_t::prepareNextRangeBatch_t(running_t &&prevState)
    : _outFile( std::move(prevState._outFile) )
    , _downloaded( prevState._downloaded )
    , _rangeAttemptIdx( prevState._rangeAttemptIdx )
  { }

  NetworkRequestPrivate::running_t::running_t( pending_t &&prevState )
    : _requireStatusPartial( prevState._requireStatusPartial )
  { }

  NetworkRequestPrivate::running_t::running_t( prepareNextRangeBatch_t &&prevState )
    : _outFile( std::move(prevState._outFile) )
    , _requireStatusPartial( true )
    , _downloaded( prevState._downloaded )
    , _rangeAttemptIdx( prevState._rangeAttemptIdx )
  { }

  NetworkRequestPrivate::NetworkRequestPrivate(Url &&url, zypp::Pathname &&targetFile, NetworkRequest::FileMode fMode , NetworkRequest &p)
    : BasePrivate(p)
    , _url ( std::move(url) )
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
    return setupHandle ( errBuf );
  }

  bool NetworkRequestPrivate::setupHandle( std::string &errBuf )
  {
    ::internal::setupZYPP_MEDIA_CURL_DEBUG( _easyHandle );
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

      if ( _options & NetworkRequest::ConnectionTest ) {
        setCurlOption( CURLOPT_CONNECT_ONLY, 1L );
        setCurlOption( CURLOPT_FRESH_CONNECT, 1L );
      }
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
      }

      if( !( _options & NetworkRequest::ConnectionTest ) && !( _options & NetworkRequest::HeadRequest ) ){
        if ( _requestedRanges.size() ) {
          if ( ! prepareNextRangeBatch ( errBuf ))
            return false;
        } else {
          std::visit( [&]( auto &arg ){
            using T = std::decay_t<decltype(arg)>;
            if constexpr ( std::is_same_v<T, pending_t> ) {
              arg._requireStatusPartial = false;
            } else {
              DBG << _easyHandle << " " << "NetworkRequestPrivate::setupHandle called in unexpected state" << std::endl;
            }
          }, _runningMode );
          _requestedRanges.push_back( NetworkRequest::Range() );
          _requestedRanges.back()._rangeState = NetworkRequest::State::Running;
        }
      }

      //make a local copy of the settings, so headers are not added multiple times
      TransferSettings locSet = _settings;

      if ( _dispatcher ) {
        locSet.setUserAgentString( _dispatcher->agentString().c_str() );

        // add custom headers as configured (bsc#955801)
        const auto &cHeaders = _dispatcher->hostSpecificHeaders();
        if ( auto i = cHeaders.find(_url.getHost()); i != cHeaders.end() ) {
          for ( const auto &[key, value] : i->second ) {
            locSet.addHeader( zypp::str::trim( zypp::str::form(
              "%s: %s", key.c_str(), value.c_str() )
            ));
          }
        }
      }

      locSet.addHeader("Pragma:");

      locSet.setTimeout( zypp::MediaConfig::instance().download_transfer_timeout() );
      locSet.setConnectTimeout( CONNECT_TIMEOUT );

      /** Force IPv4/v6 */
      switch ( zypp::env::ZYPP_MEDIA_CURL_IPRESOLVE() )
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
          DBG << _easyHandle << " "  << "Enabling HTTP authentication methods: " << use_auth
              << " (CURLOPT_HTTPAUTH=" << auth << ")" << std::endl;
          setCurlOption(CURLOPT_HTTPAUTH, auth);
        }
      }

      if ( locSet.proxyEnabled() && ! locSet.proxy().empty() )
      {
        DBG << _easyHandle << " " << "Proxy: '" << locSet.proxy() << "'" << std::endl;
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
            DBG << _easyHandle << " "  << "Proxy: ~/.curlrc does not contain the proxy-user option" << std::endl;
          else
          {
            proxyuserpwd = curlconf.proxyuserpwd;
            DBG << _easyHandle << " " << "Proxy: using proxy-user from ~/.curlrc" << std::endl;
          }
        }
        else
        {
          DBG << _easyHandle << " "  << _easyHandle << " "  << "Proxy: using provided proxy-user '" << _settings.proxyUsername() << "'" << std::endl;
        }

        if ( ! proxyuserpwd.empty() )
        {
          setCurlOption(CURLOPT_PROXYUSERPWD, ::internal::curlUnEscape( proxyuserpwd ).c_str());
        }
      }
#if CURLVERSION_AT_LEAST(7,19,4)
      else if ( locSet.proxy() == EXPLICITLY_NO_PROXY )
      {
        // Explicitly disabled in URL (see fillSettingsFromUrl()).
        // This should also prevent libcurl from looking into the environment.
        DBG << _easyHandle << " "  << "Proxy: explicitly NOPROXY" << std::endl;
        setCurlOption(CURLOPT_NOPROXY, "*");
      }

#endif
      else
      {
        DBG << _easyHandle << " " << "Proxy: not explicitly set" << std::endl;
        DBG << _easyHandle << " " << "Proxy: libcurl may look into the environment" << std::endl;
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
        MIL << _easyHandle << " " << "No cookies requested" << std::endl;
      setCurlOption(CURLOPT_COOKIEJAR, _currentCookieFile.c_str() );

#if CURLVERSION_AT_LEAST(7,18,0)
      // bnc #306272
      setCurlOption(CURLOPT_PROXY_TRANSFER_MODE, 1L );
#endif

      // append settings custom headers to curl
      for ( const auto &header : locSet.headers() ) {
        if ( !z_func()->addRequestHeader( header.c_str() ) )
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

  bool NetworkRequestPrivate::assertOutputFile()
  {
    auto rmode = std::get_if<NetworkRequestPrivate::running_t>( &_runningMode );
    if ( !rmode ) {
      DBG << _easyHandle << "Can only create output file in running mode" << std::endl;
      return false;
    }
    // if we have no open file create or open it
    if ( !rmode->_outFile ) {
      std::string openMode = "w+b";
      if ( _fMode == NetworkRequest::WriteShared )
        openMode = "r+b";

      rmode->_outFile = fopen( _targetFile.asString().c_str() , openMode.c_str() );

      //if the file does not exist create a new one
      if ( !rmode->_outFile && _fMode == NetworkRequest::WriteShared ) {
        rmode->_outFile = fopen( _targetFile.asString().c_str() , "w+b" );
      }

      if ( !rmode->_outFile ) {
        rmode->_cachedResult = NetworkRequestErrorPrivate::customError(  NetworkRequestError::InternalError
          ,zypp::str::Format("Unable to open target file (%1%). Errno: (%2%:%3%)") % _targetFile.asString() % errno % strerr_cxx() );
        return false;
      }
    }

    return true;
  }

  bool NetworkRequestPrivate::canRecover() const
  {
    // We can recover from RangeFail errors if we have more batch sizes to try
    auto rmode = std::get_if<NetworkRequestPrivate::running_t>( &_runningMode );
    if ( rmode->_cachedResult && rmode->_cachedResult->type() == NetworkRequestError::RangeFail )
      return ( rmode->_rangeAttemptIdx + 1 < sizeof( _rangeAttempt ) ) && hasMoreWork();
    return false;
  }

  bool NetworkRequestPrivate::prepareToContinue( std::string &errBuf )
  {
    auto rmode = std::get_if<NetworkRequestPrivate::running_t>( &_runningMode );

    if ( hasMoreWork() ) {
      // go to the next range batch level if we are restarted due to a failed range request
      if ( rmode->_cachedResult && rmode->_cachedResult->type() == NetworkRequestError::RangeFail ) {
        if ( rmode->_rangeAttemptIdx + 1 >= sizeof( _rangeAttempt ) ) {
          errBuf = "No more range batch sizes available";
          return false;
        }
        rmode->_rangeAttemptIdx++;
      }

      _runningMode = prepareNextRangeBatch_t( std::move(std::get<running_t>( _runningMode )) );

      // we reset the handle to default values. We do this to not run into
      // "transfer closed with outstanding read data remaining" error CURL sometimes returns when
      // we cancel a connection because of a range error to request a smaller batch.
      // The error will still happen but much less frequently than without resetting the handle.
      //
      // Note: Even creating a new handle will NOT fix the issue
      curl_easy_reset( _easyHandle );
      if ( !setupHandle (errBuf) )
        return false;
      return true;
    }
    errBuf = "Request has no more work";
    return false;

  }

  bool NetworkRequestPrivate::prepareNextRangeBatch(std::string &errBuf)
  {
    if ( _requestedRanges.size() == 0 ) {
      errBuf = "Calling the prepareNextRangeBatch function without a range to download is not supported.";
      return false;
    }

    std::string rangeDesc;
    uint rangesAdded = 0;
    if ( _requestedRanges.size() > 1 && _protocolMode != ProtocolMode::HTTP ) {
      errBuf = "Using more than one range is not supported with protocols other than HTTP/HTTPS";
      return false;
    }

    // check if we have one big range convering the whole file
    if ( _requestedRanges.size() == 1 && _requestedRanges.front().start == 0 && _requestedRanges.front().len == 0 ) {
      if ( !std::holds_alternative<pending_t>( _runningMode ) ) {
        errBuf = zypp::str::Str() << "Unexpected state when calling prepareNextRangeBatch " << _runningMode.index ();
        return false;
      }

      _requestedRanges[0]._rangeState = NetworkRequest::Running;
      std::get<pending_t>( _runningMode )._requireStatusPartial = false;

    } else {
      std::sort( _requestedRanges.begin(), _requestedRanges.end(), []( const auto &elem1, const auto &elem2 ){
        return ( elem1.start < elem2.start );
      });

      if ( std::holds_alternative<pending_t>( _runningMode ) )
        std::get<pending_t>( _runningMode )._requireStatusPartial = true;

      auto maxRanges = _rangeAttempt[0];
      if ( std::holds_alternative<prepareNextRangeBatch_t>( _runningMode ) )
        maxRanges = _rangeAttempt[std::get<prepareNextRangeBatch_t>( _runningMode )._rangeAttemptIdx];

      // helper function to build up the request string for the range
      auto addRangeString = [ &rangeDesc, &rangesAdded ]( const std::pair<size_t, size_t> &range ) {
        std::string rangeD = zypp::str::form("%llu-", static_cast<unsigned long long>( range.first ) );
        if( range.second > 0 )
          rangeD.append( zypp::str::form( "%llu", static_cast<unsigned long long>( range.second ) ) );

        if ( rangeDesc.size() )
          rangeDesc.append(",").append( rangeD );
        else
          rangeDesc = std::move( rangeD );

        rangesAdded++;
      };

      std::optional<std::pair<size_t, size_t>> currentZippedRange;
      bool closedRange = true;
      for ( auto &range : _requestedRanges ) {

        if ( range._rangeState != NetworkRequest::Pending )
          continue;

        //reset the download results
        range.bytesWritten = 0;

        //when we have a open range in the list of ranges we will get from start of range to end of file,
        //all following ranges would never be marked as valid, so we have to fail early
        if ( !closedRange ) {
          errBuf = "It is not supported to request more ranges after a open range.";
          return false;
        }

        const auto rangeEnd = range.len > 0 ? range.start + range.len - 1 : 0;
        closedRange = (rangeEnd > 0);

        // remember this range was already requested
        range._rangeState  = NetworkRequest::Running;
        range.bytesWritten = 0;
        if ( range._digest )
          range._digest->reset();

        // we try to compress the requested ranges into as big chunks as possible for the request,
        // when receiving we still track the original ranges so we can collect and test their checksums
        if ( !currentZippedRange ) {
          currentZippedRange = std::make_pair( range.start, rangeEnd );
        } else {
          //range is directly consecutive to the previous range
          if ( currentZippedRange->second + 1 == range.start ) {
            currentZippedRange->second = rangeEnd;
          } else {
            //this range does not directly follow the previous one, we build the string and start a new one
            addRangeString( *currentZippedRange );
            currentZippedRange = std::make_pair( range.start, rangeEnd );
          }
        }

        if ( rangesAdded >= maxRanges ) {
          MIL << _easyHandle << " " << "Reached max nr of ranges (" << maxRanges << "), batching the request to not break the server" << std::endl;
          break;
        }
      }

      // add the last range too
      if ( currentZippedRange )
        addRangeString( *currentZippedRange );

      MIL << _easyHandle << " " << "Requesting Ranges: " << rangeDesc << std::endl;

      setCurlOption( CURLOPT_RANGE, rangeDesc.c_str() );
    }

    return true;
  }

  bool NetworkRequestPrivate::hasMoreWork() const
  {
    // check if we have ranges that have never been requested
    return std::any_of( _requestedRanges.begin(), _requestedRanges.end(), []( const auto &range ){ return range._rangeState == NetworkRequest::Pending; });
  }

  void NetworkRequestPrivate::aboutToStart( )
  {
    bool isRangeContinuation = std::holds_alternative<prepareNextRangeBatch_t>( _runningMode );
    if ( isRangeContinuation ) {
      MIL << _easyHandle << " " << "Continuing a previously started range batch." << std::endl;
      _runningMode = running_t( std::move(std::get<prepareNextRangeBatch_t>( _runningMode )) );
    } else {
      auto mode = running_t( std::move(std::get<pending_t>( _runningMode )) );
      if ( _requestedRanges.size() == 1 && _requestedRanges.front().start == 0 && _requestedRanges.front().len == 0 )
        mode._currentRange = 0;

      _runningMode = std::move(mode);
    }

    auto &m = std::get<running_t>( _runningMode );

    if ( m._activityTimer ) {
      DBG_MEDIA << _easyHandle << " Setting activity timeout to: " << _settings.timeout() << std::endl;
      m._activityTimer->connect( &Timer::sigExpired, *this, &NetworkRequestPrivate::onActivityTimeout );
      m._activityTimer->start( static_cast<uint64_t>( _settings.timeout() * 1000 ) );
    }

    if ( !isRangeContinuation )
      _sigStarted.emit( *z_func() );
  }

  void NetworkRequestPrivate::dequeueNotify()
  {
    if ( std::holds_alternative<running_t>(_runningMode) ) {
      auto &rmode = std::get<running_t>( _runningMode );
      // if we still have a current range set it valid by checking the checksum
      if ( rmode._currentRange >= 0 ) {
        auto &currR = _requestedRanges[rmode._currentRange];
        rmode._currentRange = -1;
        validateRange( currR );
      }
    }
  }

  void NetworkRequestPrivate::setResult( NetworkRequestError &&err )
  {

    finished_t resState;
    resState._result = std::move(err);

    if ( std::holds_alternative<running_t>(_runningMode) ) {

      auto &rmode = std::get<running_t>( _runningMode );
      rmode._outFile.reset();
      resState._downloaded = rmode._downloaded;
      resState._contentLenght = rmode._contentLenght;

      if ( resState._result.type() == NetworkRequestError::NoError && !(_options & NetworkRequest::HeadRequest) && !(_options & NetworkRequest::ConnectionTest) ) {
        //we have a successful download lets see if we got everything we needed
        for ( const auto &r : _requestedRanges ) {
          if ( r._rangeState != NetworkRequest::Finished ) {
            if ( r.len > 0 && r.bytesWritten != r.len )
              resState._result = NetworkRequestErrorPrivate::customError( NetworkRequestError::MissingData, (zypp::str::Format("Did not receive all requested data from the server ( off: %1%, req: %2%, recv: %3% ).") % r.start % r.len % r.bytesWritten ) );
            else if ( r._digest && r._checksum.size() && ! checkIfRangeChkSumIsValid(r) )  {
              resState._result = NetworkRequestErrorPrivate::customError( NetworkRequestError::InvalidChecksum, (zypp::str::Format("Invalid checksum %1%, expected checksum %2%") % r._digest->digest() % zypp::Digest::digestVectorToString( r._checksum ) ) );
            } else {
              resState._result = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, (zypp::str::Format("Download of block failed.") ) );
            }
            //we only report the first error
            break;
          }
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
    std::for_each( _requestedRanges.begin (), _requestedRanges.end(), []( auto &range ) {
        range._rangeState = NetworkRequest::Pending;
    });
  }

  void NetworkRequestPrivate::onActivityTimeout( Timer &t )
  {
    auto &m = std::get<running_t>( _runningMode );

    MIL_MEDIA << _easyHandle << " Request timeout interval: " << t.interval()<< " remaining: " << t.remaining() <<  std::endl;
    std::map<std::string, boost::any> extraInfo;
    extraInfo.insert( {"requestUrl", _url } );
    extraInfo.insert( {"filepath", _targetFile } );
    _dispatcher->cancel( *z_func(), NetworkRequestErrorPrivate::customError( NetworkRequestError::Timeout, "Download timed out", std::move(extraInfo) ) );
  }

  bool NetworkRequestPrivate::checkIfRangeChkSumIsValid ( const NetworkRequest::Range &rng )
  {
    if ( rng._digest && rng._checksum.size() ) {
      auto bytesHashed = rng._digest->bytesHashed ();
      if ( rng._chksumPad && *rng._chksumPad > bytesHashed ) {
        MIL_MEDIA << _easyHandle << " " << "Padding the digest to required block size" << std::endl;
        zypp::ByteArray padding( *rng._chksumPad - bytesHashed, '\0' );
        rng._digest->update( padding.data(), padding.size() );
      }
      auto digVec = rng._digest->digestVector();
      if ( rng._relevantDigestLen ) {
        digVec.resize( *rng._relevantDigestLen );
      }
      return ( digVec == rng._checksum );
    }

    // no checksum required
    return true;
  }

  void NetworkRequestPrivate::validateRange( NetworkRequest::Range &rng )
  {
    if ( rng._digest && rng._checksum.size() ) {
      if ( ( rng.len == 0 || rng.bytesWritten == rng.len ) && checkIfRangeChkSumIsValid(rng) )
        rng._rangeState = NetworkRequest::Finished;
      else
        rng._rangeState = NetworkRequest::Error;
    } else {
      if ( rng.len == 0 ? true : rng.bytesWritten == rng.len )
        rng._rangeState = NetworkRequest::Finished;
      else
        rng._rangeState = NetworkRequest::Error;
    }
  }

  bool NetworkRequestPrivate::parseContentRangeHeader(const std::string_view &line, size_t &start, size_t &len )
  {                                     //content-range: bytes 10485760-19147879/19147880
    static const zypp::str::regex regex("^Content-Range:[[:space:]]+bytes[[:space:]]+([0-9]+)-([0-9]+)\\/([0-9]+)$", zypp::str::regex::rxdefault | zypp::str::regex::icase );

    zypp::str::smatch what;
    if( !zypp::str::regex_match( std::string(line), what, regex ) || what.size() != 4 ) {
      DBG << _easyHandle << " " << "Invalid Content-Range Header format: '" << std::string(line) << std::endl;
      return false;
    }

    size_t s = zypp::str::strtonum<size_t>( what[1]);
    size_t e = zypp::str::strtonum<size_t>( what[2]);
    start = std::move(s);
    len   = ( e - s ) + 1;
    return true;
  }

  bool NetworkRequestPrivate::parseContentTypeMultiRangeHeader(const std::string_view &line, std::string &boundary)
  {
    static const zypp::str::regex regex("^Content-Type:[[:space:]]+multipart\\/byteranges;[[:space:]]+boundary=(.*)$", zypp::str::regex::rxdefault | zypp::str::regex::icase );

    zypp::str::smatch what;
    if( zypp::str::regex_match( std::string(line), what, regex )  ) {
      if ( what.size() >= 2 ) {
        boundary = what[1];
        return true;
      }
    }
    return false;
  }

  std::string NetworkRequestPrivate::errorMessage() const
  {
    return std::string( _errorBuf.data() );
  }

  void NetworkRequestPrivate::resetActivityTimer()
  {
    if ( std::holds_alternative<running_t>( _runningMode ) ){
      auto &rmode = std::get<running_t>( _runningMode );
      if ( rmode._activityTimer && rmode._activityTimer->isRunning() )
        rmode._activityTimer->start();
    }
  }

  int NetworkRequestPrivate::curlProgressCallback( void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow )
  {
    if ( !clientp )
      return CURLE_OK;
    NetworkRequestPrivate *that = reinterpret_cast<NetworkRequestPrivate *>( clientp );

    if ( !std::holds_alternative<running_t>(that->_runningMode) ){
      DBG << that->_easyHandle << " " << "Curl progress callback was called in invalid state "<< that->z_func()->state() << std::endl;
      return -1;
    }

    auto &rmode = std::get<running_t>( that->_runningMode );

    //reset the timer
    that->resetActivityTimer();

    rmode._isInCallback = true;
    if ( rmode._lastProgressNow != dlnow ) {
      rmode._lastProgressNow = dlnow;
      that->_sigProgress.emit( *that->z_func(), dltotal, dlnow, ultotal, ulnow );
    }
    rmode._isInCallback = false;

    return rmode._cachedResult ? CURLE_ABORTED_BY_CALLBACK : CURLE_OK;
  }

  size_t NetworkRequestPrivate::headerCallback(char *ptr, size_t size, size_t nmemb)
  {
    //it is valid to call this function with no data to write, just return OK
    if ( size * nmemb == 0)
      return 0;

    resetActivityTimer();

    if ( _protocolMode == ProtocolMode::HTTP ) {

      std::string_view hdr( ptr, size*nmemb );

      hdr.remove_prefix( std::min( hdr.find_first_not_of(" \t\r\n"), hdr.size() ) );
      const auto lastNonWhitespace = hdr.find_last_not_of(" \t\r\n");
      if ( lastNonWhitespace != hdr.npos )
        hdr.remove_suffix( hdr.size() - (lastNonWhitespace + 1) );
      else
        hdr = std::string_view();

      auto &rmode = std::get<running_t>( _runningMode );
      if ( !hdr.size() ) {
        return ( size * nmemb );
      }
      if ( zypp::strv::hasPrefixCI( hdr, "HTTP/" ) ) {

        long statuscode = 0;
        (void)curl_easy_getinfo( _easyHandle, CURLINFO_RESPONSE_CODE, &statuscode);

        const auto &doRangeFail = [&](){
          WAR << _easyHandle << " " << "Range FAIL, trying with a smaller batch" << std::endl;
          rmode._cachedResult = NetworkRequestErrorPrivate::customError(  NetworkRequestError::RangeFail,  "Expected range status code 206, but got none." );

          // reset all ranges we requested to pending, we never got the data for them
          std::for_each( _requestedRanges.begin (), _requestedRanges.end(), []( auto &range ) {
            if ( range._rangeState == NetworkRequest::Running )
              range._rangeState = NetworkRequest::Pending;
          });
          return 0;
        };

        // if we have a status 204 we need to create a empty file
        if( statuscode == 204 && !( _options & NetworkRequest::ConnectionTest ) && !( _options & NetworkRequest::HeadRequest ) )
          assertOutputFile();

        if ( rmode._requireStatusPartial ) {
          // ignore other status codes, maybe we are redirected etc.
          if ( ( statuscode >= 200 && statuscode <= 299 && statuscode != 206 )
                || statuscode == 416 ) {
              return doRangeFail();
          }
        }

      } else if ( zypp::strv::hasPrefixCI( hdr, "Location:" ) ) {
        _lastRedirect = hdr.substr( 9 );
        DBG << _easyHandle << " " << "redirecting to " << _lastRedirect << std::endl;

      } else if ( zypp::strv::hasPrefixCI( hdr, "Content-Type:") ) {
        std::string sep;
        if ( parseContentTypeMultiRangeHeader( hdr, sep ) ) {
          rmode._gotMultiRangeHeader = true;
          rmode._seperatorString = "--"+sep;
        }
      } else if ( zypp::strv::hasPrefixCI( hdr, "Content-Range:") ) {
        NetworkRequest::Range r;
        if ( !parseContentRangeHeader( hdr, r.start, r.len) ) {
          rmode._cachedResult = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, "Invalid Content-Range header format." );
          return 0;
        }
        DBG << _easyHandle << " " << "Got content range :" << r.start << " len " << r.len << std::endl;
        rmode._gotContentRangeHeader = true;
        rmode._currentSrvRange = r;

      } else if ( zypp::strv::hasPrefixCI( hdr, "Content-Length:") )  {
        auto lenStr = str::trim( hdr.substr( 15 ), zypp::str::TRIM );
        auto str = std::string ( lenStr.data(), lenStr.length() );
        auto len = zypp::str::strtonum<typename zypp::ByteCount::SizeType>( str.data() );
        if ( len > 0 ) {
          DBG << _easyHandle << " " << "Got Content-Length Header: " << len << std::endl;
          rmode._contentLenght = zypp::ByteCount(len, zypp::ByteCount::B);
        }
      }
    }

    return ( size * nmemb );
  }

  size_t NetworkRequestPrivate::writeCallback(char *ptr, size_t size, size_t nmemb)
  {
    const auto max = ( size * nmemb );

    resetActivityTimer();

    //it is valid to call this function with no data to write, just return OK
    if ( max == 0)
      return 0;

    //in case of a HEAD request, we do not write anything
    if ( _options & NetworkRequest::HeadRequest ) {
      return ( size * nmemb );
    }

    auto &rmode = std::get<running_t>( _runningMode );

    auto writeDataToFile = [ this, &rmode ]( off_t offset, const char *data, size_t len ) -> off_t {

      if ( rmode._currentRange < 0 ) {
        DBG << _easyHandle << " " << "Current range is zero in write request" << std::endl;
        return 0;
      }

      // if we have no open file create or open it
      if ( !assertOutputFile() )
        return 0;

      // seek to the given offset
      if ( offset >= 0 ) {
        if ( fseek( rmode._outFile, offset, SEEK_SET ) != 0 ) {
          rmode._cachedResult = NetworkRequestErrorPrivate::customError(  NetworkRequestError::InternalError,
            "Unable to set output file pointer." );
          return 0;
        }
      }

      auto &rng = _requestedRanges[ rmode._currentRange ];
      const auto bytesToWrite = rng.len > 0 ? std::min( rng.len - rng.bytesWritten, len ) : len;

      //make sure we do not write after the expected file size
      if ( _expectedFileSize && _expectedFileSize <= static_cast<zypp::ByteCount::SizeType>(rng.start + rng.bytesWritten + bytesToWrite) ) {
        rmode._cachedResult = NetworkRequestErrorPrivate::customError(  NetworkRequestError::InternalError, "Downloaded data exceeds expected length." );
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
            rmode._cachedResult = NetworkRequestErrorPrivate::customError(  NetworkRequestError::ServerReturnedError,
              "Invalid data from server, range respone was announced but there was no range definiton." );
            return 0;
          }

          //we always download a range even if it is not explicitly requested
          if ( _requestedRanges.empty() ) {
            _requestedRanges.push_back( NetworkRequest::Range() );
            _requestedRanges.back()._rangeState = NetworkRequest::State::Running;
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

          // do not allow double ranges
          if ( currR._rangeState == NetworkRequest::Finished || currR._rangeState == NetworkRequest::Error )
            continue;

          // check if the range was already written
          if ( currR.len == currR.bytesWritten )
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
          rmode._cachedResult = NetworkRequestErrorPrivate::customError(  NetworkRequestError::InternalError
            , "Unable to find a matching range for data returned by the server." );
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

        std::string_view incoming( ptr + bytesWrittenSoFar, max - bytesWrittenSoFar );
        auto hdrEnd = incoming.find("\r\n\r\n");
        if ( hdrEnd == incoming.npos ) {
          //no header end in the data yet, push to buffer and return
           rmode._rangePrefaceBuffer.insert( rmode._rangePrefaceBuffer.end(), incoming.begin(), incoming.end() );
           return max;
        }

        //append the data of the current header to the buffer and parse it
        rmode._rangePrefaceBuffer.insert( rmode._rangePrefaceBuffer.end(), incoming.begin(), incoming.begin() + ( hdrEnd + 4 )  );
        bytesWrittenSoFar += ( hdrEnd + 4 ); //header data plus header end

        std::string_view data( rmode._rangePrefaceBuffer.data(), rmode._rangePrefaceBuffer.size() );
        auto sepStrIndex = data.find( rmode._seperatorString );
        if ( sepStrIndex == data.npos ) {
          rmode._cachedResult = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError,
            "Invalid multirange header format, seperator string missing." );
          return 0;
        }

        auto startOfHeader = sepStrIndex + rmode._seperatorString.length();
        std::vector<std::string_view> lines;
        zypp::strv::split( data.substr( startOfHeader ), "\r\n", zypp::strv::Trim::trim, [&]( std::string_view strv ) { lines.push_back(strv); } );
        for ( const auto &hdrLine : lines ) {
          if ( zypp::strv::hasPrefixCI(hdrLine, "Content-Range:") ) {
            NetworkRequest::Range r;
            //if we can not parse the header the message must be broken
            if(! parseContentRangeHeader( hdrLine, r.start, r.len ) ) {
              rmode._cachedResult = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, "Invalid Content-Range header format." );
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

  ZYPP_IMPL_PRIVATE(NetworkRequest)

  NetworkRequest::NetworkRequest(zyppng::Url url, zypp::filesystem::Pathname targetFile, zyppng::NetworkRequest::FileMode fMode)
    : Base ( *new NetworkRequestPrivate( std::move(url), std::move(targetFile), std::move(fMode), *this ) )
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

  void NetworkRequest::setPriority( NetworkRequest::Priority prio, bool triggerReschedule )
  {
    Z_D();
    d->_priority = prio;
    if ( state() == Pending && triggerReschedule && d->_dispatcher )
      d->_dispatcher->reschedule();
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

  void NetworkRequest::addRequestRange( size_t start, size_t len, DigestPtr digest, CheckSumBytes expectedChkSum , std::any userData, std::optional<size_t> digestCompareLen, std::optional<size_t> chksumpad  )
  {
    Z_D();
    if ( state() == Running )
      return;

    d->_requestedRanges.push_back( Range::make( start, len, std::move(digest), std::move( expectedChkSum ), std::move( userData ), digestCompareLen, chksumpad ) );
  }

  void NetworkRequest::addRequestRange( const NetworkRequest::Range &range )
  {
    Z_D();
    if ( state() == Running )
      return;

    d->_requestedRanges.push_back( range );
    auto &rng = d->_requestedRanges.back();
    rng._rangeState = NetworkRequest::Pending;
    rng.bytesWritten = 0;
    if ( rng._digest )
      rng._digest->reset();
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
      if ( r._rangeState != NetworkRequest::Finished )
        failed.push_back( r );
    }
    return failed;
  }

  const std::vector<NetworkRequest::Range> &NetworkRequest::requestedRanges() const
  {
    return d_func()->_requestedRanges;
  }

  const std::string &NetworkRequest::lastRedirectInfo() const
  {
    return d_func()->_lastRedirect;
  }

  void *NetworkRequest::nativeHandle() const
  {
    return d_func()->_easyHandle;
  }

  std::optional<zyppng::NetworkRequest::Timings> NetworkRequest::timings() const
  {
    const auto myerr = error();
    const auto mystate = state();
    if ( mystate != Finished )
      return {};

    Timings t;

    auto getMeasurement = [ this ]( const CURLINFO info, std::chrono::microseconds &target ){
      using FPSeconds = std::chrono::duration<double, std::chrono::seconds::period>;
      double val = 0;
      const auto res = curl_easy_getinfo( d_func()->_easyHandle, info, &val );
      if ( CURLE_OK == res ) {
        target = std::chrono::duration_cast<std::chrono::microseconds>( FPSeconds(val) );
      }
    };

    getMeasurement( CURLINFO_NAMELOOKUP_TIME, t.namelookup );
    getMeasurement( CURLINFO_CONNECT_TIME, t.connect);
    getMeasurement( CURLINFO_APPCONNECT_TIME, t.appconnect);
    getMeasurement( CURLINFO_PRETRANSFER_TIME , t.pretransfer);
    getMeasurement( CURLINFO_TOTAL_TIME, t.total);
    getMeasurement( CURLINFO_REDIRECT_TIME, t.redirect);

    return t;
  }

  std::vector<char> NetworkRequest::peekData( off_t offset, size_t count ) const
  {
    Z_D();

    if ( !std::holds_alternative<NetworkRequestPrivate::running_t>( d->_runningMode) )
      return {};

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

  void NetworkRequest::setTargetFilePath( const zypp::filesystem::Pathname &path )
  {
    Z_D();
    if ( state() == NetworkRequest::Running )
      return;
    d->_targetFile = path;
  }

  NetworkRequest::FileMode NetworkRequest::fileOpenMode() const
  {
    return d_func()->_fMode;
  }

  void NetworkRequest::setFileOpenMode( FileMode mode )
  {
    Z_D();
    if ( state() == NetworkRequest::Running )
      return;
    d->_fMode = std::move( mode );
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
    return std::visit([](auto& arg) -> zypp::ByteCount {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, NetworkRequestPrivate::pending_t> || std::is_same_v<T, NetworkRequestPrivate::prepareNextRangeBatch_t> )
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
    return std::visit([](auto& arg) -> zypp::ByteCount {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, NetworkRequestPrivate::pending_t>)
        return zypp::ByteCount();
      else if constexpr (std::is_same_v<T, NetworkRequestPrivate::running_t>
                          || std::is_same_v<T, NetworkRequestPrivate::prepareNextRangeBatch_t>
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
    return std::visit([this](auto& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, NetworkRequestPrivate::pending_t>)
        return Pending;
      else if constexpr (std::is_same_v<T, NetworkRequestPrivate::running_t> || std::is_same_v<T, NetworkRequestPrivate::prepareNextRangeBatch_t> )
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
