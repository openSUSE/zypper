/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaCurl.cc
 *
*/

#include <iostream>
#include <list>
#include <chrono>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/Gettext.h>

#include <zypp-core/parser/Sysconfig>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-core/zyppng/base/EventLoop>
#include <zypp-core/zyppng/base/private/threaddata_p.h>

#include <zypp-curl/ng/network/Downloader>
#include <zypp-curl/ng/network/NetworkRequestDispatcher>
#include <zypp-curl/ng/network/DownloadSpec>
#include <zypp-media/MediaConfig>

#include <zypp/media/MediaNetwork.h>
#include <zypp-media/auth/CredentialManager>
#include <zypp-curl/private/curlhelper_p.h>
#include <zypp/Target.h>
#include <zypp/ZYppFactory.h>
#include <zypp/ZConfig.h>


using std::endl;

namespace internal {


    struct ProgressTracker {

      using clock = std::chrono::steady_clock;

      std::optional<clock::time_point> _timeStart; ///< Start total stats
      std::optional<clock::time_point> _timeLast;	 ///< Start last period(~1sec)

      double _dnlTotal	= 0.0;	///< Bytes to download or 0 if unknown
      double _dnlLast	= 0.0;	  ///< Bytes downloaded at period start
      double _dnlNow	= 0.0;	  ///< Bytes downloaded now

      int    _dnlPercent= 0;	///< Percent completed or 0 if _dnlTotal is unknown

      double _drateTotal= 0.0;	///< Download rate so far
      double _drateLast	= 0.0;	///< Download rate in last period

      void updateStats( double dltotal = 0.0, double dlnow = 0.0 )
      {
        clock::time_point now = clock::now();

        if ( !_timeStart )
          _timeStart = _timeLast = now;

        // If called without args (0.0), recompute based on the last values seen
        if ( dltotal && dltotal != _dnlTotal )
          _dnlTotal = dltotal;

        if ( dlnow && dlnow != _dnlNow ) {
          _dnlNow = dlnow;
        }

        // percentage:
        if ( _dnlTotal )
          _dnlPercent = int(_dnlNow * 100 / _dnlTotal);

        // download rates:
        _drateTotal = _dnlNow / std::max( std::chrono::duration_cast<std::chrono::seconds>(now - *_timeStart).count(), int64_t(1) );

        if ( _timeLast < now )
        {
          _drateLast = (_dnlNow - _dnlLast) / int( std::chrono::duration_cast<std::chrono::seconds>(now - *_timeLast).count() );
          // start new period
          _timeLast  = now;
          _dnlLast   = _dnlNow;
        }
        else if ( _timeStart == _timeLast )
          _drateLast = _drateTotal;
      }
    };


  // All media handler instances share the same EventDispatcher and Downloader
  // This is released at application shutdown.
  struct SharedData {

    ~SharedData() {
      MIL << "Releasing internal::SharedData for MediaNetwork." << std::endl;
    }

    static std::shared_ptr<SharedData> instance ()  {
      static std::shared_ptr<SharedData> data = std::shared_ptr<SharedData>( new SharedData() );
      return data;
    }

    // we need to keep a reference
    zyppng::EventDispatcherRef _dispatcher;
    zyppng::DownloaderRef _downloader;

    private:
      SharedData() {
        MIL << "Initializing internal::SharedData for MediaNetwork" << std::endl;
        _dispatcher = zyppng::ThreadData::current().ensureDispatcher();
        _downloader = std::make_shared<zyppng::Downloader>();
        _downloader->requestDispatcher()->setMaximumConcurrentConnections( zypp::MediaConfig::instance().download_max_concurrent_connections() );
      }
  };

}

using namespace internal;
using namespace zypp::base;

namespace zypp {

  namespace media {

  MediaNetwork::MediaNetwork( const Url &      url_r,
                              const Pathname & attach_point_hint_r )
      : MediaNetworkCommonHandler( url_r, attach_point_hint_r,
                                  "/",   // urlpath at attachpoint
                                  true ) // does_download
  {
    MIL << "MediaNetwork::MediaNetwork(" << url_r << ", " << attach_point_hint_r << ")" << endl;

    // make sure there is a event loop and downloader instance
    _shared = internal::SharedData::instance();

    if( !attachPoint().empty())
    {
      PathInfo ainfo(attachPoint());
      Pathname apath(attachPoint() + "XXXXXX");
      char    *atemp = ::strdup( apath.asString().c_str());
      char    *atest = NULL;
      if( !ainfo.isDir() || !ainfo.userMayRWX() ||
          atemp == NULL || (atest=::mkdtemp(atemp)) == NULL)
      {
        WAR << "attach point " << ainfo.path()
            << " is not useable for " << url_r.getScheme() << endl;
        setAttachPoint("", true);
      }
      else if( atest != NULL)
        ::rmdir(atest);

      if( atemp != NULL)
        ::free(atemp);
    }
  }

  void MediaNetwork::attachTo (bool next)
  {
    if ( next )
      ZYPP_THROW(MediaNotSupportedException(_url));

    if ( !_url.isValid() )
      ZYPP_THROW(MediaBadUrlException(_url));

    // use networkdispatcher check if the scheme is supported
    if ( !_shared->_downloader->requestDispatcher()->supportsProtocol( _url ) ) {
      std::string msg("Unsupported protocol '");
      msg += _url.getScheme();
      msg += "'";
      ZYPP_THROW(MediaBadUrlException(_url, msg));
    }

    if( !isUseableAttachPoint( attachPoint() ) )
    {
      setAttachPoint( createAttachPoint(), true );
    }

    disconnectFrom();

    MediaSourceRef media( new MediaSource(_url.getScheme(), _url.asString()));
    setMediaSource(media);
  }

  bool
  MediaNetwork::checkAttachPoint(const Pathname &apoint) const
  {
    return MediaHandler::checkAttachPoint( apoint, true, true);
  }

  void MediaNetwork::disconnectFrom()
  {
  }

  void MediaNetwork::releaseFrom( const std::string & ejectDev )
  {
    disconnect();
  }

  void MediaNetwork::runRequest ( const zyppng::DownloadSpec &spec, callback::SendReport<DownloadProgressReport> *report ) const
  {
    auto ev = zyppng::EventLoop::create();
    std::vector<zyppng::connection> signalConnections;
    OnScopeExit deferred([&](){
      while( signalConnections.size() ) {
        signalConnections.back().disconnect();
        signalConnections.pop_back();
      }
    });

    zyppng::DownloadRef dl = _shared->_downloader->downloadFile( spec );
    std::optional<internal::ProgressTracker> progTracker;

    const auto &startedSlot = [&]( zyppng::Download &req ){
      if ( !report) return;
      (*report)->start( spec.url(), spec.targetPath());
    };

    const auto &aliveSlot = [&]( zyppng::Download &req, off_t dlNow ){
      if ( !report || !progTracker )
        return;
      progTracker->updateStats( 0.0, dlNow );
      if ( !(*report)->progress( progTracker->_dnlPercent, spec.url(), progTracker-> _drateTotal, progTracker->_drateLast ) )
        req.cancel();
    };

    const auto &progressSlot = [&]( zyppng::Download &req, off_t dlTotal, off_t dlNow ) {
      if ( !report || !progTracker )
        return;

      progTracker->updateStats( dlTotal, dlNow );
      if ( !(*report)->progress( progTracker->_dnlPercent, spec.url(), progTracker-> _drateTotal, progTracker->_drateLast ) )
        req.cancel();
    };

    const auto &finishedSlot = [&]( zyppng::Download & ){
      ev->quit();
    };

    bool firstTry = true;
    const auto &authRequiredSlot = [&]( zyppng::Download &req, zyppng::NetworkAuthData &auth, const std::string &availAuth ){

      //! \todo need a way to pass different CredManagerOptions here
      CredentialManager cm(CredManagerOptions(ZConfig::instance().repoManagerRoot()));
      CurlAuthData_Ptr credentials;

      // get stored credentials
      AuthData_Ptr cmcred = cm.getCred(_url);
      if ( cmcred && auth.lastDatabaseUpdate() < cmcred->lastDatabaseUpdate() ) {
        credentials.reset(new CurlAuthData(*cmcred));
        DBG << "got stored credentials:" << endl << *credentials << endl;

      } else {
        // if not found, ask user
        CurlAuthData_Ptr curlcred;
        curlcred.reset(new CurlAuthData());
        callback::SendReport<AuthenticationReport> auth_report;

        // preset the username if present in current url
        if (!_url.getUsername().empty() && firstTry)
          curlcred->setUsername(_url.getUsername());
        // if CM has found some credentials, preset the username from there
        else if (cmcred)
          curlcred->setUsername(cmcred->username());

        // indicate we have no good credentials from CM
        cmcred.reset();

        std::string prompt_msg = str::Format(_("Authentication required for '%s'")) % _url.asString();

        // set available authentication types from the signal
        // might be needed in prompt
        curlcred->setAuthType( availAuth );

        // ask user
        if (auth_report->prompt(_url, prompt_msg, *curlcred))
        {
          DBG << "callback answer: retry" << endl
              << "CurlAuthData: " << *curlcred << endl;

          if (curlcred->valid())
          {
            credentials = curlcred;
              // if (credentials->username() != _url.getUsername())
              //   _url.setUsername(credentials->username());
              /**
               *  \todo find a way to save the url with changed username
               *  back to repoinfo or dont store urls with username
               *  (and either forbid more repos with the same url and different
               *  user, or return a set of credentials from CM and try them one
               *  by one)
               */
          }
        }
        else
        {
          DBG << "callback answer: cancel" << endl;
        }
      }

      if ( !credentials  ) {
        auth = zyppng::NetworkAuthData();
        return;
      }

      auth = *credentials;
      if (!cmcred) {
        credentials->setUrl(_url);
        cm.addCred(*credentials);
        cm.save();
      }
    };

    signalConnections.insert( signalConnections.end(), {
      dl->connectFunc( &zyppng::Download::sigStarted, startedSlot),
      dl->connectFunc( &zyppng::Download::sigFinished, finishedSlot ),
      dl->connectFunc( &zyppng::Download::sigAuthRequired, authRequiredSlot )
    });

    if ( report ) {
      progTracker = internal::ProgressTracker();
      signalConnections.insert( signalConnections.end(), {
        dl->connectFunc( &zyppng::Download::sigAlive, aliveSlot ),
        dl->connectFunc( &zyppng::Download::sigProgress, progressSlot ),
      });
    }

    dl->start();
    ev->run();

    std::for_each( signalConnections.begin(), signalConnections.end(), []( auto &conn ) { conn.disconnect(); });

    if ( report ) {
      if ( dl->hasError() ) {
        auto errCode = zypp::media::DownloadProgressReport::ERROR;
        std::exception_ptr excp;
        const auto &error = dl->lastRequestError();
        switch ( error.type() ) {
          case zyppng::NetworkRequestError::InternalError:
          case zyppng::NetworkRequestError::InvalidChecksum:
          case zyppng::NetworkRequestError::UnsupportedProtocol:
          case zyppng::NetworkRequestError::MalformedURL:
          case zyppng::NetworkRequestError::PeerCertificateInvalid:
          case zyppng::NetworkRequestError::ConnectionFailed:
          case zyppng::NetworkRequestError::ServerReturnedError:
          case zyppng::NetworkRequestError::MissingData:  {
            excp = ZYPP_EXCPT_PTR( zypp::media::MediaCurlException( spec.url(), error.toString(), error.nativeErrorString() ) );
            break;
          }
          case zyppng::NetworkRequestError::Cancelled: {
            excp = ZYPP_EXCPT_PTR( zypp::media::MediaRequestCancelledException( error.toString() ) );
            break;
          }
          case zyppng::NetworkRequestError::ExceededMaxLen: {
            excp = ZYPP_EXCPT_PTR( zypp::media::MediaFileSizeExceededException( spec.url(), spec.expectedFileSize() ) );
            break;
          }
          case zyppng::NetworkRequestError::TemporaryProblem: {
            excp = ZYPP_EXCPT_PTR( zypp::media::MediaTemporaryProblemException( spec.url(), error.toString() ) );
            break;
          }
          case zyppng::NetworkRequestError::Timeout: {
            excp = ZYPP_EXCPT_PTR( zypp::media::MediaTimeoutException( spec.url(), error.toString() ) );
            break;
          }
          case zyppng::NetworkRequestError::Forbidden: {
            excp = ZYPP_EXCPT_PTR( zypp::media::MediaForbiddenException( spec.url(), error.toString() ) );
            break;
          }
          case zyppng::NetworkRequestError::NotFound: {
            errCode = zypp::media::DownloadProgressReport::NOT_FOUND;

            //@BUG using getPathName() can result in wrong error messages
            excp = ZYPP_EXCPT_PTR( zypp::media::MediaFileNotFoundException( _url, spec.url().getPathName() ) );
            break;
          }
          case zyppng::NetworkRequestError::Unauthorized:
          case zyppng::NetworkRequestError::AuthFailed: {
            errCode = zypp::media::DownloadProgressReport::ACCESS_DENIED;
            excp = ZYPP_EXCPT_PTR( zypp::media::MediaUnauthorizedException( spec.url(), error.toString(), error.nativeErrorString(), "" ) );
            break;
          }
          case zyppng::NetworkRequestError::NoError:
            // should never happen
            DBG << "BUG: Download error flag is set , but Error code is NoError" << std::endl;
            break;
        }

        if ( excp ) {
          (*report)->finish( spec.url(), errCode, error.toString() );
          std::rethrow_exception( excp );
        }
      }
      (*report)->finish( spec.url(), zypp::media::DownloadProgressReport::NO_ERROR, "" );
    }
  }

  void MediaNetwork::getFile( const OnMediaLocation &file ) const
  {
    // Use absolute file name to prevent access of files outside of the
    // hierarchy below the attach point.
    getFileCopy( file, localPath(file.filename()).absolutename() );
  }

  void MediaNetwork::getFileCopy( const OnMediaLocation & file, const Pathname & targetFilename ) const
  {
    const auto &filename = file.filename();
    Url fileurl(getFileUrl(filename));

    DBG << "FILEURL IS: " << fileurl << std::endl;
    DBG << "Downloading to: " << targetFilename << std::endl;

    if( assert_dir( targetFilename.dirname() ) ) {
      DBG << "assert_dir " << targetFilename.dirname() << " failed" << endl;
      ZYPP_THROW( MediaSystemException(getFileUrl(file.filename()), "System error on " + targetFilename.dirname().asString()) );
    }

    zyppng::DownloadSpec spec = zyppng::DownloadSpec( fileurl, targetFilename, file.downloadSize() )
      .setDeltaFile( file.deltafile() )
      .setHeaderSize( file.headerSize())
      .setHeaderChecksum( file.headerChecksum() )
      .setTransferSettings( this->_settings );

    callback::SendReport<DownloadProgressReport> report;
    runRequest( spec, &report );
  }

  bool MediaNetwork::getDoesFileExist( const Pathname & filename ) const
  {
    try
    {
      const auto &targetFilePath = localPath(filename).absolutename();
      Url fileurl(getFileUrl(filename));

      zyppng::DownloadSpec spec = zyppng::DownloadSpec( fileurl, targetFilePath )
        .setCheckExistsOnly( true )
        .setTransferSettings( this->_settings );

      runRequest( spec );
      // if we get to here the request worked.
      return true;
    }
    // unexpected exception
    catch (MediaException & excpt_r)
    {
      ZYPP_RETHROW(excpt_r);
    }

    return false;
  }

  void MediaNetwork::getDir( const Pathname & dirname, bool recurse_r ) const
  {
    filesystem::DirContent content;
    getDirInfo( content, dirname, /*dots*/false );

    for ( filesystem::DirContent::const_iterator it = content.begin(); it != content.end(); ++it ) {
        Pathname filename = dirname + it->name;
        int res = 0;

        switch ( it->type ) {
        case filesystem::FT_NOT_AVAIL: // old directory.yast contains no typeinfo at all
        case filesystem::FT_FILE:
          getFile( OnMediaLocation( filename ) );
          break;
        case filesystem::FT_DIR: // newer directory.yast contain at least directory info
          if ( recurse_r ) {
            getDir( filename, recurse_r );
          } else {
            res = assert_dir( localPath( filename ) );
            if ( res ) {
              WAR << "Ignore error (" << res <<  ") on creating local directory '" << localPath( filename ) << "'" << endl;
            }
          }
          break;
        default:
          // don't provide devices, sockets, etc.
          break;
        }
    }
  }

  void MediaNetwork::getDirInfo( std::list<std::string> & retlist,
                                const Pathname & dirname, bool dots ) const
  {
    getDirectoryYast( retlist, dirname, dots );
  }

  void MediaNetwork::getDirInfo( filesystem::DirContent & retlist,
                              const Pathname & dirname, bool dots ) const
  {
    getDirectoryYast( retlist, dirname, dots );
  }

  } // namespace media
} // namespace zypp
//
