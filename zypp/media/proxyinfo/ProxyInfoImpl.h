/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/proxyinfo/ProxyInfoImpl.h
 *
*/
#ifndef ZYPP_MEDIA_PROXYINFO_PROXYINFOIMPL_H
#define ZYPP_MEDIA_PROXYINFO_PROXYINFOIMPL_H

#include <string>
#include <list>

#include "zypp/media/ProxyInfo.h"

namespace zypp {
  namespace media {

    struct ProxyInfo::Impl
    {
      /** Ctor */
      Impl()
      {}

      /** Dtor */
      virtual ~Impl()
      {}
  
    public:
      /**  */
      virtual bool enabled() const = 0;
      /**  */
      virtual std::string proxy(const std::string & prorocol_r) const = 0;
      /**  */
      virtual ProxyInfo::NoProxyList noProxy() const = 0;
      /**  */
      virtual ProxyInfo::NoProxyIterator noProxyBegin() const = 0;
      /**  */
      virtual ProxyInfo::NoProxyIterator noProxyEnd() const = 0;
  
    public:
      /** Default Impl: empty sets. */
      static shared_ptr<Impl> _nullimpl;
    };


///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_PROXYINFO_PROXYINFOIMPL_H
