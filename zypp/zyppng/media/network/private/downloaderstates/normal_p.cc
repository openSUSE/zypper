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

#include "normal_p.h"
#include "final_p.h"

namespace zyppng {

  DlNormalFileState::DlNormalFileState(DownloadPrivate &parent) : BasicDownloaderStateBase( parent )
  {
    MIL_MEDIA << "About to enter DlNormalFileState for url " << parent._spec.url() << std::endl;
  }

  std::shared_ptr<FinishedState> DlNormalFileState::transitionToFinished()
  {
    return std::make_shared<FinishedState>( std::move(_error), stateMachine() );
  }


}
