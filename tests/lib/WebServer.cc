
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "zypp/ZYpp.h"

using namespace zypp;

#include "shttpd.h"
#include "boost/bind.hpp"
#include "WebServer.h"

#define LISTENING_PORT "8080"

static shttpd_ctx * do_init_ctx()
{
    static const char *argv[] = {"a", "-ports", LISTENING_PORT, NULL};
    static int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    shttpd_ctx *ctx;
    
    ctx = shttpd_init(argc, const_cast<char**>(argv));
    return ctx;
}

WebServer::WebServer(const Pathname root, unsigned int port)
    : _docroot(root), _port(port), _stop(false)
{
    // init with a user port
    _ctx = do_init_ctx();    
    setStrOption("root", root.asString());
    setNumOption("ports", port);
}

void WebServer::start()
{
//    _thrd.reset( new boost::thread(web_thread(_ctx)) );
    _thrd.reset( new boost::thread( boost::bind(&WebServer::worker_thread, this) ) );
}

void WebServer::worker_thread()
{
    while( ! _stop )
        shttpd_poll(_ctx, 1000);
}


void WebServer::setStrOption( std::string name, std::string value)
{
    if ( ! shttpd_set_option(_ctx, name.c_str(),
                             value.c_str() ) )
        ZYPP_THROW(Exception("bad option"));    
}

void WebServer::setNumOption( std::string name, int value)
{
    if ( ! shttpd_set_option(_ctx, name.c_str(),
                             str::numstring(value).c_str()) )
        ZYPP_THROW(Exception("bad ioption"));    
}

void WebServer::stop()
{
    _stop = true;
    _thrd->join();
    _thrd.reset();
}

WebServer::~WebServer()
{
    shttpd_fini(_ctx);
}


