
#ifndef TRANSFER_PROGRAM_H_
#define TRANSFER_PROGRAM_H_

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
 * Easy access to the transfer command line program no matter
 * which one it is
 *
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

protected:
  class Impl;
  RWCOW_pointer<Impl> _impl;
};

} // ns media
} // ns zypp

#endif
