#include "private/forkspawnengine_p.h"

#include <sstream>
#include <zypp/base/LogControl.h>
#include <zypp/base/Gettext.h>
#include <zypp/base/IOTools.h>
#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/zyppng/core/String>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <zypp-core/base/CleanerThread_p.h>
#include <zypp-core/base/LogControl.h>

#include <cstdint>
#include <iostream>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pty.h> // openpty
#include <stdlib.h> // setenv
#include <sys/prctl.h> // prctl(), PR_SET_PDEATHSIG

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::exec"


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

void zyppng::AbstractDirectSpawnEngine::mapExtraFds ( int controlFd )
{
  // we might have gotten other FDs to reuse, lets map them to STDERR_FILENO++
  // BUT we need to make sure the fds are not already in the range we need to map them to
  // so we first go over a list and collect those that are safe or move those that are not
  int lastFdToKeep = STDERR_FILENO + _mapFds.size();
  int nextBackupFd = lastFdToKeep + 1; //this we will use to count the fds upwards
  std::vector<int> safeFds;
  for ( auto fd : _mapFds ) {
    // If the fds are larger than the last one we will map to, it is safe.
    if ( fd > lastFdToKeep ) {
      safeFds.push_back( fd );
    } else {
      // we need to map the fd after the set of those we want to keep, but also make sure
      // that we do not close one of those we have already moved or might move
      while (true) {

        int backupTo = nextBackupFd;
        nextBackupFd++;
        const bool isSafe1 = std::find( _mapFds.begin(), _mapFds.end(), backupTo ) == _mapFds.end();
        const bool isSafe2 = std::find( safeFds.begin(), safeFds.end(), backupTo ) == safeFds.end();
        if ( isSafe1 && isSafe2 && ( controlFd == -1 || backupTo != controlFd) ) {
          dup2( fd, backupTo );
          safeFds.push_back( backupTo );
          break;
        }
      }
    }
  }

  // now we have a list of safe fds we need to map to the fd we want them to end up
  int nextFd = STDERR_FILENO;
  for ( auto fd : safeFds ) {
    nextFd++;
    dup2( fd, nextFd );
  }

  const auto &canCloseFd = [&]( int fd ){
    // controlFD has O_CLOEXEC set so it will be cleaned up :)
    if ( controlFd != -1 && controlFd == fd )
      return false;
    // make sure we don't close fd's still need
    if ( fd <= lastFdToKeep )
      return false;
    return true;
  };

  const auto maxFds = ( ::getdtablesize() - 1 );
  //If the rlimits are too high we need to use a different approach
  // in detecting how many fds we need to close, or otherwise we are too slow (bsc#1191324)
  if ( maxFds > 1024 && zypp::PathInfo( "/proc/self/fd" ).isExist() ) {
    
    std::vector<int> fdsToClose;
    fdsToClose.reserve (256);

    zypp::filesystem::dirForEachExt( "/proc/self/fd", [&]( const zypp::Pathname &p, const zypp::filesystem::DirEntry &entry ){
      if ( entry.type != zypp::filesystem::FT_LINK)
        return true;

      const auto &fdVal = zyppng::str::safe_strtonum<int>( entry.name );
      if ( !fdVal || !canCloseFd(*fdVal) )
        return true;

      // we can not call close() directly here because zypp::filesystem::dirForEachExt actually has a fd open on
      // /proc/self/fd that we would close as well. So we just remember which fd's we WOULD close and then do it
      // after iterating
      fdsToClose.push_back (*fdVal);
      return true;
    });
    for ( int cFd : fdsToClose )
      ::close( cFd );
  } else {
    // close all filedescriptors above the last we want to keep
    for ( int i = maxFds; i > lastFdToKeep; --i ) {
      if ( !canCloseFd(i) ) continue;
      ::close( i );
    }
  }
}

bool zyppng::ForkSpawnEngine::start( const char * const *argv, int stdin_fd, int stdout_fd, int stderr_fd )
{
  _pid = -1;
  _exitStatus = 0;
  _execError.clear();
  _executedCommand.clear();
  _args.clear();

  if ( !argv || !argv[0] ) {
    _execError = _("Invalid spawn arguments given.");
    _exitStatus = 128;
    return false;
  }

  const char * chdirTo = nullptr;

  if ( _chroot == "/" ) {
    // If _chroot is '/' do not chroot, but chdir to '/'
    // unless arglist defines another dir.
    chdirTo = "/";
    _chroot = zypp::Pathname();
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

  // we use a control pipe to figure out if the exec actually worked,
  // this is the approach:
  // - create a pipe before forking
  // - block on the read end of the pipe in the parent process
  // - in the child process we write a error tag + errno into the pipe if we encounter any error and exit
  // - If child setup works out, the pipe is auto closed by exec() and the parent process knows from just receiving EOF
  //   that starting the child was successful, otherwise the blocking read in the parent will return with actual data read from the fd
  //   which will contain the error description

  enum class ChildErrType : int8_t {
    NO_ERR,
    CHROOT_FAILED,
    CHDIR_FAILED,
    EXEC_FAILED
  };

  struct ChildErr {
    int childErrno = 0;
    ChildErrType type = ChildErrType::NO_ERR;
  };

  auto controlPipe = Pipe::create( O_CLOEXEC );
  if ( !controlPipe ) {
    _execError = _("Unable to create control pipe.");
    _exitStatus = 128;
    return false;
  }

  pid_t ppid_before_fork = ::getpid();

  // Create module process
  if ( ( _pid = fork() ) == 0 )
  {

    // child process
    controlPipe->unrefRead();

    const auto &writeErrAndExit = [&]( int errCode, ChildErrType type ){
      ChildErr buf {
        errno,
        type
      };

      zypp::io::writeAll( controlPipe->writeFd, &buf, sizeof(ChildErr) );
      _exit ( errCode );
    };

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
            _execError = zypp::str::form( _("Can't chroot to '%s' (%s)."), _chroot.c_str(), strerror(errno).c_str() );
            std::cerr << _execError << std::endl; // After fork log on stderr too
            writeErrAndExit( 128, ChildErrType::CHROOT_FAILED ); // No sense in returning! I am forked away!!
        }
        if ( ! chdirTo )
          chdirTo = "/";
    }

    if ( chdirTo && chdir( chdirTo ) == -1 )
    {
      _execError = _chroot.empty() ? zypp::str::form( _("Can't chdir to '%s' (%s)."), chdirTo, strerror(errno).c_str() )
                                   : zypp::str::form( _("Can't chdir to '%s' inside chroot '%s' (%s)."), chdirTo, _chroot.c_str(), strerror(errno).c_str() );

      std::cerr << _execError << std::endl;// After fork log on stderr too
      writeErrAndExit( 128, ChildErrType::CHDIR_FAILED ); // No sense in returning! I am forked away!!
    }

    // map the extra fds the user might have set
    mapExtraFds( controlPipe->writeFd );

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
        // no sense to write to control pipe, parent is gone
        std::cerr << "PPID changed from "<<ppid_before_fork<<" to "<< ppidNow << std::endl;// After fork log on stderr too
        _exit(128);
      }
    }

    execvp( argv[0], const_cast<char *const *>( argv ) );
    // don't want to get here
    _execError = zypp::str::form( _("Can't exec '%s' (%s)."), _args[0].c_str(), strerror(errno).c_str() );
    std::cerr << _execError << std::endl;// After fork log on stderr too
    writeErrAndExit( 129, ChildErrType::EXEC_FAILED ); // No sense in returning! I am forked away!!
    //////////////////////////////////////////////////////////////////////
  }
  else if ( _pid == -1 )	 // Fork failed, close everything.
  {
    _execError = zypp::str::form( _("Can't fork (%s)."), strerror(errno).c_str() );
    _exitStatus = 127;
    ERR << _execError << std::endl;
    return false;
  }
  else {

    // parent process, fork worked lets wait for the exec to happen
    controlPipe->unrefWrite();

    ChildErr buf;
    const auto res = zypp::io::readAll( controlPipe->readFd, &buf, sizeof(ChildErr) );
    if ( res == zypp::io::ReadAllResult::Eof ) {
      // success!!!!
      DBG << "pid " << _pid << " launched" << std::endl;
      return true;
    } else if ( res == zypp::io::ReadAllResult::Ok ) {
      switch( buf.type ) {
        case ChildErrType::CHDIR_FAILED:
          _execError = zypp::str::form( _("Can't exec '%s', chdir failed (%s)."), _args[0].c_str(), zypp::str::strerror(buf.childErrno).c_str() );
          break;
        case ChildErrType::CHROOT_FAILED:
          _execError = zypp::str::form( _("Can't exec '%s', chroot failed (%s)."), _args[0].c_str(), zypp::str::strerror(buf.childErrno).c_str() );
          break;
        case ChildErrType::EXEC_FAILED:
          _execError = zypp::str::form( _("Can't exec '%s', exec failed (%s)."), _args[0].c_str(), zypp::str::strerror(buf.childErrno).c_str() );
          break;
        // all other cases need to be some sort of error, because we only get data if the exec fails
        default:
          _execError = zypp::str::form( _("Can't exec '%s', unexpected error."), _args[0].c_str() );
          break;
      }
      ERR << "pid " << _pid << " launch failed: " << _execError << std::endl;

      // reap child and collect exit code
      isRunning( true );
      return false;
    } else {
      //reading from the fd failed, this should actually never happen
      ERR << "Reading from the control pipe failed. " << errno << ". This is not supposed to happen ever." << std::endl;
      return isRunning();
    }
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
  _execError.clear();
  _executedCommand.clear();
  _args.clear();

  if ( !argv || !argv[0] ) {
    _execError = _("Invalid spawn arguments given.");
    _exitStatus = 128;
    return false;
  }

  const char * chdirTo = nullptr;

  if ( _chroot == "/" ) {
    // If _chroot is '/' do not chroot, but chdir to '/'
    // unless arglist defines another dir.
    chdirTo = "/";
    _chroot = zypp::Pathname();
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

  for ( char **envPtr = environ; *envPtr != nullptr; envPtr++ )
    envPtrs.push_back( *envPtr );

  envStrs.reserve( _environment.size() );
  envPtrs.reserve( envPtrs.size() + _environment.size() + ( _useDefaultLocale ? 2 : 1 ) );
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

  bool needCallback = !_chroot.empty() || _dieWithParent || _switchPgid || _mapFds.size();

  auto spawnFlags = GSpawnFlags( G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH_FROM_ENVP );
  if ( _mapFds.size() )
    spawnFlags = GSpawnFlags( spawnFlags | G_SPAWN_LEAVE_DESCRIPTORS_OPEN );

  GPid childPid = -1;
  g_autoptr(GError) error = NULL;
  g_spawn_async_with_fds(
        chdirTo,
        (gchar **)argv,
        envPtrs.data(),
        spawnFlags,
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
    _execError = zypp::str::form( _("Can't fork (%s)."), strerror(errno).c_str() );
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
      execError = zypp::str::form( "Can't chroot to '%s' (%s).", d->that->_chroot.c_str(), strerror(errno).c_str() );
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
      execError = doChroot ? zypp::str::form( "Can't chdir to '%s' inside chroot '%s' (%s).", chdir.data(), d->that->_chroot.c_str(), strerror(errno).c_str() )
                           : zypp::str::form( "Can't chdir to '%s' (%s).", chdir.data(), strerror(errno).c_str() );
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

  // map the extra fds the user might have set
  d->that->mapExtraFds();
}
#endif
