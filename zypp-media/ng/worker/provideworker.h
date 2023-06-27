/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_MEDIA_PROVIDE_WORKER_H_INCLUDED
#define ZYPP_MEDIA_PROVIDE_WORKER_H_INCLUDED

#include <zypp-core/zyppng/base/Base>
#include <zypp-core/zyppng/base/EventLoop>
#include <zypp-core/zyppng/base/Timer>
#include <zypp-core/zyppng/io/AsyncDataSource>
#include <zypp-core/zyppng/rpc/MessageStream>
#include <zypp-core/zyppng/pipelines/Expected>
#include <zypp-proto/media/provider.pb.h>
#include <zypp-media/ng/provide-configvars.h>
#include <zypp-media/ng/private/providemessage_p.h>
#include <zypp-media/ng/HeaderValueMap>
#include <zypp-media/MediaException>
#include <zypp-media/Mount>

#include <string_view>
#include <deque>

namespace zyppng::worker {

  using WorkerCaps     = zypp::proto::Capabilities;
  using Message        = zypp::proto::Envelope;
  using Configuration  = zypp::proto::Configuration;

  struct AuthInfo
  {
    std::string username;
    std::string password;
    int64_t last_auth_timestamp = 0;
    std::map<std::string, std::string> extraKeys = {};
  };

  class RequestCancelException : public zypp::media::MediaException
  {
  public:
    RequestCancelException();
  };

  ZYPP_FWD_DECL_TYPE_WITH_REFS (ProvideWorker);
  ZYPP_FWD_DECL_TYPE_WITH_REFS (ProvideWorkerItem);

  class ProvideWorkerItem : public zyppng::Base
  {
  public:
    enum State {
      Pending,
      Running,
      Finished
    };

    ProvideWorkerItem( ProvideMessage &&spec ) : _spec( std::move(spec) ) { }

    State _state = Pending;
    ProvideMessage _spec;
  };

  class ProvideWorker : public Base
  {
  public:

    enum ProvideNotificatioMode {
      ONLY_NEW_PROVIDES,  // provide is called only when new provide requests are added to the queue
      QUEUE_NOT_EMTPY     // provide is called continiously until the queue is empty
    };

    ProvideWorker( std::string_view workerName );
    virtual ~ProvideWorker();

    RpcMessageStream::Ptr messageStream() const;

    expected<void> run ( int recv = STDIN_FILENO, int send = STDOUT_FILENO );

    std::deque<ProvideWorkerItemRef> &requestQueue();
    /*!
     * Called when the worker process exits
     */
    virtual void immediateShutdown (){};

    /*!
     * This will request a media change from the user and BLOCK until it was acknowledged.
     *
     */
    enum MediaChangeRes {
      SUCCESS,
      ABORT,
      SKIP
    };
    MediaChangeRes requestMediaChange ( const uint32_t id, const std::string &label, const int32_t mediaNr, const std::vector<std::string> &devices, const std::optional<std::string> &desc = {} );

    /*!
     * This will send a authorization request message to the controller, asking for credentials for a given \a url.
     * The \a lastTimstamp should be initialized with the last \ref AuthInfo timestamp that was received from the controller
     * or -1 if none was received before.
     *
     * \note this blocks until a answer is received, all other received messages are delayed
     */
    expected<AuthInfo> requireAuthorization ( const uint32_t id, const zypp::Url &url, const std::string &lastTriedUsername = "", const int64_t lastTimestamp = -1, const std::map<std::string, std::string> &extraFields = {} );

    ProvideNotificatioMode provNotificationMode() const;
    void setProvNotificationMode(const ProvideNotificatioMode &provNotificationMode);

  protected:
    virtual void initLog();
    virtual expected<WorkerCaps> initialize ( const Configuration &conf ) = 0;

    /*!
     * Automatically called whenever a new item is enqueued.
     */
    virtual void provide ( ) = 0;
    virtual void cancel  ( const std::deque<ProvideWorkerItemRef>::iterator &request ) = 0;

    /*!
     * Always called to create new items for the request queue,
     * override this to populate the queue with instances of custom \ref ProvideItem subclasses.
     *
     * Cancel requests are directly handled by calling cancel(), however Attach and Detach requests are enqueued as well
     */
    virtual ProvideWorkerItemRef makeItem (ProvideMessage &&spec );

    /*!
     * Send a \a ProvideStart signal to the controller, this is to notify the controller that we have started providing the file
     * the argument \a localFile has to refer to the file where the file will be provided into, it will be used to calculate
     * statistics about download speed on the controller side.
     * The \a stagingFile argument can be used for cases where the worker uses a staging area to download files into but later moves
     * the file over to the result filename. In those cases the provider will check both locations for the file when calculating stats
     *
     * \note Always call \ref ref before sending this message.
     */
    void provideStart   ( const uint32_t id, const zypp::Url &url, const zypp::Pathname &localFile, const zypp::Pathname &stagingFile = {}  );

    /*!
     * Send a \a ProvideSuccess message to the controller. This is to signal that we are finished with providing a file
     * and release the file to be used by the controller side.
     */
    void provideSuccess (const uint32_t id, bool cacheHit, const zypp::Pathname &localFile, const HeaderValueMap extra = {} );

    /*!
     * Send a \a ProvideFailed message to the controller. This is to signal that we are failed providing a resource
     *
     * \note If the request referenced a \a ident before make sure to manually release it after sending the message.
     */
    void provideFailed  ( const uint32_t id, const uint code, const std::string &reason, const bool transient, const HeaderValueMap extra = {} );

    /*!
     * Overload of provideFailed that takes a \ref zypp::Exception to fill in the error details
     *
     * \note If the request referenced a \a ident before make sure to manually release it after sending the message.
     */
    void provideFailed  ( const uint32_t id, const uint code, const bool transient, const zypp::Exception &e );

    /*!
     * Send a \a AttachSuccess message to the controller. This is to signal that we are finished with mounting and verifying a medium
     */
    void attachSuccess ( const uint32_t id );

    /*!
     * Send a \a DetachSuccess message to the controller. This is to signal that we are finished unmounting a medium
     */
    void detachSuccess ( const uint32_t id );

    /*!
     * Send a \a Redirect message to the controller for the given request ID. This is similar to sending a \a ProvideSuccess message
     * and the request will be removed from the queue, which means the worker also has to remove it from its internal queue.
     */
    void redirect       ( const uint32_t id, const zypp::Url &url, const zypp::Pathname &newPath );

    /*!
     * Returns the control IO datasource, only valid after \ref run was called
     */
    AsyncDataSource &controlIO ();


  private:
    expected<void> executeHandshake ();
    void maybeDelayedShutdown ();
    void messageLoop ( Timer & );
    void readFdClosed  ( uint, AsyncDataSource::ChannelCloseReason );
    void writeFdClosed ( AsyncDataSource::ChannelCloseReason );
    void messageReceived ();
    void onInvalidMessageReceived ( );
    void invalidMessageReceived ( std::exception_ptr p );
    void handleSingleMessage (const ProvideMessage &provide );
    void pushSingleMessage ( const RpcMessage &msg );
    expected<ProvideMessage> sendAndWaitForResponse ( const ProvideMessage &request, const std::vector<uint> &responseCodes );
    expected<ProvideMessage> parseReceivedMessage( const RpcMessage &m );

  private:
    ProvideNotificatioMode _provNotificationMode = QUEUE_NOT_EMTPY;
    bool _inControllerRequest = false; //< Used to signalize that we are currently in a blocking controller callback
    bool _isRunning = false;
    std::string_view _workerName;
    EventLoop::Ptr _loop = EventLoop::create();
    Timer::Ptr _msgAvail = Timer::create();
    Timer::Ptr _delayedShutdown = Timer::create();
    AsyncDataSource::Ptr _controlIO;
    RpcMessageStream::Ptr _stream;
    Configuration _workerConf;

    std::exception_ptr _fatalError; //< Error that caused the eventloop to stop

    std::deque<ProvideMessage> _pendingMessages;
    std::deque<ProvideWorkerItemRef> _pendingProvides;
  };
}


#endif
