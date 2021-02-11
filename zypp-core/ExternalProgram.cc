/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ExternalProgram.cc
*/

#define _GNU_SOURCE 1 // for ::getline

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pty.h> // openpty
#include <stdlib.h> // setenv
#include <sys/prctl.h> // prctl(), PR_SET_PDEATHSIG

#include <cstring> // strsignal
#include <iostream>
#include <sstream>

#include <zypp-core/AutoDispose.h>
#include <zypp-core/base/Logger.h>
#include <zypp-core/base/String.h>
#include <zypp-core/base/Gettext.h>
#include <zypp-core/ExternalProgram.h>
#include <zypp-core/base/CleanerThread_p.h>

#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <zypp-core/zyppng/io/private/forkspawnengine_p.h>

using std::endl;

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::exec"

namespace zypp {

    namespace  {

      enum class SpawnEngine {
        GSPAWN,
        PFORK
      };

      SpawnEngine initEngineFromEnv () {
        const std::string fBackend ( zypp::str::asString( ::getenv("ZYPP_FORK_BACKEND") ) );
        if ( fBackend.empty() || fBackend == "auto" || fBackend == "pfork" ) {
          DBG << "Starting processes via posix fork" << std::endl;
          return SpawnEngine::PFORK;
        } else if ( fBackend == "gspawn" ) {
          DBG << "Starting processes via glib spawn" << std::endl;
          return SpawnEngine::GSPAWN;
        }

        DBG << "Falling back to starting process via posix fork" << std::endl;
        return SpawnEngine::PFORK;
      }

      SpawnEngine engineFromEnv () {
        static const SpawnEngine eng = initEngineFromEnv();
        return eng;
      }

    }


    ExternalProgram::ExternalProgram()
    {}


    ExternalProgram::ExternalProgram( std::string commandline,
                                      Stderr_Disposition stderr_disp,
                                      bool use_pty,
                                      int stderr_fd,
                                      bool default_locale,
                                      const Pathname & root )
    {
      const char *argv[4];
      argv[0] = "/bin/sh";
      argv[1] = "-c";
      argv[2] = commandline.c_str();
      argv[3] = 0;

      start_program( argv, Environment(), stderr_disp, stderr_fd, default_locale, root.c_str(), false, false, use_pty );
    }

    ExternalProgram::ExternalProgram( const Arguments & argv,
                                      Stderr_Disposition stderr_disp,
                                      bool use_pty,
				      int stderr_fd,
                                      bool default_locale,
                                      const Pathname & root )
    {
      const char * argvp[argv.size() + 1];
      unsigned c = 0;
      for_( i, argv.begin(), argv.end() )
      {
	argvp[c] = i->c_str();
	++c;
      }
      argvp[c] = 0;

      start_program( argvp, Environment(), stderr_disp, stderr_fd, default_locale, root.c_str(), false, false, use_pty );
    }

    ExternalProgram::ExternalProgram( const Arguments & argv,
                                      const Environment & environment,
                                      Stderr_Disposition stderr_disp,
                                      bool use_pty,
				      int stderr_fd,
                                      bool default_locale,
				      const Pathname & root )
    {
      const char * argvp[argv.size() + 1];
      unsigned c = 0;
      for_( i, argv.begin(), argv.end() )
      {
	argvp[c] = i->c_str();
	++c;
      }
      argvp[c] = 0;

      start_program( argvp, environment, stderr_disp, stderr_fd, default_locale, root.c_str(), false, false, use_pty );
    }

    ExternalProgram::ExternalProgram( const char *const *argv,
                                      Stderr_Disposition stderr_disp,
                                      bool use_pty,
                                      int stderr_fd,
                                      bool default_locale,
                                      const Pathname & root )
    {
      start_program( argv, Environment(), stderr_disp, stderr_fd, default_locale, root.c_str(), false, false, use_pty );
    }

    ExternalProgram::ExternalProgram( const char *const * argv,
				      const Environment & environment,
				      Stderr_Disposition stderr_disp,
				      bool use_pty,
				      int stderr_fd,
				      bool default_locale,
				      const Pathname & root )
    {
      start_program( argv, environment, stderr_disp, stderr_fd, default_locale, root.c_str(), false, false, use_pty );
    }


    ExternalProgram::ExternalProgram( const char *binpath,
				      const char *const *argv_1,
				      bool use_pty )
    {
      int i = 0;
      while (argv_1[i++])
    	;
      const char *argv[i + 1];
      argv[0] = binpath;
      memcpy( &argv[1], argv_1, (i - 1) * sizeof (char *) );
      start_program( argv, Environment(), Normal_Stderr, 1, false, NULL, false, false, use_pty );
    }

    ExternalProgram::ExternalProgram( const char *binpath,
				      const char *const *argv_1,
				      const Environment & environment,
				      bool use_pty )
    {
      int i = 0;
      while (argv_1[i++])
    	;
      const char *argv[i + 1];
      argv[0] = binpath;
      memcpy( &argv[1], argv_1, (i - 1) * sizeof (char *) );
      start_program( argv, environment, Normal_Stderr, 1, false, NULL, false, false, use_pty );
    }

    ExternalProgram::~ExternalProgram()
    { }



    void ExternalProgram::start_program( const char *const *argv,
					 const Environment & environment,
					 Stderr_Disposition stderr_disp,
					 int stderr_fd,
					 bool default_locale,
					 const char * root , bool switch_pgid, bool die_with_parent , bool usePty )
    {
      if ( _backend )
        return;

      // usePty is only supported by the forking backend
      if ( usePty ) {
        DBG << "usePty was set, forcing the ForkSpawnEngine to start external processes" << std::endl;
        _backend = std::make_unique<zyppng::ForkSpawnEngine>();
        static_cast<zyppng::ForkSpawnEngine&>(*_backend).setUsePty( true );
      } else {
        switch ( engineFromEnv() ) {
          case SpawnEngine::GSPAWN:
            _backend = std::make_unique<zyppng::GlibSpawnEngine>();
          case SpawnEngine::PFORK:
          default:
            _backend = std::make_unique<zyppng::ForkSpawnEngine>();
        }
      }

      // retrieve options at beginning of arglist
      const char * redirectStdin = nullptr;	// <[file]
      const char * redirectStdout = nullptr;	// >[file]
      const char * chdirTo = nullptr;		// #/[path]

      if ( root )
      {
	if ( root[0] == '\0' )
	{
	  root = nullptr;	// ignore empty root
	}
	else if ( root[0] == '/' && root[1] == '\0' )
	{
	  // If root is '/' do not chroot, but chdir to '/'
	  // unless arglist defines another dir.
	  chdirTo = "/";
	  root = nullptr;
	}
      }

      for ( bool strip = false; argv[0] != nullptr; ++argv )
      {
	strip = false;
	switch ( argv[0][0] )
	{
	  case '<':
	    strip = true;
	    redirectStdin = argv[0]+1;
	    if ( *redirectStdin == '\0' )
	      redirectStdin = "/dev/null";
	    break;

	  case '>':
	    strip = true;
	    redirectStdout = argv[0]+1;
	    if ( *redirectStdout == '\0' )
	      redirectStdout = "/dev/null";
	    break;

	  case '#':
	    strip = true;
	    if ( argv[0][1] == '/' )	// #/[path]
	      chdirTo = argv[0]+1;
	    break;
	}
	if ( ! strip )
	  break;
      }

      // those are the FDs that the new process will receive
      // AutoFD will take care of closing them on our side
      zypp::AutoFD stdinFd  = -1;
      zypp::AutoFD stdoutFd = -1;
      zypp::AutoFD stderrFd = -1;

      // those are the fds we will keep, we put them into autofds in case
      // we need to return early without actually spawning the new process
      zypp::AutoFD childStdinParentFd = -1;
      zypp::AutoFD childStdoutParentFd = -1;

      if ( usePty )
      {

        int master_tty,	slave_tty;		// fds for pair of ttys

    	// Create pair of ttys
        DBG << "Using ttys for communication with " << argv[0] << endl;
    	if (openpty (&master_tty, &slave_tty, 0, 0, 0) != 0)
    	{
          _backend->setExecError( str::form( _("Can't open pty (%s)."), strerror(errno) ) );
          _backend->setExitStatus( 126 );
          ERR << _backend->execError() << endl;
          return;
    	}

        stdinFd  = slave_tty;
        stdoutFd = slave_tty;
        childStdinParentFd = master_tty;
        childStdoutParentFd = master_tty;
      }
      else
      {
        if ( redirectStdin ) {
          stdinFd  = open( redirectStdin, O_RDONLY );
        } else {
          int to_external[2];
          if ( pipe (to_external) != 0 )
          {
            _backend->setExecError( str::form( _("Can't open pipe (%s)."), strerror(errno) ) );
            _backend->setExitStatus( 126 );
            ERR << _backend->execError() << endl;
            return;
          }
          stdinFd            = to_external[0];
          childStdinParentFd = to_external[1];
        }

        if ( redirectStdout ) {
          stdoutFd = open( redirectStdout, O_WRONLY|O_CREAT|O_APPEND, 0600 );
        } else {

          int from_external[2];
          // Create pair of pipes
          if ( pipe (from_external) != 0 )
          {
            _backend->setExecError( str::form( _("Can't open pipe (%s)."), strerror(errno) ) );
            _backend->setExitStatus( 126 );
            ERR << _backend->execError() << endl;
            return;
          }
          stdoutFd = from_external[1];
          childStdoutParentFd = from_external[0];
        }
      }

      // Handle stderr
      if (stderr_disp == Discard_Stderr)
      {
        stderrFd = open("/dev/null", O_WRONLY);
      }
      else if (stderr_disp == Stderr_To_Stdout)
      {
        stderrFd = *stdoutFd;
        //no double close
        stderrFd.resetDispose();
      }
      else if (stderr_disp == Stderr_To_FileDesc)
      {
        // Note: We don't have to close anything regarding stderr_fd.
        // Our caller is responsible for that.
        stderrFd = stderr_fd;
        stderrFd.resetDispose();
      }

      if ( root )
        _backend->setChroot( root );
      if ( chdirTo )
        _backend->setWorkingDirectory( chdirTo );

      _backend->setDieWithParent( die_with_parent );
      _backend->setSwitchPgid( switch_pgid );
      _backend->setEnvironment( environment );
      _backend->setUseDefaultLocale( default_locale );

      if ( _backend->start( argv, stdinFd, stdoutFd, stderrFd ) ) {

        inputfile  = fdopen( childStdoutParentFd, "r" );
        childStdoutParentFd.resetDispose();
        outputfile = fdopen( childStdinParentFd, "w" );
        childStdinParentFd.resetDispose();

    	if (!inputfile || !outputfile)
    	{
    	    ERR << "Cannot create streams to external program " << argv[0] << endl;
            ExternalProgram::close();
    	}
      } else {
        // Fork failed, exit code and status was set by backend
        return;
      }
    }

    int
    ExternalProgram::close()
    {
      if ( !_backend ) {
        ExternalDataSource::close();
        return -1;
      }

      if ( _backend->isRunning() )
      {
	if ( inputFile() )
	{
	  // Discard any output instead of closing the pipe,
	  // but watch out for the command exiting while some
	  // subprocess keeps the filedescriptor open.
	  setBlocking( false );
	  FILE * inputfile = inputFile();
	  int    inputfileFd = ::fileno( inputfile );
	  long   delay = 0;
	  do
	  {
	    /* Watch inputFile to see when it has input. */
	    fd_set rfds;
	    FD_ZERO( &rfds );
	    FD_SET( inputfileFd, &rfds );

	    /* Wait up to 1 seconds. */
	    struct timeval tv;
	    tv.tv_sec  = (delay < 0 ? 1 : 0);
	    tv.tv_usec = (delay < 0 ? 0 : delay*100000);
	    if ( delay >= 0 && ++delay > 9 )
	      delay = -1;
	    int retval = select( inputfileFd+1, &rfds, NULL, NULL, &tv );

	    if ( retval == -1 )
	    {
              if ( errno != EINTR ) {
                ERR << "select error: " << strerror(errno) << endl;
		break;
              }
	    }
	    else if ( retval )
	    {
	      // Data is available now.
	      static size_t linebuffer_size = 0;      // static because getline allocs
	      static char * linebuffer = 0;           // and reallocs if buffer is too small
	      getline( &linebuffer, &linebuffer_size, inputfile );
	      // ::feof check is important as select returns
	      // positive if the file was closed.
	      if ( ::feof( inputfile ) )
		break;
	      clearerr( inputfile );
	    }
	    else
	    {
	      // No data within time.
	      if ( ! _backend->isRunning() )
		break;
	    }
	  } while ( true );
	}

	// wait for the process to end)
	_backend->isRunning( true );
      }

      ExternalDataSource::close();
      return _backend->exitStatus();
    }

    bool
    ExternalProgram::kill()
    {
      if ( _backend && _backend->isRunning() )
      {
    	::kill( _backend->pid(), SIGKILL);
    	close();
      }
      return true;
    }

    bool ExternalProgram::kill(int sig)
    {
      if ( _backend && _backend->isRunning()  )
      {
        ::kill( _backend->pid(), sig );
      }
      return true;
    }

    bool
    ExternalProgram::running()
    {
      if ( !_backend ) return false;
      return _backend->isRunning();
    }

    pid_t ExternalProgram::getpid()
    {
      if ( !_backend )
        return -1;
      return _backend->pid();
    }

    const std::string &ExternalProgram::command() const
    {
      if ( !_backend ) {
        static std::string empty;
        return empty;
      }
      return _backend->executedCommand();
    }

    const std::string &ExternalProgram::execError() const
    {
      if ( !_backend ) {
        static std::string empty;
        return empty;
      }
      return _backend->execError();
    }

    // origfd will be accessible as newfd and closed (unless they were equal)
    void ExternalProgram::renumber_fd (int origfd, int newfd)
    {
      return zyppng::renumberFd( origfd, newfd );
    }

    std::ostream & ExternalProgram::operator>>( std::ostream & out_r )
    {
      setBlocking( true );
      for ( std::string line = receiveLine(); line.length(); line = receiveLine() )
        out_r << line;
      return out_r;
    }

    //////////////////////////////////////////////////////////////////////
    //
    // class ExternalProgramWithStderr
    //
    //////////////////////////////////////////////////////////////////////

    namespace externalprogram
    {
      EarlyPipe::EarlyPipe()
      {
	_fds[R] = _fds[W] = -1;
#ifdef HAVE_PIPE2
	::pipe2( _fds, O_NONBLOCK );
#else
        ::pipe( _fds );
        ::fcntl(_fds[R], F_SETFD, O_NONBLOCK );
        ::fcntl(_fds[W], F_SETFD, O_NONBLOCK );
#endif
	_stderr = ::fdopen( _fds[R], "r" );
      }

      EarlyPipe::~EarlyPipe()
      {
	closeW();
	if ( _stderr )
	  ::fclose( _stderr );
      }
    } // namespace externalprogram

    bool ExternalProgramWithStderr::stderrGetUpTo( std::string & retval_r, const char delim_r, bool returnDelim_r )
    {
      if ( ! _stderr )
	return false;
      if ( delim_r && ! _buffer.empty() )
      {
	// check for delim already in buffer
	std::string::size_type pos( _buffer.find( delim_r ) );
	if ( pos != std::string::npos )
	{
	  retval_r = _buffer.substr( 0, returnDelim_r ? pos+1 : pos );
	  _buffer.erase( 0, pos+1 );
	  return true;
	}
      }
      ::clearerr( _stderr );
      do {
	int ch = fgetc( _stderr );
	if ( ch != EOF )
	{
	  if ( ch != delim_r || ! delim_r )
	    _buffer.push_back( ch );
	  else
	  {
	    if ( returnDelim_r )
	      _buffer.push_back( delim_r );
	    break;
	  }
	}
	else if ( ::feof( _stderr ) )
	{
	  if ( _buffer.empty() )
	    return false;
	  break;
	}
	else if ( errno != EINTR )
	  return false;
      } while ( true );
      // HERE: we left after readig at least one char (\n)
      retval_r.swap( _buffer );
      _buffer.clear();
      return true;
    }


} // namespace zypp
