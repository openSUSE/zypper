#include "CurlHelper.h"

#include "zypp/PathInfo.h"
#include "zypp/Pathname.h"
#include "zypp/Target.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/media/ProxyInfo.h"
#include "zypp/media/MediaUserAuth.h"
#include "zypp/media/MediaException.h"
#include <list>

using namespace zypp;

namespace internal
{

void globalInitCurlOnce()
{
  // function-level static <=> std::call_once
  static bool once __attribute__ ((__unused__)) = ( [] {
    if ( curl_global_init( CURL_GLOBAL_ALL ) != 0 )
      WAR << "curl global init failed" << std::endl;
  } (), true );
}

int log_curl(CURL *curl, curl_infotype info,
  char *ptr, size_t len, void *max_lvl)
{
  std::string pfx(" ");
  long        lvl = 0;
  switch( info)
  {
    case CURLINFO_TEXT:       lvl = 1; pfx = "*"; break;
    case CURLINFO_HEADER_IN:  lvl = 2; pfx = "<"; break;
    case CURLINFO_HEADER_OUT: lvl = 2; pfx = ">"; break;
    default:                                      break;
  }
  if( lvl > 0 && max_lvl != NULL && lvl <= *((long *)max_lvl))
  {
    std::string                            msg(ptr, len);
    std::list<std::string>                 lines;
    std::list<std::string>::const_iterator line;
    zypp::str::split(msg, std::back_inserter(lines), "\r\n");
    for(line = lines.begin(); line != lines.end(); ++line)
    {
      DBG << pfx << " " << *line << std::endl;
    }
  }
  return 0;
}

size_t log_redirects_curl( char *ptr, size_t size, size_t nmemb, void *userdata)
{
  // INT << "got header: " << string(ptr, ptr + size*nmemb) << endl;

  char * lstart = ptr, * lend = ptr;
  size_t pos = 0;
  size_t max = size * nmemb;
  while (pos + 1 < max)
  {
    // get line
    for (lstart = lend; *lend != '\n' && pos < max; ++lend, ++pos);

    // look for "Location"
    if ( lstart[0] == 'L'
         && lstart[1] == 'o'
         && lstart[2] == 'c'
         && lstart[3] == 'a'
         && lstart[4] == 't'
         && lstart[5] == 'i'
         && lstart[6] == 'o'
         && lstart[7] == 'n'
         && lstart[8] == ':' )
    {
      std::string line { lstart, *(lend-1)=='\r' ? lend-1 : lend };
      DBG << "redirecting to " << line << std::endl;
      if ( userdata ) {
        *reinterpret_cast<std::string *>( userdata ) = line;
      }
      return max;
    }

    // continue with the next line
    if (pos + 1 < max)
    {
      ++lend;
      ++pos;
    }
    else
      break;
  }

  return max;
}

/**
 * Fills the settings structure using options passed on the url
 * for example ?timeout=x&proxy=foo
 */
void fillSettingsFromUrl( const Url &url, media::TransferSettings &s )
{
  {
    const std::string & param { url.getQueryParam("timeout") };
    if( ! param.empty() )
    {
      long num = str::strtonum<long>(param);
      if( num >= 0 && num <= TRANSFER_TIMEOUT_MAX )
        s.setTimeout( num );
    }
  }
  {
    std::string param { url.getUsername() };
    if ( ! param.empty() )
    {
      s.setUsername( std::move(param) );
      param = url.getPassword();
      if ( ! param.empty() )
        s.setPassword( std::move(param) );
    }
    else
    {
      // if there is no username, set anonymous auth
      if ( ( url.getScheme() == "ftp" || url.getScheme() == "tftp" ) && s.username().empty() )
        s.setAnonymousAuth();
    }
  }
  if ( url.getScheme() == "https" )
  {
    s.setVerifyPeerEnabled( false );
    s.setVerifyHostEnabled( false );

    const std::string & verify { url.getQueryParam("ssl_verify") };
    if( verify.empty() || verify == "yes" )
    {
      s.setVerifyPeerEnabled( true );
      s.setVerifyHostEnabled( true );
    }
    else if ( verify == "no" )
    {
      s.setVerifyPeerEnabled( false );
      s.setVerifyHostEnabled( false );
    }
    else
    {
      std::vector<std::string> flags;
      str::split( verify, std::back_inserter(flags), "," );
      for ( const auto & flag : flags )
      {
        if ( flag == "host" )
          s.setVerifyHostEnabled( true );
        else if ( flag == "peer" )
          s.setVerifyPeerEnabled( true );
        else
          ZYPP_THROW( media::MediaBadUrlException(url, "Unknown ssl_verify flag "+flag) );
      }
    }
  }
  {
    Pathname ca_path { url.getQueryParam("ssl_capath") };
    if( ! ca_path.empty() )
    {
      if( ! PathInfo(ca_path).isDir() || ! ca_path.absolute() )
        ZYPP_THROW(media::MediaBadUrlException(url, "Invalid ssl_capath path"));
      else
        s.setCertificateAuthoritiesPath( std::move(ca_path) );
    }
  }
  {
    Pathname client_cert { url.getQueryParam("ssl_clientcert") };
    if( ! client_cert.empty() )
    {
      if( ! PathInfo(client_cert).isFile() || ! client_cert.absolute() )
        ZYPP_THROW(media::MediaBadUrlException(url, "Invalid ssl_clientcert file"));
      else
        s.setClientCertificatePath( std::move(client_cert) );
    }
  }
  {
    Pathname client_key { url.getQueryParam("ssl_clientkey") };
    if( ! client_key.empty() )
    {
      if( ! PathInfo(client_key).isFile() || ! client_key.absolute() )
        ZYPP_THROW(media::MediaBadUrlException(url, "Invalid ssl_clientkey file"));
      else
        s.setClientKeyPath( std::move(client_key) );
    }
  }
  {
    std::string param { url.getQueryParam( "proxy" ) };
    if ( ! param.empty() )
    {
      if ( param == EXPLICITLY_NO_PROXY ) {
        // Workaround TransferSettings shortcoming: With an
        // empty proxy string, code will continue to look for
        // valid proxy settings. So set proxy to some non-empty
        // string, to indicate it has been explicitly disabled.
        s.setProxy(EXPLICITLY_NO_PROXY);
        s.setProxyEnabled(false);
      }
      else {
        const std::string & proxyport { url.getQueryParam( "proxyport" ) };
        if ( ! proxyport.empty() ) {
          param += ":";
          param += proxyport;
        }
        s.setProxy( std::move(param) );
        s.setProxyEnabled( true );
      }
    }
  }
  {
    std::string param { url.getQueryParam( "proxyuser" ) };
    if ( ! param.empty() )
    {
      s.setProxyUsername( std::move(param) );
      s.setProxyPassword( url.getQueryParam( "proxypass" ) );
    }
  }
  {
    // HTTP authentication type
    std::string param { url.getQueryParam("auth") };
    if ( ! param.empty() && (url.getScheme() == "http" || url.getScheme() == "https") )
    {
      try
      {
        media::CurlAuthData::auth_type_str2long (param );	// check if we know it
      }
      catch ( const media::MediaException & ex_r )
      {
        DBG << "Rethrowing as MediaUnauthorizedException.";
        ZYPP_THROW(media::MediaUnauthorizedException(url, ex_r.msg(), "", ""));
      }
      s.setAuthType( std::move(param) );
    }
  }
  {
    // workarounds
    const std::string & param { url.getQueryParam("head_requests") };
    if( ! param.empty() && param == "no" )
      s.setHeadRequestsAllowed( false );
  }
}

/**
 * Reads the system proxy configuration and fills the settings
 * structure proxy information
 */
void fillSettingsSystemProxy( const Url& url, media::TransferSettings &s )
{
  media::ProxyInfo proxy_info;
  if ( proxy_info.useProxyFor( url ) )
  {
    // We must extract any 'user:pass' from the proxy url
    // otherwise they won't make it into curl (.curlrc wins).
    try {
      Url u( proxy_info.proxy( url ) );
      s.setProxy( u.asString( url::ViewOption::WITH_SCHEME + url::ViewOption::WITH_HOST + url::ViewOption::WITH_PORT ) );
      // don't overwrite explicit auth settings
      if ( s.proxyUsername().empty() )
      {
        s.setProxyUsername( u.getUsername( url::E_ENCODED ) );
        s.setProxyPassword( u.getPassword( url::E_ENCODED ) );
      }
      s.setProxyEnabled( true );
    }
    catch (...) {}	// no proxy if URL is malformed
  }
}


int env::getZYPP_MEDIA_CURL_IPRESOLVE()
{
  int ret = 0;
  if ( const char * envp = getenv( "ZYPP_MEDIA_CURL_IPRESOLVE" ) )
  {
    WAR << "env set: $ZYPP_MEDIA_CURL_IPRESOLVE='" << envp << "'" << std::endl;
    if (      strcmp( envp, "4" ) == 0 )	ret = 4;
    else if ( strcmp( envp, "6" ) == 0 )	ret = 6;
  }
  return ret;
}


const char * anonymousIdHeader()
{
  // we need to add the release and identifier to the
  // agent string.
  // The target could be not initialized, and then this information
  // is guessed.
  static const std::string _value(
    str::trim( str::form(
      "X-ZYpp-AnonymousId: %s",
      Target::anonymousUniqueId( Pathname()/*guess root*/ ).c_str() ) )
    );
  return _value.c_str();
}

const char * distributionFlavorHeader()
{
  // we need to add the release and identifier to the
  // agent string.
  // The target could be not initialized, and then this information
  // is guessed.
  static const std::string _value(
    str::trim( str::form(
      "X-ZYpp-DistributionFlavor: %s",
      Target::distributionFlavor( Pathname()/*guess root*/ ).c_str() ) )
    );
  return _value.c_str();
}

const char * agentString()
{
  // we need to add the release and identifier to the
  // agent string.
  // The target could be not initialized, and then this information
  // is guessed.
  static const std::string _value(
    str::form(
      "ZYpp " LIBZYPP_VERSION_STRING " (curl %s) %s"
      , curl_version_info(CURLVERSION_NOW)->version
      , Target::targetDistribution( Pathname()/*guess root*/ ).c_str()
      )
    );
  return _value.c_str();
}

void curlEscape( std::string & str_r,
  const char char_r, const std::string & escaped_r ) {
  for ( std::string::size_type pos = str_r.find( char_r );
        pos != std::string::npos; pos = str_r.find( char_r, pos ) ) {
    str_r.replace( pos, 1, escaped_r );
  }
}

std::string curlEscapedPath( std::string path_r ) {
  curlEscape( path_r, ' ', "%20" );
  return path_r;
}

std::string curlUnEscape( std::string text_r ) {
  char * tmp = curl_unescape( text_r.c_str(), 0 );
  std::string ret( tmp );
  curl_free( tmp );
  return ret;
}

Url clearQueryString(const Url &url)
{
  Url curlUrl (url);
  curlUrl.setUsername( "" );
  curlUrl.setPassword( "" );
  curlUrl.setPathParams( "" );
  curlUrl.setFragment( "" );
  curlUrl.delQueryParam("cookies");
  curlUrl.delQueryParam("proxy");
  curlUrl.delQueryParam("proxyport");
  curlUrl.delQueryParam("proxyuser");
  curlUrl.delQueryParam("proxypass");
  curlUrl.delQueryParam("ssl_capath");
  curlUrl.delQueryParam("ssl_verify");
  curlUrl.delQueryParam("ssl_clientcert");
  curlUrl.delQueryParam("timeout");
  curlUrl.delQueryParam("auth");
  curlUrl.delQueryParam("username");
  curlUrl.delQueryParam("password");
  curlUrl.delQueryParam("mediahandler");
  curlUrl.delQueryParam("credentials");
  curlUrl.delQueryParam("head_requests");
  return curlUrl;
}

// bsc#933839: propagate proxy settings passed in the repo URL
zypp::Url propagateQueryParams( zypp::Url url_r, const zypp::Url & template_r )
{
  for ( std::string param : { "proxy", "proxyport", "proxyuser", "proxypass"} )
  {
    const std::string & value( template_r.getQueryParam( param ) );
    if ( ! value.empty() )
      url_r.setQueryParam( param, value );
  }
  return url_r;
}

}
