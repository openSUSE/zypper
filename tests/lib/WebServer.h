
#ifndef ZYPP_WEBSERVER_H
#define ZYPP_WEBSERVER_H

#include "zypp/Pathname.h"

struct shttpd_ctx;

class WebServer
{
 public:
  WebServer(const zypp::Pathname root, unsigned int port);
  ~WebServer();
  void start();
  
  void operator()();


 private:
   struct shttpd_ctx *_ctx;
   zypp::Pathname _docroot;
   unsigned int _port;
};

#endif
