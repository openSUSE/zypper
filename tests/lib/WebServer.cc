#include <sstream>
#include <string>
#include <stdio.h>
#include <poll.h>
#include <signal.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/ExternalProgram.h>
#include <zypp/TmpPath.h>
#include <zypp/PathInfo.h>
#include <zypp/zyppng/base/private/linuxhelpers_p.h>

#include "WebServer.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <fastcgi/fcgiapp.h>
#include <fastcgi/fcgio.h>
#include <iostream>
#include <fstream>

#ifndef TESTS_SRC_DIR
#error "TESTS_SRC_DIR not defined"
#endif
#ifndef WEBSRV_BINARY
#error "WEBSRV_BINARY not defined"
#endif

using std::endl;
using namespace zypp;

namespace  {

  /** ExternalProgram extended to kill the child process if the parent thread exits
     */
  class ZYPP_LOCAL ExternalTrackedProgram : public ExternalProgram
  {
  public:
    ExternalTrackedProgram (const char *const *argv, const Environment & environment,
      Stderr_Disposition stderr_disp = Normal_Stderr,
      bool use_pty = false, int stderr_fd = -1, bool default_locale = false,
      const Pathname& root = "") : ExternalProgram()
    {
      start_program( argv, environment, stderr_disp, stderr_fd, default_locale, root.c_str(), false, true );
    }

  };

  bool checkLocalPort( int portno_r )
  {
    bool ret = false;

    AutoDispose<int> sockfd { socket( AF_INET, SOCK_STREAM, 0 ) };
    if ( sockfd < 0 ) {
        std::cerr << "ERROR opening socket" << endl;
	return ret;
    }
    sockfd.setDispose( ::close );

    struct hostent * server = gethostbyname( "127.0.0.1" );
    if ( server == nullptr ) {
        std::cerr << "ERROR, no such host 127.0.0.1" << endl;
        return ret;
    }

    struct sockaddr_in serv_addr;
    bzero( &serv_addr, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    bcopy( server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length );
    serv_addr.sin_port = htons(portno_r);

    return( connect( sockfd, (const sockaddr*)&serv_addr, sizeof(serv_addr) ) == 0 );
  }
}


static inline std::string hostname()
{
    static char buf[256];
    std::string result;
    if (!::gethostname(buf, 255))
        result += std::string(buf);
    else
        return "localhost";
    return result;
}

static inline std::string handlerPrefix()
{
  static std::string pref("/handler/");
  return pref;
}

WebServer::Request::Request( std::istream::__streambuf_type *rinbuf, std::ostream::__streambuf_type *routbuf, std::ostream::__streambuf_type *rerrbuf )
  : rin  ( rinbuf )
  , rout ( routbuf )
  , rerr ( rerrbuf )
{ }

class WebServer::Impl
{
public:
    Impl(const Pathname &root, unsigned int port, bool ssl)
      : _docroot(root), _port(port), _stop(false), _stopped(true), _ssl( ssl )
    {
      FCGX_Init();

      // wake up pipe to interrupt poll()
      pipe ( _wakeupPipe );
      fcntl( _wakeupPipe[0], F_SETFL, O_NONBLOCK );

      MIL << "Working dir is " << _workingDir << endl;

      filesystem::assert_dir( _workingDir / "log" );
      filesystem::assert_dir( _workingDir / "tmp" );
#ifdef HTTP_USE_LIGHTTPD
      filesystem::assert_dir( _workingDir / "state" );
      filesystem::assert_dir( _workingDir / "home" );
      filesystem::assert_dir( _workingDir / "cache" );
      filesystem::assert_dir( _workingDir / "upload" );
      filesystem::assert_dir( _workingDir / "sockets" );
#endif
    }

    ~Impl()
    {
        if ( ! _stopped )
            stop();

        close (_wakeupPipe[0]);
        close (_wakeupPipe[1]);
    }

    Pathname socketPath () const {
      return ( _workingDir.path() / "fcgi.sock" );
    }

    int port() const
    {
        return _port;
    }

    void worker_thread()
    {
      AutoDispose<int> sockFD( -1, ::close );

      ExternalProgram::Environment env;

#ifdef HTTP_USE_LIGHTTPD
      bool canContinue = true;
      Pathname confDir = Pathname(TESTS_SRC_DIR) / "data" / "lighttpdconf";
      Pathname sslDir  = Pathname(TESTS_SRC_DIR) / "data" / "webconf" / "ssl";
      env["ZYPP_TEST_SRVCONF"] = confDir.asString();

      std::string confFile    = ( confDir / "lighttpd.conf" ).asString();

      {
        std::lock_guard<std::mutex> lock ( _mut );
        sockFD.value() = FCGX_OpenSocket( socketPath().c_str(), 128 );
        env["ZYPP_TEST_SRVROOT"] = _workingDir.path().c_str();
        env["ZYPP_TEST_PORT"] = str::numstring( _port );
        env["ZYPP_TEST_DOCROOT"] = _docroot.c_str();
        env["ZYPP_TEST_SOCKPATH"] = socketPath().c_str();
        env["ZYPP_SSL_CONFDIR"] = sslDir.c_str();
        if ( _ssl )
          env["ZYPP_TEST_USE_SSL"] = "1";
      }

      const char* argv[] =
        {
          "/usr/sbin/lighttpd",
          "-D",
          "-f", confFile.c_str(),
          nullptr
        };
#else

      const auto writeConfFile = []( const Pathname &fileName, const std::string &data ){
        std::ofstream file;
        file.open( fileName.c_str() );
        if ( file.fail() ) {
          std::cerr << "Failed to create file " << fileName << std::endl;
          return false;
        }
        file << data;
        file.close();
        return true;
      };

      const auto confPath = _workingDir.path();
      const auto confFile = confPath/"nginx.conf";
      bool canContinue = true;

      {
        std::lock_guard<std::mutex> lock ( _mut );
        bool canContinue = ( zypp::filesystem::symlink( Pathname(TESTS_SRC_DIR)/"data"/"nginxconf"/"nginx.conf",  confFile ) == 0 );
        if ( canContinue ) canContinue = writeConfFile( confPath / "srvroot.conf", str::Format("root    %1%;") % _docroot );
        if ( canContinue ) canContinue = writeConfFile( confPath / "fcgisock.conf", str::Format("fastcgi_pass unix:%1%;") % socketPath().c_str() );
	if ( canContinue ) canContinue = writeConfFile( confPath / "user.conf", getuid() != 0 ? "" : "user root;" );
        if ( canContinue ) {
          if ( _ssl )
            canContinue = writeConfFile( confPath / "port.conf", str::Format("listen    %1% ssl;") % _port );
          else
            canContinue = writeConfFile( confPath / "port.conf", str::Format("listen    %1%;") % _port );
        }
        if ( canContinue ) {
          if ( _ssl )
            canContinue = ( zypp::filesystem::symlink( Pathname(TESTS_SRC_DIR)/"data"/"nginxconf"/"ssl.conf",  confPath/"ssl.conf" ) == 0 );
          else
            canContinue = ( zypp::filesystem::symlink( Pathname(TESTS_SRC_DIR)/"data"/"nginxconf"/"nossl.conf",  confPath/"ssl.conf" ) == 0);
        }
        if ( canContinue ) canContinue = zypp::filesystem::symlink( Pathname(TESTS_SRC_DIR)/"data"/"nginxconf"/"mime.types",  confPath/"mime.types") == 0;
        if ( canContinue && _ssl ) canContinue = zypp::filesystem::symlink( Pathname(TESTS_SRC_DIR)/"data"/"webconf"/"ssl"/"server.pem",  confPath/"cert.pem") == 0;
        if ( canContinue && _ssl ) canContinue = zypp::filesystem::symlink( Pathname(TESTS_SRC_DIR)/"data"/"webconf"/"ssl"/"server.key",  confPath/"cert.key") == 0;

        if ( canContinue )
          sockFD.value() = FCGX_OpenSocket( socketPath().c_str(), 128 );
      }

      const char* argv[] =
      {
          WEBSRV_BINARY,
          "-p", confPath.c_str(),
          "-c", confFile.c_str(),
          nullptr
      };
#endif

      if ( !canContinue ) {
        _stopped = true;
        _cv.notify_all();
        return;
      }

        ExternalTrackedProgram prog( argv, env, ExternalProgram::Stderr_To_Stdout, false, -1, true);
        prog.setBlocking( false );

        while(1) {
          std::string line = prog.receiveLine();
          if ( line.empty() )
            break;
          if ( line.find_first_of( "nginx: [alert] could not open error log file:") == 0 )
            continue;
          std::cerr << line << endl;
        };

	// Wait max 10 sec for the socket becoming available
	bool isup { checkLocalPort( port() ) };
	if ( !isup )
	{
	  unsigned i = 0;
	  do {
	    std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
	    isup = checkLocalPort( port() );
	  } while ( !isup && ++i < 10 );

	  if ( !isup && prog.running() ) {
	    prog.kill( SIGTERM );
            prog.close();
	  }
	}

        if ( !prog.running() ) {
          _stop = true;
          _stopped = true;
        } else {
          _stopped = false;
        }

        FCGX_Request request;
        FCGX_InitRequest(&request, sockFD,0);
	AutoDispose<FCGX_Request*> guard( &request, boost::bind( &FCGX_Free, _1, 0 ) );

        struct pollfd fds[] { {
            _wakeupPipe[0],
            POLLIN,
            0
          }, {
            sockFD,
            POLLIN,
            0
          }, {
            fileno( prog.inputFile() ),
            POLLIN | POLLHUP | POLLERR,
            0
          }
        };

        bool firstLoop = true;

        while ( 1 ) {

          if ( firstLoop ) {
            firstLoop = false;
            _cv.notify_all();
            if ( _stopped )
              return;
          }

          //there is no way to give a timeout to FCGX_Accept_r, so we poll the socketfd for new
          //connections and listen on a pipe for a wakeup event to stop the worker if required
          int ret = zyppng::eintrSafeCall( ::poll, fds, 3, -1 );

          if ( ret < 0 ) {
            std::cerr << "Poll error " << errno << endl;
            continue;
          }

          if ( fds[0].revents ) {
            //clear pipe
            char dummy;
            while ( read( _wakeupPipe[0], &dummy, 1 ) > 0 ) { continue; }
          }

          if ( fds[2].revents ) {
            while(1) {
              std::string line = prog.receiveLine();
              if ( line.empty() )
                break;
              std::cerr << line << endl;
            };

            if ( !prog.running() ) {
              std::cerr << "Webserver exited too early" << endl;
              _stop = true;
              _stopped = true;
            }
          }

          if ( _stop )
            break;

          //no events on the listen socket, repoll
          if ( ! ( fds[1].revents & POLLIN ) )
            continue;

          if ( FCGX_Accept_r(&request) == 0 ) {

            fcgi_streambuf cout_fcgi(request.out);
            fcgi_streambuf cin_fcgi(request.in);
            fcgi_streambuf cerr_fcgi(request.err);

            WebServer::Request req( &cin_fcgi, &cout_fcgi, &cerr_fcgi );

            int i = 0;
            for ( char *env = request.envp[i]; env != nullptr; env = request.envp[++i] ) {
              char * eq = strchr( env, '=' );
              if ( !eq ) {
                continue;
              }
              req.params.insert( { std::string( env, eq ), std::string( eq+1 ) } );
            }

            if ( !req.params.count("SCRIPT_NAME") ) {
              req.rerr << "Status: 400 Bad Request\r\n"
                       << "Content-Type: text/html\r\n"
                       << "\r\n"
                       << "Invalid request";
              FCGX_Finish_r(&request);
              continue;
            }

            {
              std::lock_guard<std::mutex> lock( _mut );
              //remove "/handler/" prefix
              std::string key = req.params.at("SCRIPT_NAME").substr( handlerPrefix().length() );

              auto it = _handlers.find( key );
              if ( it == _handlers.end() ) {
                req.rerr << "Status: 404 Not Found\r\n"
                         << "Content-Type: text/html\r\n"
                         << "\r\n"
                         << "Request handler was not found";
                FCGX_Finish_r(&request);
                continue;
              }
              //call the handler
              it->second( req );
              FCGX_Finish_r(&request);
            }
          }
        }

        if ( prog.running() ) {
            prog.kill( SIGTERM );
            prog.close();
        }
    }

    std::string log() const
    {
      //read logfile
      return std::string();
    }

    void stop()
    {
        MIL << "Waiting for server thread to finish" << endl;
        _stop = true;

        //signal the thread to wake up
        write( _wakeupPipe[1], "\n", 1);

        _thrd->join();
        MIL << "server thread finished" << endl;

        _thrd.reset();
        _stopped = true;
    }

    void start()
    {
      if ( !filesystem::PathInfo( _docroot ).isExist() ) {
        std::cerr << "Invalid docroot" << std::endl;
        throw zypp::Exception("Invalid docroot");
      }

      if ( !_stopped ) {
        stop();
      }

      MIL << "Using socket " <<  socketPath() << std::endl;
      _stop = _stopped = false;

      std::unique_lock<std::mutex> lock( _mut );
      _thrd.reset( new std::thread( &Impl::worker_thread, this ) );
      _cv.wait(lock);

      if ( _stopped ) {
        _thrd->join();
        throw zypp::Exception("Failed to start the webserver");
      }
    }

    std::mutex _mut; //<one mutex to rule em all
    std::condition_variable _cv; //< to sync server startup

    filesystem::TmpDir _workingDir;
    zypp::Pathname _docroot;
    zypp::shared_ptr<std::thread> _thrd;
    std::map<std::string, WebServer::RequestHandler> _handlers;
    unsigned int _port;
    int _wakeupPipe[2];

    std::atomic_bool _stop;
    bool _stopped;
    bool _ssl;
};


WebServer::WebServer(const Pathname &root, unsigned int port, bool useSSL)
    : _pimpl(new Impl(root, port, useSSL))
{
}

bool WebServer::start()
{
    _pimpl->start();
    return !isStopped();
}


std::string WebServer::log() const
{
  return _pimpl->log();
}

bool WebServer::isStopped() const
{
  return _pimpl->_stopped;
}

void WebServer::addRequestHandler( const std::string &path, RequestHandler &&handler )
{
  std::lock_guard<std::mutex> lock( _pimpl->_mut );
  _pimpl->_handlers[path] = std::move(handler);
}

void WebServer::removeRequestHandler(const std::string &path)
{
  std::lock_guard<std::mutex> lock( _pimpl->_mut );
  auto it = _pimpl->_handlers.find( path );
  if ( it != _pimpl->_handlers.end() )
    _pimpl->_handlers.erase( it );
}

int WebServer::port() const
{
    return _pimpl->port();
}


Url WebServer::url() const
{
    Url url;
    url.setHost("localhost");
    url.setPort(str::numstring(port()));
    if ( _pimpl->_ssl )
      url.setScheme("https");
    else
      url.setScheme("http");

    return url;
}

media::TransferSettings WebServer::transferSettings() const
{
  media::TransferSettings set;
  set.setCertificateAuthoritiesPath(zypp::Pathname(TESTS_SRC_DIR)/"data/webconf/ssl/certstore");
  return set;
}

void WebServer::stop()
{
    _pimpl->stop();
}

WebServer::RequestHandler WebServer::makeResponse( std::string resp )  {
  return [ resp ]( WebServer::Request & req ){
    req.rout << resp;
  };
};

WebServer::RequestHandler WebServer::makeResponse( std::string status, std::string content )  {
  return makeResponse( status , std::vector<std::string>(), content );
};

WebServer::RequestHandler WebServer::makeResponse( const std::string &status, const std::vector<std::string> &headers, const std::string &content )  {
  return makeResponse( makeResponseString( status, headers, content ) );
}

std::string WebServer::makeResponseString(const std::string &status, const std::vector<std::string> &headers, const std::string &content)
{
  static const std::string genericResp = "Status: %1%\r\n"
                                         "%2%"
                                         "\r\n"
                                         "%3%";
  bool hasCType = false;
  bool hasLType = false;
  for( const std::string &hdr : headers ) {
    if ( zypp::str::startsWith( hdr, "Content-Type:" ) ) {
      hasCType = true;
    }
    if ( zypp::str::startsWith( hdr, "Content-Length:" ) ) {
      hasLType = true;
    }
  }

  std::string allHeaders;
  if ( !hasCType )
    allHeaders += "Content-Type: text/html; charset=utf-8\r\n";
  if ( !hasLType )
    allHeaders += str::Format("Content-Length: %1%\r\n") % content.length();
  allHeaders += zypp::str::join( headers.begin(), headers.end(), "\r\n");

  return ( zypp::str::Format( genericResp ) % status % allHeaders %content );
};

WebServer::~WebServer()
{
}
