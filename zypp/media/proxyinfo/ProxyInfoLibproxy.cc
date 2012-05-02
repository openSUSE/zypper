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
#include "zypp/base/WatchFile.h"
#include "zypp/Pathname.h"

#include "zypp/media/proxyinfo/ProxyInfoLibproxy.h"

using namespace std;
using namespace zypp::base;

namespace zypp {
  namespace media {

    struct TmpUnsetEnv
    {
      TmpUnsetEnv( const char * var_r )
      : _set( false )
      , _var( var_r )
      {
	const char * val = getenv( _var.c_str() );
	if ( val )
	{
	  _set = true;
	  _val = val;
	  ::unsetenv( _var.c_str() );
	}
      }

      ~TmpUnsetEnv()
      {
	if ( _set )
	{
	  setenv( _var.c_str(), _val.c_str(), 1 );
	}
      }

      bool _set;
      std::string _var;
      std::string _val;
    };

    static pxProxyFactory * getProxyFactory()
    {
      static pxProxyFactory * proxyFactory = 0;

      // Force libproxy into using "/etc/sysconfig/proxy"
      // if it exists.
      static WatchFile sysconfigProxy( "/etc/sysconfig/proxy", WatchFile::NO_INIT );
      if ( sysconfigProxy.hasChanged() )
      {
	MIL << "Build Libproxy Factory from /etc/sysconfig/proxy" << endl;
	if ( proxyFactory )
	  ::px_proxy_factory_free( proxyFactory );

	TmpUnsetEnv envguard[] __attribute__ ((__unused__)) = { "KDE_FULL_SESSION", "GNOME_DESKTOP_SESSION_ID", "DESKTOP_SESSION" };
	proxyFactory = ::px_proxy_factory_new();
      }
      else if ( ! proxyFactory )
      {
	MIL << "Build Libproxy Factory" << endl;
	proxyFactory = ::px_proxy_factory_new();
      }

      return proxyFactory;
    }

    ProxyInfoLibproxy::ProxyInfoLibproxy()
    : ProxyInfo::Impl()
    {
      _factory = getProxyFactory();
      _enabled = !(_factory == NULL);
    }

    ProxyInfoLibproxy::~ProxyInfoLibproxy()
    {}

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
