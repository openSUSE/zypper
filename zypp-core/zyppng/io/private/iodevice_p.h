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

#ifndef ZYPPNG_IO_IODEVICE_P_DEFINED
#define ZYPPNG_IO_IODEVICE_P_DEFINED

#include <vector>
#include <functional>
#include <zypp-core/zyppng/io/iodevice.h>
#include <zypp-core/zyppng/base/private/base_p.h>
#include "iobuffer_p.h"


namespace zyppng {

  namespace constants {
    constexpr std::string_view outOfRangeErrMsg("Channel index out of range");
  }

  enum {
    DefIoDeviceBufChunkSize = 16384
  };

  class IODevicePrivate : public BasePrivate {
  public:
    IODevicePrivate ( IODevice &p );

    std::vector<IOBuffer> _readChannels;
    uint _currentReadChannel = 0;
    int64_t _readBufChunkSize = DefIoDeviceBufChunkSize;

    IODevice::OpenMode _mode = IODevice::Closed;
    Signal<void()>    _readyRead;
    Signal<void(uint)> _channelReadyRead;
    Signal< void (int64_t)> _sigBytesWritten;
    Signal< void ()> _sigAllBytesWritten;
  };

}


#endif
