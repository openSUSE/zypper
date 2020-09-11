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
#include "commands/commandhelpformatter.h"

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
// SubcommandOptions
///////////////////////////////////////////////////////////////////

namespace	// subcommand detetction
{///////////////////////////////////////////////////////////////////

  /** Strictly compare \ref SubcommandOptions::Detected according to _cmd. */
  struct DetectedCommandsCompare
  {
    bool operator()( const SubcommandOptions::Detected & lhs, const SubcommandOptions::Detected & rhs ) const
    { return lhs._cmd < rhs._cmd; }
  };

  /** First cmd detected shadows later ones. */
  using DetectedCommands = std::set<SubcommandOptions::Detected, DetectedCommandsCompare>;

  /** Command name,summaries for help. */
  using CommandSummaries = std::map<std::string,std::string>;

  /** Get command summaries for help. */
  inline CommandSummaries getCommandsummaries( const DetectedCommands & commands_r )
  {
    CommandSummaries ret;
    for ( auto & cmd : commands_r ) {
      std::string sum { ExternalProgram( { "man", "-f", cmd._name }, ExternalProgram::Discard_Stderr ).receiveLine() };
      if ( ! sum.empty() ) {
	// # man -f zypper
	// zypper (8)           - Command-line interface to ZYpp system management library (libzypp)
	static const std::string_view sep { " - " };
	std::string::size_type pos = sum.find( sep );
	if ( pos != std::string::npos )
	  sum.erase( 0, pos + sep.size() );
      }
      else {
	// translators: %1% is the name of the command which has no man page available.
	static str::Format fmt( "<"+ LOWLIGHTString(_("No manual entry for %1%")).str() + ">" );
	sum = ( fmt % cmd._name ).str();
      }
      ret[cmd._cmd] = sum;
    }
    return ret;
  }

  inline std::ostream & dumpDetectedCommandsOn( std::ostream & str, const DetectedCommands & commands_r )
  {
    CommandHelpFormater fmt;

    if ( commands_r.empty() ) {
      fmt.gDef( "<"+std::string(_("none"))+">" );
    }
    else {
      for ( const auto & p : getCommandsummaries( std::move(commands_r) ) ) {
	fmt.gDef( p.first, p.second );
      }
    }

    return str << std::string(fmt) << endl;
  }


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

  /** Return whether \a dir_r/name_r forms a valid subcommand. */
  inline SubcommandOptions::Detected detectSubcommand( const Pathname & dir_r, std::string name_r )
  {
    SubcommandOptions::Detected ret;
    if ( str::startsWith( name_r, "zypper-" ) && canExecute( dir_r/name_r ) ) {
      ret._cmd  = name_r.substr( 7 /*"zypper-"*/ );
      ret._name = std::move(name_r);
      ret._path = dir_r;
    }
    return ret;
  }

  /** Collect subcommands found in \a dir_r. */
  inline void detectSubcommandsIn( const Pathname & dir_r, std::function<void(SubcommandOptions::Detected)> fnc_r )
  {
    if ( !fnc_r )
      return;

    filesystem::dirForEach( dir_r,
			    [&fnc_r]( const Pathname & dir_r, std::string name_r )->bool
			    {
			      SubcommandOptions::Detected cmd { detectSubcommand( dir_r, std::move(name_r) ) };
			      if ( ! cmd._cmd.empty() )
				fnc_r( std::move(cmd) );
			      return true;
			    } );
  }

  /* Just the command names for the short help. */
  inline void collectAllSubcommandNames( std::set<std::string> & allCommands_r )
  {
    auto collectSubcommandsIn = [&allCommands_r]( const Pathname & dir_r ) {
      detectSubcommandsIn( dir_r,
			   [&allCommands_r]( SubcommandOptions::Detected cmd_r )
			   {
			     allCommands_r.insert( std::move(cmd_r._cmd) );
			   } );
    };

    collectSubcommandsIn( SubcommandOptions::_execdir );

    std::set<Pathname> dirs;
    str::split( env::PATH(), std::inserter(dirs,dirs.end()), ":" );
    for ( const auto & dir : dirs )
    {
      collectSubcommandsIn( dir );
    }
  }

  /* The command details for the long help. */
  inline void collectAllSubcommands( DetectedCommands & execdirCommands_r,
				     DetectedCommands & pathCommands_r )
  {
    // Commands in _execdir shadow commands in the path.
    auto collectSubcommandsIn = []( const Pathname & dir_r, DetectedCommands & commands_r, DetectedCommands * shaddow_r = nullptr ) {
      detectSubcommandsIn( dir_r,
			   [&commands_r,shaddow_r]( SubcommandOptions::Detected cmd_r )
			   {
			     if ( ! ( shaddow_r && shaddow_r->count( cmd_r ) ) )
			       commands_r.insert( std::move(cmd_r) );
			   } );
    };

    collectSubcommandsIn( SubcommandOptions::_execdir, execdirCommands_r );

    std::set<Pathname> dirs;
    str::split( env::PATH(), std::inserter(dirs,dirs.end()), ":" );
    for ( const auto & dir : dirs )
    {
      collectSubcommandsIn( dir, pathCommands_r, &execdirCommands_r );
    }
  }

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


      DetectedCommands execdirCommands;
      DetectedCommands pathCommands;
      collectAllSubcommands( execdirCommands, pathCommands );

      // translators: headline of an enumeration; %1% is a directory name
      out << str::Format(_("Available zypper subcommands in '%1%'") ) % _execdir << endl;
      out << endl;
      dumpDetectedCommandsOn( out, execdirCommands ) << endl;

      // translators: headline of an enumeration
      out << _("Zypper subcommands available from elsewhere on your $PATH") << endl;
      out << endl;
      dumpDetectedCommandsOn( out, pathCommands ) << endl;

      // translators: helptext; %1% is a zypper command
      out << str::Format(_("Type '%1%' to get subcommand-specific help if available.") ) % "zypper help <subcommand>" << endl;
    }
    else
    {
      std::set<std::string> allCommands;
      collectAllSubcommandNames( allCommands );
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

bool SubCmd::isSubcommand(const std::string &strval_r )
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

CommandSummaries SubCmd::getSubcommandSummaries()
{
  DetectedCommands detetctedCommands;
  collectAllSubcommands( detetctedCommands, detetctedCommands ); // all in one is ok
  return getCommandsummaries( detetctedCommands );
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
