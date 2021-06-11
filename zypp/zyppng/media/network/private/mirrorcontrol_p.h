#ifndef ZYPP_NG_MEDIA_HTTP_PRIVATE_MIRRORCONTROL_P_H
#define ZYPP_NG_MEDIA_HTTP_PRIVATE_MIRRORCONTROL_P_H

#include <zypp-core/zyppng/core/Url>
#include <zypp-core/zyppng/base/Signals>
#include <zypp-core/zyppng/base/Base>
#include <zypp-core/zyppng/base/Timer>
#include <zypp/zyppng/media/network/networkrequestdispatcher.h>
#include <zypp/zyppng/media/network/request.h>
#include <zypp/media/MetaLinkParser.h>
#include <vector>
#include <unordered_map>

namespace zyppng {

  class MirrorControl : public Base {

  public:

    struct Mirror {

      Url mirrorUrl;
      uint rating              = 100; // rating based on connection time higher is worse
      uint penalty             = 0; //additional value that is added to the rating when sorting the mirrors, is increased and lowered for failed or successful transactions
      uint maxRanges           = 0; //the maximum number of ranges that can be requested from this mirror
      uint finishedTransfers   = 0; //how many transfers did we already send to the mirror
      uint runningTransfers    = 0; //currently running transfers
      uint failedTransfers     = 0; //how many transfers have failed in a row using this mirror
      uint successfulTransfers = 0; //how many transfers were successful

      void startTransfer();
      void finishTransfer( const bool success );
      void cancelTransfer();
      uint maxConnections () const;
      bool hasFreeConnections () const;

    private:
      Mirror( MirrorControl &parent );
      void transferUnref ();

    private:
      friend class MirrorControl;
      MirrorControl &_parent;
      NetworkRequest::Ptr _request;
      sigc::connection _finishedConn;

      uint _maxConnections      = 0; //the maximum number of concurrent connections to this mirror, 0 means use system default
    };

    using Ptr = std::shared_ptr<MirrorControl>;
    using MirrorHandle = std::shared_ptr<Mirror>;
    using MirrorPick   = std::pair< std::vector<Url>::const_iterator, MirrorHandle >;

    static Ptr create ();
    virtual ~MirrorControl();
    void registerMirrors( const std::vector<zypp::media::MetalinkMirror> &urls );

    /*!
     * Tries to pick the best mirror from the set of URLs passed.
     * In case of a pending request, the result code will be set to "Again".
     */
    struct PickResult {
      enum {
        Ok,
        Again,
        Unknown
      } code = Unknown;
      MirrorPick result;
    };
    PickResult pickBestMirror( const std::vector<Url> &mirrors );

    bool allMirrorsReady () const;

    SignalProxy<void()> sigNewMirrorsReady();
    SignalProxy<void()> sigAllMirrorsReady();
  private:
    MirrorControl();
    std::string makeKey ( const zypp::Url &url ) const;
    NetworkRequestDispatcher::Ptr _dispatcher; //Mirror Control using its own NetworkRequestDispatcher, to avoid waiting for other downloads
    std::unordered_map<std::string, MirrorHandle> _handles;

    Timer::Ptr _newMirrSigDelay; // we use a delay timer to emit the "someMirrorsReady" signal

    Signal<void()> _sigAllMirrorsReady;
    Signal<void()> _sigNewMirrorsReady;
  };

#if 0

  /*!
   * Simple helper class to automatically cancel running transfers on destruction
   */
  class MirrorRef
  {
  public:
    MirrorRef ( MirrorControl::MirrorHandle handle );
    ~MirrorRef();

    void startTransfer();
    void finishTransfer( const bool success );
    void cancelTransfer();
    operator bool() const;

    MirrorControl::MirrorHandle get();

  private:
    struct Helper {
      ~Helper();
      MirrorControl::MirrorHandle _myHandle;
      bool _cancelOnDestruct = false;
    };
    std::shared_ptr<Helper> _data;
  };
#endif

}

#endif // ZYPP_NG_MEDIA_HTTP_PRIVATE_MIRRORCONTROL_P_H
