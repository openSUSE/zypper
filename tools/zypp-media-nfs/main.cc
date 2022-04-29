#include "nfsprovider.h"

#include <csignal>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>

int main( int , char *[] )
{
  // lets ignore those, SIGPIPE is better handled via the EPIPE error, and we do not want anyone
  // to CTRL+C us
  zyppng::blockSignalsForCurrentThread( { SIGPIPE, SIGINT } );

  auto provider = std::make_shared<NfsProvider>("zypp-media-nfs");
  auto res = provider->run (STDIN_FILENO, STDOUT_FILENO);
  provider->immediateShutdown();
  if ( res )
    return 0;

  //@TODO print error
  return 1;
}
