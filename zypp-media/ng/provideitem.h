/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_MEDIA_PROVIDEITEM_H_INCLUDED
#define ZYPP_MEDIA_PROVIDEITEM_H_INCLUDED

#include <zypp-core/zyppng/base/zyppglobal.h>
#include <zypp-core/zyppng/base/Base>
#include <zypp-media/ng/ProvideFwd>
#include <zypp-core/ByteCount.h>

namespace zyppng
{
  class ProvidePrivate;
  class ProvideItemPrivate;

 /*!
   * Represents a operation added to the provide queue by the user code. A "user" operation can have multiple
   * steps of downloading and processing one or multiple files. Even though this class is in public API space it's
   * not possible to implement custom Items since more internal API would be required for it. It is only public to
   * support the \ref ProvideStatus class, so it can query details of a Item.
   */
  class ProvideItem : public Base
  {
    ZYPP_DECLARE_PRIVATE(ProvideItem);
    friend class Provide;
    friend class ProvidePrivate;
    friend class ProvideQueue;
  public:
    enum State {
      Uninitialized, //< Item was added to the queue but not yet initialized
      Pending,      //< The Item is waiting to be queued and does nothing
      Downloading,  //< The Item is fetching data
      Processing,   //< The Item has fetched all data but is still doing other things
      Cancelling,   //< The Item was cancelled and waits
      Finalizing,   //< The Item has finished all work and is currently cleaning up
      Finished      //< The Item is done and can be dequeued
    };

    struct ItemStats {
      std::chrono::steady_clock::time_point _pulseTime;
      uint _runningRequests;
      zypp::ByteCount _bytesProvided;
      zypp::ByteCount _bytesExpected;
    };

    ProvideItem( ProvidePrivate &parent );
    ~ProvideItem ();

    /*!
     * Called by the controller when the item is supposed to start fetching / processing
     */
    virtual void initialize ()  = 0;

    /*!
     * Called when the promise reference is released by the user process, cancel all running requests if there are any and clean up
     */
    virtual void released ();

    State state () const;

    /*!
     * Signal that is emitted when the state of the Item has changed
     */
    SignalProxy<void( ProvideItem &item, State oldState, State newState )> sigStateChanged();

    ProvidePrivate &provider();

    /*!
     * Returns true if a redirect is allowed and does not conflict with previous redirects.
     * Otherwise false is returned. This does not remember the passed URL as a redirect
     */
    virtual bool canRedirectTo ( ProvideRequestRef startedReq, const zypp::Url &url );

    /*!
     * Returns the item statistics that were collected the last time pulse() was called on the item.
     * If the item has not been started yet, this returns a empty optional
     */
    const std::optional<ItemStats> &currentStats() const;

    /*!
     * Returns the item statistics that were collected the previous time pulse() was called on the item.
     * \note The item needs to be started and pulse() needs to be called at least once for this func to return something
     */
    const std::optional<ItemStats> &previousStats() const;

    /*!
     * Returns the time point when the item started to process/download.
     * If the item has not started yet, this returns epoch
     */
    virtual std::chrono::steady_clock::time_point startTime() const;

    /*!
     * Returns the time point when the item was finished.
     * If the item was not finished yet, this returns epoch
     */
    virtual std::chrono::steady_clock::time_point finishedTime() const;

    /*!
     * Updates the item statistics, this is called automatically by the \ref Provide instance and usually does not
     * need to be called explicitely by usercode.
     */
    void pulse ();

    /*!
     * Returns the bytes the item expects to provide, the default impl returns 0
     */
    virtual zypp::ByteCount bytesExpected () const;

  protected:
    virtual ItemStats makeStats ();

    /*!
     * Request received a informal message, e.g. ProvideStarted
     */
    virtual void informalMessage ( ProvideQueue &, ProvideRequestRef req, const ProvideMessage &msg  );

    /*!
     * Request had a cache miss and will be queued again, forget all about the request
     */
    virtual void cacheMiss ( ProvideRequestRef req );

    /*!
     * Request was finished by the queue
     * Base implementation handles redirect, metalink and error messages. If a different message is
     * received, \ref cancelWithError is called.
     *
     * A subclass has to overload this function to handle success messages
     */
    virtual void finishReq ( ProvideQueue &queue, ProvideRequestRef finishedReq, const ProvideMessage &msg );

    /*!
     * Request was finished with a error
     * The base implementation simply calls \ref cancelWithError
     *
     * \note \a queue is allowed to be a nullptr here
     */
    virtual void finishReq ( ProvideQueue *queue, ProvideRequestRef finishedReq, const std::exception_ptr excpt );

    /*!
     * Request needs authentication data, the function is supposed to return the AuthData to use for the response, or an error
     * The default implementation simply uses the given URL to look for a Auth match in the \ref zypp::media::CredentialManager.
     */
    virtual expected<zypp::media::AuthData> authenticationRequired ( ProvideQueue &queue, ProvideRequestRef req, const zypp::Url &effectiveUrl, int64_t lastTimestamp, const std::map<std::string, std::string> &extraFields );

    /*!
     * Remembers previous redirects and returns false if the URL was encountered before, use this
     * to prevent the item getting caught in a redirect loop
     */
    bool safeRedirectTo ( ProvideRequestRef startedReq, const zypp::Url &url );

    /*!
     * Similar to \ref safeRedirectTo, but does not check if a URL was already used by this Request before.
     */
    void redirectTo ( ProvideRequestRef startedReq, const zypp::Url &url );

    /*!
     * Enqueue the request in the correct queue, the item implementation is supposed to hold its own
     * reference to all started requests, the base implementation just keeps track of 1 request at a time.
     */
    virtual bool enqueueRequest( ProvideRequestRef request );

    /*!
     * Cancels all running requests and immediately moves to error state
     */
    virtual void cancelWithError ( std::exception_ptr error ) = 0;

    /*!
     * Dequeue this item and stop all requests in queues running
     * Call this when the item is cancelled or finished.
     */
    bool dequeue ();

    /*!
     * Call this when the state of the item changes.
     *
     * \note calling updateState with state \ref Finished will potentially delete the Item instance
     */
    void updateState( const State newState );

    void setFinished ();

  protected:
    ProvideRequestRef _runningReq;
  };
}
#endif // ZYPP_MEDIA_PROVIDEITEM_H_INCLUDED
