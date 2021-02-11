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
        ERR << "Connection failed with error: " << errno << " " << zyppng::strerr_cxx( errno ) << std::endl;
        ::close( sockFD );
        sockFD = -1;
        return false;
      }
    } while ( res == -1 && zyppng::Timer::elapsedSince( opStarted ) < timeout );
    return ( res == 0 );
  }

  void renumberFd (int origfd, int newfd)
  {
    // It may happen that origfd is already the one we want
    // (Although in our circumstances, that would mean somebody has closed
    // our stdin or stdout... weird but has appened to Cray, #49797)
    if (origfd != newfd)
    {
      dup2 (origfd, newfd);
      ::close (origfd);
    }
  }

}
