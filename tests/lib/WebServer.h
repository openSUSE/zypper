
#ifndef ZYPP_TEST_WEBSERVER_H
#define ZYPP_TEST_WEBSERVER_H

#include <zypp/Url.h>
#include <zypp/Pathname.h>
#include <zypp/base/PtrTypes.h>
#include <zypp-curl/TransferSettings>

#include <functional>

/**
 *
 * Starts a webserver to simulate remote transfers in
 * testcases
 * \author Duncan Mac-Vicar P. <dmacvicar@suse.de>
 *
 * \code
 * #include "WebServer.h"
 *
 * BOOST_AUTO_TEST_CASE(Foo)
 * {
 *
 *      WebServer web((Pathname(TESTS_SRC_DIR) + "/datadir").c_str() );
 *      web.start();
 *
 *     MediaSetAccess media( Url("http://localhost:9099"), "/" );
 *
 *     // do something with the url
 *
 *
 *     web.stop();
 *
 * \endcode
 */
class WebServer
{
 public:

   struct Request {
     Request ( std::streambuf *rinbuf,
               std::streambuf *routbuf,
               std::streambuf *rerrbuf );
     std::string path;
     std::map< std::string, std::string > params;
     std::istream rin;
     std::ostream rout;
     std::ostream rerr;
   };

   using RequestHandler = std::function<void ( Request &req )>;

  /**
   * creates a web server on \ref root and \port
   */
  WebServer(const zypp::Pathname &root, unsigned int port=10001, bool useSSL = false );
  ~WebServer();
  /**
   * Starts the webserver worker thread
   * Waits up to 10 seconds and returns whether the port is now active.
   */
  bool start();

  /**
   * Stops the worker thread
   */
  void stop();

  /**
   * returns the port we are listening to
   */
  int port() const;

  /**
   * returns the base url where the webserver is listening
   */
  zypp::Url url() const;

  /**
   * generates required transfer settings
   */
  zypp::media::TransferSettings transferSettings () const;

  /**
   * shows the log of last run
   */
  std::string log() const;

  /**
   * Checks if the server was stopped
   */
  bool isStopped() const;

  /**
   * Sets the request handler callback, if a callback for the path does already exist
   * it is replaced, can be called at any time
   *
   * \note a request handler path is always prefixed with /handler so to call a handler named test, the
   *       URL is "http://localhost/handler/test"
   */
  void addRequestHandler ( const std::string &path, RequestHandler &&handler );

  /*!
   * Removes a registered request hander, can be called at any time
   */
  void removeRequestHandler ( const std::string &path );

  /*!
   * Creates a request handler that simply returns the string in \a resp
   */
  static RequestHandler makeResponse(std::string resp);

  /*!
   * Creates a request handler that sends a HTTP response with \a status and \a content
   * in the respose
   */
  static RequestHandler makeResponse(std::string status, std::string content);

  /*!
   * Creates a request handler that sends a HTTP response with \a status \a content
   * and \a headers in the respose
   */
  static RequestHandler makeResponse(const std::string &status, const std::vector<std::string> &headers, const std::string &content);

  /*!
   * Creates the corresponding HTTP response to the given parameters
   */
  static std::string makeResponseString ( const std::string &status, const std::vector<std::string> &headers, const std::string &content );

  class Impl;
private:
  /** Pointer to implementation */
  zypp::RW_pointer<Impl> _pimpl;
};

#endif
