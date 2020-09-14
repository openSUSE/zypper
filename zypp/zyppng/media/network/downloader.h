#ifndef ZYPP_NG_MEDIA_CURL_DOWNLOADER_H_INCLUDED
#define ZYPP_NG_MEDIA_CURL_DOWNLOADER_H_INCLUDED

#include <zypp/zyppng/base/zyppglobal.h>
#include <zypp/zyppng/base/Base>
#include <zypp/zyppng/base/signals.h>
#include <zypp/zyppng/core/Url>
#include <zypp/zyppng/media/network/networkrequesterror.h>
#include <zypp/zyppng/media/network/AuthData>

#include <zypp/ByteCount.h>

namespace zypp {
  namespace media {
    class TransferSettings;
  }
}

namespace zyppng {

  class NetworkRequestDispatcher;
  class DownloaderPrivate;
  class Download;
  class MirrorControl;

  using TransferSettings = zypp::media::TransferSettings;

  /**
   * @brief The Downloader class
   *
   * Provides a high level interface to the \sa HttpRequestDispatcher,
   * implementing Metalink on top. If in doubt which one to use, always
   * use this one.
   */
  class LIBZYPP_NG_EXPORT Downloader : public Base
  {
    ZYPP_DECLARE_PRIVATE( Downloader )
  public:

    using Ptr = std::shared_ptr<Downloader>;
    using WeakPtr = std::shared_ptr<Downloader>;

    Downloader();
    Downloader( std::shared_ptr<MirrorControl> mc );
    virtual ~Downloader();

    /*!
     * Generates a new Download object in waiting state
     * \param file the \sa zyppng::Url to the source file that should be downloaded
     * \param targetPath the local file path where the downloaded file needs to be stored
     * \param expectedFileSize The expected file size of the source file, can be empty
     */
    std::shared_ptr<Download> downloadFile ( Url file, zypp::filesystem::Pathname targetPath, zypp::ByteCount expectedFileSize = zypp::ByteCount() );

    /*!
     * Returns the internally used \sa zyppng::NetworkRequestDispatcher used by the \a Downloader
     * to enqueue network requests
     */
    std::shared_ptr<NetworkRequestDispatcher> requestDispatcher () const;

    /*!
     * Emitted when a \sa zyppng::Download created by this Downloader instance was started
     */
    SignalProxy<void ( Downloader &parent, Download& download )> sigStarted  ( );

    /*!
     * Signal that is emitted when a \sa zyppng::Download created by this Downloader instance was finished
     * \note Just as with \sa zyppng::NetworkRequest 's the finished signal does not mean the Download was successful
     */
    SignalProxy<void ( Downloader &parent, Download& download )> sigFinished ( );

    /*!
     * Signal is always emitted when there are not Downloads anymore waiting in the queue
     */
    SignalProxy<void ( Downloader &parent )> queueEmpty ( );
  };

  class DownloadPrivate;

  /*!
   * The Download class represents a possibly multipart download.
   *
   * The \a Download class is a more high level interface compared to \sa zyppng::NetworkRequest, it can internally
   * make use of features like metalinks and tries to be as resilient as possible with errors during a download. Due to this
   * it is represented internally as a state machine. All transitions are signalled and can be followed.
   *
   * \code
   * zyppng::EventLoop::Ptr loop = zyppng::EventLoop::create();
   * zyppng::Downloader dl;
   *
   * dl.queueEmpty().connect( [ &loop ]( zyppng::Downloader & ) {
   *   loop->quit();
   * });
   *
   * zypp::Url url ( "https://download.opensuse.org/distribution/leap/15.0/repo/oss/x86_64/0ad-0.0.22-lp150.2.10.x86_64.rpm" );
   * zypp::Pathname target("/tmp/0ad-0.0.22-lp150.2.10.x86_64.rpm");
   *
   * std::shared_ptr<zyppng::Download> req = dl.downloadFile( url, target );
   * req->sigStarted().connect( []( zyppng::Download &dl ) {
   *   std::cout << "Download started: " << dl.targetPath() << std::endl;
   * });
   *
   * req->sigFinished().connect( []( zyppng::Download &dl ) {
   *   std::cout << "Download finished: " << dl.targetPath() << std::endl;
   *   if ( dl.state() != zyppng::Download::Success )
   *     std::cout << "\t has error: " << dl.errorString() << std::endl;
   * });
   *
   * req->sigAlive().connect( []( zyppng::Download &dl, off_t dlnow ) {
   *   std::cout << dl.targetPath().asString() << " at: " << std::endl
   *             << "dlnow: "<< dlnow<< std::endl;
   * });
   *
   * req->sigProgress().connect( []( zyppng::Download &dl, off_t dltotal, off_t dlnow ) {
   *   std::cout << dl.targetPath().asString() << " at: " << std::endl
   *             << "dltotal: "<< dltotal<< std::endl
   *             << "dlnow: "<< dlnow<< std::endl;
   * });
   *
   * req->start();
   * loop->run();
   *
   * \endcode
   *
   */
  class LIBZYPP_NG_EXPORT Download : public Base
  {
    ZYPP_DECLARE_PRIVATE( Download )

  public:

    using Ptr = std::shared_ptr<Download>;
    using WeakPtr = std::shared_ptr<Download>;

    /*!
     * The states of the internal state machine. Each of them represents a different
     * stage of the lifetime of a Download.
     */
    enum State {
      InitialState,  //< This is the initial state, its only set before a download starts
      Initializing = 10,  //< This state is kept during the first 265Bytes or until the data looks like a Metalink download
      Running      = 20,  //< This state is set once there have been more than 256 bytes downloaded and it does not look like a metalink download
      PrepareMulti = 25,  //< This state is set for async preparations of the multi download, like mirror rating
      RunningMulti = 30,  //< Signals that the file is downloaded in chunks from different mirrors
      Success = 200, //< Shows that the Download was successful
      Failed         //< Shows that the Download failed
    };


    /*!
     * A download can only directly be created by the \sa zyppng::Downloader
     */
    Download ( DownloadPrivate &&prv );

    virtual ~Download();

    /*!
     * Returns the source URL of the download
     */
    Url url () const;

    /*!
     * Returns the target file path, this is where the downloaded data is stored
     */
    zypp::Pathname targetPath () const;

    /*!
     * Returns the current internal state of the Download
     * \sa zyppng::Download::State
     */
    State state () const;

    /*!
     * Returns the last \sa zyppng::NetworkRequestError enountered while downloading the file.
     * This will just represent the very last error that could not be recovered from. In case of
     * a Metalink download that ususally means that all mirrors and the initial URL failed to download for
     * some reason.
     */
    NetworkRequestError lastRequestError () const;

    /*!
     * Returns a readable reason why the download failed.
     * \sa lastRequestError
     */
    std::string errorString () const;

    /*!
     * Returns a writeable reference to the \sa zyppng::TransferSettings for the download. The settings are reused for
     * possible sub downloads, however authentication data is stripped if the subdownload uses a different host to
     * fetch the data from. If there is no auth data known \sa sigAuthRequired is emitted.
     */
    TransferSettings &settings ();

    /*!
     * Triggers the start of the download, this needs to be called in order for the statemachine
     * to advance.
     */
    void start ();

    /*!
     * This will raise all requests currenty in pending state to have \ref NetworkRequest::Critial priority,
     * which means they will be started even before requests with \ref NetworkRequest::High priority.
     *
     * \note Use this feature only if the request result is required right away because the application is waiting for it, overuse of this
     *       might hurt performance
     */
    void prioritize();

    /*!
     * Aborts the current download
     */
    void cancel ();

    /*!
     * Enabled or disabled multipart handling. Enabled by default.
     * \note if Multipart is enabled the Download tells the server that it accepts metalink files by adding a specific header
     *       to the request.
     */
    void setMultiPartHandlingEnabled ( bool enable = true );

    /*!
     * Enables a special mode, in this case only the existance of the file is checked but no data is actually downloaded
     */
    void setCheckExistsOnly ( bool set = true );

    /*!
     * Set a already existing local file to be used for partial downloading, in case of a multichunk download all chunks from the
     * file that have the expected checksum will be reused instead of downloaded
     */
    void setDeltaFile ( const zypp::Pathname &file );

    /*!
     * Sets the prefered amount of bytes the downloader tries to request from a single server per metalink chunk request.
     * If the metalink description has smaller chunks those are coalesced to match the preferred size.
     */
    void setPreferredChunkSize ( const zypp::ByteCount &bc );

    /*!
     * Returns a reference to the internally used \sa zyppng::NetworkRequestDispatcher
     */
    NetworkRequestDispatcher &dispatcher () const;

    /*!
     * Signals that the dispatcher dequeued the request and actually starts downloading data
     */
    SignalProxy<void ( Download &req )> sigStarted  ();

    /*!
     * Signals that the state of the \a Download has changed
     */
    SignalProxy<void ( Download &req, State state )> sigStateChanged  ();

    /*!
     * Signals that the download is alive but still in initial stage ( trying to figure out if metalink / zsync )
     */
    SignalProxy<void ( Download &req, off_t dlnow  )> sigAlive ();

    /*!
     * Signals if there was data read from the download
     */
    SignalProxy<void ( Download &req, off_t dltotal, off_t dlnow )> sigProgress ();

    /*!
     * Signals that the download finished.
     */
    SignalProxy<void ( Download &req )> sigFinished ( );

    /*!
     * Is emitted when a request requires authentication and it was not given or if auth failed.
     * A connected slot should fill in the \a auth information in order to provide login credentials.
     */
    SignalProxy<void ( Download &req, NetworkAuthData &auth, const std::string &availAuth )> sigAuthRequired ( );
  };
}

#endif
