/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/ProxyInfo.cc
 *
*/

#include <iostream>

#include "zypp/base/Logger.h"

#include "zypp/media/ProxyInfo.h"
#include "zypp/media/proxyinfo/ProxyInfoImpl.h"
#include "zypp/media/proxyinfo/ProxyInfos.h"

using namespace std;
using namespace zypp::base;

namespace zypp {
  namespace media {

    shared_ptr<ProxyInfo::Impl> ProxyInfo::Impl::_nullimpl;

    ProxyInfo::ProxyInfo()
#ifdef WITH_LIBPROXY_SUPPORT
    : _pimpl( new ProxyInfoLibproxy() )
#else
    : _pimpl( new ProxyInfoSysconfig("proxy") )
#endif
    {}

    ProxyInfo::ProxyInfo(ProxyInfo::ImplPtr pimpl_r)
    : _pimpl(pimpl_r)
    {}

    bool ProxyInfo::enabled() const
    { return _pimpl->enabled(); }

    std::string ProxyInfo::proxy(const Url & url_r) const
    { return _pimpl->proxy(url_r); }

    ProxyInfo::NoProxyList ProxyInfo::noProxy() const
    { return _pimpl->noProxy(); }

    ProxyInfo::NoProxyIterator ProxyInfo::noProxyBegin() const
    { return _pimpl->noProxyBegin(); }

    ProxyInfo::NoProxyIterator ProxyInfo::noProxyEnd() const
    { return _pimpl->noProxyEnd(); }

    bool ProxyInfo::useProxyFor( const Url & url_r ) const
    { return _pimpl->useProxyFor( url_r ); }

  } // namespace media
} // namespace zypp
