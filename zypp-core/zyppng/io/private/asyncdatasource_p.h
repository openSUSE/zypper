#ifndef ZYPP_CORE_ZYPPNG_IO_PRIVATE_ASYNCDATASOURCE_P_H_INCLUDED
#define ZYPP_CORE_ZYPPNG_IO_PRIVATE_ASYNCDATASOURCE_P_H_INCLUDED

#include <zypp-core/zyppng/io/AsyncDataSource>
#include <zypp-core/zyppng/base/SocketNotifier>
#include "iodevice_p.h"
#include "iobuffer_p.h"

namespace zyppng {

  class AsyncDataSourcePrivate : public IODevicePrivate {
    ZYPP_DECLARE_PUBLIC(AsyncDataSource);
  public:
    AsyncDataSourcePrivate ( AsyncDataSource &pub ) : IODevicePrivate(pub) {}
    struct ReadChannelDev {
      int _readFd = -1;
      SocketNotifier::Ptr _readNotifier;
    };
    std::vector<ReadChannelDev> _readFds;

    SocketNotifier::Ptr _writeNotifier;
    IOBuffer _writeBuffer;
    int _writeFd = -1;

    void notifierActivated (const SocketNotifier &notify, int evTypes );
    void readyRead  ( uint channel );
    void readyWrite ( );

    void closeWriteChannel ( AsyncDataSource::ChannelCloseReason reason );
    void closeReadChannel  ( uint channel, AsyncDataSource::ChannelCloseReason reason );

    Signal<void( AsyncDataSource::ChannelCloseReason )> _sigWriteFdClosed;
    Signal<void( uint, AsyncDataSource::ChannelCloseReason )> _sigReadFdClosed;
  };

}


#endif // ZYPP_CORE_ZYPPNG_IO_PRIVATE_ASYNCDATASOURCE_P_H_INCLUDED
