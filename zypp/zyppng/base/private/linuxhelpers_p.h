#ifndef ZYPP_BASE_LINUXHELPERS_P_H_DEFINED
#define ZYPP_BASE_LINUXHELPERS_P_H_DEFINED

#include <string>
#include <zypp/zyppng/core/ByteArray>
#include <errno.h>

namespace zyppng {

  inline std::string strerr_cxx ( const int err = -1 ) {
    ByteArray strBuf( 1024, '\0' );
    strerror_r( err == -1 ? errno : err , strBuf.data(), strBuf.size() );
    return std::string( strBuf.data() );
  }

  template<typename Fun, typename... Args >
  auto eintrSafeCall ( Fun &&function, Args&&... args ) {
    int res;
    do {
      res = std::forward<Fun>(function)( std::forward<Args>(args)... );
    } while ( res == -1 && errno == EINTR );
    return res;
  }

  bool blockSignalsForCurrentThread ( std::vector<int> sigs );
}

#endif // LINUXHELPERS_P_H
