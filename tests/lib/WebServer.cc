
#include <sstream>
#include <string>
#include "boost/bind.hpp"
#include "boost/thread.hpp"
#include "boost/version.hpp"

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Exception.h"
#include "zypp/ExternalProgram.h"
#include "WebServer.h"

#include "mongoose.h"

using namespace zypp;
using namespace std;

#if ( BOOST_VERSION >= 105000 )
// https://svn.boost.org/trac/boost/ticket/7085
namespace boost
{
  namespace system
  {
    class fake_error_category : public error_category
    {
      virtual const char *     name() const noexcept(true)
      { return "falke_name"; }
      virtual std::string      message( int ev ) const
      { return "falke_message"; }
    };
    const error_category &  generic_category()
    {
      static fake_error_category _e;
      return _e;
      throw std::exception(/*"boost/ticket/7085 workaound sucks :("*/);
    }
    const error_category &  system_category()
    {
      static fake_error_category _e;
      return _e;
      throw std::exception(/*"boost/ticket/7085 workaound sucks :("*/);
    }
  }
}
#endif

static inline string hostname()
{
    static char buf[256];
    string result;
    if (!::gethostname(buf, 255))
        result += string(buf);
    else
        return "localhost";
    return result;
}

#define WEBRICK 0

class WebServer::Impl
{
public:
    Impl()
    {}

    virtual ~Impl()
    {}

    virtual string log() const
    { return string(); }

    virtual void start()
    {}

    virtual void stop()
    {}

    virtual void worker_thread()
    {}

    virtual int port() const
    {
        return 0;
    }



private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
};

class WebServerWebrickImpl : public WebServer::Impl
{
public:
    WebServerWebrickImpl(const Pathname &root, unsigned int port)
        : _docroot(root), _port(port), _stop(false), _stopped(true)
    {
    }

    ~WebServerWebrickImpl()
    {
        if ( ! _stopped )
            stop();
    }

    virtual int port() const
    {
        return _port;
    }


    virtual void worker_thread()
    {
        _log.clear();

        stringstream strlog(_log);

        string webrick_code = str::form("require \"webrick\"; s = WEBrick::HTTPServer.new(:Port => %d, :DocumentRoot    => \"%s\"); trap(\"INT\"){ s.shutdown }; trap(\"SIGKILL\") { s.shutdown }; s.start;", _port, _docroot.c_str());

        const char* argv[] =
        {
            "/usr/bin/ruby",
            "-e",
            webrick_code.c_str(),
            NULL
        };

        ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
        string line;

        _stopped = false;

        while ( ! _stop );

        MIL << "Thread end requested" << endl;
        //prog.close();
        if ( prog.running() )
            prog.kill();
        MIL << "Thread about to finish" << endl;
    }

    virtual string log() const
    {
        return _log;
    }

    virtual void stop()
    {
        MIL << "Waiting for Webrick thread to finish" << endl;
        _stop = true;
        _thrd->join();
        MIL << "Webrick thread finished" << endl;
        _thrd.reset();
        _stopped = true;
    }

    virtual void start()
    {
        //_thrd.reset( new boost::thread( boost::bind(&WebServerWebrickImpl::worker_thread, this) ) );
    }

    zypp::Pathname _docroot;
    unsigned int _port;
    zypp::shared_ptr<boost::thread> _thrd;
    bool _stop;
    bool _stopped;
    std::string _log;
};

class WebServerMongooseImpl : public WebServer::Impl
{
public:
    WebServerMongooseImpl(const Pathname &root, unsigned int port)
        : _ctx(0L), _docroot(root)
        , _port(port)
        , _stopped(true)
    {
    }

    ~WebServerMongooseImpl()
    {
        MIL << "Destroying web server" << endl;

        if ( ! _stopped )
            stop();
    }

    virtual void start()
    {
        if ( ! _stopped )
        {
            MIL << "mongoose server already running, stopping." << endl;
            stop();
        }

        MIL << "Starting shttpd (mongoose)" << endl;
        _log.clear();
        _ctx = mg_start();

        int ret = 0;
        ret = mg_set_option(_ctx, "ports", str::form("%d", _port).c_str());
        if (  ret != 1 )
            ZYPP_THROW(Exception(str::form("Failed to set port: %d", ret)));

        MIL << "Setting root directory to : '" << _docroot << "'" << endl;
        ret = mg_set_option(_ctx, "root", _docroot.c_str());
        if (  ret != 1 )
            ZYPP_THROW(Exception(str::form("Failed to set docroot: %d", ret)));

        _stopped = false;
    }

    virtual int port() const
    {
        return _port;
    }


    virtual string log() const
    {
        return _log;
    }

    virtual void stop()
    {
        MIL << "Stopping shttpd" << endl;
        mg_stop(_ctx);
        MIL << "shttpd finished" << endl;
        _ctx = 0;
        _stopped = true;
    }

    mg_context *_ctx;
    zypp::Pathname _docroot;
    unsigned int _port;
    bool _stopped;
    std::string _log;
};


WebServer::WebServer(const Pathname &root, unsigned int port)
#if WEBRICK
    : _pimpl(new WebServerWebrickImpl(root, port))
#else
    : _pimpl(new WebServerMongooseImpl(root, port))
#endif
{
}

void WebServer::start()
{
    _pimpl->start();
}


std::string WebServer::log() const
{
    return _pimpl->log();
}

int WebServer::port() const
{
    return _pimpl->port();
}


Url WebServer::url() const
{
    Url url;
    url.setHost(hostname());
    url.setPort(str::numstring(port()));
    url.setScheme("http");
    return url;
}

void WebServer::stop()
{
    _pimpl->stop();
}

WebServer::~WebServer()
{
}


