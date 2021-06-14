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
#ifndef ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_FINAL_P_H_INCLUDED
#define ZYPP_NG_MEDIA_HTTP_PRIVATE_DOWNLOADERSTATES_FINAL_P_H_INCLUDED

#include "base_p.h"
#include <zypp-core/zyppng/base/statemachine.h>

namespace zyppng {

  /*!
   * Final state implementation, we enter this state as the very last step. It carries
   * the result of the whole operation.
   */
  struct FinishedState : public SimpleState< DownloadPrivate, Download::Finished, true >
  {
    FinishedState ( NetworkRequestError &&error, DownloadPrivate &parent );

    void enter (){}
    void exit (){}

    NetworkRequestError _error;
  };

}

#endif
