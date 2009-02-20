#include <iostream>
#include <vector>
#include <sstream>

#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/WatchFile.h"
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/ExternalProgram.h"
#include "zypp/media/TransferSettings.h"

using namespace std;

#define ARIA2C_BINARY "/usr/bin/aria2c"
#define CURL_BINARY "/usr/bin/curl"

namespace zypp
{
namespace media
{
    
class TransferSettings::Impl
{
public:
    Impl()
        : _useproxy(false)
        , _timeout(0)
        , _connect_timeout(0)
    {}

    virtual ~Impl()
    {}    

    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }

private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }

public:
    vector<string> _headers;
    string _useragent;
    string _username;
    string _password;
    bool _useproxy;
    string _proxy;
    string _proxy_username;
    string _proxy_password;
    long _timeout;
    long _connect_timeout;
    Url _url;
    Pathname _targetdir;
};
    
TransferSettings::TransferSettings()
    : _impl(new TransferSettings::Impl())
{

}

void TransferSettings::addHeader( const std::string &header )
{
    _impl->_headers.push_back(header);
}

TransferSettings::Headers::const_iterator TransferSettings::headersBegin() const
{
    return _impl->_headers.begin();
}

TransferSettings::Headers::const_iterator TransferSettings::headersEnd() const
{
    return _impl->_headers.end();
}

void TransferSettings::setUserAgentString( const std::string &agent )
{
    _impl->_useragent = agent;
}

std::string TransferSettings::userAgentString() const
{
    return _impl->_useragent;
}

void TransferSettings::setUsername( const std::string &username )
{
    _impl->_username = username;
}

std::string TransferSettings::username() const
{
    return _impl->_username;
}

void TransferSettings::setPassword( const std::string &password )
{
    _impl->_password = password;
}

std::string TransferSettings::password() const
{
    return _impl->_password;
}

void TransferSettings::setProxyEnabled( bool enabled )
{
    _impl->_useproxy = enabled;
}

bool TransferSettings::proxyEnabled() const
{
    return _impl->_useproxy;
}

void TransferSettings::setProxy( const std::string &proxy )
{
    _impl->_proxy = proxy;
}

std::string TransferSettings::proxy() const
{
    return _impl->_proxy;
}

void TransferSettings::setProxyUsername( const std::string &proxyuser )
{
    _impl->_proxy_username = proxyuser;
}

std::string TransferSettings::proxyUsername() const
{
    return _impl->_proxy_username;
}

void TransferSettings::setProxyPassword( const std::string &proxypass )
{
    _impl->_proxy_password = proxypass;
}

std::string TransferSettings::proxyPassword() const
{
    return _impl->_proxy_password;
}

void TransferSettings::setTimeout( long t )
{
    _impl->_timeout = t;
}

long TransferSettings::timeout() const
{
    return _impl->_timeout;
}

void TransferSettings::setConnectTimeout( long t )
{
    _impl->_connect_timeout = t;
}

long TransferSettings::connectTimeout() const
{
    return _impl->_connect_timeout;
}

} // ns media
} // ns zypp

