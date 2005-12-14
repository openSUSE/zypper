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
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/Pathname.h"

#include "zypp/media/ProxyInfo.h"

using namespace std;
using namespace zypp::base;

namespace zypp {
  namespace media {

    shared_ptr<ProxyInfo::Impl> ProxyInfo::Impl::_nullimpl;

    ProxyInfo::ProxyInfo()
    : _pimpl( Impl::_nullimpl )
    {}
    ProxyInfo::ProxyInfo(RW_pointer<Impl> impl)
    : _pimpl(impl)
    {}

    bool ProxyInfo::enabled()
    { return _pimpl->enabled(); }

    std::string ProxyInfo::http()
    { return _pimpl->http(); }

    std::string ProxyInfo::ftp()
    { return _pimpl->ftp(); }

    std::string ProxyInfo::https()
    { return _pimpl->https(); }

    std::list<std::string> ProxyInfo::noProxy()
    { return _pimpl->noProxy(); }

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

    ProxyInfo_sysconfig::ProxyInfo_sysconfig(const Pathname & path)
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
	_http = it->second;
      it = data.find("HTTPS_PROXY");
      if (it != data.end())
	_https = it->second;
      it = data.find("FTP_PROXY");
      if (it != data.end())
	_ftp = it->second;
      it = data.find("NO_PROXY");
      if (it != data.end())
	_no_proxy.push_back(it->second);
#warning FIXME once splitting a string is in str:: namespace
    }


  } // namespace media
} // namespace zypp
