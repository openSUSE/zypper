#include "sockaddr.h"
#include <sys/un.h>

namespace zyppng {

  UnixSockAddr::UnixSockAddr(const std::string &path , bool abstract ) : _data( std::make_shared<struct sockaddr_un>() )
  {
    memset( _data.get(), 0, size() );

    _data->sun_family = AF_UNIX;
    if ( path.size() ) {
      const auto align = abstract ? 1 : 0;
      path.copy( _data->sun_path + align, sizeof( _data->sun_path ) - align - 1 );
    }
  }

  sockaddr *UnixSockAddr::nativeSockAddr() const
  {
    return reinterpret_cast<sockaddr *>(_data.get());
  }

  std::size_t UnixSockAddr::size() const
  {
    return sizeof(struct sockaddr_un);
  }

  bool UnixSockAddr::isAbstract() const
  {
    return _data->sun_path[0];
  }

}
