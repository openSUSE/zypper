
#ifndef ZYPP_WEBSERVER_H
#define ZYPP_WEBSERVER_H

#include "boost/thread.hpp"
#include "boost/smart_ptr.hpp"

#include "zypp/Pathname.h"
#include "zypp/base/PtrTypes.h"

struct shttpd_ctx;

class WebServer
{
 public:
  WebServer(const zypp::Pathname root, unsigned int port=9099);
  ~WebServer();
  void start();

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
