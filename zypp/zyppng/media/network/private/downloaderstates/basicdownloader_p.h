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
#ifndef ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_BASICDOWNLOADER_P_H_INCLUDED
#define ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_BASICDOWNLOADER_P_H_INCLUDED

#include "base_p.h"
#include "mirrorhandling_p.h"
#include <zypp-core/zyppng/base/statemachine.h>

namespace zyppng {

  /*!
   * State implementation for the generic part of a simple file downloader, that knows
   * how to use the mirrors if available.
   */
  struct BasicDownloaderStateBase : public MirrorHandlingStateBase {

    using Request = DownloadPrivateBase::Request;

    BasicDownloaderStateBase ( DownloadPrivate &parent ) : MirrorHandlingStateBase( parent ){}

    void enter ();
    void exit ();

    virtual bool initializeRequest ( std::shared_ptr<Request> r );
    virtual void gotFinished ();
    virtual void failed(NetworkRequestError &&err);
    void failed (std::string &&str );

    void onRequestStarted  ( NetworkRequest & );
    void onRequestProgress ( NetworkRequest &, off_t dltotal, off_t dlnow, off_t, off_t );
    void onRequestFinished ( NetworkRequest &req , const NetworkRequestError &err );

    const NetworkRequestError &error () const {
      return _error;
    }

    std::shared_ptr<Request> _request;
    std::optional<std::string> _chksumtype; //< The file checksum type if available
    std::optional<UByteArray>  _chksumVec;  //< The file checksum if available

    // MirrorHandlingStateBase interface
    void mirrorReceived(MirrorControl::MirrorPick mirror) override;
    void failedToPrepare() override;

  protected:
    void startWithMirror ( MirrorControl::MirrorHandle mirror, const zypp::Url &url, const TransferSettings &set );
    void startWithoutMirror (  );
    virtual void handleRequestProgress (NetworkRequest &req, off_t dltotal, off_t dlnow );
    NetworkRequestError _error;
    Signal< void () > _sigFinished;
    Signal< void () > _sigFailed;

  };

}

#endif
