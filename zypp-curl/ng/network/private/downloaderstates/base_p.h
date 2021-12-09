/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_BASE_P_H_INCLUDED
#define ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_BASE_P_H_INCLUDED

#include <zypp-core/zyppng/base/private/base_p.h>
#include <zypp-core/zyppng/base/signals.h>
#include <zypp-core/TriBool.h>
#include <zypp-curl/ng/network/Downloader>
#include <zypp-curl/ng/network/DownloadSpec>
#include <zypp-core/zyppng/core/ByteArray>
#include <zypp-curl/ng/network/request.h>
#include <zypp-curl/ng/network/TransferSettings>
#include <zypp-curl/ng/network/private/mirrorcontrol_p.h>
#include <zypp-curl/ng/network/networkrequesterror.h>

namespace zyppng {

  class NetworkRequestDispatcher;
  class DownloadPrivate;

  /*!
   * The pimpl for Downloader, the reason this is split up in DownloadPrivateBase and DownloadBase
   * is that for defining the States in the Statemachine while making DownloadPrivate the statemachine itself
   * we need this separation. This trick allowes the states to "know" the statemachine type and access some of
   * its features.
   */
  class DownloadPrivateBase : public BasePrivate
  {
    ZYPP_DECLARE_PUBLIC(Download)
  public:
    DownloadPrivateBase ( Downloader &parent, std::shared_ptr<NetworkRequestDispatcher> requestDispatcher, std::shared_ptr<MirrorControl> mirrors, DownloadSpec &&spec, Download &p );
    ~DownloadPrivateBase ();

    struct Block {
      off_t  start = 0;
      size_t len = 0;

      std::string chksumtype;
      std::optional<UByteArray> chksumVec;
      std::optional<size_t> chksumCompareLen; //< initialized if only the first few bytes of the checksum should be considered

      int _retryCount = 0;  //< how many times was this request restarted
      NetworkRequestError _failedWithErr; //< what was the error this request failed with
    };

    struct Request : public NetworkRequest {

      using NetworkRequest::NetworkRequest;
      using Ptr = std::shared_ptr<Request>;
      using WeakPtr = std::shared_ptr<Request>;

      template <typename Receiver>
      void connectSignals ( Receiver &dl ) {
        _sigStartedConn  = connect ( &NetworkRequest::sigStarted,  dl, &Receiver::onRequestStarted );
        _sigProgressConn = connect ( &NetworkRequest::sigProgress, dl, &Receiver::onRequestProgress );
        _sigFinishedConn = connect ( &NetworkRequest::sigFinished, dl, &Receiver::onRequestFinished );
      }
      void disconnectSignals ();

      bool _triedCredFromStore = false; //< already tried to authenticate from credential store?
      time_t _authTimestamp = 0; //< timestamp of the AuthData we tried from the store
      Url _originalUrl;  //< The unstripped URL as it was passed to Download , before transfer settings are removed
      MirrorControl::MirrorHandle _myMirror;

      connection _sigStartedConn;
      connection _sigProgressConn;
      connection _sigFinishedConn;
    };


    bool _emittedSigStart = false;
    bool handleRequestAuthError(std::shared_ptr<Request> req, const zyppng::NetworkRequestError &err);

    NetworkRequestError safeFillSettingsFromURL ( const Url &url, TransferSettings &set );

#if ENABLE_ZCHUNK_COMPRESSION
    bool hasZckInfo () const;
#endif

    std::shared_ptr<NetworkRequestDispatcher> _requestDispatcher;
    std::shared_ptr<MirrorControl> _mirrorControl;

    DownloadSpec _spec; // the download settings
    mutable zypp::TriBool _specHasZckInfo = zypp::indeterminate;

    Downloader *_parent = nullptr;

    time_t _lastTriedAuthTime = 0; //< if initialized this shows the last timestamp that we loaded a cred for the given URL from CredentialManager
    NetworkRequest::Priority _defaultSubRequestPriority = NetworkRequest::High;

    Signal< void ( Download &req )> _sigStarted;
    Signal< void ( Download &req, Download::State state )> _sigStateChanged;
    Signal< void ( Download &req, off_t dlnow  )> _sigAlive;
    Signal< void ( Download &req, off_t dltotal, off_t dlnow )> _sigProgress;
    Signal< void ( Download &req )> _sigFinished;
    Signal< void ( zyppng::Download &req, zyppng::NetworkAuthData &auth, const std::string &availAuth )> _sigAuthRequired;

  };

}

#endif
