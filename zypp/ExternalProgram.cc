/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ExternalProgram.cc
 *
 * \todo replace by Blocxx
 *
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

#include "zypp/base/Logger.h"
#include "zypp/ExternalProgram.h"

using namespace std;

namespace zypp {

    ExternalProgram::ExternalProgram (string commandline,
    				  Stderr_Disposition stderr_disp, bool use_pty,
    				  int stderr_fd, bool default_locale,
    				  const Pathname& root)
      : use_pty (use_pty)
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
    
    
    ExternalProgram::ExternalProgram (const char *const *argv,
    				  Stderr_Disposition stderr_disp, bool use_pty,
    				  int stderr_fd, bool default_locale,
    				  const Pathname& root)
      : use_pty (use_pty)
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
      int to_external[2], from_external[2];  // fds for pair of pipes
      int master_tty,	slave_tty;	   // fds for pair of ttys
    
      if (use_pty)
      {
    	// Create pair of ttys
          DBG << "Using ttys for communication with " << argv[0] << endl;
    	if (openpty (&master_tty, &slave_tty, 0, 0, 0) != 0)
    	{
    	    ERR << "openpty failed" << endl;
    	    return;
    	}
      }
      else
      {
    	// Create pair of pipes
    	if (pipe (to_external) != 0 || pipe (from_external) != 0)
    	{
    	    ERR << "pipe failed" << endl;
    	    return;
    	}
      }
    
      // do not remove the single quotes around every argument, copy&paste of
      // command to shell will not work otherwise!
      DBG << "Executing ";
      for (int i = 0; argv[i]; i++)
      {
    	if (i>0) DBG << ' ';
    	DBG << '\'';
    	DBG << argv[i];
    	DBG << '\'';
      }
      DBG << endl;
    
      // Create module process
      if ((pid = fork()) == 0)
      {
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
    		ERR << "chroot to " << root << " failed: " << strerror(errno) << endl;
    		_exit (3);			// No sense in returning! I am forked away!!
    	    }
    	    if(chdir("/") == -1)
    	    {
    		ERR << "chdir to / inside chroot failed: " << strerror(errno) << endl;
    		_exit (4);			// No sense in returning! I am forked away!!
    	    }
    	}
    
    	// close all filedesctiptors above stderr
    	for ( int i = ::getdtablesize() - 1; i > 2; --i ) {
    	  ::close( i );
    	}
    
    	execvp(argv[0], const_cast<char *const *>(argv));
    	ERR << "Cannot execute external program "
    		 << argv[0] << ":" << strerror(errno) << endl;
    	_exit (5);			// No sense in returning! I am forked away!!
      }
    
      else if (pid == -1)	 // Fork failed, close everything.
      {
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
    	ERR << "Cannot fork " << strerror(errno) << endl;
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
    	ExternalDataSource::close();
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
    	    status = checkStatus( status );
    	}
          pid = -1;
          return status;
      }
      else
      {
          return _exitStatus;
      }
    }
    
    
    int ExternalProgram::checkStatus( int status )
    {
      if (WIFEXITED (status))
      {
    	status = WEXITSTATUS (status);
    	if(status)
    	{
    	    DBG << "pid " << pid << " exited with status " << status << endl;
    	}
    	else
    	{
    	    // if 'launch' is logged, completion should be logged,
    	    // even if successfull.
    	    DBG << "pid " << pid << " successfully completed" << endl;
    	}
      }
      else if (WIFSIGNALED (status))
      {
    	status = WTERMSIG (status);
    	WAR << "pid " << pid << " was killed by signal " << status
    		<< " (" << strsignal(status);
    	if (WCOREDUMP (status))
    	{
    	    WAR << ", core dumped";
    	}
    	WAR << ")" << endl;
    	status+=128;
      }
      else {
    	ERR << "pid " << pid << " exited with unknown error" << endl;
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
      if ( p < 0 ) return false;
    
      status = checkStatus( status );
    
      if ( status == 0 )
      {
          return true;
      }
      else
      {
          _exitStatus = status;
          pid = -1;
          return false;
      }
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
    
} // namespace zypp
