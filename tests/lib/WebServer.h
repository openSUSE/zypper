
#ifndef ZYPP_WEBSERVER_H
#define ZYPP_WEBSERVER_H

#include "boost/thread.hpp"
#include "boost/smart_ptr.hpp"

#include "zypp/Pathname.h"
#include "zypp/base/PtrTypes.h"

struct shttpd_ctx;

/**
 *
 * Starts a webserver to simulate remote transfers in
 * testcases
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
  /**
   * creates a web server on \ref root and \port
   */
  WebServer(const zypp::Pathname root, unsigned int port=9099);
  ~WebServer();
  /**
   * Starts the webserver worker thread
   */
  void start();
  /**
   * Stops the worker thread
   */
  void stop();
 
 private:

  void setStrOption( std::string name, std::string value);
  void setNumOption( std::string name, int value);

  void worker_thread();
   struct shttpd_ctx *_ctx;
   zypp::Pathname _docroot;
   unsigned int _port;
   zypp::shared_ptr<boost::thread> _thrd;
   bool _stop;
};

#endif
