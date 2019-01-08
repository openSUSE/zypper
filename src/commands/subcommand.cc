/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <csignal>
#include <cerrno>
#include <cstring>

#include <iostream>
#include <algorithm>
#include <set>
#include <zypp/base/LogTools.h>
#include <zypp/ExternalProgram.h>

#include "Zypper.h"
#include "Table.h"
#include "subcommand.h"
#include "utils/messages.h"

#include <boost/utility/string_ref.hpp>

///////////////////////////////////////////////////////////////////
namespace env
{
  std::string PATH()
  {
    std::string ret;
    if ( const char * env = ::getenv("PATH") )
      ret = env;
    return ret;
  }
} // namespace env
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace
{

  std::ostream & dumpNameTableOn( std::ostream & str, const std::vector<boost::string_ref> & names_r )
  {
    const std::string indent( 2, ' ' );

    if ( names_r.empty() )
    {
      str << indent << "<" << _("none") << ">" << endl;
    }
    else
    {
      unsigned colw = 0;
      for ( const auto & el : names_r )
      {
	unsigned w = el.size();
	if ( w > colw ) colw = w;
      }

      str::Format fmter( indent+"%-"+str::numstring(colw)+"s" );
      unsigned maxcols = Zypper::instance().out().defaultFormatWidth() / ( colw + indent.size() );
      if ( maxcols == 0 ) maxcols = 1;

#if 1
      // vertical
      unsigned maxrows = ( names_r.size() + maxcols - 1 ) / maxcols;
      maxcols = ( names_r.size() + maxrows - 1 ) / maxrows;

      for ( unsigned r = 0; r < maxrows; ++r )
      {
	unsigned idx = r;
	for ( unsigned c = 0; c < maxcols; ++c )
	{
	  str << ( fmter % names_r[idx] );
	  if ( (idx += maxrows) >= names_r.size() )
	    break;
	}
	str << endl;
      }
#else
      // horizontal
      for ( unsigned idx = 0; idx < names_r.size(); ++idx )
      {
	if ( idx && idx % maxcols == 0 )
	  str << endl;
	str << ( fmter % names_r[idx] );
      }
      str << endl;
#endif
    }
    return str;
  }

  template <class Container_>
  std::ostream & dumpNameTableOn( std::ostream & str, const Container_ & container_r )
  { return dumpNameTableOn( str, std::vector<boost::string_ref>( container_r.begin(), container_r.end() ) ); }

} // namespace env
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// SubcommandOptions
///////////////////////////////////////////////////////////////////

namespace	// subcommand detetction
{///////////////////////////////////////////////////////////////////

  SubcommandOptions::Detected & lastSubcommandDetected()
  {
    static SubcommandOptions::Detected _ref;
    return _ref;
  }

  /** Return empty string on error */
  inline std::string buildExecname( const std::string & name_r )
  {
    std::string ret;
    if ( ! name_r.empty() && name_r.find( '/' ) == std::string::npos )	// no pathsep in name!
    { ret = "zypper-"+name_r; }
    return ret;
  }

  inline bool canExecute( Pathname path_r )
  {
    PathInfo pi( std::move(path_r) );
    return(  pi.isFile() && pi.userMayRX() );
  }

  inline bool testAndRememberSubcommand( const Pathname & path_r, const std::string & name_r, const std::string & cmd_r  )
  {
    if ( canExecute( path_r/name_r ) )
    {
      SubcommandOptions::Detected & ref( lastSubcommandDetected() );
      ref = SubcommandOptions::Detected();	// reset
      ref._cmd  = cmd_r;
      ref._name = name_r;
      ref._path = path_r;
      return true;
    }
    return false;
  }

  template <class OutputIterator_>
  unsigned collectSubommandsIn( const Pathname & dir_r, OutputIterator_ result_r )
  {
    unsigned cnt = 0;
    filesystem::dirForEach( dir_r,
			    [&result_r]( const Pathname & dir_r, std::string str_r )->bool
			    {
			      if ( str::startsWith( str_r, "zypper-" ) && canExecute( dir_r/str_r ) )
			      {
				str_r.erase( 0, 7 /*"zypper-"*/ );
				*result_r = str_r;
			      }
			      return true;
			    } );
    return cnt;
  }

  inline void collectAllSubommands( std::set<std::string> & execdirCommands_r, std::set<std::string> & pathCommands_r )
  {
    collectSubommandsIn( SubcommandOptions::_execdir, std::inserter(execdirCommands_r,execdirCommands_r.end()) );

    std::set<Pathname> dirs;
    str::split( env::PATH(), std::inserter(dirs,dirs.end()), ":" );
    for ( const auto & dir : dirs )
    {
      collectSubommandsIn( dir, std::inserter(pathCommands_r,pathCommands_r.end()) );
    }
  }

  inline void collectAllSubommands( std::set<std::string> & allCommands_r )
  { collectAllSubommands( allCommands_r, allCommands_r ); }

} // namespace
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace	// command execution
{
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
    std::string 		_execError;
  };

  int RunCommand::run()
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
} // namespace
///////////////////////////////////////////////////////////////////

inline std::ostream & operator<<( std::ostream & str, const SubcommandOptions & obj )
{ return str << obj._detected._name; }

const Pathname SubcommandOptions::_execdir( "/usr/lib/zypper/commands" );

void SubcommandOptions::loadDetected()
{ _detected = lastSubcommandDetected(); }

std::ostream & SubcommandOptions::showHelpOn( std::ostream & out ) const
{
  if ( _detected._name.empty() )
  {
    // common subcommand help
    Zypper & _zypper = Zypper::instance();

    if ( _zypper.out().verbosity() > Out::QUIET )
    {
      out << "<subcommand> [--command-options] [arguments]" << endl;
      out << endl;


      // translators: %1% is a directory name
      out << str::Format(_(
        "Zypper subcommands are standalone executables that live in the\n"
	"zypper_execdir ('%1%').\n"
	"\n"
	"For subcommands zypper provides a wrapper that knows where the\n"
	"subcommands live, and runs them by passing command-line arguments\n"
	"to them.\n"
	"\n"
	"If a subcommand is not found in the zypper_execdir, the wrapper\n"
	"will look in the rest of your $PATH for it. Thus, it's possible\n"
	"to write local zypper extensions that don't live in system space.\n"
      ) ) % _execdir;
      out << endl;

      // translators: %1% is a zypper command
      out << str::Format(_(
	"Using zypper global-options together with subcommands, as well as\n"
	"executing subcommands in '%1%' is currently not supported.\n"
      ) ) % "zypper shell";
      out << endl;


      std::set<std::string> execdirCommands;
      std::set<std::string> pathCommandsALL;
      collectAllSubommands( execdirCommands, pathCommandsALL );

      std::set<std::string> pathCommands;	// filter out execdirCommands
      std::set_difference( pathCommandsALL.begin(), pathCommandsALL.end(),
			   execdirCommands.begin(), execdirCommands.end(),
			   std::inserter(pathCommands,pathCommands.end()) );


      // translators: headline of an enumeration; %1% is a directory name
      out << str::Format(_("Available zypper subcommands in '%1%'") ) % _execdir << endl;
      out << endl;
      dumpNameTableOn( out, execdirCommands ) << endl;

      // translators: headline of an enumeration
      out << _("Zypper subcommands available from elsewhere on your $PATH") << endl;
      out << endl;
      dumpNameTableOn( out, pathCommands ) << endl;

      // translators: helptext; %1% is a zypper command
      out << str::Format(_("Type '%1%' to get subcommand-specific help if available.") ) % "zypper help <subcommand>" << endl;
    }
    else
    {
      std::set<std::string> allCommands;
      collectAllSubommands( allCommands );
      if ( ! allCommands.empty() )
	dumpRange( out, allCommands.begin(),allCommands.end(), "", "", ", ", "", "" );
    }
  }
  else
  {
    RunCommand cmd = {
      "/usr/bin/man",
      _detected._name,
    };
    if ( cmd.run() != 0 )
    {
      Zypper & _zypper = Zypper::instance();
      if ( cmd.exitStatus() != 16 )	// man already returned 'No manual entry for...'
      {
	_zypper.out().error( cmd.execError() );
	// translators: %1% - command name
	_zypper.out().info( str::Format(_("Manual entry for %1% can't be shown")) % _detected._name );
      }
      _zypper.setExitCode( cmd.exitStatus() );
    }
  }
  return out;	// FAKE!
}

SubCmd::SubCmd(std::vector<std::string> &&commandAliases_r , boost::shared_ptr<SubcommandOptions> options_r) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    "subcommand",
    // translators: command summary: subcommand
    _("Lists available subcommands."),
    "", //no help text, its created on demand
    DisableAll
    ),
  _options ( options_r )
{
  //we handle options ourselfes
  setFillRawOptions( true );
  disableArgumentParser( );

  if ( !_options ) {
    _options.reset ( new SubcommandOptions() );
  }
}

bool SubCmd::isSubCommand(const std::string &strval_r )
{
  if ( strval_r.empty() )
  {
    // remember an empty name; it's the 'subcommand' builtin (it's not executable)
    SubcommandOptions::Detected & ref( lastSubcommandDetected() );
    ref = SubcommandOptions::Detected();	// reset
    return false;
  }

  std::string execname( buildExecname( strval_r ) );
  if ( execname.empty() )
    return false;	// illegal name (e.g. pathsep in name)

  // Execdir first..
  if ( testAndRememberSubcommand( SubcommandOptions::_execdir, execname, strval_r ) )
    return true;

  // Search in $PATH...
  std::vector<Pathname> dirs;
  str::split( env::PATH(), std::back_inserter(dirs), ":" );
  for ( const auto & dir : dirs )
  {
    if ( testAndRememberSubcommand( dir, execname, strval_r ) )
      return true;
  }

  return false;
}

int SubCmd::runCmd( Zypper &zypper )
{
  try {
    zypper.cleanupForSubcommand();
    setArg0( ( _options->_detected._path / _options->_detected._name).asString() );
    RunCommand cmd( _options->_args );
    if ( cmd.run() != 0 )
      throw( Out::Error( cmd.exitStatus(), cmd.execError() ) );
    return cmd.exitStatus();

    //handle this error directly for now until the new command flow is implemented
  } catch ( const Out::Error & error_r ) {
    error_r.report( zypper );
  }
  return zypper.exitCode();
}

boost::shared_ptr<SubcommandOptions> SubCmd::subCmdOptions()
{
  return _options;
}

void SubCmd::setArg0(std::string arg0_r)
{
  if ( _options->_args.empty() )
    _options->_args.push_back( std::move(arg0_r) );
  else
    _options->_args[0] = std::move(arg0_r);
}

std::string SubCmd::help()
{
  _options->loadDetected();
  std::ostringstream str;
  _options->showHelpOn( str );
  return str.str();
}

ZyppFlags::CommandGroup SubCmd::cmdOptions() const
{
  return {};
}

void SubCmd::doReset()
{
  return;
}

int SubCmd::execute( Zypper &zypper, const std::vector<std::string> & )
{
  if ( zypper.runningShell() ) {
    // Currently no concept how to handle global options and ZYPPlock
    zypper.out().error(_("Zypper shell does not support execution of subcommands.") );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  _options->loadDetected();
  if (  _options->_detected._name.empty()  ) {
    //in case we end up here, we just print help
    zypper.out().info( help(), Out::QUIET );
    return ZYPPER_EXIT_OK;
  }

  if ( zypper.commandArgOffset() >= 2 ) {
    zypper.out().error(
      // translators: %1%  - is the name of a subcommand
      str::Format(_("Subcommand %1% does not support zypper global options."))
      % _options->_detected._name );
    print_command_help_hint( zypper );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  // save args (incl. the command itself as argv[0])
  SubcommandOptions::Arglist args {
    _options->_detected._cmd
  };

  const auto &opts = rawOptions();
  args.insert( args.end(), opts.begin(), opts.end() );
  _options->args( args );

  return runCmd ( zypper );
}
