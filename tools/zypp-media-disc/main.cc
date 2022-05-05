
#include "discprovider.h"

#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <zypp-media/ng/worker/MountingWorker>

#include <csignal>


int main( int , char *[] )
{
  // lets ignore those, SIGPIPE is better handled via the EPIPE error, and we do not want anyone
  // to CTRL+C us
  zyppng::blockSignalsForCurrentThread( { SIGPIPE, SIGINT } );

  auto driver = std::make_shared<DiscProvider>();
  auto provider = std::make_shared<zyppng::worker::MountingWorker>( "zypp-media-disc", driver );
  driver->setProvider( provider );

  auto res = provider->run (STDIN_FILENO, STDOUT_FILENO);
  provider->immediateShutdown();
  if ( res )
    return 0;

  //@TODO print error
  return 1;
}
