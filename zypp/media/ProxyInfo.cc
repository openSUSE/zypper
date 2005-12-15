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

using namespace std;
using namespace zypp::base;

namespace zypp {
  namespace media {

    shared_ptr<ProxyInfo::Impl> ProxyInfo::Impl::_nullimpl;

    ProxyInfo::ProxyInfo()
    : _pimpl( Impl::_nullimpl )
    {}
    ProxyInfo::ProxyInfo(ProxyInfo::ImplPtr pimpl_r)
    : _pimpl(pimpl_r)
    {}

    bool ProxyInfo::enabled() const
    { return _pimpl->enabled(); }

    std::string ProxyInfo::proxy(const std::string & protocol_r) const
    { return _pimpl->proxy(protocol_r); }

    ProxyInfo::NoProxyList ProxyInfo::noProxy() const
    { return _pimpl->noProxy(); }

    ProxyInfo::NoProxyIterator ProxyInfo::noProxyBegin() const
    { return _pimpl->noProxyBegin(); }

    ProxyInfo::NoProxyIterator ProxyInfo::noProxyEnd() const
    { return _pimpl->noProxyEnd(); }

  } // namespace media
} // namespace zypp
