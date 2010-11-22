/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/proxyinfo/ProxyInfoLibproxy.cc
 *
*/

#include <iostream>
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/Pathname.h"

#include "zypp/media/proxyinfo/ProxyInfoLibproxy.h"

using namespace std;
using namespace zypp::base;

namespace zypp {
  namespace media {

    ProxyInfoLibproxy::ProxyInfoLibproxy()
    : ProxyInfo::Impl()
    {
      _factory = px_proxy_factory_new();
      _enabled = !(_factory == NULL);
    }

    ProxyInfoLibproxy::~ProxyInfoLibproxy()
    {
      if (_enabled) {
	px_proxy_factory_free(_factory);
	_factory = NULL;
	_enabled = false;
      }
    }

    std::string ProxyInfoLibproxy::proxy(const Url & url_r) const
    {
      if (!_enabled)
	return "";

      const url::ViewOption vopt =
	      url::ViewOption::WITH_SCHEME
	      + url::ViewOption::WITH_HOST
	      + url::ViewOption::WITH_PORT
	      + url::ViewOption::WITH_PATH_NAME;

      char **proxies = px_proxy_factory_get_proxies(_factory,
						    (char *)url_r.asString(vopt).c_str());
      if (!proxies)
	      return "";

      /* cURL can only handle HTTP proxies, not SOCKS. And can only handle
	 one. So look through the list and find an appropriate one. */
      char *result = NULL;

      for (int i = 0; proxies[i]; i++) {
	      if (!result &&
		  !strncmp(proxies[i], "http://", 7))
		      result = proxies[i];
	      else
		      free(proxies[i]);
      }
      free(proxies);

      if (!result)
	      return "";

      std::string sresult = result;
      free(result);
      return sresult;
    }

    ProxyInfo::NoProxyIterator ProxyInfoLibproxy::noProxyBegin() const
    { return _no_proxy.begin(); }

    ProxyInfo::NoProxyIterator ProxyInfoLibproxy::noProxyEnd() const
    { return _no_proxy.end(); }

  } // namespace media
} // namespace zypp
