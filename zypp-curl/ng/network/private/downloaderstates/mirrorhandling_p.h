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
#ifndef ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_MIRRORHANDLING_P_H_INCLUDED
#define ZYPP_CURL_NG_NETWORK_PRIVATE_DOWNLOADERSTATES_MIRRORHANDLING_P_H_INCLUDED

#include <zypp-core/zyppng/base/statemachine.h>
#include "base_p.h"

namespace zyppng {

  /*!
   * superclass state implementation for all states that need to handle async mirror requests
   */
  struct MirrorHandlingStateBase  : public zyppng::BasicState< DownloadPrivate, false >
  {

    MirrorHandlingStateBase ( DownloadPrivate &parent );
    virtual ~MirrorHandlingStateBase();
    enum PrepareResult {
      Failed,
      Ok,
      Delayed
    };

    /*!
     * Request a new mirror, the \a PrepareResult shows if \ref setupMirror was already called
     * or if the request was \a Delayed.
     */
    PrepareResult prepareNextMirror ();


    /*!
     * Common code to setup a mirror after it was received, can result in errors if the mirror url contains
     * settings that are invalid.
     * Will return settings and url via parameters.
     */
    NetworkRequestError setupMirror( const MirrorControl::MirrorPick &pick, Url &url, TransferSettings &set );

    /*!
     * This is called once a mirror became ready, either directly if a mirror is ready or it was delayed
     * and called asynchronously
     */
    virtual void mirrorReceived ( MirrorControl::MirrorPick mirror ) = 0;

    /*!
     * Gets called in case a mirror failed to prepare,
     * probably because none of the mirrors in \a _fileMirrors is known by \ref MirrorControl
     */
    virtual void failedToPrepare (){};

    std::vector<Url> _fileMirrors; //< all mirrors of the currently requested file

  private:
    connection _sigMirrorsReadyConn; //< the internal connection to the mirrors ready signal
  };

}

#endif
