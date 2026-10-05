/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_UTILS_RUNCOMMAND_H
#define ZYPPER_UTILS_RUNCOMMAND_H

#include <unistd.h>
#include <sys/wait.h>

#include <vector>
#include <string>
#include <initializer_list>

#include <zypp-core/base/LogTools.h>

///////////////////////////////////////////////////////////////////
/// \class RunCommand
/// \brief Run external command
/// Simple version without redirections. Used for subcommands.
///////////////////////////////////////////////////////////////////
class RunCommand
{
public:
  typedef std::vector<std::string> Arglist;

public:
  RunCommand() {}
  RunCommand( const Arglist & args_r )	: _args( args_r ) {}
  RunCommand( Arglist && args_r ) 		: _args( std::move(args_r) ) {}
  RunCommand( const std::initializer_list<std::string> & args_r ) : _args( std::move(args_r) ) {}

public:
  /** The command (quoted if empty or contains whitespace) */
  std::string command() const
  { std::string ret; if ( !_args.empty() ) ret = quotearg(_args[0]); return ret; }

  /** The command + args (each quoted if empty or contains whitespace) */
  std::string commandline() const
  {
    std::string ret;
    if ( !_args.empty() )
    {
      for ( const auto & arg : _args )
      {
        if ( !ret.empty() ) ret += " ";
        ret += quotearg( arg );
      }
    }
    return ret;
  }

public:
  /** Run command and return it's exit status. */
  int run();

  /** Commands exit status; -1 while running or if wait failed. */
  int exitStatus() const
  { return _exitStatus; }

  /** Execution errors like failed fork/exec; empty while running or on success. */
  std::string execError() const
  { return _execError; }

private:
  /** Quoted \a arg_r if it contains whitespace */
  std::string quotearg( const std::string & arg_r ) const
  {
    static str::Format fmt("'%1%'");
    return( fmt % arg_r ).str();
  }

private:
  Arglist 			_args;		///< Command and args.
  DefaultIntegral<pid_t,-1>	_pid;		///< Pid while command is running, else -1.
  DefaultIntegral<int,-1>	_exitStatus;
  std::string 			_execError;
};

inline int RunCommand::run()
{
  DBG << "Executing " << commandline() << endl;
  _exitStatus = -1;
  _execError.clear();


  fflush(nullptr);
  pid_t pid = fork();
  if ( pid == 0 )
  {
    //////////////////////////////////////////////////////////////////////

    // close all *open* file descriptors
    std::list<Pathname> fdlist;
    int maxfd = STDERR_FILENO;
    int ret = readdir( fdlist, "/proc/self/fd", /*dots*/false );
    if ( ret != 0 )
    {
      // cannot open /proc/self/fd, fall back to expensive close-all approach.
      for ( int i = ::getdtablesize() - 1; i > maxfd; --i )
      { fcntl(i, F_SETFD, FD_CLOEXEC, true); }
    }
    else
    {
      for (const auto & fdstr : fdlist)
      {
        int fd = -1;
        try { fd = std::stoi(fdstr.c_str()); }
        catch (const std::invalid_argument &_) {
          continue;
        }
        if (fd > maxfd)
          fcntl(fd, F_SETFD, FD_CLOEXEC, true);
      }
    }

    if ( ! _args.empty() )
    {
      const char * argv[_args.size()+1];
      unsigned idx = 0;
      for( ; idx < _args.size(); ++idx )
      { argv[idx] = _args[idx].c_str(); }
        argv[idx] = nullptr;

      if ( ! execvp( argv[0], (char**)argv ) )
      { _exit (0); }	// does not happen!
    }
    cerr << ( str::Format(_("cannot exec %1% (%2%)")) % command() % strerror(errno) ) << endl;
    _exit (128);
    // No sense in returning! I am forked away!!
    //////////////////////////////////////////////////////////////////////
  }
  else if ( pid < 0 )
  {
    // translators: %1% - command name or path
    // translators: %2% - system error message
    const char * txt = N_("fork for %1% failed (%2%)");
    ERR <<     ( str::Format(txt)    % command() % strerror(errno) ) << endl;
    _execError = str::Format(_(txt)) % command() % strerror(errno);
    _exitStatus = 127;
    return _exitStatus;
  }

  //////////////////////////////////////////////////////////////////////
  // Wait for child to exit
  DBG << "Waiting for " << pid << " - " << commandline() << endl;
  int status, code = -1;

  while ( (code = waitpid( pid, &status, 0 )) < 0 && errno == EINTR )
  {;} // just loop

  if ( code < 0 )
  {
    // translators: %1% - command name or path
    // translators: %2% - system error message
    const char * txt = N_("waitpid for %1% failed (%2%)");
    ERR <<     ( str::Format(txt)    % command() % strerror(errno) ) << endl;
    _execError = str::Format(_(txt)) % command() % strerror(errno);
    _exitStatus = -1;
  }
  else if ( code != pid )
  {
    // translators: %1% - command name or path
    // translators: %2% - returned PID (number)
    // translators: %3% - expected PID (number)
    const char * txt = N_("waitpid for %1% returns unexpected pid %2% while waiting for %3%");
    ERR <<     ( str::Format(txt)    % command() % code % pid ) << endl;
    _execError = str::Format(_(txt)) % command() % code % pid;
    _exitStatus = -1;
  }
  else if ( WIFSIGNALED(status) )
  {
    code = WTERMSIG(status);
    // translators: %1% - command name or path
    // translators: %2% - signal number
    // translators: %3% - signal name
    const char * txt = N_("%1% was killed by signal %2% (%3%)");
    WAR <<     ( str::Format(txt)    % command() % code % strsignal(code) ) << endl;
    _execError = str::Format(_(txt)) % command() % code % strsignal(code);
    if ( WCOREDUMP(status) )
      _execError += str::Format(" (%1)") % _("core dumped");
    _exitStatus = 128 + code;
  }
  else if ( WIFEXITED(status) )
  {
    code = WEXITSTATUS(status);
    if ( code )
    {
      // translators: %1% - command name or path
      // translators: %2% - exit code (number)
      const char * txt = N_("%1% exited with status %2%");
      WAR <<     ( str::Format(txt)    % command() % code ) << endl;
      _execError = str::Format(_(txt)) % command() % code;
    }
    else
    {
      DBG << command() << " successfully completed" << endl;
      _execError.clear(); // empty if running or successfully completed
    }
    _exitStatus = code;
  }
  else
  {
    // translators: %1% - command name or path
    // translators: %2% - status (number)
    const char * txt = N_("waitpid for %1% returns unexpected exit status %2%");
    ERR <<     ( str::Format(txt)    % command() % status ) << endl;
    _execError = str::Format(_(txt)) % command() % status;
    _exitStatus = -1;
  }
  return _exitStatus;
}

#endif // ZYPPER_UTILS_RUNCOMMAND_H
