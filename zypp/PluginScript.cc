/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PluginScript.cc
 *
*/
#include <sys/types.h>
#include <signal.h>

#include <iostream>
#include <sstream>

#include "zypp/base/LogTools.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/base/String.h"
#include "zypp/base/Signal.h"
#include "zypp/base/IOStream.h"

#include "zypp/PluginScript.h"
#include "zypp/ExternalProgram.h"
#include "zypp/PathInfo.h"

using std::endl;

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::plugin"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
    const char * PLUGIN_DEBUG = getenv( "ZYPP_PLUGIN_DEBUG" );

    /** Dump buffer string if PLUGIN_DEBUG is on.
     * \ingroup g_RAII
     */
    struct PluginDebugBuffer
    {
      PluginDebugBuffer( const std::string & buffer_r ) : _buffer( buffer_r ) {}
      ~PluginDebugBuffer()
      {
	if ( PLUGIN_DEBUG )
	{
	  if ( _buffer.empty() )
	  {
	    L_DBG("PLUGIN") << "< (empty)" << endl;
	  }
	  else
	  {
	    std::istringstream datas( _buffer );
	    iostr::copyIndent( datas, L_DBG("PLUGIN"), "< "  ) << endl;
	  }
	}
      }
      const std::string & _buffer;
    };

    /** Dump programms stderr.
     * \ingroup g_RAII
     */
    struct PluginDumpStderr
    {
      PluginDumpStderr( ExternalProgramWithStderr & prog_r ) : _prog( prog_r ) {}
      ~PluginDumpStderr()
      {
	std::string line;
	while ( _prog.stderrGetline( line ) )
	  L_WAR("PLUGIN") << "! " << line << endl;
      }
      ExternalProgramWithStderr & _prog;
    };

    inline void setBlocking( FILE * file_r, bool yesno_r = true )
    {
      if ( ! file_r )
	ZYPP_THROW( PluginScriptException( "setNonBlocking" ) );

      int fd = ::fileno( file_r );
      if ( fd == -1 )
	ZYPP_THROW( PluginScriptException( "setNonBlocking" ) );

      int flags = ::fcntl( fd, F_GETFL );
      if ( flags == -1 )
	ZYPP_THROW( PluginScriptException( "setNonBlocking" ) );

      if ( ! yesno_r )
	flags |= O_NONBLOCK;
      else if ( flags & O_NONBLOCK )
	flags ^= O_NONBLOCK;

      flags = ::fcntl( fd, F_SETFL, flags );
      if ( flags == -1 )
	ZYPP_THROW( PluginScriptException( "setNonBlocking" ) );
    }

    inline void setNonBlocking( FILE * file_r, bool yesno_r = true )
    { setBlocking( file_r, !yesno_r ); }
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PluginScript::Impl
  //
  /** \ref PluginScript implementation. */
  struct PluginScript::Impl
  {
    public:
      Impl( const Pathname & script_r = Pathname(), const Arguments & args_r = Arguments() )
        : _sendTimeout( _defaultSendTimeout )
        , _receiveTimeout( _defaultReceiveTimeout )
        , _script( script_r )
        , _args( args_r )
      {}

      ~ Impl()
      { try { close(); } catch(...) {} }

    public:
      static long _defaultSendTimeout;
      static long _defaultReceiveTimeout;

      long _sendTimeout;
      long _receiveTimeout;

   public:
      const Pathname & script() const
      { return _script; }

      const Arguments & args() const
      { return _args; }

      pid_t getPid() const
      { return _cmd ? _cmd->getpid() : NotConnected; }

      bool isOpen() const
      { return _cmd != nullptr; }

      int lastReturn() const
      { return _lastReturn; }

      const std::string & lastExecError() const
      { return _lastExecError; }

    public:
      void open( const Pathname & script_r = Pathname(), const Arguments & args_r = Arguments() );

      int close();

      void send( const PluginFrame & frame_r ) const;

      PluginFrame receive() const;

    private:
      Pathname _script;
      Arguments _args;
      scoped_ptr<ExternalProgramWithStderr> _cmd;
      DefaultIntegral<int,0> _lastReturn;
      std::string _lastExecError;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PluginScrip::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const PluginScript::Impl & obj )
  {
    return str << "PluginScript[" << obj.getPid() << "] " << obj.script();
  }

  ///////////////////////////////////////////////////////////////////

  namespace
  {
    const long PLUGIN_TIMEOUT = 	str::strtonum<long>( getenv( "ZYPP_PLUGIN_TIMEOUT" ) );
    const long PLUGIN_SEND_TIMEOUT = 	str::strtonum<long>( getenv( "ZYPP_PLUGIN_SEND_TIMEOUT" ) );
    const long PLUGIN_RECEIVE_TIMEOUT =	str::strtonum<long>( getenv( "ZYPP_PLUGIN_RECEIVE_TIMEOUT" ) );
  }

  long PluginScript::Impl::_defaultSendTimeout =    ( PLUGIN_SEND_TIMEOUT > 0    ? PLUGIN_SEND_TIMEOUT
										 : ( PLUGIN_TIMEOUT > 0 ? PLUGIN_TIMEOUT : 30 ) );
  long PluginScript::Impl::_defaultReceiveTimeout = ( PLUGIN_RECEIVE_TIMEOUT > 0 ? PLUGIN_RECEIVE_TIMEOUT
										 : ( PLUGIN_TIMEOUT > 0 ? PLUGIN_TIMEOUT : 30 ) );

  ///////////////////////////////////////////////////////////////////

  void PluginScript::Impl::open( const Pathname & script_r, const Arguments & args_r )
  {
    dumpRangeLine( DBG << "Open " << script_r, args_r.begin(), args_r.end() ) << endl;

    if ( _cmd )
      ZYPP_THROW( PluginScriptException( "Already connected", str::Str() << *this ) );

    {
      PathInfo pi( script_r );
      if ( ! ( pi.isFile() && pi.isX() ) )
	ZYPP_THROW( PluginScriptException( "Script is not executable", str::Str() << pi ) );
    }

    // go and launch script
    // TODO: ExternalProgram::maybe use Stderr_To_FileDesc for script loging
    Arguments args;
    args.reserve( args_r.size()+1 );
    args.push_back( script_r.asString() );
    args.insert( args.end(), args_r.begin(), args_r.end() );
    _cmd.reset( new ExternalProgramWithStderr( args ) );

    // Be protected against full pipe, etc.
    setNonBlocking( _cmd->outputFile() );
    setNonBlocking( _cmd->inputFile() );

    // store running scripts data
    _script = script_r;
    _args = args_r;
    _lastReturn.reset();
    _lastExecError.clear();

    dumpRangeLine( DBG << *this, _args.begin(), _args.end() ) << endl;
  }

  int PluginScript::Impl::close()
  {
    if ( _cmd )
    {
      DBG << "Close:" << *this << endl;
      bool doKill = true;
      try {
	// do not kill script if _DISCONNECT is ACKed.
	send( PluginFrame( "_DISCONNECT" ) );
	PluginFrame ret( receive() );
	if ( ret.isAckCommand() )
	{
	  doKill = false;
	  str::strtonum( ret.getHeaderNT( "exit" ), _lastReturn.get() );
	  _lastExecError = ret.body();
	}
      }
      catch (...)
      { /* NOP */ }

      if ( doKill )
      {
	_cmd->kill();
	_lastReturn = _cmd->close();
	_lastExecError = _cmd->execError();
      }
      DBG << *this << " -> [" << _lastReturn << "] " << _lastExecError << endl;
      _cmd.reset();
    }
    return _lastReturn;
  }

  void PluginScript::Impl::send( const PluginFrame & frame_r ) const
  {
    if ( !_cmd )
      ZYPP_THROW( PluginScriptNotConnected( "Not connected", str::Str() << *this ) );

    if ( frame_r.command().empty() )
      WAR << "Send: No command in frame" << frame_r << endl;

    // prepare frame data to write
    std::string data;
    {
      std::ostringstream datas;
      frame_r.writeTo( datas );
      datas.str().swap( data );
    }
    DBG << *this << " ->send " << frame_r << endl;

    if ( PLUGIN_DEBUG )
    {
      std::istringstream datas( data );
      iostr::copyIndent( datas, L_DBG("PLUGIN") ) << endl;
    }

    // try writing the pipe....
    FILE * filep = _cmd->outputFile();
    if ( ! filep )
      ZYPP_THROW( PluginScriptException( "Bad file pointer." ) );

    int fd = ::fileno( filep );
    if ( fd == -1 )
      ZYPP_THROW( PluginScriptException( "Bad file descriptor" ) );

    //DBG << " ->[" << fd << " " << (::feof(filep)?'e':'_') << (::ferror(filep)?'F':'_') << "]" << endl;
    {
      PluginDumpStderr _dump( *_cmd ); // dump scripts stderr before leaving
      SignalSaver sigsav( SIGPIPE, SIG_IGN );
      const char * buffer = data.c_str();
      ssize_t buffsize = data.size();
      do {
	fd_set wfds;
	FD_ZERO( &wfds );
	FD_SET( fd, &wfds );

	struct timeval tv;
	tv.tv_sec = _sendTimeout;
	tv.tv_usec = 0;

	int retval = select( fd+1, NULL, &wfds, NULL, &tv );
	if ( retval > 0 )	// FD_ISSET( fd, &wfds ) will be true.
	{
	  //DBG << "Ready to write..." << endl;
	  ssize_t ret = ::write( fd, buffer, buffsize );
	  if ( ret == buffsize )
	  {
	    //DBG << "::write(" << buffsize << ") -> " << ret << endl;
	    ::fflush( filep );
	    break; 		// -> done
	  }
	  else if ( ret > 0 )
	  {
	    //WAR << "::write(" << buffsize << ") -> " << ret << " INCOMPLETE..." << endl;
	    ::fflush( filep );
	    buffsize -= ret;
	    buffer += ret;	// -> continue
	  }
	  else // ( retval == -1 )
	  {
	    if ( errno != EINTR )
	    {
	      ERR << "write(): " << Errno() << endl;
	      if ( errno == EPIPE )
		ZYPP_THROW( PluginScriptDiedUnexpectedly( "Send: script died unexpectedly", str::Str() << Errno() ) );
	      else
		ZYPP_THROW( PluginScriptException( "Send: send error", str::Str() << Errno() ) );
	    }
	  }
	}
	else if ( retval == 0 )
	{
	  WAR << "Not ready to write within timeout." << endl;
	  ZYPP_THROW( PluginScriptSendTimeout( "Not ready to write within timeout." ) );
	}
	else // ( retval == -1 )
	{
	  if ( errno != EINTR )
	  {
	    ERR << "select(): " << Errno() << endl;
	    ZYPP_THROW( PluginScriptException( "Error waiting on file descriptor", str::Str() << Errno() ) );
	  }
	}
      } while( true );
    }
  }

  PluginFrame PluginScript::Impl::receive() const
  {
    if ( !_cmd )
      ZYPP_THROW( PluginScriptNotConnected( "Not connected", str::Str() << *this ) );

    // try reading the pipe....
    FILE * filep = _cmd->inputFile();
    if ( ! filep )
      ZYPP_THROW( PluginScriptException( "Bad file pointer." ) );

    int fd = ::fileno( filep );
    if ( fd == -1 )
      ZYPP_THROW( PluginScriptException( "Bad file descriptor" ) );

    ::clearerr( filep );
    std::string data;
    {
      PluginDebugBuffer _debug( data ); // dump receive buffer if PLUGIN_DEBUG
      PluginDumpStderr _dump( *_cmd ); // dump scripts stderr before leaving
      do {
	int ch = fgetc( filep );
	if ( ch != EOF )
	{
	  data.push_back( ch );
	  if ( ch == '\0' )
	    break;
	}
	else if ( ::feof( filep ) )
	{
	  WAR << "Unexpected EOF" << endl;
	  ZYPP_THROW( PluginScriptDiedUnexpectedly( "Receive: script died unexpectedly", str::Str() << Errno() ) );
	}
	else if ( errno != EINTR )
	{
	  if ( errno == EWOULDBLOCK )
	  {
	    // wait a while for fd to become ready for reading...
	    fd_set rfds;
	    FD_ZERO( &rfds );
	    FD_SET( fd, &rfds );

	    struct timeval tv;
	    tv.tv_sec = _receiveTimeout;
	    tv.tv_usec = 0;

	    int retval = select( fd+1, &rfds, NULL, NULL, &tv );
	    if ( retval > 0 )	// FD_ISSET( fd, &rfds ) will be true.
	    {
	      ::clearerr( filep );
	    }
	    else if ( retval == 0 )
	    {
	      WAR << "Not ready to read within timeout." << endl;
	      ZYPP_THROW( PluginScriptReceiveTimeout( "Not ready to read within timeout." ) );
	    }
	    else // ( retval == -1 )
	    {
	      if ( errno != EINTR )
	      {
		ERR << "select(): " << Errno() << endl;
		ZYPP_THROW( PluginScriptException( "Error waiting on file descriptor", str::Str() << Errno() ) );
	      }
	    }
	  }
	  else
	  {
	    ERR << "read(): " << Errno() << endl;
	    ZYPP_THROW( PluginScriptException( "Receive: receive error", str::Str() << Errno() ) );
	  }
	}
      } while ( true );
    }
    // DBG << " <-read " << data.size() << endl;
    std::istringstream datas( data );
    PluginFrame ret( datas );
    DBG << *this << " <-" << ret << endl;
    return ret;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PluginScript
  //
  ///////////////////////////////////////////////////////////////////

  const pid_t PluginScript::NotConnected( -1 );

  long PluginScript::defaultSendTimeout()
  { return Impl::_defaultSendTimeout; }

  long PluginScript::defaultReceiveTimeout()
  { return Impl::_defaultReceiveTimeout; }

  void PluginScript::defaultSendTimeout( long newval_r )
  { Impl::_defaultSendTimeout = newval_r > 0 ? newval_r : 0; }

  void PluginScript::defaultReceiveTimeout( long newval_r )
  { Impl::_defaultReceiveTimeout = newval_r > 0 ? newval_r : 0; }

  long PluginScript::sendTimeout() const
  { return _pimpl->_sendTimeout; }

  long PluginScript::receiveTimeout() const
  { return _pimpl->_receiveTimeout; }

  void PluginScript::sendTimeout( long newval_r )
  { _pimpl->_sendTimeout = newval_r > 0 ? newval_r : 0; }

  void PluginScript::receiveTimeout( long newval_r )
  { _pimpl->_receiveTimeout = newval_r > 0 ? newval_r : 0; }

  PluginScript::PluginScript()
    : _pimpl( new Impl )
  {}

  PluginScript::PluginScript( const Pathname & script_r )
    : _pimpl( new Impl( script_r ) )
  {}

  PluginScript::PluginScript( const Pathname & script_r, const Arguments & args_r )
    : _pimpl( new Impl( script_r, args_r ) )
  {}

  const Pathname & PluginScript::script() const
  { return _pimpl->script(); }

  const PluginScript::Arguments & PluginScript::args() const
  { return _pimpl->args(); }

  bool PluginScript::isOpen() const
  { return _pimpl->isOpen(); }

  pid_t PluginScript::getPid() const
  { return _pimpl->getPid(); }

  int PluginScript::lastReturn() const
  { return _pimpl->lastReturn(); }

  const std::string & PluginScript::lastExecError() const
  { return _pimpl->lastExecError(); }

  void PluginScript::open()
  { _pimpl->open( _pimpl->script(), _pimpl->args() ); }

  void PluginScript::open( const Pathname & script_r )
  { _pimpl->open( script_r ); }

  void PluginScript::open( const Pathname & script_r, const Arguments & args_r )
  { _pimpl->open( script_r, args_r ); }

  int PluginScript::close()
  { return _pimpl->close(); }

  void PluginScript::send( const PluginFrame & frame_r ) const
  { _pimpl->send( frame_r ); }

  PluginFrame PluginScript::receive() const
  { return _pimpl->receive(); }

  ///////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const PluginScript & obj )
  { return str << *obj._pimpl; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
