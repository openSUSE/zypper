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
#include <string>
#include <string_view>

namespace zypp {
  class ByteArray : public std::vector<char>
  {
  public:
    using vector<char>::vector;
    explicit ByteArray ( const char *data, const int len = -1 ) : ByteArray( data, data + (len == -1 ? strlen(data) : len) ) { }
    std::string asString () const {
      if ( size() == 0 )
        return std::string();
      return std::string( data(), size() );
    }

#ifdef __cpp_lib_string_view
    std::string_view asStringView () const {
      if ( size() == 0 )
        return std::string_view();
      return std::string_view( data(), size() );
    }
#endif

    static std::size_t maxSize () {
      static const auto size = ByteArray().max_size();
      return size;
    }

  };

  class UByteArray : public std::vector<unsigned char>
  {
  public:
    using vector<unsigned char>::vector;
    explicit UByteArray ( const char *data, const int len = -1 ) : UByteArray( data, data + (len == -1 ? strlen(data) : len) ) { }

    static std::size_t maxSize () {
      static const auto size = UByteArray().max_size();
      return size;
    }
  };
}


#endif
