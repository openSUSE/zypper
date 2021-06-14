/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/

#include <zypp/zyppng/media/network/private/downloader_p.h>
#include <zypp/zyppng/media/network/private/mediadebug_p.h>
#include "final_p.h"

namespace zyppng {

  FinishedState::FinishedState(NetworkRequestError &&error, DownloadPrivate &parent)
    : SimpleState( parent )
    , _error( std::move(error) )
  {
    MIL_MEDIA << "About to enter FinishedState for url " << parent._spec.url() << std::endl;
  }

}
