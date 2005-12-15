/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/proxyinfo/ProxyInfoSysconfig.h
 *
*/
#ifndef ZYPP_MEDIA_PROXYINFO_PROXYINFOSYSCONFIG_H
#define ZYPP_MEDIA_PROXYINFO_PROXYINFOSYSCONFIG_H

#include <string>
#include <list>

#include "zypp/media/ProxyInfo.h"
#include "zypp/media/proxyinfo/ProxyInfoImpl.h"

namespace zypp {
  namespace media {


    class ProxyInfo_sysconfig : public ProxyInfo::Impl
    {
    public:
      ProxyInfo_sysconfig(const Pathname & path);
      /**  */
      bool enabled() const
      { return _enabled; }
      /**  */
      std::string proxy(const std::string & protocol_r) const;
      /**  */
      std::list<std::string> noProxy() const
      { return _no_proxy; }
    private:
      bool _enabled;
      std::list<std::string> _no_proxy;
    };

    namespace proxyinfo {
      std::map<std::string,std::string> sysconfigRead(const Pathname & _path);
    } // namespace proxyinfo

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_PROXYINFO_PROXYINFOSYSCONFIG_H
