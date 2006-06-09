/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/Sysconfig.h
 *
*/
#ifndef ZYPP_BASE_SYSCONFIG_H
#define ZYPP_BASE_SYSCONFIG_H

#include <string>
#include <map>
#include "zypp/Pathname.h"

namespace zypp {
  namespace base {
    namespace sysconfig {

      std::map<std::string,std::string> read( const Pathname & _path );

    } // namespace sysconfig
  } // namespace base
} // namespace zypp

#endif // ZYPP_BASE_SYSCONFIG_H
