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


    namespace proxyinfo {
      map<string,string> sysconfigRead(const Pathname & _path)
      {
        DBG << "Load '" << _path << "'" << endl;
        map<string,string> ret;
      
        string line;
        ifstream in( _path.asString().c_str() );
        if ( in.fail() ) {
          WAR << "Unable to load '" << _path << "'" << endl;
          return ret;
        }
        while( getline( in, line ) ) {
          if ( *line.begin() != '#' ) {
            string::size_type pos = line.find( '=', 0 );
            if ( pos != string::npos ) {
              string key = str::trim( line.substr( 0, pos ) );
              string value = str::trim( line.substr( pos + 1, line.length() - pos - 1 ) );
              if ( value.length() >= 2 && *(value.begin()) == '"' &&
                   *(value.rbegin()) == '"' ) {
                value = value.substr( 1, value.length() - 2 );
              }
              if ( value.length() >= 2 && *(value.begin()) == '\'' &&
                   *(value.rbegin()) == '\'' ) {
                value = value.substr( 1, value.length() - 2 );
              }
              DBG << "KEY: '" << key << "' VALUE: '" << value << "'" << endl;
              ret[key] = value;
            }
          }
        }
      return ret;
      }
    } // namespace proxyinfo

    ProxyInfoSysconfig::ProxyInfoSysconfig(const Pathname & path)
    : ProxyInfo::Impl()
    {
      map<string,string> data = proxyinfo::sysconfigRead(
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

    std::string ProxyInfoSysconfig::proxy(const std::string & protocol_r) const
    { 
      map<string,string>::const_iterator it = _proxies.find(protocol_r);
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
