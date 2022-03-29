/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp-media/cdtools.h
 *
*/

#ifndef ZYPP_MEDIA_CDTOOLS_H
#define ZYPP_MEDIA_CDTOOLS_H

#include <string>

namespace zypp::media {

  class CDTools
  {
  public:
    static bool openTray( const std::string & device_r );
    static bool closeTray( const std::string & device_r );
  };

}

#endif
