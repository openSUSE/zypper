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
#ifndef ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_METALINKINFO_P_H_INCLUDED
#define ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_METALINKINFO_P_H_INCLUDED

#include "base_p.h"
#include "basicdownloader_p.h"
#include <zypp-core/zyppng/base/statemachine.h>

namespace zyppng {

  struct FinishedState;
  struct PrepareMultiState;

  enum class MetaDataType {
    None  = 0,
    Zsync,
    MetaLink
  };

  /*!
     * State to download the actual metalink file, we can however not be 100% sure that we actually
     * will get a metalink file, so we need to check the content type or in bad cases the
     * data we get from the server.
     */
  struct DlMetaLinkInfoState : public BasicDownloaderStateBase {
    static constexpr auto stateId = Download::DlMetaLinkInfo;

    DlMetaLinkInfoState( DownloadPrivate &parent );
    DlMetaLinkInfoState( std::shared_ptr<Request> &&prevRequest,  DownloadPrivate &parent );

    SignalProxy< void () > sigFinished() {
      return _sigFinished;
    }
    SignalProxy< void () > sigGotMetadata() {
      return _sigGotMetadata;
    }
    SignalProxy< void () > sigFailed() {
      return _sigFailed;
    }

    std::shared_ptr<FinishedState> transitionToFinished ();
    std::shared_ptr<PrepareMultiState> transitionToPrepareMulti ();

    bool initializeRequest( std::shared_ptr<Request> &r ) override;
    virtual void gotFinished () override;

  protected:
    MetaDataType _detectedMetaType = MetaDataType::None;
    Signal< void () > _sigGotMetadata;

    virtual void handleRequestProgress ( NetworkRequest &req, off_t dltotal, off_t dlnow ) override;

  private:
    bool _fallbackMilWritten = false; //< Flag to know if we already logged that we fall back to the normal progress
  };

}

#endif
