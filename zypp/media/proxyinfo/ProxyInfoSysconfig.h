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
#include <map>

#include "zypp/base/Sysconfig.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/media/ProxyInfo.h"
#include "zypp/media/proxyinfo/ProxyInfoImpl.h"

namespace zypp {
  namespace media {


    class ProxyInfoSysconfig : public ProxyInfo::Impl
    {
    public:
      ProxyInfoSysconfig(const Pathname & path);
      /**  */
      bool enabled() const
      { return _enabled; }
      /**  */
      std::string proxy(const Url & url_r) const;
      /**  */
      ProxyInfo::NoProxyList noProxy() const
      { return _no_proxy; }
      /**  */
      virtual ProxyInfo::NoProxyIterator noProxyBegin() const;
      /**  */
      virtual ProxyInfo::NoProxyIterator noProxyEnd() const;
    private:
      DefaultIntegral<bool,false> _enabled;
      ProxyInfo::NoProxyList _no_proxy;
      std::map<std::string,std::string> _proxies;
    };

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_PROXYINFO_PROXYINFOSYSCONFIG_H
