
#ifndef ZYPP_TEST_WEBSERVER_H
#define ZYPP_TEST_WEBSERVER_H

#include "zypp/Url.h"
#include "zypp/Pathname.h"
#include "zypp/base/PtrTypes.h"

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
  /**
   * creates a web server on \ref root and \port
   */
  WebServer(const zypp::Pathname &root, unsigned int port=10001);
  ~WebServer();
  /**
   * Starts the webserver worker thread
   */
  void start();
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
   * shows the log of last run
   */
  std::string log() const;

  class Impl;
private:
  /** Pointer to implementation */
  zypp::RWCOW_pointer<Impl> _pimpl;
};

#endif
