#include "private/forkspawnengine_p.h"

#include <sstream>
#include <zypp/base/LogControl.h>
#include <zypp/base/Gettext.h>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <zypp-core/base/CleanerThread_p.h>

#include <iostream>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pty.h> // openpty
#include <stdlib.h> // setenv
#include <sys/prctl.h> // prctl(), PR_SET_PDEATHSIG

zyppng::AbstractDirectSpawnEngine::~AbstractDirectSpawnEngine()
{
  if ( AbstractDirectSpawnEngine::isRunning() ) {
    // we got destructed while the external process is still alive
    // make sure the zombie is cleaned up once it exits
    zypp::CleanerThread::watchPID( _pid );
  }
}

bool zyppng::AbstractDirectSpawnEngine::isRunning( bool wait )
{
  if ( _pid < 0 ) return false;

  int status = 0;
  int p = zyppng::eintrSafeCall( ::waitpid, _pid, &status, wait ? 0 : WNOHANG );
  switch ( p )
    {
    case -1:
      ERR << "waitpid( " << _pid << ") returned error '" << strerror(errno) << "'" << std::endl;
      return false;
      break;
    case 0:
      return true; // still running
      break;
    }

  // Here: completed...
  _exitStatus = checkStatus( status );
  _pid = -1;
  return false;
}

bool zyppng::ForkSpawnEngine::start( const char * const *argv, int stdin_fd, int stdout_fd, int stderr_fd )
{
  _pid = -1;
  _exitStatus = 0;

  const char * chdirTo = nullptr;

  if ( !argv || !argv[0] ) {
    _execError = _("Invalid spawn arguments given.");
    _exitStatus = 128;
    return false;
  }

  if ( !_chroot.empty() )
  {
    const auto chroot = _chroot.c_str();
    if ( chroot[0] == '\0' )
    {
      _chroot = zypp::Pathname();	// ignore empty _chroot
    }
    else if ( chroot[0] == '/' && chroot[1] == '\0' )
    {
      // If _chroot is '/' do not chroot, but chdir to '/'
      // unless arglist defines another dir.
      chdirTo = "/";
      _chroot = zypp::Pathname();
    }
  }

  if ( !_workingDirectory.empty() )
    chdirTo = _workingDirectory.c_str();

  // do not remove the single quotes around every argument, copy&paste of
  // command to shell will not work otherwise!
  {
    _args.clear();
    std::stringstream cmdstr;
    for (int i = 0; argv[i]; i++) {
      if ( i != 0 ) cmdstr << ' ';
      cmdstr << '\'';
      cmdstr << argv[i];
      cmdstr << '\'';
      _args.push_back( argv[i] );
    }
    _executedCommand = cmdstr.str();
  }
  DBG << "Executing" << ( _useDefaultLocale?"[C] ":" ") << _executedCommand << std::endl;


  pid_t ppid_before_fork = ::getpid();

  // Create module process
  if ( ( _pid = fork() ) == 0 )
  {
    //////////////////////////////////////////////////////////////////////
    // Don't write to the logfile after fork!
    //////////////////////////////////////////////////////////////////////
    if ( _use_pty )
    {
        setsid();
        dup2 ( stdout_fd, 1);	  // set new stdout
        dup2 ( stdin_fd , 0);	  // set new stdin

        // We currently have no controlling terminal (due to setsid).
        // The first open call will also set the new ctty (due to historical
        // unix guru knowledge ;-) )

        char name[512];
        ttyname_r( stdout_fd , name, sizeof(name) );
        ::close(open(name, O_RDONLY));
    }
    else
    {
        if ( _switchPgid )
          setpgid( 0, 0);
        if ( stdin_fd != -1 )
          dup2 ( stdin_fd, 0); // set new stdin
        if ( stdout_fd != -1 )
          dup2 ( stdout_fd, 1); // set new stdout
    }

    // Handle stderr
    if ( stderr_fd != -1 )
      dup2 ( stderr_fd, 2); // set new stderr

    for ( Environment::const_iterator it = _environment.begin(); it != _environment.end(); ++it ) {
      setenv( it->first.c_str(), it->second.c_str(), 1 );
    }

    if( _useDefaultLocale )
      setenv("LC_ALL","C",1);

    if( !_chroot.empty() )
    {
        if( ::chroot(_chroot.c_str()) == -1)
        {
            _execError = zypp::str::form( _("Can't chroot to '%s' (%s)."), _chroot.c_str(), strerror(errno) );
            std::cerr << _execError << std::endl; // After fork log on stderr too
            _exit (128);			  // No sense in returning! I am forked away!!
        }
        if ( ! chdirTo )
          chdirTo = "/";
    }

    if ( chdirTo && chdir( chdirTo ) == -1 )
    {
      _execError = _chroot.empty() ? zypp::str::form( _("Can't chdir to '%s' (%s)."), chdirTo, strerror(errno) )
                                   : zypp::str::form( _("Can't chdir to '%s' inside chroot '%s' (%s)."), chdirTo, _chroot.c_str(), strerror(errno) );

      std::cerr << _execError << std::endl;// After fork log on stderr too
      _exit (128);			// No sense in returning! I am forked away!!
    }

    // close all filedesctiptors above stderr
    for ( int i = ::getdtablesize() - 1; i > 2; --i ) {
      ::close( i );
    }

    if ( _dieWithParent ) {
      // process dies with us
      int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
      if (r == -1) {
        //ignore if it did not work, worst case the process lives on after the parent dies
        std::cerr << "Failed to set PR_SET_PDEATHSIG" << std::endl;// After fork log on stderr too
      }

      // test in case the original parent exited just
      // before the prctl() call
      pid_t ppidNow = getppid();
      if (ppidNow != ppid_before_fork) {
        std::cerr << "PPID changed from "<<ppid_before_fork<<" to "<< ppidNow << std::endl;// After fork log on stderr too
        _exit(128);
      }
    }

    execvp( argv[0], const_cast<char *const *>( argv ) );
    // don't want to get here
    _execError = zypp::str::form( _("Can't exec '%s' (%s)."), _args[0].c_str(), strerror(errno) );
    std::cerr << _execError << std::endl;// After fork log on stderr too
    _exit (129);			// No sense in returning! I am forked away!!
    //////////////////////////////////////////////////////////////////////
  }
  else if ( _pid == -1 )	 // Fork failed, close everything.
  {
    _execError = zypp::str::form( _("Can't fork (%s)."), strerror(errno) );
    _exitStatus = 127;
    ERR << _execError << std::endl;
    return false;
  }
  else {
    DBG << "pid " << _pid << " launched" << std::endl;
  }
  return true;
}

bool zyppng::ForkSpawnEngine::usePty() const
{
  return _use_pty;
}

void zyppng::ForkSpawnEngine::setUsePty( const bool set )
{
  _use_pty = set;
}


#if ZYPP_HAS_GLIBSPAWNENGINE

struct GLibForkData {
  zyppng::GlibSpawnEngine *that = nullptr;
  pid_t pidParent = -1;
};

bool zyppng::GlibSpawnEngine::start( const char * const *argv, int stdin_fd, int stdout_fd, int stderr_fd )
{
  _pid = -1;
  _exitStatus = 0;

  if ( !argv || !argv[0] ) {
    _execError = _("Invalid spawn arguments given.");
    _exitStatus = 128;
    return false;
  }

  const char * chdirTo = nullptr;

  if ( !_chroot.empty() ) {
    const auto chroot = _chroot.c_str();
    if ( chroot[0] == '\0' )
    {
      _chroot = zypp::Pathname();	// ignore empty _chroot
    }
    else if ( chroot[0] == '/' && chroot[1] == '\0' )
    {
      // If _chroot is '/' do not chroot, but chdir to '/'
      // unless arglist defines another dir.
      chdirTo = "/";
      _chroot = zypp::Pathname();
    }
  }

  if ( !_workingDirectory.empty() )
    chdirTo = _workingDirectory.c_str();

  // do not remove the single quotes around every argument, copy&paste of
  // command to shell will not work otherwise!
  {
    _args.clear();
    std::stringstream cmdstr;
    for (int i = 0; argv[i]; i++) {
      if ( i != 0 ) cmdstr << ' ';
      cmdstr << '\'';
      cmdstr << argv[i];
      cmdstr << '\'';
      _args.push_back( argv[i] );
    }
    _executedCommand = cmdstr.str();
  }
  DBG << "Executing" << ( _useDefaultLocale?"[C] ":" ") << _executedCommand << std::endl;

  // build the env var ptrs
  std::vector<std::string> envStrs;
  std::vector<gchar *> envPtrs;
  envStrs.reserve( _environment.size() );
  envPtrs.reserve( _environment.size() + ( _useDefaultLocale ? 2 : 1 ) );
  for ( const auto &env : _environment ) {
    envStrs.push_back( env.first + "=" + env.second );
    envPtrs.push_back( envStrs.back().data() );
  }
  if ( _useDefaultLocale ) {
    envStrs.push_back( "LC_ALL=C" );
    envPtrs.push_back( envStrs.back().data() );
  }
  envPtrs.push_back( nullptr );

  GLibForkData data;
  data.that = this;
  data.pidParent = ::getpid();

  bool needCallback = !_chroot.empty() || _dieWithParent || _switchPgid;

  GPid childPid = -1;
  g_autoptr(GError) error = NULL;
  g_spawn_async_with_fds(
        chdirTo,
        (gchar **)argv,
        envPtrs.data(),
        GSpawnFlags(G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH_FROM_ENVP),
        needCallback ? &GlibSpawnEngine::glibSpawnCallback : nullptr,
        needCallback ? &data : nullptr,
        &childPid,
        stdin_fd,  //in
        stdout_fd,  //out
        stderr_fd,  //err
        &error
  );

  if ( !error ) {
    _pid = childPid;
  } else {
    _execError = zypp::str::form( _("Can't fork (%s)."), strerror(errno) );
    _exitStatus = 127;
    ERR << _execError << std::endl;
    return false;
  }
  return true;
}

void zyppng::GlibSpawnEngine::glibSpawnCallback(void *data)
{
  GLibForkData *d = reinterpret_cast<GLibForkData *>(data);

  bool doChroot = !d->that->_chroot.empty();

  std::string execError;

  if ( d->that->_switchPgid )
    setpgid( 0, 0);

  if ( doChroot ) {
    if ( ::chroot( d->that->_chroot.c_str() ) == -1 ) {
      execError = zypp::str::form( "Can't chroot to '%s' (%s).", d->that->_chroot.c_str(), strerror(errno) );
      std::cerr << execError << std::endl;// After fork log on stderr too
      _exit (128); // No sense in returning! I am forked away!!
    }

    std::string chdir; //if we are in chroot we need to chdir again
    if ( d->that->_workingDirectory.empty() ) {
      chdir = "/";
    } else {
      chdir = d->that->_workingDirectory.asString();
    }

    if ( !chdir.empty() && ::chdir( chdir.data() ) == -1 )
    {
      execError = doChroot ? zypp::str::form( "Can't chdir to '%s' inside chroot '%s' (%s).", chdir.data(), d->that->_chroot.c_str(), strerror(errno) )
                           : zypp::str::form( "Can't chdir to '%s' (%s).", chdir.data(), strerror(errno) );
      std::cerr << execError << std::endl; // After fork log on stderr too
      _exit (128);			     // No sense in returning! I am forked away!!
    }

  }

  if ( d->that->_dieWithParent ) {
    // process dies with us
    int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (r == -1) {
      //ignore if it did not work, worst case the process lives on after the parent dies
      std::cerr << "Failed to set PR_SET_PDEATHSIG" << std::endl;// After fork log on stderr too
    }

    // test in case the original parent exited just
    // before the prctl() call
    pid_t ppidNow = getppid();
    if (ppidNow != d->pidParent ) {
      std::cerr << "PPID changed from "<<d->pidParent<<" to "<< ppidNow << std::endl;// After fork log on stderr too
      _exit(128);
    }
  }
}
#endif
