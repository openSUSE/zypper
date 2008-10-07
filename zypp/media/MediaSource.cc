/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaSource.cc
*/
#include <iostream>

#include "zypp/media/MediaSource.h"

namespace zypp {
  namespace media {

    std::ostream & operator<<( std::ostream & str, const AttachPoint & obj )
    {
      return str << (obj.temp ? "*" : "") << obj.path;
    }

    std::ostream & operator<<( std::ostream & str, const AttachedMedia & obj )
    {
      return str << "media("  << obj.mediaSource << ")attached(" << obj.attachPoint << ")";
    }

  } // namespace media
} // namespace zypp
