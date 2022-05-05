#include "testvmprovider.h"

#include <csignal>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <zypp-media/ng/worker/MountingWorker>

int main( int , char *[] )
{
  // lets ignore those, SIGPIPE is better handled via the EPIPE error, and we do not want anyone
  // to CTRL+C us
  zyppng::blockSignalsForCurrentThread( { SIGPIPE, SIGINT } );

  auto driver = std::make_shared<TestVMProvider>();
  auto worker = std::make_shared<zyppng::worker::MountingWorker>( "zypp-media-tvm", driver );
  driver->setProvider(worker);

  auto res = worker->run (STDIN_FILENO, STDOUT_FILENO);
  if ( res )
    return 0;

  //@TODO print error
  return 1;
}
