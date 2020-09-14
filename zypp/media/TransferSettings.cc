#include <iostream>
#include <sstream>

#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/base/WatchFile.h>
#include <zypp/base/ReferenceCounted.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/ExternalProgram.h>
#include <zypp/media/TransferSettings.h>
#include <zypp/ZConfig.h>

#include <transfersettings.pb.h>

using std::endl;

#define CURL_BINARY "/usr/bin/curl"

namespace zypp
{
  namespace media
  {
    class TransferSettings::Impl
    {
    public:
      Impl() {
        _settingsObj.set_useproxy( false );
        _settingsObj.set_timeout( ZConfig::instance().download_transfer_timeout() );
        _settingsObj.set_connect_timeout( 60 );
        _settingsObj.set_maxconcurrentconnections( ZConfig::instance().download_max_concurrent_connections() );
        _settingsObj.set_mindownloadspeed(ZConfig::instance().download_min_download_speed());
        _settingsObj.set_maxdownloadspeed(ZConfig::instance().download_max_download_speed());
        _settingsObj.set_maxsilenttries(ZConfig::instance().download_max_silent_tries() );
        _settingsObj.set_verify_host(false);
        _settingsObj.set_verify_peer(false);
        _settingsObj.set_ca_path("/etc/ssl/certs");
        _settingsObj.set_head_requests_allowed(true);
      }

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
      std::vector<std::string> _headers;
      zypp::proto::TransferSettings _settingsObj;
    };

    TransferSettings::TransferSettings()
    : _impl(new TransferSettings::Impl())
    {}

    TransferSettings::TransferSettings( const proto::TransferSettings &settings )
      : _impl(new TransferSettings::Impl())
    {
      _impl->_settingsObj = settings;
    }

    void TransferSettings::reset()
    { _impl.reset(new TransferSettings::Impl()); }


    void TransferSettings::addHeader( std::string && val_r )
    { if ( ! val_r.empty() ) _impl->_settingsObj.add_header( std::move(val_r) ); }

    TransferSettings::Headers TransferSettings::headers() const
    {
      //@TODO check if we could use a vector of std::string_view here
      auto vec = Headers();
      for ( const auto &head : _impl->_settingsObj.header() ) {
        vec.push_back( head );
      }
      return vec;
    }

    void TransferSettings::setUserAgentString( std::string && val_r )
    { _impl->_settingsObj.set_useragent( std::move(val_r) ); }

    std::string TransferSettings::userAgentString() const
    { return _impl->_settingsObj.useragent(); }


    void TransferSettings::setUsername( std::string && val_r )
    { _impl->_settingsObj.set_username( std::move(val_r) ); }

    std::string TransferSettings::username() const
    { return _impl->_settingsObj.username(); }

    void TransferSettings::setPassword( std::string && val_r )
    { _impl->_settingsObj.set_password( std::move(val_r) ); }

    std::string TransferSettings::password() const
    { return _impl->_settingsObj.password(); }

    std::string TransferSettings::userPassword() const
    {
      std::string userpwd = username();
      if ( password().size() ) {
	userpwd += ":" + password();
      }
      return userpwd;
    }

    void TransferSettings::setAnonymousAuth()
    {
      setUsername("anonymous");
      setPassword("yast@" LIBZYPP_VERSION_STRING);
    }


    void TransferSettings::setProxyEnabled( bool enabled )
    { _impl->_settingsObj.set_useproxy( enabled ); }

    bool TransferSettings::proxyEnabled() const
    { return _impl->_settingsObj.useproxy(); }


    void TransferSettings::setProxy( std::string && val_r )
    { _impl->_settingsObj.set_proxy( std::move(val_r) ); }

    std::string TransferSettings::proxy() const
    { return _impl->_settingsObj.proxy(); }


    void TransferSettings::setProxyUsername( std::string && val_r )
    { _impl->_settingsObj.set_proxy_username( std::move(val_r) ); }

    std::string TransferSettings::proxyUsername() const
    { return _impl->_settingsObj.proxy_username(); }

    void TransferSettings::setProxyPassword( std::string && val_r )
    { _impl->_settingsObj.set_proxy_password( std::move(val_r) ); }

    std::string TransferSettings::proxyPassword() const
    { return _impl->_settingsObj.proxy_password(); }

    std::string TransferSettings::proxyUserPassword() const
    {
      std::string userpwd = proxyUsername();
      if ( proxyPassword().size() ) {
	userpwd += ":" + proxyPassword();
      }
      return userpwd;
    }


    void TransferSettings::setTimeout( long t )
    { _impl->_settingsObj.set_timeout(t); }

    long TransferSettings::timeout() const
    { return _impl->_settingsObj.timeout(); }


    void TransferSettings::setConnectTimeout( long t )
    { _impl->_settingsObj.set_connect_timeout(t); }

    long TransferSettings::connectTimeout() const
    { return _impl->_settingsObj.connect_timeout(); }


    void TransferSettings::setMaxConcurrentConnections( long v )
    { _impl->_settingsObj.set_maxconcurrentconnections(v); }

    long TransferSettings::maxConcurrentConnections() const
    { return _impl->_settingsObj.maxconcurrentconnections(); }


    void TransferSettings::setMinDownloadSpeed( long v )
    { _impl->_settingsObj.set_mindownloadspeed(v); }

    long TransferSettings::minDownloadSpeed() const
    { return _impl->_settingsObj.mindownloadspeed(); }


    void TransferSettings::setMaxDownloadSpeed( long v )
    { _impl->_settingsObj.set_maxdownloadspeed(v); }

    long TransferSettings::maxDownloadSpeed() const
    { return _impl->_settingsObj.maxdownloadspeed(); }


    void TransferSettings::setMaxSilentTries( long v )
    { _impl->_settingsObj.set_maxsilenttries(v); }

    long TransferSettings::maxSilentTries() const
    { return _impl->_settingsObj.maxsilenttries(); }


    void TransferSettings::setVerifyHostEnabled( bool enabled )
    { _impl->_settingsObj.set_verify_host(enabled); }

    bool TransferSettings::verifyHostEnabled() const
    { return _impl->_settingsObj.verify_host(); }


    void TransferSettings::setVerifyPeerEnabled( bool enabled )
    { _impl->_settingsObj.set_verify_peer(enabled); }

    bool TransferSettings::verifyPeerEnabled() const
    { return _impl->_settingsObj.verify_peer(); }


    void TransferSettings::setClientCertificatePath( Pathname && val_r )
    { _impl->_settingsObj.set_client_cert_path( val_r.asString() ); }

    Pathname TransferSettings::clientCertificatePath() const
    { return _impl->_settingsObj.client_cert_path(); }


    void TransferSettings::setClientKeyPath( Pathname && val_r )
    { _impl->_settingsObj.set_client_key_path( val_r.asString() ); }

    Pathname TransferSettings::clientKeyPath() const
    { return _impl->_settingsObj.client_key_path(); }

    proto::TransferSettings &TransferSettings::protoData()
    {
      return _impl->_settingsObj;
    }

    const proto::TransferSettings &TransferSettings::protoData() const
    {
      return _impl->_settingsObj;
    }

    void TransferSettings::setCertificateAuthoritiesPath( Pathname && val_r )
    { _impl->_settingsObj.set_ca_path(val_r.asString()); }

    Pathname TransferSettings::certificateAuthoritiesPath() const
    { return _impl->_settingsObj.ca_path(); }


    void TransferSettings::setAuthType( std::string && val_r )
    { _impl->_settingsObj.set_authtype( std::move(val_r) ); }

    std::string TransferSettings::authType() const
    { return _impl->_settingsObj.authtype(); }


    void TransferSettings::setHeadRequestsAllowed( bool allowed )
    { _impl->_settingsObj.set_head_requests_allowed(allowed); }

    bool TransferSettings::headRequestsAllowed() const
    { return _impl->_settingsObj.head_requests_allowed(); }

  } // namespace media
} // namespace zypp

