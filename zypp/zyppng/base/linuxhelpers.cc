#include "private/linuxhelpers_p.h"

#include <pthread.h>
#include <csignal>
#include <iostream>

namespace zyppng {

  bool blockSignalsForCurrentThread( std::vector<int> sigs )
  {
    sigset_t set;
    ::sigemptyset(&set);
    for ( const int sig : sigs )
      ::sigaddset( &set, sig );

    int res = ::pthread_sigmask(SIG_BLOCK, &set, NULL);
    return ( res == 0 );
  }

}
