#ifndef ZYPP_BASE_LINUXHELPERS_P_H_DEFINED
#define ZYPP_BASE_LINUXHELPERS_P_H_DEFINED

#include <string>
#include <optional>
#include <zypp-core/zyppng/core/ByteArray>
#include <zypp-core/AutoDispose.h>
#include <errno.h>

namespace zyppng {

  class SockAddr;

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

  bool blockSignalsForCurrentThread ( const std::vector<int> &sigs );

  bool trySocketConnection (int &sockFD, const SockAddr &addr, uint64_t timeout );

  // origfd will be accessible as newfd and closed (unless they were equal)
  void renumberFd (int origfd, int newfd);

  /*!
   * Tries to use the FIONREAD ioctl to detect how many bytes are available on a file descriptor,
   * this can fail and return 0 so just use it as a indicator on how many bytes are pending
   */
  int64_t bytesAvailableOnFD( int fd );

  /*!
   * Small helper struct around creating a Unix pipe to ensure RAII with pipes
   */
  struct Pipe {
    zypp::AutoFD readFd;
    zypp::AutoFD writeFd;
    static std::optional<Pipe> create ( int flags = 0 );

    void unrefWrite( ) {
      writeFd = -1;
    }

    void unrefRead( ) {
      readFd = -1;
    }
  };
}

#endif // LINUXHELPERS_P_H
