/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

// zypper - command line interface for libzypp, the package management library
// http://en.opensuse.org/Zypper

#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <list>
#include <map>
#include <iterator>

#include <unistd.h>
#include <readline/history.h>

#include <zypp/ZYppFactory.h>
#include <zypp/zypp_detail/ZYppReadOnlyHack.h>

#include <zypp/base/LogTools.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/UserRequestException.h>
#include <zypp/base/DtorReset.h>

#include <zypp/sat/SolvAttr.h>
#include <zypp/AutoDispose.h>
#include <zypp/PoolQuery.h>
#include <zypp/Locks.h>
#include <zypp/Edition.h>

#include <zypp/target/rpm/RpmHeader.h> // for install <.rpmURI>

#include "main.h"
#include "Zypper.h"
#include "Command.h"
#include "SolverRequester.h"

#include "Table.h"
#include "utils/text.h"
#include "utils/misc.h"
#include "utils/messages.h"
#include "utils/getopt.h"
#include "utils/misc.h"

#include "repos.h"
#include "update.h"
#include "solve-commit.h"
#include "misc.h"
#include "search.h"
#include "info.h"

#include "output/OutNormal.h"
#include "output/OutXML.h"

#include "utils/flags/zyppflags.h"
#include "utils/flags/exceptions.h"
#include "global-settings.h"

#include "commands/search/search-packages-hinthack.h"
#include "commands/help.h"
#include "commands/subcommand.h"
using namespace zypp;

bool sigExitOnce = true;	// Flag to prevent nested calls to Zypper::immediateExit

ZYpp::Ptr God = NULL;
void Zypper::assertZYppPtrGod()
{
  if ( God )
    return;	// already have it.

  try
  {
    God = getZYpp();	// lock it
  }
  catch ( ZYppFactoryException & excpt_r )
  {
    ZYPP_CAUGHT (excpt_r);

    bool still_locked = true;
    // check for packagekit (bnc #580513)
    if ( excpt_r.lockerName().find( "packagekitd" ) != std::string::npos )
    {
      // ask user whether to tell it to quit
      mbs_write_wrapped( Out::Info(out()) << "", _(
	"PackageKit is blocking zypper. This happens if you have an"
	" updater applet or other software management application using"
	" PackageKit running."
      ), 0, out().defaultFormatWidth( 100 ) );

      mbs_write_wrapped( Out::Info(out()) << "", _(
	"We can ask PackageKit to interrupt the current action as soon as possible, but it depends on PackageKit how fast it will respond to this request."
      ), 0, out().defaultFormatWidth( 100 ) );

      bool reply = read_bool_answer( PROMPT_PACKAGEKIT_QUIT, _("Ask PackageKit to quit?"), false );

      // tell it to quit
      while ( reply && still_locked )
      {
	packagekit_suggest_quit();
	::sleep( 2 );
	if ( packagekit_running() )
	{
	  out().info(_("PackageKit is still running (probably busy)."));
	  reply = read_bool_answer( PROMPT_PACKAGEKIT_QUIT, _("Try again?"), false );
	}
	else
	  still_locked = false;
      }
    }

    if ( still_locked )
    {
      ERR  << "A ZYpp transaction is already in progress." << endl;
      out().error( excpt_r.asString() );

      setExitCode( ZYPPER_EXIT_ZYPP_LOCKED );
      ZYPP_THROW( ExitRequestException("ZYpp locked") );
    }
    else
    {
      // try to get the lock again
      try
      { God = getZYpp(); }
      catch ( ZYppFactoryException & e )
      {
	// this should happen only rarely, so no special handling here
	ERR  << "still locked." << endl;
	out().error( e.asString() );

	setExitCode( ZYPPER_EXIT_ZYPP_LOCKED );
	ZYPP_THROW( ExitRequestException("ZYpp locked") );
      }
    }
  }
  catch ( Exception & excpt_r )
  {
    ZYPP_CAUGHT( excpt_r );
    out().error( excpt_r.msg() );
    setExitCode( ZYPPER_EXIT_ERR_ZYPP );
    ZYPP_THROW( ExitRequestException("ZYpp error, cannot get ZYpp lock") );
  }
}

///////////////////////////////////////////////////////////////////

namespace {

  /** Whether user may create \a dir_r or has rw-access to it. */
  inline bool userMayUseDir( const Pathname & dir_r )
  {
    bool mayuse = true;
    if ( dir_r.empty()  )
      mayuse = false;
    else
    {
      PathInfo pi( dir_r );
      if ( pi.isExist() )
      {
	if ( ! ( pi.isDir() && pi.userMayRWX() ) )
	  mayuse = false;
      }
      else
	mayuse = userMayUseDir( dir_r.dirname() );
    }
    return mayuse;
  }

} //namespace

///////////////////////////////////////////////////////////////////

Zypper::Zypper()
: _argc( 0 )
, _argv( nullptr )
, _out_ptr( nullptr )
, _command( ZypperCommand::NONE )
, _exitCode( ZYPPER_EXIT_OK )
, _exitInfoCode( ZYPPER_EXIT_OK )
, _running_shell( false )
, _running_help( false )
, _exit_requested( 0 )
, _sh_argc( 0 )
, _sh_argv( NULL )
{
  MIL << "Zypper instance created." << endl;
}


Zypper::~Zypper()
{
  delete _out_ptr;
  MIL << "Zypper instance destroyed. Bye!" << endl;
}


Zypper & Zypper::instance()
{
  static Zypper _instance;
  // PENDING SigINT? Some frequently called place to avoid exiting from within the signal handler?
  _instance.immediateExitCheck();
  return _instance;
}


int Zypper::main( int argc, char ** argv )
{
  _argc = argc;
  _argv = argv;

  try {
    // parse global options and the command
    processGlobalOptions();

    if ( runningHelp() )
    {
      safeDoCommand();
    }
    else
    {
      switch( command().toEnum() )
      {
	case ZypperCommand::SHELL_e:
	  commandShell();
	  cleanup();
	  break;

  case ZypperCommand::SUBCOMMAND_e: {
    std::shared_ptr<SubCmd>  cmd = std::dynamic_pointer_cast<SubCmd>( command().commandObject() );
    if ( cmd )
      cmd->runCmd( *this );
    else {
      ZYPP_THROW( Exception ("Invalid command object") );
    }
	  break;
  }

	case ZypperCommand::NONE_e:
	  setExitCode( ZYPPER_EXIT_ERR_SYNTAX );
	  break;

	default:
	  safeDoCommand();
	  cleanup();
	  break;
      }
    }
  }
  // Actually safeDoCommand also catches these exceptions.
  // Here we gather what escapes from other places.
  // TODO Someday redesign the Exceptions flow.
  catch ( const AbortRequestException & ex )
  {
    ZYPP_CAUGHT( ex );
    out().error( ex.asUserString() );
  }
  catch ( const ExitRequestException & ex )
  {
    ZYPP_CAUGHT( ex );
    WAR << "Caught exit request: exitCode " << exitCode() << endl;
  }
  catch ( const Out::Error & error_r )
  {
    error_r.report( *this );
    report_a_bug( out() );
  }
  catch ( const Exception & ex )
  {
    ZYPP_CAUGHT( ex );
    {
      SCOPED_VERBOSITY( out(), Out::DEBUG );
      out().error( ex, _("Unexpected exception.") );
    }
    report_a_bug( out() );
    if ( ! exitCode() )
      setExitCode( ZYPPER_EXIT_ERR_BUG );
  }

  return exitCode();
}

Out & Zypper::out()
{
  // PENDING SigINT? Some frequently called place to avoid exiting from within the signal handler?
  immediateExitCheck();

  if ( _out_ptr )
    return *_out_ptr;

  cerr << "uninitialized output writer" << endl;
  ZYPP_THROW( ExitRequestException("no output writer") );
}

Out *Zypper::outputWriter()
{
  return _out_ptr;
}

void Zypper::setOutputWriter(Out *out)
{
  if ( _out_ptr ) {
    delete _out_ptr;
    _out_ptr = nullptr;
  }
  _out_ptr = out;
}

void print_command_help_hint( Zypper & zypper )
{
  zypper.out().info(
    // translators: %s is "help" or "zypper help" depending on whether
    // zypper shell is running or not
    str::Format(_("Type '%s' to get command-specific help."))
    % (zypper.runningShell() ? "help <COMMAND>" : "zypper help <COMMAND>") );
}

/*
 * parses global options, returns the command
 *
 * \returns ZypperCommand object representing the command or ZypperCommand::NONE
 *          if an unknown command has been given.
 */
void Zypper::processGlobalOptions()
{
  MIL << "START" << endl;

  //setup the default output, this could be overridden by cli values
  OutNormal * p = new OutNormal( _config.verbosity );
  p->setUseColors( _config.do_colors );
  setOutputWriter( p );

  std::vector<ZyppFlags::CommandGroup> globalOpts = _config.cliOptions();
  optind = searchPackagesHintHack::argvCmdIdx = ZyppFlags::parseCLI( _argc, _argv, globalOpts );

  out().info( str::Format(_("Verbosity: %d")) % _config.verbosity , Out::HIGH );
  DBG << "Verbosity " << _config.verbosity << endl;
  DBG << "Output type " << _out_ptr->type() << endl;

  //  ======== get command ========
  if ( optind < _argc )
  {
    try { setCommand( ZypperCommand( _argv[optind++] ) ); }

    // exception from command parsing
    catch ( const Exception & e )
    {
      out().error( e.asUserString() );
      print_unknown_command_hint( *this, _argv[optind-1] );
      setExitCode( ZYPPER_EXIT_ERR_SYNTAX );
      ZYPP_THROW( ExitRequestException("unknown command") );
    }
  }

  // Help is parsed by setting the help flag for a command, which may be empty
  // $0 -h,--help
  // $0 command -h,--help
  // The help command is eaten and transformed to the help option
  // $0 help
  // $0 help command
  if ( _config.wantHelp )
    setRunningHelp( true );	// help for current command
  else if ( command() == ZypperCommand::NONE )
    setRunningHelp( true );	// no command => global help
  else if ( command() == ZypperCommand::HELP )
  {
    setRunningHelp( true );
    if ( optind < _argc )	// help on help or next command
    {
      std::string arg = _argv[optind++];
      if ( arg != "-h" && arg != "--help" )
      {
        try { setCommand( ZypperCommand( arg ) ); }
        // exception from command parsing
        catch ( const Exception & e )
        {
          out().error( e.asUserString() );
          print_unknown_command_hint( *this, arg );
          setExitCode( ZYPPER_EXIT_ERR_SYNTAX );
          ZYPP_THROW( ExitRequestException("unknown command") );
        }
      }
    }
    else
      setCommand( ZypperCommand::NONE );	// global help
  }

  if ( runningHelp() )
  {
    if ( command() == ZypperCommand::NONE )	// global help
    {

      HelpCmd::printMainHelp( *this );
      ZYPP_THROW( ExitRequestException("help provided") );
    }
    else if ( command() == ZypperCommand::HELP )// help on help
    {
      HelpCmd::printMainHelp( *this );
      ZYPP_THROW( ExitRequestException("help provided") );
    }
  }
  else if ( command() == ZypperCommand::SHELL && optind < _argc )
  {
    // shell command args are handled here because
    // the command is treated differently in main
    std::string arg = _argv[optind++];
    if ( arg == "-h" || arg == "--help" )
      setRunningHelp(true);
    else
    {
      report_too_many_arguments( "shell\n" );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      ZYPP_THROW( ExitRequestException("invalid args") );
    }
  }
  else if ( command() == ZypperCommand::SUBCOMMAND )
  {
    // subcommand command args are handled here because
    // the command is treated differently in main.

    std::shared_ptr<SubCmd> cmd = std::dynamic_pointer_cast<SubCmd>(command().commandObject());
    if ( cmd ) {
      shared_ptr<SubcommandOptions> myOpts( cmd->subCmdOptions() );
      myOpts->loadDetected();

      if ( myOpts->_detected._name.empty() )
      {
        // Command name is the builtin 'subcommand', no executable.
        // For now we turn on the help.
        setRunningHelp( true );
      }
      else
      {
        if ( optind > 2 )
        {
          out().error(
            // translators: %1%  - is the name of a subcommand
            str::Format(_("Subcommand %1% does not support zypper global options."))
            % myOpts->_detected._name );
          print_command_help_hint( *this );
          setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
          ZYPP_THROW( ExitRequestException("invalid args") );
        }
        // save args (incl. the command itself as argv[0])
        myOpts->args( _argv+(optind-1), _argv+_argc );
      }
    }
  }

  // ======== other global options ========
  {
    const char * env = ::getenv( "ZYPP_REPO_RELEASEVER" );
    if ( env && *env )
    {
      out().warning( str::Str() << _("Enforced setting") << ": $releasever=" << env );
      WAR << "Enforced setting: $releasever=" << env << endl;
    }
  }

  ///////////////////////////////////////////////////////////////////
  // Rug compatibility is dropped since SLE12.
  // Rug options are removed from documantation(commit#53ffd419) but
  // will stay active in code for a while.
  std::string rug_test( _argv[0] );
  if ( Pathname::basename( _argv[0] ) == "rug" )
  {
    exit_rug_compat();
  }
  ///////////////////////////////////////////////////////////////////

  // on the fly check the baseproduct symlink
  {
    PathInfo pi( _config.root_dir + "/etc/products.d/baseproduct" );
    if ( ! pi.isFile() && PathInfo( _config.root_dir + "/etc/products.d" ).isDir() )
    {
      ERR << "baseproduct symlink is dangling or missing: " << pi << endl;
      out().warning(_(
        "The /etc/products.d/baseproduct symlink is dangling or missing!\n"
        "The link must point to your core products .prod file in /etc/products.d.\n"
              ));
    }
  }

  // cache dirs

  DBG << "repos.d dir = "       << _config.rm_options.knownReposPath        << endl;
  DBG << "cache dir = "         << _config.rm_options.repoCachePath         << endl;
  DBG << "raw cache dir = "     << _config.rm_options.repoRawCachePath      << endl;
  DBG << "solv cache dir = "    << _config.rm_options.repoSolvCachePath     << endl;
  DBG << "package cache dir = " << _config.rm_options.repoPackagesCachePath << endl;

  if ( ! _config.disable_system_sources )
  {
    MIL << "Repositories enabled" << endl;
  }

  // additional repositories by URL
  if ( _config.plusRepoFromCLI.size() )
  {
    switch ( command().toEnum() )
    {
    case ZypperCommand::ADD_REPO_e:
    case ZypperCommand::REMOVE_REPO_e:
    case ZypperCommand::MODIFY_REPO_e:
    case ZypperCommand::RENAME_REPO_e:
    case ZypperCommand::REFRESH_e:
    case ZypperCommand::CLEAN_e:
    case ZypperCommand::REMOVE_LOCK_e:
    case ZypperCommand::LIST_LOCKS_e:
    {
      // TranslatorExplanation The %s is "--plus-repo"
      out().warning( str::Format(_("The %s option has no effect here, ignoring.")) % "--plus-repo" );
      break;
    }
    default:
    {
      int count = 1;
      for ( std::vector<std::string>::const_iterator it = _config.plusRepoFromCLI.begin(); it != _config.plusRepoFromCLI.end(); ++it )
      {
        Url url = make_url( *it );
        if (!url.isValid())
        {
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }

        RepoInfo repo;
        repo.addBaseUrl( url );
        repo.setEnabled( true );
        repo.setAutorefresh( true );
        repo.setAlias( str::Format("~plus-repo-%d") % count );
        repo.setName( url.asString() );

        repo.setMetadataPath( runtimeData().tmpdir / repo.alias() / "%AUTO%" );
        repo.setPackagesPath( Pathname::assertprefix( _config.root_dir, ZYPPER_RPM_CACHE_DIR ) );

        _rdata.temporary_repos.push_back( repo );
        DBG << "got additional repo: " << url << endl;
        count++;
      }
    }
    }
  }

  // additional repositories by content (keywords)
  if ( _config.plusContentFromCLI.size() )
  {
    switch ( command().toEnum() )
    {
    case ZypperCommand::ADD_REPO_e:
    case ZypperCommand::REMOVE_REPO_e:
    case ZypperCommand::MODIFY_REPO_e:
    case ZypperCommand::RENAME_REPO_e:
    //case ZypperCommand::REFRESH_e:
    case ZypperCommand::CLEAN_e:
    case ZypperCommand::REMOVE_LOCK_e:
    case ZypperCommand::LIST_LOCKS_e:
    {
      // TranslatorExplanation The %s is "--option-name"
      out().warning( str::Format(_("The %s option has no effect here, ignoring.")) % "--plus-content" );
      break;
    }
    default:
    {
      _rdata.plusContentRepos.insert( _config.plusContentFromCLI.begin(), _config.plusContentFromCLI.end() );
    }
    }
  }

  MIL << "DONE" << endl;
}


void Zypper::commandShell()
{
  MIL << "Entering the shell" << endl;

  setRunningShell( true );

  if ( _config.changedRoot && _config.root_dir != "/" )
  {
    // bnc#575096: Quick fix
    ::setenv( "ZYPP_LOCKFILE_ROOT", _config.root_dir.c_str(), 0 );
  }

  assertZYppPtrGod();
  init_target( *this );

  std::string histfile;
  try
  {
    const char * env = getenv("HOME");
    if ( env )
    {
      Pathname p( env );
      p /= ".zypper_history";
      histfile = p.asString();
    }
  }
  catch (...)
  { /*no history*/ }

  using_history();
  if ( !histfile.empty() )
    read_history( histfile.c_str () );

  while ( true )
  {
    // read a line
    std::string line = readline_getline();
    out().info( str::Format("Got: %s") % line, Out::DEBUG );
    // reset optind etc
    optind = 0;
    // split it up and create sh_argc, sh_argv
    Args args( line );
    _sh_argc = args.argc();
    _sh_argv = args.argv();

    std::string command_str = _sh_argv[0] ? _sh_argv[0] : "";

    if ( command_str == "\004" ) // ^D
    {
      cout << endl; // print newline after ^D
      break;
    }

    try
    {
      MIL << "Reloading..." << endl;
      God->target()->reload();   // reload system in case rpm database has changed
      setCommand( ZypperCommand( command_str ) );
      if ( command() == ZypperCommand::SHELL_QUIT )
        break;
      else if ( command() == ZypperCommand::NONE )
        print_unknown_command_hint( *this, command_str );
      else if ( command() == ZypperCommand::SUBCOMMAND )
      {
	// Currently no concept how to handle global options and ZYPPlock
	out().error(_("Zypper shell does not support execution of subcommands.") );
      }
      else
        safeDoCommand();
    }
    catch ( const Exception & e )
    {
      out().error( e.msg() );
      print_unknown_command_hint( *this, command_str ); // TODO: command_str should come via the Exception, same for other print_unknown_command_hint's
    }

    shellCleanup();
  }

  if ( !histfile.empty() )
    write_history( histfile.c_str() );

  MIL << "Leaving the shell" << endl;
  setRunningShell( false );
}

void Zypper::shellCleanup()
{
  MIL << "Cleaning up for the next command." << endl;

  switch( command().toEnum() )
  {
  case ZypperCommand::INSTALL_e:
  case ZypperCommand::REMOVE_e:
  case ZypperCommand::UPDATE_e:
  case ZypperCommand::PATCH_e:
  {
    remove_selections( *this );
    break;
  }
  default:;
  }

  // clear any previous arguments
  _arguments.clear();
  // clear the command
  _command = ZypperCommand::NONE;
  // clear command help text
  _command_help.clear();
  // reset help flag
  setRunningHelp( false );
  // ... and the exit code
  setExitCode( ZYPPER_EXIT_OK );

  // runtime data
  _rdata.current_repo = RepoInfo();

  // cause the RepoManager to be reinitialized
  _rm.reset();

  // TODO:
  // _rdata.repos re-read after repo operations or modify/remove these very repoinfos
  // _rdata.repo_resolvables re-read only after certain repo operations (all?)
  // _rdata.target_resolvables re-read only after installation/removal/update
  // call target commit refresh pool after installation/removal/update (#328855)
}


/// process one command from the OS shell or the zypper shell
// catch unexpected exceptions and tell the user to report a bug (#224216)
void Zypper::safeDoCommand()
{
  try
  {
    processCommandOptions();
    if ( command() == ZypperCommand::NONE || exitCode() )
      return;
    doCommand();
  }
  // The same catch block as in zypper::main.
  // TODO Someday redesign the Exceptions flow.
  catch ( const AbortRequestException & ex )
  {
    ZYPP_CAUGHT( ex );
    out().error( ex.asUserString() );
  }
  catch ( const ExitRequestException & ex )
  {
    ZYPP_CAUGHT( ex );
    WAR << "Caught exit request: exitCode " << exitCode() << endl;
  }
  catch ( const Out::Error & error_r )
  {
    error_r.report( *this );
    report_a_bug( out() );
  }
  catch ( const Exception & ex )
  {
    ZYPP_CAUGHT( ex );
    {
      SCOPED_VERBOSITY( out(), Out::DEBUG );
      out().error( ex, _("Unexpected exception.") );
    }
    report_a_bug( out() );
    if ( ! exitCode() )
      setExitCode( ZYPPER_EXIT_ERR_BUG );
  }
}

// === command-specific options ===
void Zypper::processCommandOptions()
{
  MIL << "START" << endl;

  if ( command() == ZypperCommand::HELP )
  {
    // in shell, check next argument to see if command-specific help is wanted
    if ( runningShell() )
    {
      if ( argc() > 1 )
      {
        std::string cmd = argv()[1];
        try
        {
          setRunningHelp( true );
          setCommand( ZypperCommand( cmd ) );
        }
        catch ( Exception & ex )
        {
          // unknown command. Known command will be handled in the switch
          // and doCommand()
          if ( !cmd.empty() && cmd != "-h" && cmd != "--help" )
          {
            out().error( ex.asUserString() );
            print_unknown_command_hint( *this, cmd );
            ZYPP_THROW( ExitRequestException("help provided") );
          }
        }
      }
      // if no command is requested, show main help
      else
      {
        HelpCmd::printMainHelp( *this );
        ZYPP_THROW( ExitRequestException("help provided") );
      }
    }
  }

  // handle new style commands
  ZypperBaseCommandPtr newStyleCmd = command().commandObject();
  if ( newStyleCmd ) {

    //reset the command to default
    newStyleCmd->reset();

    //get command help
    _command_help = newStyleCmd->help();

    MIL << "Found new style command << " << newStyleCmd->command().front() << endl;

    //no need to parse args if we want help anyway
    if ( runningHelp() )
      return;

    // parse command options
    try {
      //keep compat by setting optind
      searchPackagesHintHack::argvArgIdx = optind = newStyleCmd->parseArguments( *this, optind );

      //keep compatibility
      _arguments = newStyleCmd->positionalArguments();

      //make sure help is shown if required
      setRunningHelp( newStyleCmd->helpRequested() );

    } catch ( const ZyppFlags::ZyppFlagsException &e) {
      ERR << e.asString() << endl;
      out().error( e.asUserString() );
      setExitCode( ZYPPER_EXIT_ERR_SYNTAX );
      return;
    }
  } else {
    if ( !runningHelp() ) {
      ERR << "Unknown or unexpected command" << endl;
      out().error(_("Unexpected program flow."));
      report_a_bug( out() );
    }
  }
  MIL << "Done " << endl;
}

/// process one command from the OS shell or the zypper shell
void Zypper::doCommand()
{
  // help check is common to all commands
  if ( runningHelp() )
  {
    out().info( _command_help, Out::QUIET );
    if ( command() == ZypperCommand::SEARCH )
     searchPackagesHintHack::callOrNotify( *this );
    return;
  }

  // === ZYpp lock ===
  switch ( command().toEnum() )
  {
    case ZypperCommand::PS_e:
    case ZypperCommand::SUBCOMMAND_e:
      // bnc#703598: Quick fix as few commands do not need a zypp lock
      break;

    default:
      if ( _config.changedRoot && _config.root_dir != "/" )
      {
        // bnc#575096: Quick fix
        ::setenv( "ZYPP_LOCKFILE_ROOT", _config.root_dir.c_str(), 0 );
      }
      {
        const char *roh = getenv( "ZYPP_READONLY_HACK" );
        if ( roh != NULL && roh[0] == '1' )
          zypp_readonly_hack::IWantIt ();

        else if ( command() == ZypperCommand::LIST_REPOS
                  || command() == ZypperCommand::LIST_SERVICES
                  || command() == ZypperCommand::VERSION_CMP
                  || command() == ZypperCommand::TARGET_OS )
          zypp_readonly_hack::IWantIt (); // #247001, #302152
      }
      assertZYppPtrGod();
  }

  // === execute command ===

  MIL << "Going to process command " << command() << endl;

  //handle new style commands
  ZypperBaseCommandPtr newStyleCmd = command().commandObject();
  if ( newStyleCmd ) {
    setExitCode( newStyleCmd->run( *this ) );
    return;
  }

  // if the program reaches this line, something went wrong
  report_a_bug( out() );
  setExitCode( ZYPPER_EXIT_ERR_BUG );
}

void Zypper::cleanup()
{
  // NOTE: Via immediateExit this may be invoked from within
  // a signal handler.
  MIL << "START" << endl;
  _rm.reset();	// release any pending appdata trigger now.
}

void Zypper::cleanupForSubcommand()
{
  // Clear resources and release the zypp lock.
  // Currently we do not expect to return to zypper after the subcommand
  // (just reporting it's status and exit; no subcommands in `zypper shell`)
  MIL << "START cleanupForSubcommand" << endl;
  _rm.reset();	// release any pending appdata trigger now.
  if ( God )
  {
    if ( God->getTarget() )
      God->finishTarget();
    God.reset();
  }
  MIL << "DONE cleanupForSubcommand" << endl;
}

ExitRequestException::~ExitRequestException() {}

// Local Variables:
// c-basic-offset: 2
// End:
