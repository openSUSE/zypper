
//#include "zypp/ZYpp.h"

#include <sstream>
#include <string>
#include "boost/bind.hpp"

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Exception.h"
#include "zypp/ExternalProgram.h"
#include "WebServer.h"

#include "mongoose.h"

using namespace zypp;
using namespace std;

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
    
    virtual void worker_thread()
    {
        _log.clear();
            
        stringstream strlog(_log);

        string webrick_code = str::form("require 'webrick'; s = WEBrick::HTTPServer.new(:Port => %d, :DocumentRoot    => '%s'); trap('INT'){ s.shutdown }; trap('SIGKILL') { s.shutdown }; s.start;", _port, _docroot.c_str());
    
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
    
        while ( _stop );
        
        /*
        for(line = prog.receiveLine();
            !line.empty();
            line = prog.receiveLine() )
        {
            strlog << line;
            if ( _stop )
                break;
        }
        */
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
        _thrd.reset( new boost::thread( boost::bind(&WebServerWebrickImpl::worker_thread, this) ) );
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
        , _port(port), _stop(false)
        , _stopped(true)
    {
    }
    
    ~WebServerMongooseImpl()
    {
        if ( ! _stopped )
            stop();
    }
    
    virtual void worker_thread()
    {
        _log.clear();
        _ctx = mg_start();
        if ( ! mg_set_option(_ctx, "ports", str::form("%d", _port).c_str()) )
            ZYPP_THROW(Exception("Failed to set port"));
        
        mg_set_option(_ctx, "root", _docroot.c_str());
        stringstream strlog(_log);
        
        while ( !_stop )
        {
            // loop
        }
        MIL << "Thread end requested, shutting down shttpd" << endl;
        mg_stop(_ctx);
        _ctx = 0;
    }
    
    virtual string log() const
    {
        return _log;
    }
    
    virtual void stop()
    {
        MIL << "Waiting for shttpd thread to finish" << endl;
        _stop = true;
        _thrd->join();
        MIL << "shttpd thread to finished" << endl;
        _thrd.reset();
        _stopped = true;
    }
    
    virtual void start()
    {
        MIL << "Starting shttpd (mongoose) thread" << endl;
        _thrd.reset( new boost::thread( boost::bind(&WebServerMongooseImpl::worker_thread, this) ) );
    }
    
    mg_context *_ctx;
    zypp::Pathname _docroot;
    unsigned int _port;
    zypp::shared_ptr<boost::thread> _thrd;
    bool _stop;
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


void WebServer::stop()
{
    _pimpl->stop();
}

WebServer::~WebServer()
{
}


