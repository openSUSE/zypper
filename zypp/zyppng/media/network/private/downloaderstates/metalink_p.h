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
#ifndef ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_METALINK_P_H_INCLUDED
#define ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_METALINK_P_H_INCLUDED

#include "base_p.h"
#include "rangedownloader_p.h"
#include <zypp-core/zyppng/base/statemachine.h>

#include <zypp/media/MediaBlockList.h>

namespace zyppng {

  struct FinishedState;

  /*!
   * Metalink download state implementation, this downloads the requested file
   * in metalink ranges.
   */
  struct DlMetalinkState : public RangeDownloaderBaseState {

    static constexpr auto stateId = Download::DlMetalink;

    DlMetalinkState ( zypp::media::MediaBlockList &&blockList, std::vector<Url> &&mirrors, DownloadPrivate &parent );

    void enter ();
    void exit ();
    virtual void setFinished () override;

    std::shared_ptr<FinishedState> transitionToFinished ();

    // in case of error we might fall back, except for the errors listed here
    bool toFinalStateCondition () {
      return (  _error.type() == NetworkRequestError::Unauthorized
               || _error.type() == NetworkRequestError::AuthFailed );
    }

    bool toSimpleDownloadCondition () {
      return !toFinalStateCondition();
    }

    SignalProxy< void () > sigFinished() {
      return _sigFinished;
    }

    SignalProxy< void () > sigFailed() {
      return _sigFailed;
    }

  private:
    zypp::media::MediaBlockList _blockList;
    std::string        _fileChecksumType;
    std::optional<std::vector<unsigned char>> _fileChksumVec;
  };

}

#endif
