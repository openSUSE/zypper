/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPPNG_IO_SOCKADDR_H_DEFINED
#define ZYPPNG_IO_SOCKADDR_H_DEFINED

#include <cstddef>
#include <sys/socket.h>
#include <memory>
#include <string>

struct sockaddr_un;

namespace zyppng {

  class SockAddr {
  public:
    virtual ~SockAddr(){};
    virtual struct ::sockaddr* nativeSockAddr () = 0;
    virtual std::size_t size () = 0;
  protected:

  };

  class UnixSockAddr : public SockAddr
  {
  public:

    using Ptr = std::shared_ptr<UnixSockAddr>;

    UnixSockAddr( const std::string &path, bool abstract );

    // SockAddr interface
    sockaddr *nativeSockAddr() override;
    std::size_t size() override;

    bool isAbstract () const;

  private:
    std::shared_ptr<struct sockaddr_un> _data;
  };
}

#endif // SOCKADDR_H
