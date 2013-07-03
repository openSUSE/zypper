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

#include <cstring> // strsignal
#include <iostream>
#include <sstream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/ExternalProgram.h"

using namespace std;

namespace zypp {

    ExternalProgram::ExternalProgram()
      : use_pty (false)
      , pid( -1 )
    {
    }


    ExternalProgram::ExternalProgram( std::string commandline,
                                      Stderr_Disposition stderr_disp,
                                      bool use_pty,
                                      int stderr_fd,
                                      bool default_locale,
                                      const Pathname & root )
      : use_pty (use_pty)
      , pid( -1 )
    {
      const char *argv[4];
      argv[0] = "/bin/sh";
      argv[1] = "-c";
      argv[2] = commandline.c_str();
      argv[3] = 0;

      const char* rootdir = NULL;
      if(!root.empty() && root != "/")
      {
    	rootdir = root.asString().c_str();
      }
      Environment environment;
      start_program (argv, environment, stderr_disp, stderr_fd, default_locale, rootdir);
    }


    ExternalProgram::ExternalProgram (const Arguments &argv,
                                      Stderr_Disposition stderr_disp,
                                      bool use_pty, int stderr_fd,
                                      bool default_locale,
                                      const Pathname& root)
      : use_pty (use_pty)
      , pid( -1 )
    {
        const char * argvp[argv.size() + 1];
        unsigned c = 0;
        for_( i, argv.begin(), argv.end() )
        {
            argvp[c] = i->c_str();
            ++c;
        }
        argvp[c] = 0;

        Environment environment;
        const char* rootdir = NULL;
        if(!root.empty() && root != "/")
        {
            rootdir = root.asString().c_str();
        }
        start_program (argvp, environment, stderr_disp, stderr_fd, default_locale, rootdir);
    }


    ExternalProgram::ExternalProgram (const Arguments &argv,
                                      const Environment & environment,
                                      Stderr_Disposition stderr_disp,
                                      bool use_pty, int stderr_fd,
                                      bool default_locale,
                                      const Pathname& root)
      : use_pty (use_pty)
      , pid( -1 )
    {
        const char * argvp[argv.size() + 1];
        unsigned c = 0;
        for_( i, argv.begin(), argv.end() )
        {
            argvp[c] = i->c_str();
            ++c;
        }
        argvp[c] = 0;

        const char* rootdir = NULL;
        if(!root.empty() && root != "/")
        {
            rootdir = root.asString().c_str();
        }
        start_program (argvp, environment, stderr_disp, stderr_fd, default_locale, rootdir);

    }




    ExternalProgram::ExternalProgram( const char *const *argv,
                                      Stderr_Disposition stderr_disp,
                                      bool use_pty,
                                      int stderr_fd,
                                      bool default_locale,
                                      const Pathname & root )
      : use_pty (use_pty)
      , pid( -1 )
    {
      const char* rootdir = NULL;
      if(!root.empty() && root != "/")
      {
    	rootdir = root.asString().c_str();
      }
      Environment environment;
      start_program (argv, environment, stderr_disp, stderr_fd, default_locale, rootdir);
    }


    ExternalProgram::ExternalProgram (const char *const *argv, const Environment & environment,
    				  Stderr_Disposition stderr_disp, bool use_pty,
    				  int stderr_fd, bool default_locale,
    				  const Pathname& root)
      : use_pty (use_pty)
      , pid( -1 )
    {
      const char* rootdir = NULL;
      if(!root.empty() && root != "/")
      {
    	rootdir = root.asString().c_str();
      }
      start_program (argv, environment, stderr_disp, stderr_fd, default_locale, rootdir);
    }


    ExternalProgram::ExternalProgram (const char *binpath, const char *const *argv_1,
    				  bool use_pty)
      : use_pty (use_pty)
      , pid( -1 )
    {
      int i = 0;
      while (argv_1[i++])
    	;
      const char *argv[i + 1];
      argv[0] = binpath;
      memcpy (&argv[1], argv_1, (i - 1) * sizeof (char *));
      Environment environment;
      start_program (argv, environment);
    }


    ExternalProgram::ExternalProgram (const char *binpath, const char *const *argv_1, const Environment & environment,
    				  bool use_pty)
      : use_pty (use_pty)
      , pid( -1 )
    {
      int i = 0;
      while (argv_1[i++])
    	;
      const char *argv[i + 1];
      argv[0] = binpath;
      memcpy (&argv[1], argv_1, (i - 1) * sizeof (char *));
      start_program (argv, environment);
    }


    ExternalProgram::~ExternalProgram()
    {
    }


    void
    ExternalProgram::start_program (const char *const *argv, const Environment & environment,
    				Stderr_Disposition stderr_disp,
    				int stderr_fd, bool default_locale, const char* root)
    {
      pid = -1;
      _exitStatus = 0;
      int to_external[2], from_external[2];	// fds for pair of pipes
      int master_tty,	slave_tty;		// fds for pair of ttys

      // retrieve options at beginning of arglist
      const char * redirectStdin = nullptr;	// <[file]
      const char * chdirTo = nullptr;		// #/[path]

      for ( bool strip = false; argv[0]; ++argv )
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

	  case '#':
	    strip = true;
	    if ( argv[0][1] == '/' )	// #/[path]
	      chdirTo = argv[0]+1;
	    break;
	}
	if ( ! strip )
	  break;
      }

      // do not remove the single quotes around every argument, copy&paste of
      // command to shell will not work otherwise!
      {
        stringstream cmdstr;
        for (int i = 0; argv[i]; i++)
        {
          if (i>0) cmdstr << ' ';
          cmdstr << '\'';
          cmdstr << argv[i];
          cmdstr << '\'';
        }
        if ( redirectStdin )
          cmdstr << " < '" << redirectStdin << "'";
        _command = cmdstr.str();
      }
      DBG << "Executing " << _command << endl;


      if (use_pty)
      {
    	// Create pair of ttys
        DBG << "Using ttys for communication with " << argv[0] << endl;
    	if (openpty (&master_tty, &slave_tty, 0, 0, 0) != 0)
    	{
          _execError = str::form( _("Can't open pty (%s)."), strerror(errno) );
          _exitStatus = 126;
          ERR << _execError << endl;
          return;
    	}
      }
      else
      {
    	// Create pair of pipes
    	if (pipe (to_external) != 0 || pipe (from_external) != 0)
    	{
          _execError = str::form( _("Can't open pipe (%s)."), strerror(errno) );
          _exitStatus = 126;
          ERR << _execError << endl;
          return;
    	}
      }

      // Create module process
      if ((pid = fork()) == 0)
      {
        //////////////////////////////////////////////////////////////////////
        // Don't write to the logfile after fork!
        //////////////////////////////////////////////////////////////////////
    	if (use_pty)
    	{
    	    setsid();
    	    if(slave_tty != 1)
    		dup2 (slave_tty, 1);	  // set new stdout
    	    renumber_fd (slave_tty, 0);	  // set new stdin
    	    ::close(master_tty);	  // Belongs to father process

    	    // We currently have no controlling terminal (due to setsid).
    	    // The first open call will also set the new ctty (due to historical
    	    // unix guru knowledge ;-) )

    	    char name[512];
    	    ttyname_r(slave_tty, name, sizeof(name));
    	    ::close(open(name, O_RDONLY));
    	}
    	else
    	{
    	    renumber_fd (to_external[0], 0); // set new stdin
    	    ::close(from_external[0]);	  // Belongs to father process

    	    renumber_fd (from_external[1], 1); // set new stdout
    	    ::close(to_external	 [1]);	  // Belongs to father process
    	}

        if ( redirectStdin )
        {
          ::close( 0 );
          int inp_fd = open( redirectStdin, O_RDONLY );
          dup2( inp_fd, 0 );
        }

    	// Handle stderr
    	if (stderr_disp == Discard_Stderr)
    	{
    	    int null_fd = open("/dev/null", O_WRONLY);
    	    dup2(null_fd, 2);
    	    ::close(null_fd);
    	}
    	else if (stderr_disp == Stderr_To_Stdout)
    	{
    	    dup2(1, 2);
    	}
    	else if (stderr_disp == Stderr_To_FileDesc)
    	{
    	    // Note: We don't have to close anything regarding stderr_fd.
    	    // Our caller is responsible for that.
    	    dup2 (stderr_fd, 2);
    	}

    	for ( Environment::const_iterator it = environment.begin(); it != environment.end(); ++it ) {
    	  setenv( it->first.c_str(), it->second.c_str(), 1 );
    	}

    	if(default_locale)
    		setenv("LC_ALL","C",1);

    	if(root)
    	{
    	    if(chroot(root) == -1)
    	    {
                _execError = str::form( _("Can't chroot to '%s' (%s)."), root, strerror(errno) );
                std::cerr << _execError << endl;// After fork log on stderr too
    		_exit (128);			// No sense in returning! I am forked away!!
    	    }
	    if ( ! chdirTo )
	      chdirTo = "/";
    	}

	if ( chdirTo && chdir( chdirTo ) == -1 )
	{
	  _execError = root ? str::form( _("Can't chdir to '%s' inside chroot '%s' (%s)."), chdirTo, root, strerror(errno) )
			    : str::form( _("Can't chdir to '%s' (%s)."), chdirTo, strerror(errno) );
	  std::cerr << _execError << endl;// After fork log on stderr too
	  _exit (128);			// No sense in returning! I am forked away!!
	}

    	// close all filedesctiptors above stderr
    	for ( int i = ::getdtablesize() - 1; i > 2; --i ) {
    	  ::close( i );
    	}

    	execvp(argv[0], const_cast<char *const *>(argv));
        // don't want to get here
        _execError = str::form( _("Can't exec '%s' (%s)."), argv[0], strerror(errno) );
        std::cerr << _execError << endl;// After fork log on stderr too
        _exit (129);			// No sense in returning! I am forked away!!
        //////////////////////////////////////////////////////////////////////
      }

      else if (pid == -1)	 // Fork failed, close everything.
      {
        _execError = str::form( _("Can't fork (%s)."), strerror(errno) );
        _exitStatus = 127;
        ERR << _execError << endl;

   	if (use_pty) {
    	    ::close(master_tty);
    	    ::close(slave_tty);
    	}
    	else {
    	    ::close(to_external[0]);
    	    ::close(to_external[1]);
    	    ::close(from_external[0]);
    	    ::close(from_external[1]);
    	}
      }

      else {
    	if (use_pty)
    	{
    	    ::close(slave_tty);	       // belongs to child process
    	    inputfile  = fdopen(master_tty, "r");
    	    outputfile = fdopen(master_tty, "w");
    	}
    	else
    	{
    	    ::close(to_external[0]);   // belongs to child process
    	    ::close(from_external[1]); // belongs to child process
    	    inputfile = fdopen(from_external[0], "r");
    	    outputfile = fdopen(to_external[1], "w");
    	}

    	DBG << "pid " << pid << " launched" << endl;

    	if (!inputfile || !outputfile)
    	{
    	    ERR << "Cannot create streams to external program " << argv[0] << endl;
    	    close();
    	}
      }
    }


    int
    ExternalProgram::close()
    {
      if (pid > 0)
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
	      ERR << "select error: " << strerror(errno) << endl;
	      if ( errno != EINTR )
		break;
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
	      if ( ! running() )
		break;
	    }
	  } while ( true );
	}

	// Wait for child to exit
	int ret;
	int status = 0;
	do
	{
	  ret = waitpid(pid, &status, 0);
	}
	while (ret == -1 && errno == EINTR);

	if (ret != -1)
	{
	 _exitStatus = checkStatus( status );
	}
	pid = -1;
      }

      return _exitStatus;
    }


    int ExternalProgram::checkStatus( int status )
    {
      if (WIFEXITED (status))
      {
    	status = WEXITSTATUS (status);
    	if(status)
    	{
    	    DBG << "Pid " << pid << " exited with status " << status << endl;
            _execError = str::form( _("Command exited with status %d."), status );
    	}
    	else
    	{
    	    // if 'launch' is logged, completion should be logged,
    	    // even if successfull.
    	    DBG << "Pid " << pid << " successfully completed" << endl;
            _execError.clear(); // empty if running or successfully completed
    	}
      }
      else if (WIFSIGNALED (status))
      {
    	status = WTERMSIG (status);
    	WAR << "Pid " << pid << " was killed by signal " << status
    		<< " (" << strsignal(status);
    	if (WCOREDUMP (status))
    	{
    	    WAR << ", core dumped";
    	}
    	WAR << ")" << endl;
        _execError = str::form( _("Command was killed by signal %d (%s)."), status, strsignal(status) );
    	status+=128;
      }
      else {
    	ERR << "Pid " << pid << " exited with unknown error" << endl;
        _execError = _("Command exited with unknown error.");
      }

      return status;
    }

    bool
    ExternalProgram::kill()
    {
      if (pid > 0)
      {
    	::kill(pid, SIGKILL);
    	close();
      }
      return true;
    }


    bool
    ExternalProgram::running()
    {
      if ( pid < 0 ) return false;

      int status = 0;
      int p = waitpid( pid, &status, WNOHANG );
      switch ( p )
        {
        case -1:
          ERR << "waitpid( " << pid << ") returned error '" << strerror(errno) << "'" << endl;
          return false;
          break;
        case 0:
          return true; // still running
          break;
        }

      // Here: completed...
      _exitStatus = checkStatus( status );
      pid = -1;
      return false;
    }

    // origfd will be accessible as newfd and closed (unless they were equal)
    void ExternalProgram::renumber_fd (int origfd, int newfd)
    {
      // It may happen that origfd is already the one we want
      // (Although in our circumstances, that would mean somebody has closed
      // our stdin or stdout... weird but has appened to Cray, #49797)
      if (origfd != newfd)
      {
    	dup2 (origfd, newfd);
    	::close (origfd);
      }
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

    namespace _ExternalProgram
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
    }

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
