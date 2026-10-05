/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <fcntl.h>
#include <sys/types.h>
#include <csignal>
#include <cerrno>
#include <cstring>

#include <iostream>
#include <algorithm>
#include <set>
#include <zypp-core/base/LogTools.h>
#include <zypp-core/ExternalProgram.h>

#include "Zypper.h"
#include "Table.h"
#include "subcommand.h"
#include "utils/messages.h"
#include "utils/RunCommand.h"
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

  inline std::vector<Pathname> pathDirsIf( bool yesno_r )
  {
    std::vector<Pathname> ret;
    if ( yesno_r )
      str::split( env::PATH(), std::back_inserter(ret), ":" );
    return ret;
  }

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
  inline CommandSummaries getCommandsummaries( const DetectedCommands & commands_r, bool withPath = false )
  {
    CommandSummaries ret;
    for ( auto & cmd : commands_r ) {
      std::string sum { str::rtrim( ExternalProgram( { "man", "-f", cmd._name }, ExternalProgram::Discard_Stderr ).receiveLine() ) };

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
      ret[cmd._cmd] = std::move(sum);
      if ( withPath )
        ret[cmd._cmd] += "\n("+(cmd._path/cmd._name).asString()+")";
    }
    return ret;
  }

  inline std::ostream & dumpDetectedCommandsOn( std::ostream & str, const DetectedCommands & commands_r, bool withPath = false )
  {
    CommandHelpFormater fmt;

    if ( commands_r.empty() ) {
      fmt.gDef( "<"+std::string(_("none"))+">" );
    }
    else {
      for ( const auto & p : getCommandsummaries( commands_r, withPath ) ) {
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
  inline void collectAllSubcommandNames( std::set<std::string> & allCommands_r, const std::vector<Pathname> pathDirs_r )
  {
    auto collectSubcommandsIn = [&allCommands_r]( const Pathname & dir_r ) {
      detectSubcommandsIn( dir_r,
                           [&allCommands_r]( SubcommandOptions::Detected cmd_r )
                           {
                             allCommands_r.insert( std::move(cmd_r._cmd) );
                           } );
    };

    collectSubcommandsIn( SubcommandOptions::_execdir );

    for ( const auto & dir : pathDirs_r )
      collectSubcommandsIn( dir );
  }

  /* The command details for the long help. */
  inline void collectAllSubcommands( DetectedCommands & execdirCommands_r,
                                     DetectedCommands & pathCommands_r, const std::vector<Pathname> pathDirs_r )
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

    for ( const auto & dir : pathDirs_r )
      collectSubcommandsIn( dir, pathCommands_r, &execdirCommands_r );
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
      collectAllSubcommands( execdirCommands, pathCommands, pathDirsIf( _zypper.config().seach_subcommand_in_path ) );

      // translators: headline of an enumeration; %1% is a directory name
      out << str::Format(_("Available zypper subcommands in '%1%'") ) % _execdir << ":" << endl;
      out << endl;
      dumpDetectedCommandsOn( out, execdirCommands ) << endl;

      if ( _zypper.config().seach_subcommand_in_path ) {
        // translators: headline of an enumeration
        out << _("Zypper subcommands available from elsewhere on your $PATH") << ":" << endl;
        out << endl;
        dumpDetectedCommandsOn( out, pathCommands, /*withPath*/true ) << endl;
      }
      else {
        out << _("Using zypper subcommands available from elsewhere on your $PATH is disabled in zypper.conf.") << endl;
        out << endl;
      }

      // translators: helptext; %1% is a zypper command
      out << str::Format(_("Type '%1%' to get subcommand-specific help if available.") ) % "zypper help <subcommand>" << endl;
    }
    else
    {
      std::set<std::string> allCommands;
      collectAllSubcommandNames( allCommands, pathDirsIf( _zypper.config().seach_subcommand_in_path ) );
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
    Zypper::instance().config().seach_subcommand_in_path
    ? _("Lists available subcommands.")
    : _("Lists available subcommands. Using zypper subcommands found on your $PATH is disabled in zypper.conf.")
    ,
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

  if ( Zypper::instance().config().seach_subcommand_in_path ) {
    // Search in $PATH...
    for ( const auto & dir : pathDirsIf( true ) ) {
      if ( testAndRememberSubcommand( dir, execname, strval_r ) )
        return true;
    }
  }
  return false;
}

CommandSummaries SubCmd::getSubcommandSummaries()
{
  DetectedCommands detetctedCommands;
  collectAllSubcommands( detetctedCommands, detetctedCommands, pathDirsIf( Zypper::instance().config().seach_subcommand_in_path ) ); // all in one is ok
  return getCommandsummaries( detetctedCommands );
}


int SubCmd::doRunAsSubcommand( Zypper & zypper_r, Arglist args_r ) // static
{
  try {
    zypper_r.cleanupForSubcommand();
    RunCommand cmd( std::move(args_r) );
    if ( cmd.run() != 0 )
      throw( Out::Error( cmd.exitStatus(), cmd.execError() ) );
    return cmd.exitStatus();
  }
  catch ( const Out::Error & error_r ) {
    error_r.report( zypper_r ); // also sets the zypper.exitCode
  }
  return zypper_r.exitCode();
}


int SubCmd::runCmd( Zypper &zypper )
{
  setArg0( ( _options->_detected._path / _options->_detected._name).asString() );
  return doRunAsSubcommand( zypper, _options->_args );
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
