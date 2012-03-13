/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/proxyinfo/ProxyInfoSysconfig.cc
 *
*/

#include <iostream>
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/Pathname.h"

#include "zypp/media/proxyinfo/ProxyInfoSysconfig.h"

using namespace std;
using namespace zypp::base;

namespace zypp {
  namespace media {

    ProxyInfoSysconfig::ProxyInfoSysconfig(const Pathname & path)
    : ProxyInfo::Impl()
    {
      map<string,string> data = sysconfig::read(
	path.relative()
	  ? "/etc/sysconfig" + path
	  : path);
      map<string,string>::const_iterator it = data.find("PROXY_ENABLED");
      if (it != data.end())
	_enabled = it->second != "no";
      it = data.find("HTTP_PROXY");
      if (it != data.end())
	_proxies["http"] = it->second;
      it = data.find("HTTPS_PROXY");
      if (it != data.end())
	_proxies["https"] = it->second;
      it = data.find("FTP_PROXY");
      if (it != data.end())
	_proxies["ftp"] = it->second;
      it = data.find("NO_PROXY");
      if (it != data.end())
	str::split(it->second, std::back_inserter(_no_proxy), ", \t");
    }

    std::string ProxyInfoSysconfig::proxy(const Url & url_r) const
    {
      map<string,string>::const_iterator it = _proxies.find(url_r.getScheme());
      if (it != _proxies.end())
	return it->second;
      return "";
    }

    ProxyInfo::NoProxyIterator ProxyInfoSysconfig::noProxyBegin() const
    { return _no_proxy.begin(); }

    ProxyInfo::NoProxyIterator ProxyInfoSysconfig::noProxyEnd() const
    { return _no_proxy.end(); }

  } // namespace media
} // namespace zypp
