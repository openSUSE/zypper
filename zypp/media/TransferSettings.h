
#ifndef TRANSFER_SETTINGS_H_
#define TRANSFER_SETTINGS_H_

#include <string>
#include <vector>
#include "zypp/base/Flags.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"
#include "zypp/Url.h"

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
  /**
   * Constructs a transfer program cmd line access.
   */
  TransferSettings();

  /**
   * Constructs the settings from a url object where.
   * authentication/proxy  information can be extracted
   * from the url
   */
  TransferSettings( const zypp::Url &url );

  typedef std::vector<std::string> Headers;

  /**
   * reset the settings to the defaults
   */
  void reset();

  /**
   * add a header, on the form "Foo: Bar"
   */
  void addHeader( const std::string &header );

  /**
   * begin iterators to additional headers
   */
  Headers::const_iterator headersBegin() const;

  /**
   * end iterators to additional headers
   */
  Headers::const_iterator headersEnd() const;

  /**
   * sets the user agent ie: "Mozilla v3"
   */
  void setUserAgentString( const std::string &agent );

  /**
   * user agent string
   */
  std::string userAgentString() const;

  /**
   * sets the auth username
   */
  void setUsername( const std::string &username );

  /**
   * auth username
   */
  std::string username() const;

  /**
   * sets the auth password
   */
  void setPassword( const std::string &password );

  /**
   * auth password
   */
  std::string password() const;

  /**
   * returns the user and password as
   * a user:pass string
   */
  std::string userPassword() const;

  /**
   * sets anonymous authentication (ie: for ftp)
   */
  void setAnonymousAuth();

  /**
   * whether the proxy is used or not
   */
  void setProxyEnabled( bool enabled );

  /**
   * proxy is enabled
   */
  bool proxyEnabled() const;

  /**
   * proxy to use if it is enabled
   */
  void setProxy( const std::string &proxyhost );

  /**
   * proxy host
   */
  std::string proxy() const;

  /**
   * sets the proxy user
   */
  void setProxyUsername( const std::string &proxyuser );

  /**
   * proxy auth username
   */
  std::string proxyUsername() const;

  /**
   * sets the proxy password
   */
  void setProxyPassword( const std::string &proxypass );

  /**
   * proxy auth password
   */
  std::string proxyPassword() const;

  /**
   * returns the proxy user and password as
   * a user:pass string
   */
  std::string proxyUserPassword() const;

  /**
   * set the connect timeout
   */
  void setConnectTimeout( long t );

  /**
   * connection timeout
   */
  long connectTimeout() const;

  /**
   * set the transfer timeout
   */
  void setTimeout( long t );

  /**
   * transfer timeout
   */
  long timeout() const;

  /**
   * Maximum number of concurrent connections for a single transfer
   */
  long maxConcurrentConnections() const;

  /**
   * Set maximum number of concurrent connections for a single transfer
   */
  void setMaxConcurrentConnections(long v);

  /**
   * Minimum download speed (bytes per second)
   * until the connection is dropped
   */
  long minDownloadSpeed() const;

  /**
   * Set minimum download speed (bytes per second)
   * until the connection is dropped
   */
  void setMinDownloadSpeed(long v);

  /**
   * Maximum download speed (bytes per second)
   */
  long maxDownloadSpeed() const;

  /**
   * Set max download speed (bytes per second)
   */
  void setMaxDownloadSpeed(long v);

  /**
   * Maximum silent retries
   */
  long maxSilentTries() const;

  /**
   * Set maximum silent retries
   */
  void setMaxSilentTries(long v);

  /**
   * Whether to verify host for ssl
   */
  bool verifyHostEnabled() const;

  /**
   * Sets whether to verify host for ssl
   */
  void setVerifyHostEnabled( bool enabled );

  /**
   * Whether to verify peer for ssl
   */
  bool verifyPeerEnabled() const;

  /**
   * Sets whether to verify host for ssl
   */
  void setVerifyPeerEnabled( bool enabled );

  /**
   * SSL certificate authorities path
   * ( default: /etc/ssl/certs )
   */
  Pathname certificateAuthoritiesPath() const;

  /**
   * Sets the SSL certificate authorities path
   */
  void setCertificateAuthoritiesPath( const zypp::Pathname &path );

  /**
   * set the allowed authentication types
   */
  void setAuthType( const std::string &authtype );

  /**
   * get the allowed authentication types
   */
  std::string authType() const;

  /**
   * set whether HEAD requests are allowed
   */
  void setHeadRequestsAllowed(bool allowed);

  /**
   * whether HEAD requests are allowed
   */
  bool headRequestsAllowed() const;

  /**
   * SSL client certificate file
   */
  Pathname clientCertificatePath() const;

  /**
   * Sets the SSL client certificate file
   */
  void setClientCertificatePath( const zypp::Pathname &path );

protected:
  class Impl;
  RWCOW_pointer<Impl> _impl;
};

} // ns media
} // ns zypp

#endif
