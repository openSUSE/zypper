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
#ifndef ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_NORMAL_P_H_INCLUDED
#define ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_NORMAL_P_H_INCLUDED

#include "base_p.h"
#include "basicdownloader_p.h"
#include <zypp-core/zyppng/base/statemachine.h>

namespace zyppng {

  struct FinishedState;

  /*!
     * Just a plain normal file download, no metalink, nothing fancy.
     * If this fails we have no more fallbacks
     */
  struct DlNormalFileState : public BasicDownloaderStateBase {
    static constexpr auto stateId = Download::DlSimple;

    DlNormalFileState( DownloadPrivate &parent );
    DlNormalFileState( std::shared_ptr<Request> &&oldReq, DownloadPrivate &parent );

    std::shared_ptr<FinishedState> transitionToFinished ();

    SignalProxy< void () > sigFinished() {
      return _sigFinished;
    }
    SignalProxy< void () > sigFailed() {
      return _sigFailed;
    }
  };

}

#endif
