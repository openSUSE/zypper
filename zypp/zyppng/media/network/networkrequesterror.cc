#include <zypp/zyppng/media/network/private/networkrequesterror_p.h>
#include <zypp/zyppng/media/network/request.h>
#include <zypp/media/CurlHelper.h>
#include <zypp/base/Gettext.h>
#include <zypp/media/MediaUserAuth.h>
#include <curl/curl.h>

namespace zyppng {

ZYPP_IMPL_PRIVATE(NetworkRequestError);

NetworkRequestErrorPrivate::NetworkRequestErrorPrivate(NetworkRequestError::Type code, std::string &&msg, std::map<std::string, boost::any> &&extraInfo)
  : _errorCode(code)
  , _errorMessage( std::move(msg) )
, _extraInfo( std::move(extraInfo) )
{ }

NetworkRequestErrorPrivate *NetworkRequestErrorPrivate::clone() const
{
  return new NetworkRequestErrorPrivate( *this );
}

NetworkRequestError NetworkRequestErrorPrivate::customError( NetworkRequestError::Type t, std::string &&errorMsg, std::map<std::string, boost::any> &&extraInfo )
{
  return NetworkRequestError( *new NetworkRequestErrorPrivate(t, errorMsg.empty() ? typeToString(t) : std::move(errorMsg), std::move(extraInfo)) );
}

NetworkRequestError NetworkRequestErrorPrivate::fromCurlError(NetworkRequest &req, int nativeCode , const std::string &nativeError )
{

  Url url = req.url();
  NetworkRequestError::Type c = NetworkRequestError::NoError;
  std::string err;
  std::map<std::string, boost::any> extraInfo;

  if ( nativeCode != 0 ) {

    const char *nativeErr = curl_easy_strerror( static_cast<CURLcode>(nativeCode) );
    if ( nativeErr != nullptr )
      extraInfo.insert( { "nativeErrorCodeDesc",  std::string( nativeErr ) } );

    if ( !nativeError.empty() )
      extraInfo.insert( { "nativeErrorDesc",  nativeError } );

    extraInfo.insert( { "requestUrl", url } );
    extraInfo.insert( { "curlCode", nativeCode } );
    extraInfo.insert( { "filepath", req.targetFilePath().asString() } );
    extraInfo.insert( { "lastRedirect", req.lastRedirectInfo() } );

    switch ( nativeCode )
    {
      case CURLE_UNSUPPORTED_PROTOCOL:
        c = NetworkRequestError::UnsupportedProtocol;
        err = typeToString( c );
        if ( !req.lastRedirectInfo().empty() )
        {
          err += " or redirect (";
          err += req.lastRedirectInfo();
          err += ")";
        }
        break;
      case CURLE_URL_MALFORMAT: case CURLE_URL_MALFORMAT_USER:
        c = NetworkRequestError::MalformedURL;
        break;
      case CURLE_LOGIN_DENIED:
        c = NetworkRequestError::Unauthorized;
        break;
      case CURLE_HTTP_RETURNED_ERROR: {
        long httpReturnCode = 0;
        CURLcode infoRet = curl_easy_getinfo( req.nativeHandle(),
          CURLINFO_RESPONSE_CODE,
          &httpReturnCode );

        if ( infoRet == CURLE_OK ) {
          extraInfo.insert( { "responseCode", httpReturnCode } );

          std::string msg = "HTTP response: " + zypp::str::numstring( httpReturnCode );
          switch ( httpReturnCode )
          {
            case 401: {
              std::string auth_hint;
              {
                long auth_info = CURLAUTH_NONE;

                CURLcode infoRet =
                  curl_easy_getinfo(req.nativeHandle(), CURLINFO_HTTPAUTH_AVAIL, &auth_info);

                if(infoRet == CURLE_OK) {
                  extraInfo.insert( { "authHint", zypp::media::CurlAuthData::auth_type_long2str(auth_info) } );
                }
              }

              //if there is already a user:password entry in the settings the auth simply failed
              //@TODO write a testcase for this
              if ( !req.transferSettings().userPassword().empty() ) {
                c = NetworkRequestError::AuthFailed;
              } else {
                c = NetworkRequestError::Unauthorized;
              }

              break;
            }

            case 502: // bad gateway (bnc #1070851)
            case 503: // service temporarily unavailable (bnc #462545)
              c = NetworkRequestError::TemporaryProblem;
              err = zypp::str::form( _("Location '%s' is temporarily unaccessible."), url.asString().c_str() );
              break;
            case 504: // gateway timeout
              c = NetworkRequestError::Timeout;
              err = zypp::str::form(_("Timeout exceeded when accessing '%s'."), url.asString().c_str() );
              break;
            case 403: {
              std::string msg403;
              if ( url.getHost().find(".suse.com") != std::string::npos )
                msg403 = _("Visit the SUSE Customer Center to check whether your registration is valid and has not expired.");
              else if (url.asString().find("novell.com") != std::string::npos)
                msg403 = _("Visit the Novell Customer Center to check whether your registration is valid and has not expired.");

              c = NetworkRequestError::Forbidden;
              err = msg403;
              break;
            }
            case 404:
            case 410:
              c = NetworkRequestError::NotFound;
              err = zypp::str::form( _("File '%s' not found on medium '%s'"), req.targetFilePath().c_str(), url.asString().c_str() );
              break;

            default:
              c = NetworkRequestError::ServerReturnedError;
              err = zypp::str::form(_("Download (curl) error for '%s':\n"
                                        "Error code: %s\n"), url.asString().c_str(), zypp::str::numstring( httpReturnCode ).c_str() ) ;
              break;
          }
        } else {
          c = NetworkRequestError::ServerReturnedError;
          err = zypp::str::form(_("Download (curl) error for '%s':\n"
                                    "Unable to retrieve HTTP response\n"), url.asString().c_str() ) ;
        }
      }
      break;
      case CURLE_FTP_COULDNT_RETR_FILE:
#if CURLVERSION_AT_LEAST(7,16,0)
      case CURLE_REMOTE_FILE_NOT_FOUND:
#endif
      case CURLE_FTP_ACCESS_DENIED:
      case CURLE_TFTP_NOTFOUND:
        c = NetworkRequestError::NotFound;
        break;
      case CURLE_BAD_PASSWORD_ENTERED:
      case CURLE_FTP_USER_PASSWORD_INCORRECT:
        c = NetworkRequestError::AuthFailed;
        break;
      case CURLE_COULDNT_RESOLVE_PROXY:
      case CURLE_COULDNT_RESOLVE_HOST:
      case CURLE_COULDNT_CONNECT:
      case CURLE_FTP_CANT_GET_HOST:
        c = NetworkRequestError::ConnectionFailed;
        break;
      case CURLE_WRITE_ERROR:
        c = NetworkRequestError::InternalError;
        break;
      case CURLE_PARTIAL_FILE:
      case CURLE_OPERATION_TIMEDOUT:
        c = NetworkRequestError::Timeout;
        break;
      case CURLE_ABORTED_BY_CALLBACK:
        c = NetworkRequestError::Cancelled;
        break;
      case CURLE_PEER_FAILED_VERIFICATION:
        c = NetworkRequestError::PeerCertificateInvalid;
        break;
      default:
        c = NetworkRequestError::ServerReturnedError;
        err = "Curl error " + zypp::str::numstring( nativeCode );
        break;
    }
  }

  if ( err.empty() )
    err = typeToString( c );

  return NetworkRequestError( *new NetworkRequestErrorPrivate(c, std::move(err), std::move(extraInfo)) );
}

NetworkRequestError NetworkRequestErrorPrivate::fromCurlMError( int nativeCode )
{
  const char *nativeErr = curl_multi_strerror( static_cast<CURLMcode>(nativeCode) );

  std::map<std::string, boost::any> extraInfo;
  extraInfo.insert( { "curlMCode", nativeCode } );

  std::string err;
  if ( nativeErr == nullptr )
    err = "The dispatcher returned an unknown error";
  else
    err = std::string( nativeErr );

  return NetworkRequestError( *new NetworkRequestErrorPrivate(NetworkRequestError::InternalError, std::move(err), std::move(extraInfo)) );
}


NetworkRequestError::NetworkRequestError(zyppng::NetworkRequestErrorPrivate &d )
  : d_ptr( &d )
{ }

NetworkRequestError::NetworkRequestError()
  : d_ptr( new NetworkRequestErrorPrivate( NoError, {}, {} ) )
{ }

NetworkRequestError::Type NetworkRequestError::type() const
{
  return d_func()->_errorCode;
}

std::string NetworkRequestError::toString() const
{
  return d_func()->_errorMessage;
}

bool NetworkRequestError::isError() const
{
  return d_func()->_errorCode != NoError;
}

const std::map<std::string, boost::any> &NetworkRequestError::extraInfo() const
{
  return d_func()->_extraInfo;
}

std::string NetworkRequestErrorPrivate::typeToString( NetworkRequestError::Type t )
{
  switch ( t ) {
    case NetworkRequestError::NoError:
      return "No error";
    case NetworkRequestError::InternalError:
      return "Internal Error";
    case NetworkRequestError::Cancelled:
      return "The request was cancelled";
    case NetworkRequestError::ExceededMaxLen:
      return "The request exceeded the maximum download size";
    case NetworkRequestError::InvalidChecksum:
      return "The downloaded data did not result in a valid checksum";
    case NetworkRequestError::PeerCertificateInvalid:
      return "The peer certificate could not be verified";
    case NetworkRequestError::ConnectionFailed:
      return "Connection failed";
    case NetworkRequestError::UnsupportedProtocol:
      return "Unsupported protocol";
    case NetworkRequestError::MalformedURL:
      return "Bad URL";
    case NetworkRequestError::TemporaryProblem:
      return "Requested location is temporarily unaccessible.";
    case NetworkRequestError::Timeout:
      return "Timeout reached";
    case NetworkRequestError::Forbidden:
      return "Access to requested URL is forbidden.";
    case NetworkRequestError::NotFound:
      return "File not found";
    case NetworkRequestError::Unauthorized:
      return "Authentication required but not provided.";
    case NetworkRequestError::AuthFailed:
      return "Login failed.";
    case NetworkRequestError::ServerReturnedError:
      return "Server returned an error for the given request.";
    case NetworkRequestError::MissingData:
      return "Server did not send all requested ranges.";
  }
  return std::string();
}

std::string NetworkRequestError::nativeErrorString() const
{
  Z_D();

  auto it = d->_extraInfo.find("nativeErrorDesc");
  if ( it != d->_extraInfo.end() ) {
    try {
      return boost::any_cast<std::string>( it->second );
    } catch ( const boost::bad_any_cast &) { }
  }

  it = d->_extraInfo.find("nativeErrorCodeDesc");
  if ( it != d->_extraInfo.end() ) {
    try {
      return boost::any_cast<std::string>( it->second );
    } catch ( const boost::bad_any_cast &) { }
  }

  return std::string();
}

}
