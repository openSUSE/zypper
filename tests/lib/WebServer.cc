
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

#include "WebServer.h"

WebServer::WebServer(const Pathname root, unsigned int port)
    : _docroot(root), _port(port)
{
    _ctx = shttpd_init(0, NULL);
    shttpd_set_option(_ctx, "root", root.c_str());
    shttpd_set_option(_ctx, "ports", str::numstring(port).c_str());

}

void WebServer::operator()()
{
    for (;;)
        shttpd_poll(_ctx, 1000);
}

WebServer::~WebServer()
{
    shttpd_fini(_ctx);
}


