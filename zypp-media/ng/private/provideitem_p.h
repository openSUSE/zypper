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
#ifndef ZYPP_MEDIA_PRIVATE_PROVIDE_ITEM_P_H_INCLUDED
#define ZYPP_MEDIA_PRIVATE_PROVIDE_ITEM_P_H_INCLUDED

#include "providefwd_p.h"
#include "providequeue_p.h"
#include "attachedmediainfo_p.h"
#include "providemessage_p.h"
#include <zypp-media/ng/Provide>
#include <zypp-media/ng/ProvideItem>
#include <zypp-media/ng/ProvideRes>
#include <zypp-media/ng/ProvideSpec>
#include <zypp-core/zyppng/base/private/base_p.h>
#include <set>
#include <variant>

namespace zyppng {

  /*!
   * The internal request type, which represents all possible
   * user requests and exports some convenience functions for the scheduler to
   * directly access relevant data
   */
  class ProvideRequest {
  public:

    friend class ProvideItem;

    static expected<ProvideRequestRef> create( ProvideItem &owner, const std::vector<zypp::Url> &urls, const std::string &id, ProvideMediaSpec &spec );
    static expected<ProvideRequestRef> create ( ProvideItem &owner, const std::vector<zypp::Url> &urls, ProvideFileSpec &spec );
    static expected<ProvideRequestRef> createDetach( const zypp::Url &url );

    ProvideItem * owner() { return _owner; }

    uint code () const { return _message.code(); }

    void setCurrentQueue ( ProvideQueueRef ref );
    ProvideQueueRef currentQueue ();

    const ProvideMessage &provideMessage () const { return _message; }
    ProvideMessage &provideMessage () { return _message; }

    const std::optional<zypp::Url> activeUrl() const;
    void setActiveUrl ( const zypp::Url &urlToUse );

    void setUrls( const std::vector<zypp::Url> & urls ) {
      _mirrors = urls;
    }

    const std::vector<zypp::Url> &urls() const {
      return _mirrors;
    }

    zypp::Url url() const {
      return _mirrors.front();
    }

    void setUrl( const zypp::Url & url ) {
      _mirrors = {url};
    }

    void clearForRestart () {
      _pastRedirects.clear();
      _activeUrl.reset();
      _myQueue.reset();
    }

  private:
    ProvideRequest( ProvideItem *owner, const std::vector<zypp::Url> &urls, ProvideMessage &&msg ) : _owner(owner), _message(std::move(msg) ), _mirrors(urls) {}
    ProvideItem *_owner = nullptr; // destructor of ProvideItem will dequeue the item, so no need to do refcount here
    ProvideMessage _message;
    std::vector<zypp::Url>   _mirrors;
    std::vector<zypp::Url>   _pastRedirects;
    std::optional<zypp::Url> _activeUrl;
    ProvideQueueWeakRef _myQueue;
  };

  class ProvideItemPrivate : public BasePrivate
  {
    public:
      ProvideItemPrivate( ProvidePrivate & parent, ProvideItem &pub ) : BasePrivate(pub), _parent(parent) {}
      ProvidePrivate &_parent;
      ProvideItem::State _itemState = ProvideItem::Uninitialized;
      std::chrono::steady_clock::time_point _itemStarted;
      std::chrono::steady_clock::time_point _itemFinished;
      std::optional<ProvideItem::ItemStats> _prevStats;
      std::optional<ProvideItem::ItemStats> _currStats;
      Signal<void( ProvideItem &item, ProvideItem::State oldState, ProvideItem::State newState )> _sigStateChanged;
  };

  /*!
   * The object returned to the user code to track the internal Item.
   * Releasing the last reference to it will cancel the operation but the corresponding ProvideItem
   * will remain in the Queue until the cancel operation was finished.
   */
  template< typename T >
  class ProvidePromise : public AsyncOp<expected<T>>
  {
  public:
    ProvidePromise( ProvideItemRef provideItem )
      : _myProvide( provideItem )
    {}

    ~ProvidePromise()
    {
      auto prov = _myProvide.lock();
      if ( prov )
        prov->released();
    }

  private:
    ProvideItemWeakRef _myProvide; //weak reference to the internal item so we can cancel the op on desctruction
  };

  /*!
   * Item downloading and providing a file
   */
  class ProvideFileItem : public ProvideItem
  {
  public:

    static ProvideFileItemRef create ( const std::vector<zypp::Url> &urls,const ProvideFileSpec &request, ProvidePrivate &parent );

    // ProvideItem interface
    void initialize () override;
    ProvidePromiseRef<ProvideRes> promise();

    void setMediaRef ( Provide::MediaHandle &&hdl );
    Provide::MediaHandle & mediaRef ();

    ItemStats makeStats () override;
    zypp::ByteCount bytesExpected () const override;

  protected:
    ProvideFileItem ( const std::vector<zypp::Url> &urls,const ProvideFileSpec &request, ProvidePrivate &parent );

    void informalMessage ( ProvideQueue &, ProvideRequestRef req, const ProvideMessage &msg  ) override;

    using ProvideItem::finishReq;
    void finishReq ( ProvideQueue &queue, ProvideRequestRef finishedReq, const ProvideMessage &msg ) override;
    void cancelWithError ( std::exception_ptr error ) override;
    expected<zypp::media::AuthData> authenticationRequired ( ProvideQueue &queue, ProvideRequestRef req, const zypp::Url &effectiveUrl, int64_t lastTimestamp, const std::map<std::string, std::string> &extraFields ) override;

  private:
    Provide::MediaHandle _handleRef;    //< If we are using a attached media, this will keep the reference around
    bool _promiseCreated = false;
    std::vector<zypp::Url> _mirrorList; //< All available URLs, first one is the primary
    ProvideFileSpec     _initialSpec;   //< The initial spec as defined by the user code
    zypp::Pathname      _targetFile;    //< The target file as reported by the worker
    zypp::Pathname      _stagingFile;   //< The staging file as reported by the worker
    zypp::ByteCount     _expectedBytes; //< The nr of bytes we want to provide
    ProvidePromiseWeakRef<ProvideRes> _promise;
  };


  /*!
   * Item attaching and verifying a medium
   */
  class AttachMediaItem : public ProvideItem
  {
  public:
    ~AttachMediaItem();
    static AttachMediaItemRef create ( const std::vector<zypp::Url> &urls, const ProvideMediaSpec &request, ProvidePrivate &parent );
    SignalProxy< void( const zyppng::expected<AttachedMediaInfo *> & ) > sigReady ();

    ProvidePromiseRef<Provide::MediaHandle> promise();

  protected:
    AttachMediaItem ( const std::vector<zypp::Url> &urls, const ProvideMediaSpec &request, ProvidePrivate &parent );

    // ProvideItem interface
    void initialize () override;

    using ProvideItem::finishReq;
    void finishReq (  ProvideQueue &queue, ProvideRequestRef finishedReq, const ProvideMessage &msg ) override;
    void cancelWithError( std::exception_ptr error ) override;
    void finishWithSuccess (AttachedMediaInfo &medium );
    expected<zypp::media::AuthData> authenticationRequired ( ProvideQueue &queue, ProvideRequestRef req, const zypp::Url &effectiveUrl, int64_t lastTimestamp, const std::map<std::string, std::string> &extraFields ) override;

    void onMasterItemReady ( const zyppng::expected<AttachedMediaInfo *>& result );

  private:
    Signal< void( const zyppng::expected<AttachedMediaInfo *> & )> _sigReady;
    bool _promiseCreated = false;
    connection _masterItemConn;
    std::vector<zypp::Url> _mirrorList;   //< All available URLs, first one is the primary
    ProvideMediaSpec    _initialSpec;    //< The initial spec as defined by the user code
    ProvideQueue::Config::WorkerType _workerType = ProvideQueue::Config::Invalid;
    ProvidePromiseWeakRef<Provide::MediaHandle> _promise;
    MediaDataVerifierRef _verifier;
  };
}

#endif
