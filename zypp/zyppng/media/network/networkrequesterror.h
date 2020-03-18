#ifndef ZYPP_NG_MEDIA_CURL_NETWORK_REQUEST_ERROR_H_INCLUDED
#define ZYPP_NG_MEDIA_CURL_NETWORK_REQUEST_ERROR_H_INCLUDED

#include <zypp/zyppng/base/zyppglobal.h>
#include <zypp/base/PtrTypes.h>
#include <boost/any.hpp>
#include <string>
#include <map>

namespace zyppng {

class NetworkRequest;
class NetworkRequestErrorPrivate;

  /**
   * @brief The NetworkRequestError class
   * Represents a error that occured in \see NetworkDownloadRequest
   * or \see NetworkRequestDispatcher
   */
  class LIBZYPP_NG_EXPORT NetworkRequestError
  {
  public:
    enum Type {
      NoError = 0,
      InternalError,     //< A error in the dispatcher that is not caused by the backend, check the error string
      Cancelled,         //< The request was cancelled
      PeerCertificateInvalid, //< the peer certificate validation failed
      ConnectionFailed,       //< connecting to the server failed
      ExceededMaxLen,         //< the downloaded data exceeded the requested maximum lenght
      InvalidChecksum,        //< The downloaded data has a different checksum than expected
      UnsupportedProtocol,    //< The protocol given in the URL scheme is not supported by the backend
      MalformedURL,           //< The given URL is malformed
      TemporaryProblem,       //< There was a temporary problem with the server side
      Timeout,                //< The request timed out
      Forbidden,              //< Accessing the requested ressource on the server was forbidden
      NotFound,               //< The requested path in the URL does not exist on the server
      Unauthorized,        //<< No auth data given but authorization required
      AuthFailed,          //<< Auth data was given, but authorization failed
      ServerReturnedError, //<< A error was returned by the server that is not explicitely handled
      MissingData          //<< The download was a multirange download and we did not get all data that was requested, if that is returned some ranges might have been downloaded successful
    };

    NetworkRequestError ();

    /**
     * @brief type
     * Returns the type of the error
     */
    Type type () const;

    /**
     * @brief toString
     * Returns a string representation of the error
     */
    std::string toString () const;

    /**
     * @brief isError
     * Will return true if this is a actual error
     */
    bool isError () const;

    /*!
     * Tries to find \a key in the extraInfo map, if the key is not found
     * or the value can not be converted to the requested type \a defaultVal is returned.
     */
    template<typename T>
    T extraInfoValue ( const std::string &key, T &&defaultVal = T()  ) const {
      auto &t = extraInfo();
      auto it = t.find(key);
      if ( it != t.end() ) {
        try {
          return boost::any_cast<T>( it->second );
        } catch ( const boost::bad_any_cast &) { }
      }
      return defaultVal;
    }

    /*!
     * Returns the error extraInfo map.
     */
    const std::map<std::string, boost::any> &extraInfo () const;

    /*!
     * Returns the string returned by the backend if available.
     */
    std::string nativeErrorString() const;

  protected:
    NetworkRequestError( NetworkRequestErrorPrivate &d );

  private:
    ZYPP_FWD_DECLARE_PRIVATE(NetworkRequestError)
    zypp::RWCOW_pointer<NetworkRequestErrorPrivate> d_ptr;
  };
}

#endif
