#include "private/linuxhelpers_p.h"

#include <zypp-core/zyppng/io/SockAddr>
#include <zypp-core/zyppng/base/Timer>

#include <pthread.h>
#include <csignal>
#include <iostream>
#include <unistd.h>

namespace zyppng {

  bool blockSignalsForCurrentThread( const std::vector<int> &sigs )
  {
    sigset_t set;
    ::sigemptyset(&set);
    for ( const int sig : sigs )
      ::sigaddset( &set, sig );

    int res = ::pthread_sigmask(SIG_BLOCK, &set, NULL);
    return ( res == 0 );
  }

  bool trySocketConnection( int &sockFD, const SockAddr &addr, uint64_t timeout )
  {
    int res = -1;
    const auto opStarted = zyppng::Timer::now();
    do {
      res = zyppng::eintrSafeCall( ::connect, sockFD, addr.nativeSockAddr(), addr.size() );
      if ( res < 0 && errno != ECONNREFUSED && errno != EADDRNOTAVAIL ) {
        ::close( sockFD );
        sockFD = -1;
        return false;
      }
    } while ( res == -1 && zyppng::Timer::elapsedSince( opStarted ) < timeout );
    return ( res == 0 );
  }

}
