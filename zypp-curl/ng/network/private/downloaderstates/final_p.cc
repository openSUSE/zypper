/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/

#include <zypp-curl/ng/network/private/downloader_p.h>
#include <zypp-curl/ng/network/private/mediadebug_p.h>
#include "final_p.h"

namespace zyppng {

  FinishedState::FinishedState(NetworkRequestError &&error, DownloadPrivate &parent)
    : SimpleState( parent )
    , _error( std::move(error) )
  {
    MIL << "About to enter FinishedState for url " << parent._spec.url() << std::endl;
  }

}
