/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_BYTEARRAY_H
#define ZYPP_BYTEARRAY_H

#include <vector>
#include <cstring>

namespace zypp {
  class ByteArray : public std::vector<char>
  {
  public:
    using vector<char>::vector;
    explicit ByteArray ( const char *data, const int len = -1 ) : ByteArray( data, data + (len == -1 ? strlen(data) : len) ) { }
  };

  class UByteArray : public std::vector<unsigned char>
  {
  public:
    using vector<unsigned char>::vector;
    explicit UByteArray ( const char *data, const int len = -1 ) : UByteArray( data, data + (len == -1 ? strlen(data) : len) ) { }
  };
}


#endif
