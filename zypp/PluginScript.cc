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

#include "zypp/PluginScript.h"
#include "zypp/ExternalProgram.h"
#include "zypp/PathInfo.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
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
        : _script( script_r )
        , _args( args_r )
      {}

      ~ Impl()
      { try { close(); } catch(...) {} }

    public:
      /** Timeout (sec.) when sending data. */
      static const long send_timeout;
      /** Timeout (sec.) when receiving data. */
      static const long receive_timeout;

    public:
      const Pathname & script() const
      { return _script; }

      const Arguments & args() const
      { return _args; }

      pid_t getPid() const
      { return _cmd ? _cmd->getpid() : NotConnected; }

      bool isOpen() const
      { return _cmd; }

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
      scoped_ptr<ExternalProgram> _cmd;
      DefaultIntegral<int,0> _lastReturn;
      std::string _lastExecError;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PluginScrip::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const PluginScript::Impl & obj )
  {
    return dumpRangeLine( str << "PluginScript[" << obj.getPid() << "] " << obj.script(),
			  obj.args().begin(), obj.args().end() );
  }

  ///////////////////////////////////////////////////////////////////

  const long PluginScript::Impl::send_timeout = 3;
  const long PluginScript::Impl::receive_timeout = 5;

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
    _cmd.reset( new ExternalProgram( args, ExternalProgram::Discard_Stderr ) );

    // Be protected against full pipe, etc.
    setNonBlocking( _cmd->outputFile() );
    setNonBlocking( _cmd->inputFile() );

    // store running scripts data
    _script = script_r;
    _args = args_r;
    _lastReturn.reset();
    _lastExecError.clear();

    DBG << *this << endl;
  }

  int PluginScript::Impl::close()
  {
    if ( _cmd )
    {
      DBG << "Close:" << *this << endl;
      _cmd->kill();
      _lastReturn = _cmd->close();
      _lastExecError = _cmd->execError();
      _cmd.reset();
      DBG << *this << " -> [" << _lastReturn << "] " << _lastExecError << endl;
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
    DBG << "->send " << frame_r << endl;

    // try writing the pipe....
    FILE * filep = _cmd->outputFile();
    if ( ! filep )
      ZYPP_THROW( PluginScriptException( "Bad file pointer." ) );

    int fd = ::fileno( filep );
    if ( fd == -1 )
      ZYPP_THROW( PluginScriptException( "Bad file descriptor" ) );

    //DBG << " ->[" << fd << " " << (::feof(filep)?'e':'_') << (::ferror(filep)?'F':'_') << "]" << endl;
    {
      SignalSaver sigsav( SIGPIPE, SIG_IGN );
      const char * buffer = data.c_str();
      ssize_t buffsize = data.size();
      do {
	fd_set wfds;
	FD_ZERO( &wfds );
	FD_SET( fd, &wfds );

	struct timeval tv;
	tv.tv_sec = send_timeout;
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
	  tv.tv_sec = receive_timeout;
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

    DBG << " <-read " << data.size() << endl;
    std::istringstream datas( data );
    PluginFrame ret( datas );
    DBG << ret << endl;
    return ret;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PluginScript
  //
  ///////////////////////////////////////////////////////////////////

  const pid_t PluginScript::NotConnected( -1 );

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
