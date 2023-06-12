/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp-curl/TransferSettings
 *
 */
#ifndef ZYPP_CURL_TRANSFER_SETTINGS_H_INCLUDED
#define ZYPP_CURL_TRANSFER_SETTINGS_H_INCLUDED

#include <string>
#include <vector>
#include <zypp-core/base/Flags.h>
#include <zypp-core/base/PtrTypes.h>
#include <zypp-core/Pathname.h>
#include <zypp-core/Url.h>
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

      typedef std::vector<std::string> Headers;

      /** reset the settings to the defaults */
      void reset();

      /** add a header, on the form "Foo: Bar" */
      void addHeader( std::string && val_r );
      void addHeader( const std::string & val_r );

      /** returns a list of all added headers */
      const Headers &headers() const;

      /** sets the user agent ie: "Mozilla v3" */
      void setUserAgentString( std::string && val_r );
      void setUserAgentString( const std::string &val_r );

      /** user agent string */
      const std::string &userAgentString() const;


      /** sets the auth username */
      void setUsername( const std::string &val_r );
      void setUsername( std::string && val_r );

      /** auth username */
      const std::string &username() const;

      /** sets the auth password */
      void setPassword( const std::string & val_r );
      void setPassword( std::string && val_r );

      /** auth password */
      const std::string &password() const;

      /** returns the user and password as a user:pass string */
      std::string userPassword() const;

      /** sets anonymous authentication (ie: for ftp) */
      void setAnonymousAuth();


      /** whether the proxy is used or not */
      void setProxyEnabled( bool enabled );

      /** proxy is enabled */
      bool proxyEnabled() const;


      /** proxy to use if it is enabled */
      void setProxy( const std::string &val_r );
      void setProxy( std::string && val_r );

      /** proxy host */
      const std::string &proxy() const;


      /** sets the proxy user */
      void setProxyUsername( const std::string &val_r );
      void setProxyUsername( std::string && val_r );

      /** proxy auth username */
      const std::string &proxyUsername() const;

      /** sets the proxy password */
      void setProxyPassword( const std::string &val_r );
      void setProxyPassword( std::string && val_r );

      /** proxy auth password */
      const std::string &proxyPassword() const;

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
      void setCertificateAuthoritiesPath( const Pathname &val_r );
      void setCertificateAuthoritiesPath( Pathname && val_r );

      /** SSL certificate authorities path ( default: /etc/ssl/certs ) */
      const Pathname &certificateAuthoritiesPath() const;


      /** set the allowed authentication types */
      void setAuthType( const std::string &val_r );
      void setAuthType( std::string && val_r );

      /** get the allowed authentication types */
      const std::string &authType() const;


      /** set whether HEAD requests are allowed */
      void setHeadRequestsAllowed(bool allowed);

      /** whether HEAD requests are allowed */
      bool headRequestsAllowed() const;


      /** Sets the SSL client certificate file */
      void setClientCertificatePath( const Pathname &val_r );
      void setClientCertificatePath( Pathname && val_r );

      /** SSL client certificate file */
      const Pathname &clientCertificatePath() const;


      /** Sets the SSL client key file */
      void setClientKeyPath( const Pathname &val_r );
      void setClientKeyPath( Pathname && val_r );

      /** SSL client key file */
      const Pathname &clientKeyPath() const;

    protected:
      class Impl;
      RWCOW_pointer<Impl> _impl;
    };

  } // namespace media
} // namespece zypp

#endif // ZYPP_CURL_TRANSFER_SETTINGS_H_INCLUDED
