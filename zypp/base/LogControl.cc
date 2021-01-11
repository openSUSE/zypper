/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/LogControl.cc
 *
*/
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

#include <zypp/base/Logger.h>
#include <zypp/base/LogControl.h>
#include <zypp/base/ProfilingFormater.h>
#include <zypp/base/String.h>
#include <zypp/Date.h>
#include <zypp/TriBool.h>
#include <zypp/PathInfo.h>
#include <zypp/AutoDispose.h>

#include <zypp/zyppng/io/Socket>
#include <zypp/zyppng/io/SockAddr>
#include <zypp/zyppng/base/EventLoop>
#include <zypp/zyppng/base/EventDispatcher>
#include <zypp/zyppng/base/Timer>
#include <zypp/zyppng/base/private/linuxhelpers_p.h>
#include <zypp/zyppng/thread/Wakeup>
#include <zypp/zyppng/base/private/threaddata_p.h>
#include <zypp/zyppng/base/SocketNotifier>

#include <thread>
#include <variant>
#include <atomic>
#include <csignal>

using std::endl;

std::once_flag flagReadEnvAutomatically;

namespace zypp
{
  constexpr std::string_view ZYPP_MAIN_THREAD_NAME( "Zypp-main" );

  template<class> inline constexpr bool always_false_v = false;

  /*!
   * \internal Provides a very simply lock that is using atomics so it can be
   * safely used in signal handlers
   */
  class SpinLock {
  public:
    void lock () {
      // acquire lock
      while ( _atomicLock.test_and_set())
        // reschedule the current thread while we wait, maybe when its our next turn the lock is free again
        std::this_thread::yield();
    }

    void unlock() {
      _atomicLock.clear();
    }

  private:
    // we use a lock free atomic flag here so this lock can be safely obtained in a signal handler as well
    std::atomic_flag _atomicLock = ATOMIC_FLAG_INIT;
  };

  class LogThread
  {

  public:

    ~LogThread() {
      stop();
    }

    static LogThread &instance () {
      static LogThread t;
      return t;
    }

    void setLineWriter ( boost::shared_ptr<log::LineWriter> writer ) {
      std::lock_guard lk( _lineWriterLock );
      _lineWriter = writer;
    }

    boost::shared_ptr<log::LineWriter> getLineWriter () {
      std::lock_guard lk( _lineWriterLock );
      auto lw = _lineWriter;
      return lw;
    }

    void stop () {
      _stopSignal.notify();
      if ( _thread.get_id() != std::this_thread::get_id() )
        _thread.join();
    }

    std::thread::id threadId () {
      return _thread.get_id();
    }

    static std::string sockPath () {
      static std::string path = zypp::str::Format("zypp-logsocket-%1%") % getpid();
      return path;
    }

  private:

    LogThread ()
    {
      // Name the thread that started the logger assuming it's main thread.
      zyppng::ThreadData::current().setName(ZYPP_MAIN_THREAD_NAME);
      _thread = std::thread( [this] () {
        workerMain();
      });
    }

    void workerMain () {

      // force the kernel to pick another thread to handle those signals
      zyppng::blockSignalsForCurrentThread( { SIGTERM, SIGINT, SIGPIPE, } );

      zyppng::ThreadData::current().setName("Zypp-Log");

      auto ev = zyppng::EventLoop::create();
      auto server = zyppng::Socket::create( AF_UNIX, SOCK_STREAM, 0 );
      auto stopNotifyWatch = _stopSignal.makeNotifier( );

      std::vector<zyppng::Socket::Ptr> clients;

      // bind to a abstract unix domain socket address, which means we do not need to care about cleaning it up
      server->bind( std::make_shared<zyppng::UnixSockAddr>( sockPath(), true ) );
      server->listen();

      // wait for incoming connections from other threads
      server->connectFunc( &zyppng::Socket::sigIncomingConnection, [&](){

        auto cl = server->accept();
        if ( !cl ) return;
        clients.push_back( cl );

        // wait until data is available, we operate line by line so we only
        // log a string once we encounter \n
        cl->connectFunc( &zyppng::Socket::sigReadyRead, [ this, sock = cl.get() ](){
          auto writer = getLineWriter();
          if ( !writer ) return;
          while ( sock->canReadLine() ) {
            auto br = sock->readLine();
            writer->writeOut( std::string( br.data(), br.size() - 1 ) );
          }
        }, *cl);

        // once a client disconnects we remove it from the std::vector so that the socket is not leaked
        cl->connectFunc( &zyppng::Socket::sigDisconnected, [&clients, sock = cl.get()](){
          auto idx = std::find_if( clients.begin(), clients.end(), [sock]( const auto &s ){ return sock == s.get(); } );
          clients.erase( idx );
        }, *cl);

      });

      stopNotifyWatch->connectFunc( &zyppng::SocketNotifier::sigActivated, [&ev]( const auto &, auto ) {
        ev->quit();
      });

      ev->run();

      // make sure we have written everything
      auto writer = getLineWriter();
      if ( writer ) {
        for ( auto &sock : clients ){
          while ( sock->canReadLine() ) {
            auto br = sock->readLine();
            writer->writeOut( std::string( br.data(), br.size() - 1 ) );
          }
        }
      }
    }

  private:
    std::thread _thread;
    zyppng::Wakeup _stopSignal;

    // since the public API uses boost::shared_ptr we can not use the atomic
    // functionalities provided in std.
    // this lock type can be used safely in signals
    SpinLock _lineWriterLock;
    // boost shared_ptr has a lock free implementation of reference counting so it can be used from signal handlers as well
    boost::shared_ptr<log::LineWriter> _lineWriter{ nullptr };
  };

  class LogClient
  {
   public:
    LogClient(){
      // make sure the thread is running
      LogThread::instance();
    }

    ~LogClient() {
      ::close( _sockFD );
    }

    /*!
     * Tries to connect to the log threads socket, returns true on success or
     * if the socket is already connected
     */
    bool ensureConnection () {
      if ( _sockFD >= 0 )
        return true;

      _sockFD = ::socket( AF_UNIX, SOCK_STREAM, 0 );
      if ( _sockFD == -1 )
        return false;

      zyppng::UnixSockAddr addr( LogThread::sockPath(), true );
      return zyppng::trySocketConnection( _sockFD, addr, 100 );
    }

    /*!
     * Sends a message to the log thread.
     */
    void pushMessage ( std::string &&msg ) {
      if ( inPushMessage ) {
        return;
      }

      // make sure we do not end up in a busy loop
      zypp::AutoDispose<bool *> res( &inPushMessage, [](auto val){
        *val = false;
      });
      inPushMessage = true;

      // if we are in the same thread as the Log worker we can directly push our messages out, no need to use the socket
      if ( std::this_thread::get_id() == LogThread::instance().threadId() ) {
        auto writer = LogThread::instance().getLineWriter();
        writer->writeOut( msg );
        return;
      }

      if(!ensureConnection())
        return;

      if ( msg.back() != '\n' )
        msg.push_back('\n');

      size_t written = 0;
      while ( written < msg.size() ) {
        const auto res = zyppng::eintrSafeCall( ::send, _sockFD, msg.data() + written, msg.size() - written, MSG_NOSIGNAL );
        if ( res == -1 ) {
          //assume broken socket
          ::close( _sockFD );
          _sockFD = -1;
          return;
        }
        written += res;
      }
    }

    private:
      int _sockFD = -1;
      bool inPushMessage = false;
  };

#ifndef ZYPP_NDEBUG
  namespace debug
  {
    // Fg::Black:   30  Bg: 40 Attr::Normal:  22;27
    // Fg::Red:     31  ...    Attr::Bright:  1
    // Fg::Green:   32         Attr::Reverse: 7
    // Fg::Yellow:  33
    // Fg::Blue:    34
    // Fg::Magenta: 35
    // Fg::Cyan:    36
    // Fg::White:   37
    // Fg::Default: 39
    static constexpr std::string_view OO { "\033[0m" };
    static constexpr std::string_view WH { "\033[37;40m" };
    static constexpr std::string_view CY { "\033[36;40m" };
    static constexpr std::string_view YE { "\033[33;1;40m" };
    static constexpr std::string_view GR { "\033[32;40m" };
    static constexpr std::string_view RE { "\033[31;1;40m" };
    static constexpr std::string_view MA { "\033[35;40m" };

    unsigned TraceLeave::_depth = 1;

    std::string tracestr( char tag_r, unsigned depth_r, const char * file_r, const char * fnc_r, int line_r )
    {
      static str::Format fmt { "*** %s %s(%s):%d" };
      fmt % std::string(depth_r,tag_r) % file_r % fnc_r % line_r;
      return fmt;
    }

    TraceLeave::TraceLeave( const char * file_r, const char * fnc_r, int line_r )
    : _file( std::move(file_r) )
    , _fnc( std::move(fnc_r) )
    , _line( line_r )
    {
      const std::string & m { tracestr( '>',_depth++, _file,_fnc,_line ) };
      USR << m << endl;
      Osd(L_USR("TRACE"),1) << m << endl;
    }

    TraceLeave::~TraceLeave()
    {
      const std::string & m { tracestr( '<',--_depth, _file,_fnc,_line ) };
      USR << m << endl;
      Osd(L_USR("TRACE"),1) << m << endl;
    }

    Osd::Osd( std::ostream & str, int i )
    : _strout { std::cerr }
    , _strlog { str }
    { _strout << (i?WH:YE); }

    Osd::~Osd()
    { _strout << OO; }

    Osd & Osd::operator<<( std::ostream& (*iomanip)( std::ostream& ) )
    {
      _strout << iomanip;
      _strlog << iomanip;
      return *this;
    }
}
#endif // ZYPP_NDEBUG

  ///////////////////////////////////////////////////////////////////
  namespace log
  { /////////////////////////////////////////////////////////////////

    StdoutLineWriter::StdoutLineWriter()
      : StreamLineWriter( std::cout )
    {}

    StderrLineWriter::StderrLineWriter()
      : StreamLineWriter( std::cerr )
    {}

    FileLineWriter::FileLineWriter( const Pathname & file_r, mode_t mode_r )
    {
      if ( file_r == Pathname("-") )
      {
        _str = &std::cerr;
      }
      else
      {
	if ( mode_r )
	{
          // not filesystem::assert_file as filesystem:: functions log,
	  // and this FileWriter is not yet in place.
	  int fd = ::open( file_r.c_str(), O_CREAT|O_EXCL, mode_r );
	  if ( fd != -1 )
	    ::close( fd );
	}
        // set unbuffered write
        std::ofstream * fstr = 0;
        _outs.reset( (fstr = new std::ofstream( file_r.asString().c_str(), std::ios_base::app )) );
        fstr->rdbuf()->pubsetbuf(0,0);
        _str = &(*fstr);
      }
    }

    /////////////////////////////////////////////////////////////////
  } // namespace log
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace logger
    { /////////////////////////////////////////////////////////////////

      inline void putStream( const std::string & group_r, LogLevel level_r,
                             const char * file_r, const char * func_r, int line_r,
                             const std::string & buffer_r );

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Loglinebuf
      //
      class Loglinebuf : public std::streambuf {

      public:
        /** */
        Loglinebuf( const std::string & group_r, LogLevel level_r )
        : _group( group_r )
        , _level( level_r )
        , _file( "" )
        , _func( "" )
        , _line( -1 )
        {}
        /** */
        ~Loglinebuf()
        {
          if ( !_buffer.empty() )
            writeout( "\n", 1 );
        }

        /** */
        void tagSet( const char * fil_r, const char * fnc_r, int lne_r )
        {
          _file = fil_r;
          _func = fnc_r;
          _line = lne_r;
        }

      private:
        /** */
        virtual std::streamsize xsputn( const char * s, std::streamsize n )
        { return writeout( s, n ); }
        /** */
        virtual int overflow( int ch = EOF )
        {
          if ( ch != EOF )
            {
              char tmp = ch;
              writeout( &tmp, 1 );
            }
          return 0;
        }
        /** */
        virtual int writeout( const char* s, std::streamsize n )
        {
	  //logger::putStream( _group, _level, _file, _func, _line, _buffer );
	  //return n;
          if ( s && n )
            {
              const char * c = s;
              for ( int i = 0; i < n; ++i, ++c )
                {
                  if ( *c == '\n' ) {
                    _buffer += std::string( s, c-s );
                    logger::putStream( _group, _level, _file, _func, _line, _buffer );
                    _buffer = std::string();
                    s = c+1;
                  }
                }
              if ( s < c )
                {
                  _buffer += std::string( s, c-s );
                }
            }
          return n;
        }

      private:
        std::string  _group;
        LogLevel     _level;
        const char * _file;
        const char * _func;
        int          _line;
        std::string  _buffer;
      };

      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Loglinestream
      //
      class Loglinestream {

      public:
        /** */
        Loglinestream( const std::string & group_r, LogLevel level_r )
        : _mybuf( group_r, level_r )
        , _mystream( &_mybuf )
        {}
        /** */
        ~Loglinestream()
        { _mystream.flush(); }

      public:
        /** */
        std::ostream & getStream( const char * fil_r, const char * fnc_r, int lne_r )
        {
          _mybuf.tagSet( fil_r, fnc_r, lne_r );
          return _mystream;
        }

      private:
        Loglinebuf   _mybuf;
        std::ostream _mystream;
      };
      ///////////////////////////////////////////////////////////////////

      struct LogControlImpl;

      /*
       * Horrible ugly hack to prevent the use of LogControlImpl when libzypp is shutting down.
       * Due to the c++ std thread_local static instances are cleaned up before the first global static
       * destructor is called. So all classes that use logging after that point in time would crash the
       * application because its accessing a variable that has already been destroyed.
       *
       * This does not check if the current thread requesting a instance actually has one, it just keeps count
       * of how many instances are still available. Usually only the main thread should run into the condition
       * of getting a nullptr back.
       */
      static std::atomic_int & logControlImplReg() {
        static std::atomic_int instCount;
        return instCount;
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : LogControlImpl
      //
      /** LogControl implementation (thread_local Singleton).
       *
       * \note There is a slight difference in using the _lineFormater and _lineWriter!
       * \li \c _lineFormater must not be NULL (create default LogControl::LineFormater)
       * \li \c _lineWriter is NULL if no logging is performed, this way we can pass
       *        _no_stream as logstream to the application, and avoid unnecessary formating
       *        of logliles, which would then be discarded when passed to some dummy
       *        LineWriter.
      */
      struct LogControlImpl
      {
      public:
	bool isExcessive() const
	{ return _excessive; }

        void excessive( bool onOff_r )
        { _excessive = onOff_r; }


        /** Hint for Formater whether to hide the thread name. */
	bool hideThreadName() const
	{
	  if ( indeterminate(_hideThreadName) )
	    _hideThreadName = ( zyppng::ThreadData::current().name() == ZYPP_MAIN_THREAD_NAME );
	  return bool(_hideThreadName);
	}
	/** \overload Setter */
        void hideThreadName( bool onOff_r )
        { _hideThreadName = onOff_r; }
        /** \overload Static getter */
	static bool instanceHideThreadName()
	{
	  auto impl = LogControlImpl::instance();
	  return impl ? impl->hideThreadName() : false;
	}
	/** \overload Static setter */
        static void instanceHideThreadName( bool onOff_r )
        {
	  auto impl = LogControlImpl::instance();
	  if ( impl ) impl->hideThreadName( onOff_r );
	}


        /** NULL _lineWriter indicates no loggin. */
        void setLineWriter( const shared_ptr<LogControl::LineWriter> & writer_r )
        { LogThread::instance().setLineWriter( writer_r ); }

        shared_ptr<LogControl::LineWriter> getLineWriter() const
        { return LogThread::instance().getLineWriter(); }

        /** Assert \a _lineFormater is not NULL. */
        void setLineFormater( const shared_ptr<LogControl::LineFormater> & format_r )
        {
          if ( format_r )
            _lineFormater = format_r;
          else
            _lineFormater.reset( new LogControl::LineFormater );
        }

        void logfile( const Pathname & logfile_r, mode_t mode_r = 0640 )
        {
          if ( logfile_r.empty() )
            setLineWriter( shared_ptr<LogControl::LineWriter>() );
          else if ( logfile_r == Pathname( "-" ) )
            setLineWriter( shared_ptr<LogControl::LineWriter>(new log::StderrLineWriter) );
          else
            setLineWriter( shared_ptr<LogControl::LineWriter>(new log::FileLineWriter(logfile_r, mode_r)) );
        }

      private:
        LogClient    _logClient;
        std::ostream _no_stream;
        bool         _excessive;
	mutable TriBool _hideThreadName = indeterminate;	///< Hint for Formater whether to hide the thread name.

        shared_ptr<LogControl::LineFormater> _lineFormater;

      public:
        /** Provide the log stream to write (logger interface) */
        std::ostream & getStream( const std::string & group_r,
                                  LogLevel            level_r,
                                  const char *        file_r,
                                  const char *        func_r,
                                  const int           line_r )
        {
          if ( ! getLineWriter() )
            return _no_stream;
          if ( level_r == E_XXX && !_excessive )
            return _no_stream;

          if ( !_streamtable[group_r][level_r] )
            {
              _streamtable[group_r][level_r].reset( new Loglinestream( group_r, level_r ) );
            }
          std::ostream & ret( _streamtable[group_r][level_r]->getStream( file_r, func_r, line_r ) );
	  if ( !ret )
	  {
	    ret.clear();
	    ret << "---<RESET LOGSTREAM FROM FAILED STATE]" << endl;
	  }
          return ret;
        }

        /** Format and write out a logline from Loglinebuf. */
        void putStream( const std::string & group_r,
                        LogLevel            level_r,
                        const char *        file_r,
                        const char *        func_r,
                        int                 line_r,
                        const std::string & message_r )
        {
          _logClient.pushMessage( _lineFormater->format( group_r, level_r,
                                                                    file_r, func_r, line_r,
                                                                    message_r ) );
        }

      private:
        typedef shared_ptr<Loglinestream>        StreamPtr;
        typedef std::map<LogLevel,StreamPtr>     StreamSet;
        typedef std::map<std::string,StreamSet>  StreamTable;
        /** one streambuffer per group and level */
        StreamTable _streamtable;
        zyppng::Socket::Ptr _sock;

      private:

        void readEnvVars () {
          if ( getenv("ZYPP_LOGFILE") )
            logfile( getenv("ZYPP_LOGFILE") );

          if ( getenv("ZYPP_PROFILING") )
          {
            shared_ptr<LogControl::LineFormater> formater(new ProfilingFormater);
            setLineFormater(formater);
          }
        }
        /** Singleton ctor.
         * No logging per default, unless enabled via $ZYPP_LOGFILE.
        */
        LogControlImpl()
        : _no_stream( NULL )
        , _excessive( getenv("ZYPP_FULLLOG") )
        , _lineFormater( new LogControl::LineFormater )
        {
          logControlImplReg().fetch_add(1);
          std::call_once( flagReadEnvAutomatically, &LogControlImpl::readEnvVars, this);
        }

      public:

        ~LogControlImpl()
        {
          logControlImplReg().fetch_sub(1);
        }

        /** The LogControlImpl singleton
         * \note As most dtors log, it is inportant that the
         * LogControlImpl instance is the last static variable
         * destructed. At least destucted after all statics
         * which log from their dtor.
        */
        static LogControlImpl *instance();
      };
      ///////////////////////////////////////////////////////////////////

      // 'THE' LogControlImpl singleton
      inline LogControlImpl *LogControlImpl::instance()
      {
        thread_local static LogControlImpl _instance;
        if ( logControlImplReg().load() > 0 )
          return &_instance;
        return nullptr;
      }

      ///////////////////////////////////////////////////////////////////

      /** \relates LogControlImpl Stream output */
      inline std::ostream & operator<<( std::ostream & str, const LogControlImpl & )
      {
        return str << "LogControlImpl";
      }

      ///////////////////////////////////////////////////////////////////
      //
      // Access from logger::
      //
      ///////////////////////////////////////////////////////////////////

      std::ostream & getStream( const char * group_r,
                                LogLevel     level_r,
                                const char * file_r,
                                const char * func_r,
                                const int    line_r )
      {
        static std::ostream nstream(NULL);
        auto control = LogControlImpl::instance();
        if ( !control || !group_r || strlen(group_r ) == 0 ) {
          return nstream;
        }



        return control->getStream( group_r,
                                   level_r,
                                   file_r,
                                   func_r,
                                   line_r );
      }

      /** That's what Loglinebuf calls.  */
      inline void putStream( const std::string & group_r, LogLevel level_r,
                             const char * file_r, const char * func_r, int line_r,
                             const std::string & buffer_r )
      {
        auto control = LogControlImpl::instance();
        if ( !control )
          return;

        control->putStream( group_r, level_r,
                            file_r, func_r, line_r,
                            buffer_r );
      }

      bool isExcessive()
      {
        auto impl = LogControlImpl::instance();
        if ( !impl )
          return false;
        return impl->isExcessive();
      }

      /////////////////////////////////////////////////////////////////
    } // namespace logger
    ///////////////////////////////////////////////////////////////////

    using logger::LogControlImpl;

    ///////////////////////////////////////////////////////////////////
    // LineFormater
    ///////////////////////////////////////////////////////////////////
    std::string LogControl::LineFormater::format( const std::string & group_r,
                                                  logger::LogLevel    level_r,
                                                  const char *        file_r,
                                                  const char *        func_r,
                                                  int                 line_r,
                                                  const std::string & message_r )
    {
      static char hostname[1024];
      static char nohostname[] = "unknown";
      std::string now( Date::now().form( "%Y-%m-%d %H:%M:%S" ) );
      std::string ret;
      if ( LogControlImpl::instanceHideThreadName() )
	ret = str::form( "%s <%d> %s(%d) [%s] %s(%s):%d %s",
			 now.c_str(), level_r,
			 ( gethostname( hostname, 1024 ) ? nohostname : hostname ),
			 getpid(),
			 group_r.c_str(),
			 file_r, func_r, line_r,
			 message_r.c_str() );
      else
	ret = str::form( "%s <%d> %s(%d) [%s] %s(%s):%d {T:%s} %s",
			 now.c_str(), level_r,
			 ( gethostname( hostname, 1024 ) ? nohostname : hostname ),
			 getpid(),
			 group_r.c_str(),
			 file_r, func_r, line_r,
			 zyppng::ThreadData::current().name().c_str(),
			 message_r.c_str() );
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LogControl
    //  Forward to LogControlImpl singleton.
    //
    ///////////////////////////////////////////////////////////////////


    void LogControl::logfile( const Pathname & logfile_r )
    {
      auto impl = LogControlImpl::instance();
      if ( !impl )
        return;

      impl->logfile( logfile_r );
    }

    void LogControl::logfile( const Pathname & logfile_r, mode_t mode_r )
    {
      auto impl = LogControlImpl::instance();
      if ( !impl )
        return;

      impl->logfile( logfile_r, mode_r );
    }

    shared_ptr<LogControl::LineWriter> LogControl::getLineWriter() const
    {
      auto impl = LogControlImpl::instance();
      if ( !impl )
        return nullptr;

      return impl->getLineWriter();
    }

    void LogControl::setLineWriter( const shared_ptr<LineWriter> & writer_r )
    {
      auto impl = LogControlImpl::instance();
      if ( !impl )
        return;
      impl->setLineWriter( writer_r );
    }

    void LogControl::setLineFormater( const shared_ptr<LineFormater> & formater_r )
    {
      auto impl = LogControlImpl::instance();
      if ( !impl )
        return;
      impl->setLineFormater( formater_r );
    }

    void LogControl::logNothing()
    {
      auto impl = LogControlImpl::instance();
      if ( !impl )
        return;
      impl->setLineWriter( shared_ptr<LineWriter>() );
    }

    void LogControl::logToStdErr()
    {
      auto impl = LogControlImpl::instance();
      if ( !impl )
        return;
      impl->setLineWriter( shared_ptr<LineWriter>( new log::StderrLineWriter ) );
    }

    void base::LogControl::emergencyShutdown()
    {
      LogThread::instance().stop();
    }

    ///////////////////////////////////////////////////////////////////
    //
    // LogControl::TmpExcessive
    //
    ///////////////////////////////////////////////////////////////////
    LogControl::TmpExcessive::TmpExcessive()
    {
      auto impl = LogControlImpl::instance();
      if ( !impl )
        return;
      impl->excessive( true );
    }
    LogControl::TmpExcessive::~TmpExcessive()
    {
      auto impl = LogControlImpl::instance();
      if ( !impl )
        return;
      impl->excessive( false );
    }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const LogControl & )
    {
      auto impl = LogControlImpl::instance();
      if ( !impl )
        return str;
      return str << *impl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
