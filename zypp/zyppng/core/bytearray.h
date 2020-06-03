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

#ifndef ZYPP_NG_CORE_BYTEARRAY_H_INCLUDED
#define ZYPP_NG_CORE_BYTEARRAY_H_INCLUDED

#include <vector>
#include <cstring>

namespace zyppng {

  class ByteArray : public std::vector<char>
  {
  public:
    using vector<char>::vector;
    explicit ByteArray ( const char *data, const int len = -1 ) : ByteArray( data, data + (len == -1 ? strlen(data) : len) ) { }
  };

}

#endif
