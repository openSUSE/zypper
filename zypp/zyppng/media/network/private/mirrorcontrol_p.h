#ifndef ZYPP_NG_MEDIA_HTTP_PRIVATE_MIRRORCONTROL_P_H
#define ZYPP_NG_MEDIA_HTTP_PRIVATE_MIRRORCONTROL_P_H

#include <zypp/zyppng/core/Url>
#include <zypp/zyppng/base/Signals>
#include <zypp/zyppng/base/Base>
#include <zypp/zyppng/media/network/networkrequestdispatcher.h>
#include <zypp/zyppng/media/network/request.h>
#include <zypp/media/MetaLinkParser.h>
#include <vector>
#include <unordered_map>

namespace zyppng {

  class MirrorControl : public sigc::trackable {

  public:

    struct Mirror {
      Url mirrorUrl;
      uint rating              = 100; // rating based on connection time higher is worse
      uint penalty             = 0; //additional value that is added to the rating when sorting the mirrors, is increased and lowered for failed or successful transactions
      uint maxConnections      = 0; //the maximum number of concurrent connections to this mirror, 0 means use system default
      uint maxRanges           = 0; //the maximum number of ranges that can be requested from this mirror
      uint finishedTransfers   = 0; //how many transfers did we already send to the mirror
      uint runningTransfers    = 0; //currently running transfers
      uint failedTransfers     = 0; //how many transfers have failed in a row using this mirror
      uint successfulTransfers = 0; //how many transfers were successful

      void startTransfer();
      void finishTransfer( const bool success );
      void cancelTransfer();


    private:
      friend class MirrorControl;
      NetworkRequest::Ptr _request;
      sigc::connection _finishedConn;
    };

    using Ptr = std::shared_ptr<MirrorControl>;
    using MirrorHandle = std::shared_ptr<Mirror>;

    static Ptr create ();
    virtual ~MirrorControl();
    void registerMirrors( const std::vector<zypp::media::MetalinkMirror> &urls );
    std::pair< std::vector<Url>::const_iterator, MirrorHandle > pickBestMirror( const std::vector<Url> &mirrors );

    bool allMirrorsReady () const;
    SignalProxy<void()> sigAllMirrorsReady();
  private:
    MirrorControl();
    std::string makeKey ( const zypp::Url &url ) const;
    NetworkRequestDispatcher::Ptr _dispatcher; //Mirror Control using its own NetworkRequestDispatcher, to avoid waiting for other downloads
    std::unordered_map<std::string, MirrorHandle> _handles;

    signal<void()> _sigAllMirrorsReady;
  };

}

#endif // ZYPP_NG_MEDIA_HTTP_PRIVATE_MIRRORCONTROL_P_H
