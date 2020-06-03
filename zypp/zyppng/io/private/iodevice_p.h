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

#include <zypp/zyppng/io/iodevice.h>
#include <zypp/zyppng/base/private/base_p.h>
#include "iobuffer_p.h"


namespace zyppng {

  class IODevicePrivate : public BasePrivate {
  public:
    IOBuffer _readBuf;
    IODevice::OpenMode _mode = IODevice::Closed;
  };

}


#endif
