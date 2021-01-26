#ifndef ZYPP_NG_MEDIA_CURL_CURL_H_INCLUDED
#define ZYPP_NG_MEDIA_CURL_CURL_H_INCLUDED

#include <zypp-core/zyppng/base/zyppglobal.h>
#include <zypp-core/zyppng/base/Base>
#include <zypp-core/zyppng/base/signals.h>
#include <zypp-core/zyppng/core/Url>
#include <vector>

#include <zypp/zyppng/media/network/networkrequesterror.h>

namespace zyppng {

  class NetworkRequestDispatcherPrivate;
  class NetworkRequest;

  /*!
   * The NetworkRequestDispatcher class is used to run multiple NetworkRequest instances
   * at the same time, making full use of the event loop.
   *
   * Dispatching is implemented using a internal priority queue, all requests in the
   * queue are set to waiting. Once a request is dequeued it is initialized and started
   * right away. Its possible to change the maximum number of concurrent connections to control
   * the load on the network.
   *
   * \code
   * zyppng::EventLoop::Ptr loop = zyppng::EventLoop::create();
   * zyppng::NetworkRequestDispatcher downloader;
   *
   * zypp::Url url ( "https://download.opensuse.org/distribution/leap/15.0/repo/oss/x86_64/0ad-0.0.22-lp150.2.10.x86_64.rpm" );
   * zypp::Pathname target("/tmp/0ad-0.0.22-lp150.2.10.x86_64.rpm");
   *
   * std::shared_ptr<zyppng::NetworkRequest> req = std::make_shared<zyppng::NetworkRequest>( zypp::Url(url), target );
   *
   * std::shared_ptr<zypp::Digest> dig = std::make_shared<zypp::Digest>();
   * if ( !dig->create( zypp::Digest::md5() ) ) {
   *   std::cerr << "Unable to create Digest " << std::endl;
   *   return 1;
   * }
   *
   * req->setDigest( dig );
   * req->setExpectedChecksum( hexstr2bytes("11822f1421ae50fb1a07f72220b79000") );
   *
   * req->sigStarted().connect( [] ( const zyppng::NetworkRequest &r ){
   *   std::cout << r.url() << " started downloading " << std::endl;
   * });
   *
   * req->sigFinished().connect( [] ( const zyppng::NetworkRequest &r, const zyppng::NetworkRequestError &err ){
   *   if ( err.isError() ) {
   *     std::cout << r.url() << " finsihed with err " << err.nativeErrorString() << " : "<< err.isError() << " : " << err.toString() << std::endl;
   *   } else {
   *     std::cout << r.url() << " finished downloading " << std::endl;
   *   }
   * });
   *
   * req->sigProgress().connect( [] ( const zyppng::NetworkRequest &r, off_t dltotal, off_t dlnow, off_t ultotal, off_t ulnow ){
   *   std::cout << r.url() << " at: " << std::endl
   *             << "dltotal: "<< dltotal<< std::endl
   *             << "dlnow: "<< dlnow<< std::endl
   *             << "ultotal: "<< ultotal<< std::endl
   *             << "ulnow: "<< ulnow<< std::endl;
   * });
   *
   * downloader.enqueue( req );
   *
   * downloader.sigQueueFinished().connect( [ &loop ]( const zyppng::NetworkRequestDispatcher & ) {
   *   loop->quit();
   * });
   *
   * downloader.run( );
   * loop->run();
   * \code
   *
   * \sa zyppng::Downloader
   */
  class LIBZYPP_NG_EXPORT NetworkRequestDispatcher : public Base
  {
    ZYPP_DECLARE_PRIVATE(NetworkRequestDispatcher)
    public:

      using Ptr = std::shared_ptr<NetworkRequestDispatcher>;
      using WeakPtr = std::weak_ptr<NetworkRequestDispatcher>;

      NetworkRequestDispatcher ( );

      /*!
       * Returns true if the protocol used in the Url scheme is supported by
       * the dispatcher backend
       */
      static bool supportsProtocol ( const Url &url );

      /*!
       * Change the number of the concurrently started requests, the default is 10.
       * Setting this to -1 means there is no limit.
       */
      void setMaximumConcurrentConnections ( const int maxConn );

      /*!
       * Enqueues a new \a request and puts it into the waiting queue. If the dispatcher
       * is already running and has free capacatly the request might be started right away
       */
      void enqueue ( const std::shared_ptr<NetworkRequest> &req );

      /*!
       * Cancels the request \a req setting the error description to \a reason.
       */
      void cancel  ( NetworkRequest &req , std::string reason = std::string() );

      /*!
       * Cancels the request \a req setting the error to \a err.
       */
      void cancel  ( NetworkRequest &req , const NetworkRequestError &err );

      /*!
       * Start dispatching requests, this needs to be done explicitely before any request can be executed.
       */
      void run ( );

      /*!
       * Reschedule enqueued requests based on their priorities
       */
      void reschedule ();

      /*!
       * Returns the number of requests in the running and waiting queues
       */
      size_t count ();

      /*!
       * Returns the last encountered error in a request.
       */
      const NetworkRequestError &lastError() const;

      /*!
       * Signal is emitted when a download is started
       */
      SignalProxy<void ( NetworkRequestDispatcher &, NetworkRequest & )> sigDownloadStarted();

      /*!
       * Emitted when a request was finished, check the request to see if it was successful
       */
      SignalProxy<void ( NetworkRequestDispatcher &, NetworkRequest & )> sigDownloadFinished();

      /*!
       * Emitted when the internal request queue is completely processed and all requests are finished.
       */
      SignalProxy<void ( NetworkRequestDispatcher & )> sigQueueFinished ();

      /*!
       * Emitted when there is a error in the backend the dispatcher can not recover from. All requests are cancelled
       * use \a lastError to get more informations.
       */
      SignalProxy<void ( NetworkRequestDispatcher & )> sigError ();
   };
}


#endif
