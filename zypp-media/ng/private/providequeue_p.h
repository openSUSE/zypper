/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPP_MEDIA_PRIVATE_PROVIDE_QUEUE_P_H_INCLUDED
#define ZYPP_MEDIA_PRIVATE_PROVIDE_QUEUE_P_H_INCLUDED

#include "providefwd_p.h"
#include <zypp-media/ng/Provide>
#include <zypp-proto/media/provider.pb.h>
#include <zypp-core/zyppng/io/Process>
#include <zypp-core/ByteCount.h>

#include <deque>
#include <chrono>
#include <variant>

namespace zyppng {

  class RpcMessageStream;
  using RpcMessageStreamPtr = std::shared_ptr<RpcMessageStream>;

  class ProvideQueue : public Base
  {
  public:
    friend struct ProvideResourceData;

    static constexpr uint32_t InvalidId = (uint32_t) -1;
    using Config = zypp::proto::Capabilities;

    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

    struct Item {

      enum State {
        Pending,
        Queued,
        Running,
        Cancelling,
        Finished
      };
      State _state = Pending;
      bool isAttachRequest () const;
      bool isFileRequest () const;
      bool isDetachRequest() const;

      ProvideRequestRef _request;
    };

    ProvideQueue( ProvidePrivate &parent );
    ~ProvideQueue();
    bool startup ( const std::string &workerScheme, const zypp::Pathname &workDir, const std::string &hostname = "" );
    void enqueue ( ProvideRequestRef request );
    void cancel  ( ProvideRequest *item, std::exception_ptr error );
    void detach  ( const std::string &id );
    void scheduleNext ();
    bool canScheduleMore () const;;
    bool empty () const;

    /*!
     * Check if the queue is currently idle
     */
    bool isIdle () const;

    /*!
     * Time point since the queue started to be idle
     */
    std::optional<TimePoint> idleSince () const;

    /*!
     * How many items does this queue currently have
     */
    uint requestCount () const;

    /*!
     * How many active items does this queue currently have
     */
    uint activeRequests () const;

    /*!
     * How much bytes does this queue has to download / process,
     * for pending requests this is only set if the \ref ProvideSpec
     * has a expected download size set.
     */
    zypp::ByteCount expectedProvideSize() const;

    /*!
     * Returns the hostname this worker belongs to.
     * If the worker was not associated with a hostname this will return a empty string.
     */
    const std::string &hostname () const;

    const Config &workerConfig () const;

    SignalProxy<void()> sigIdle();

  private:
    bool doStartup ();
    void processMessage ( );
    void readAllStderr ();
    void forwardToLog ( std::string &&logLine );
    void processReadyRead( int channel );
    void procFinished ( int exitCode );
    uint32_t nextRequestId();

    /*!
     * Dequeues the request referenced by \a it.
     * Returns a iterator to the next element in the active list
     */
    std::list< ProvideQueue::Item >::iterator dequeueActive ( std::list<Item>::iterator it );
    void fatalWorkerError ( const std::exception_ptr &reason = nullptr );
    void immediateShutdown ( const std::exception_ptr &reason );

    /*!
     * Cancels the item the iterator \a i is pointing to, advancing the iterator to the next element in the list
     */
    std::list< ProvideQueue::Item >::iterator  cancelActiveItem (std::list<Item>::iterator i,  const std::exception_ptr &error );

  private:
    bool _queueShuttingDown = false;
    uint8_t  _crashCounter = 0;
    Config _capabilities;
    zypp::Pathname _currentExe;
    std::string _myHostname;
    ProvidePrivate &_parent;
    std::deque< Item > _waitQueue;
    std::list< Item >  _activeItems;
    Process::Ptr _workerProc;
    RpcMessageStreamPtr _messageStream;
    Signal<void()> _sigIdle;
    std::optional<TimePoint> _idleSince;
  };

}

#endif
