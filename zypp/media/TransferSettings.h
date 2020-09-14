
#ifndef TRANSFER_SETTINGS_H_
#define TRANSFER_SETTINGS_H_

#include <string>
#include <vector>
#include <zypp/base/Flags.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/Pathname.h>
#include <zypp/Url.h>

namespace zypp::proto {
  class TransferSettings;
}

namespace zypp
{
  namespace media
  {

    /**
     * Holds transfer setting
     */
    class TransferSettings
    {
    public:
      /** Constructs a transfer program cmd line access. */
      TransferSettings();

      TransferSettings( const zypp::proto::TransferSettings &settings );

      typedef std::vector<std::string> Headers;

      /** reset the settings to the defaults */
      void reset();


      /** add a header, on the form "Foo: Bar" */
      void addHeader( std::string && val_r );

      /** returns a list of all added headers */
      Headers headers() const;

      /** sets the user agent ie: "Mozilla v3" */
      void setUserAgentString( std::string && val_r );

      /** user agent string */
      std::string userAgentString() const;


      /** sets the auth username */
      void setUsername( std::string && val_r );

      /** auth username */
      std::string username() const;

      /** sets the auth password */
      void setPassword( std::string && val_r );

      /** auth password */
      std::string password() const;

      /** returns the user and password as a user:pass string */
      std::string userPassword() const;

      /** sets anonymous authentication (ie: for ftp) */
      void setAnonymousAuth();


      /** whether the proxy is used or not */
      void setProxyEnabled( bool enabled );

      /** proxy is enabled */
      bool proxyEnabled() const;


      /** proxy to use if it is enabled */
      void setProxy( std::string && val_r );

      /** proxy host */
      std::string proxy() const;


      /** sets the proxy user */
      void setProxyUsername( std::string && val_r );

      /** proxy auth username */
      std::string proxyUsername() const;

      /** sets the proxy password */
      void setProxyPassword( std::string && val_r );

      /** proxy auth password */
      std::string proxyPassword() const;

      /** returns the proxy user and password as a user:pass string */
      std::string proxyUserPassword() const;


      /** set the connect timeout */
      void setConnectTimeout( long t );

      /** connection timeout */
      long connectTimeout() const;


      /** set the transfer timeout */
      void setTimeout( long t );

      /** transfer timeout */
      long timeout() const;


      /** Set maximum number of concurrent connections for a single transfer */
      void setMaxConcurrentConnections(long v);

      /** Maximum number of concurrent connections for a single transfer */
      long maxConcurrentConnections() const;


      /** Set minimum download speed (bytes per second) until the connection is dropped */
      void setMinDownloadSpeed(long v);

      /** Minimum download speed (bytes per second) until the connection is dropped */
      long minDownloadSpeed() const;


      /** Set max download speed (bytes per second) */
      void setMaxDownloadSpeed(long v);

      /** Maximum download speed (bytes per second) */
      long maxDownloadSpeed() const;


      /** Set maximum silent retries */
      void setMaxSilentTries(long v);

      /** Maximum silent retries */
      long maxSilentTries() const;


      /** Sets whether to verify host for ssl */
      void setVerifyHostEnabled( bool enabled );

      /** Whether to verify host for ssl */
      bool verifyHostEnabled() const;


      /** Sets whether to verify host for ssl */
      void setVerifyPeerEnabled( bool enabled );

      /** Whether to verify peer for ssl */
      bool verifyPeerEnabled() const;


      /** Sets the SSL certificate authorities path */
      void setCertificateAuthoritiesPath( Pathname && val_r );

      /** SSL certificate authorities path ( default: /etc/ssl/certs ) */
      Pathname certificateAuthoritiesPath() const;


      /** set the allowed authentication types */
      void setAuthType( std::string && val_r );

      /** get the allowed authentication types */
      std::string authType() const;


      /** set whether HEAD requests are allowed */
      void setHeadRequestsAllowed(bool allowed);

      /** whether HEAD requests are allowed */
      bool headRequestsAllowed() const;


      /** Sets the SSL client certificate file */
      void setClientCertificatePath( Pathname && val_r );

      /** SSL client certificate file */
      Pathname clientCertificatePath() const;


      /** Sets the SSL client key file */
      void setClientKeyPath( Pathname && val_r );

      /** SSL client key file */
      Pathname clientKeyPath() const;

      const zypp::proto::TransferSettings &protoData() const;
      zypp::proto::TransferSettings &protoData();

    protected:
      class Impl;
      RWCOW_pointer<Impl> _impl;
    };

  } // namespace media
} // namespece zypp

#endif
