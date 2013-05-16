#include <iostream>
#include <sstream>

#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/WatchFile.h"
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/ExternalProgram.h"
#include "zypp/media/TransferSettings.h"
#include "zypp/ZConfig.h"

using namespace std;

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
        , _maxConcurrentConnections(ZConfig::instance().download_max_concurrent_connections())
        , _minDownloadSpeed(ZConfig::instance().download_min_download_speed())
        , _maxDownloadSpeed(ZConfig::instance().download_max_download_speed())
        , _maxSilentTries(ZConfig::instance().download_max_silent_tries())
        , _verify_host(false)
        , _verify_peer(false)
        , _ca_path("/etc/ssl/certs")
        , _head_requests_allowed(true)
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
    string _authtype;
    long _timeout;
    long _connect_timeout;
    Url _url;
    Pathname _targetdir;

    long _maxConcurrentConnections;
    long _minDownloadSpeed;
    long _maxDownloadSpeed;
    long _maxSilentTries;

    bool _verify_host;
    bool _verify_peer;
    Pathname _ca_path;

    // workarounds
    bool _head_requests_allowed;
};

TransferSettings::TransferSettings()
    : _impl(new TransferSettings::Impl())
{

}

void TransferSettings::reset()
{
    _impl.reset(new TransferSettings::Impl());
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

void TransferSettings::setAnonymousAuth()
{
    setUsername("anonymous");
    string id = "yast@";
    setPassword(id + VERSION);
}

std::string TransferSettings::password() const
{
    return _impl->_password;
}

std::string TransferSettings::userPassword() const
{
    string userpwd = username();
    if ( password().size() ) {
        userpwd += ":" + password();
    }
    return userpwd;
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

std::string TransferSettings::proxyUserPassword() const
{
    string userpwd = proxyUsername();
    if ( proxyPassword().size() ) {
        userpwd += ":" + proxyPassword();
    }
    return userpwd;
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

long TransferSettings::maxConcurrentConnections() const
{
    return _impl->_maxConcurrentConnections;
}

void TransferSettings::setMaxConcurrentConnections(long v)
{
    _impl->_maxConcurrentConnections = v;
}

long TransferSettings::minDownloadSpeed() const
{
    return _impl->_minDownloadSpeed;
}

void TransferSettings::setMinDownloadSpeed(long v)
{
    _impl->_minDownloadSpeed = v;
}

long TransferSettings::maxDownloadSpeed() const
{
    return _impl->_maxDownloadSpeed;
}

void TransferSettings::setMaxDownloadSpeed(long v)
{
    _impl->_maxDownloadSpeed = v;
}

long TransferSettings::maxSilentTries() const
{
    return _impl->_maxSilentTries;
}

void TransferSettings::setMaxSilentTries(long v)
{
    _impl->_maxSilentTries = v;
}

bool TransferSettings::verifyHostEnabled() const
{
    return _impl->_verify_host;
}

void TransferSettings::setVerifyHostEnabled( bool enabled )
{
    _impl->_verify_host = enabled;
}

bool TransferSettings::verifyPeerEnabled() const
{
    return _impl->_verify_peer;
}


void TransferSettings::setVerifyPeerEnabled( bool enabled )
{
    _impl->_verify_peer = enabled;
}

Pathname TransferSettings::certificateAuthoritiesPath() const
{
    return _impl->_ca_path;
}

void TransferSettings::setCertificateAuthoritiesPath( const zypp::Pathname &path )
{
    _impl->_ca_path = path;
}

void TransferSettings::setAuthType( const std::string &authtype)
{
    _impl->_authtype = authtype;
}

std::string TransferSettings::authType() const
{
    return _impl->_authtype;
}

void TransferSettings::setHeadRequestsAllowed(bool allowed)
{
    _impl->_head_requests_allowed = allowed;
}

bool TransferSettings::headRequestsAllowed() const
{
    return _impl->_head_requests_allowed;
}

} // ns media
} // ns zypp

