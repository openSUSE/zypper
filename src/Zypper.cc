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
#include "output/OutNormal.h"

#include "utils/misc.h"
#include "utils/messages.h"
#include "utils/getopt.h"
#include "utils/misc.h"
#include "utils/prompt.h"

#include "repos.h"
#include "misc.h"


#include "utils/flags/zyppflags.h"
#include "utils/flags/exceptions.h"

#include "commands/search/search-packages-hinthack.h"
#include "commands/help.h"
#include "utils/console.h"
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


Zypper::Zypper()
: Application( std::make_shared<::Config>() )
, _argc( 0 )
, _argv( nullptr )
, _config( static_cast<::Config &>( Application::mutableConfig () ) )
, _command( ZypperCommand::NONE )
, _exitInfoCode( ZYPPER_EXIT_OK )
, _running_shell( false )
, _running_help( false )
, _exit_requested( 0 )
{
  MIL << "Zypper instance created." << endl;
}


Zypper::~Zypper()
{
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
    _commandArgOffset = processGlobalOptions();
    doCommand( argc , argv, _commandArgOffset );
    cleanup();
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
  catch ( const ZyppFlags::ZyppFlagsException &e) {
    ERR << e.asString() << endl;
    out().error( e.asUserString() );
    setExitCode( ZYPPER_EXIT_ERR_SYNTAX );
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
    return Application::out();
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
 * \returns The index of the next flag to be parsed from CLI
 */
int Zypper::processGlobalOptions()
{
  MIL << "START" << endl;

  //setup the default output, this could be overridden by cli values
  out().setVerbosity( _config.verbosity );
  out().setUseColors( _config.do_colors );

  std::vector<ZyppFlags::CommandGroup> globalOpts = _config.cliOptions();
  int nextFlag = searchPackagesHintHack::argvCmdIdx = ZyppFlags::parseCLI( _argc, _argv, globalOpts );

  out().info( str::Format(_("Verbosity: %d")) % _config.verbosity , Out::HIGH );
  DBG << "Verbosity " << _config.verbosity << endl;
  DBG << "Output type " << out().type() << endl;

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

  MIL << "DONE" << endl;
  return nextFlag;
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

  //will be reset by ShellQuitCmd
  _continue_running_shell = true;
  while ( _continue_running_shell )
  {
    // read a line
    std::string line = readline_getline();

    out().info( str::Format("Got: %s") % line, Out::DEBUG );

    // split it up and create argc, argv
    Args args( line );

    std::string command_str = args.argv()[0] ? args.argv()[0] : "";

    if ( command_str == "\004" ) // ^D
    {
      cout << endl; // print newline after ^D
      break;
    }

    try
    {
      MIL << "Reloading..." << endl;
      God->target()->reload();   // reload system in case rpm database has changed
      doCommand( args.argc(), args.argv(), 0 );
    }
    catch ( const Exception & e )
    {
      out().error( e.msg() );
      print_unknown_command_hint( *this, command_str ); // TODO: command_str should come via the Exception, same for other print_unknown_command_hint's
    }

    if ( _continue_running_shell )
      shellCleanup();
  }

  if ( !histfile.empty() )
    write_history( histfile.c_str() );

  MIL << "Leaving the shell" << endl;
  setRunningShell( false );
  cleanup();
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

  // clear the command
  _command = ZypperCommand::NONE;
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
void Zypper::doCommand( int cmdArgc, char **cmdArgv, int firstFlag )
{
  try
  {
    MIL << "START" << endl;
    //  ======== get command ========
    if ( firstFlag < cmdArgc )
    {
      //if wantHelp is set, we need to force the HELP_e command
      if ( _config.wantHelp ) {
        firstFlag -= 1; //point the argparser to the cli command as a positional arg
        setCommand ( ZypperCommand( ZypperCommand::HELP_e ) );
      } else {
        //try to turn the command argument into its object representation
        try {
          setCommand( ZypperCommand( cmdArgv[firstFlag] ) );
        }
        // exception from command parsing
        catch ( const Exception & e )
        {
          out().error( e.asUserString() );
          print_unknown_command_hint( *this, cmdArgv[firstFlag] );
          setExitCode( ZYPPER_EXIT_ERR_SYNTAX );
          ZYPP_THROW( ExitRequestException("unknown command") );
        }

      }
    }

    if ( command() == ZypperCommand::NONE ) {
      if ( runningShell() ) {
        setExitCode( ZYPPER_EXIT_ERR_SYNTAX );
        ZYPP_THROW( ExitRequestException("unknown command") );
      }
      setCommand ( ZypperCommand( ZypperCommand::HELP_e ) );	// no command => global help
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

            // Use @System dir name because libzypp will not automatically remove it and there is no user defined repository with that name
            // Fixes bsc 1130873 - zypper package cache relocation broken
            repo.setPackagesPath( Pathname::assertprefix( _config.root_dir, _config.rm_options.repoPackagesCachePath ) / "@System" );

            _rdata.temporary_repos.push_back( repo );
            DBG << "got additional repo: " << url << endl;
            count++;
          }
        }
      }
      _config.plusRepoFromCLI.clear();
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
      _config.plusContentFromCLI.clear();
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
                    || command() == ZypperCommand::HELP
                    || command() == ZypperCommand::VERSION_CMP
                    || command() == ZypperCommand::TARGET_OS )
          zypp_readonly_hack::IWantIt (); // #247001, #302152
        }
        assertZYppPtrGod();
    }

    // === execute command ===

    MIL << "Going to process command " << command() << endl;

    // handle new style commands
    ZypperBaseCommandPtr newStyleCmd = command().commandObject();
    if ( newStyleCmd ) {

      //reset the command to default
      newStyleCmd->reset();

      MIL << "Found new style command << " << newStyleCmd->command().front() << endl;

      // parse command options
      try {
        //align argc and argv to the first flag
        //Remember that parseArguments will always ignore the very first argument in the array due to how getopt works
        //so we pass the command name here as well
        int myArgc    = cmdArgc - firstFlag;
        char **myArgv = cmdArgv + firstFlag;

        //parse all option flags
        int firstPositionalArg = newStyleCmd->parseArguments( *this, myArgc, myArgv );

        //make sure to calculate the correct index of the first positional arg
        searchPackagesHintHack::argvArgIdx = firstPositionalArg + firstFlag;

        std::vector<std::string> positionalArguments;
        if ( firstPositionalArg < myArgc ) {
          int counter = firstPositionalArg;
          std::ostringstream s;
          s << _("Non-option program arguments: ");
          while ( counter < myArgc )  {
            std::string argument = myArgv[counter++];
            s << "'" << argument << "' ";
            positionalArguments.push_back( argument );
          }
          out().info( s.str(), Out::HIGH );
        }

        newStyleCmd->setPositionalArguments( positionalArguments );

      } catch ( const ZyppFlags::ZyppFlagsException &e) {
        ERR << e.asString() << endl;
        out().error( e.asUserString() );
        setExitCode( ZYPPER_EXIT_ERR_SYNTAX );
        return;
      }

      if ( newStyleCmd->helpRequested() ) {
        ZypperCommand helpCmd ( ZypperCommand::HELP_e );
        HelpCmd &help = helpCmd.assertCommandObject<HelpCmd>();
        help.setPositionalArguments( { newStyleCmd->command().at(0) } );
        setExitCode ( help.run( *this ) );
      } else {
        setExitCode( newStyleCmd->run( *this ) );
      }

      MIL << "Done " << endl;
      return;
    }

    // if the program reaches this line, something went wrong
    report_a_bug( out() );
    setExitCode( ZYPPER_EXIT_ERR_BUG );
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

int Zypper::commandArgOffset() const
{
  return _commandArgOffset;
}

void Zypper::stopCommandShell()
{
  _continue_running_shell = false;
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
