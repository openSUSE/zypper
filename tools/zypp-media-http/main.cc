#include "networkprovider.h"

#include <csignal>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>

int main( int , char *[] )
{
  // lets ignore those, SIGPIPE is better handled via the EPIPE error, and we do not want anyone
  // to CTRL+C us
  zyppng::blockSignalsForCurrentThread( { SIGPIPE, SIGINT } );

  auto provider = std::make_shared<NetworkProvider>("zypp-media-http");
  auto res = provider->run (STDIN_FILENO, STDOUT_FILENO);
  provider->immediateShutdown();
  if ( res )
    return 0;

  //@TODO print error
  return 1;
    //@TODO @WARNING @BUG Initialize the requestmanager with user agent and anon headers \sa anonymousIdHeader, distributionFlavorHeader, agentString
}
