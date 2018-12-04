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
#include "subcommand.h"

#include "output/OutNormal.h"
#include "output/OutXML.h"

#include "utils/flags/zyppflags.h"
#include "utils/flags/exceptions.h"
#include "global-settings.h"

#include "commands/commandhelpformatter.h"
#include "commands/locks.h"
#include "commands/search/search-packages-hinthack.h"
#include "commands/services/common.h"
#include "commands/services/refresh.h"
#include "commands/conditions.h"
#include "commands/reposerviceoptionsets.h"
#include "commands/repos/refresh.h"
#include "commands/search/search.h"
using namespace zypp;

bool sigExitOnce = true;	// Flag to prevent nested calls to Zypper::immediateExit

///////////////////////////////////////////////////////////////////
namespace cli
{
  inline std::string errorMutuallyExclusiveOptions( const std::string & options_r )
  {
    // translator: %1% is a list of command line option names
    return str::Format(_("These options are mutually exclusive: %1%")) % options_r;
  }

} // namespace cli
///////////////////////////////////////////////////////////////////


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

/// \todo Investigate why the global copts here is used in addition to
/// Zypper::_copts? There should not be 2 instances with possibly
/// different content. The global here is also unnecessarily exported
/// to utils/getopt.
parsed_opts copts; // command options

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

  inline void legacyCLITranslate( parsed_opts & copts_r, const std::string & old_r, const std::string & new_r, Out::Verbosity verbosity_r = Out::NORMAL, LegacyCLIMsgType type = LegacyCLIMsgType::Local )
  {
    if ( copts_r.count( old_r ) )
    {
      Zypper::instance().out().warning( legacyCLIStr( old_r, new_r, type ), verbosity_r );
      if ( new_r.size() ) {
        if ( ! copts_r.count( new_r ) )
          copts_r[new_r];
      }
      copts_r.erase( old_r );
    }
  }
} //namespace
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
Zypper::Zypper()
: _argc( 0 )
, _argv( NULL )
, _out_ptr( NULL )
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

	case ZypperCommand::SUBCOMMAND_e:
	  subcommand( *this );
	  break;

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


void print_main_help( Zypper & zypper )
{
  static std::string globalHelp = CommandHelpFormater()
  ///////////////////////////////////////////////////////////////////
  .gMainUsage()

  .gSynopsis( // translators: command synopsis; do not translate lowercase words
  _("zypper [--GLOBAL-OPTIONS] <COMMAND> [--COMMAND-OPTIONS] [ARGUMENTS]")
  )
  .gSynopsis( // translators: command synopsis; do not translate lowercase words
  _("zypper <SUBCOMMAND> [--COMMAND-OPTIONS] [ARGUMENTS]")
  )

  ///////////////////////////////////////////////////////////////////
  .gMainGlobalOpts()

  .gDef( "--help, -h",	// translators: --help, -h
	 _("Help.") )
  .gDef( "--version, -V",	// translators: --version, -V
	 _("Output the version number.") )
  .gDef( "--promptids",	// translators: --promptids
	 _("Output a list of zypper's user prompts.") )
  .gDef( "--config, -c <FILE>",	// translators: --config, -c <FILE>
	 _("Use specified config file instead of the default.") )
  .gDef( "--userdata <STRING>",	// translators: --userdata <STRING>
	 _("User defined transaction id used in history and plugins.") )
  .gDef( "--quiet, -q",	// translators: --quiet, -q
	 _("Suppress normal output, print only error messages.") )
  .gDef( "--verbose, -v",	// translators: --verbose, -v
	 _("Increase verbosity.") )
  .gDef( "--color" )
  .gDef( "--no-color",	// translators: --color / --no-color
	 _("Whether to use colors in output if tty supports it.") )
  .gDef( "--no-abbrev, -A",	// translators: --no-abbrev, -A
	 _("Do not abbreviate text in tables.") )
  .gDef( "--table-style, -s",	// translators: --table-style, -s
	 _("Table style (integer).") )
  .gDef( "--non-interactive, -n",	// translators: --non-interactive, -n
	 _("Do not ask anything, use default answers automatically.") )
  .gDef( "--non-interactive-include-reboot-patches",	// translators: --non-interactive-include-reboot-patches
	 _("Do not treat patches as interactive, which have the rebootSuggested-flag set.") )
  .gDef( "--xmlout, -x",	// translators: --xmlout, -x
	 _("Switch to XML output.") )
  .gDef( "--ignore-unknown, -i",	// translators: --ignore-unknown, -i
	 _("Ignore unknown packages.") )

  .gSection()
  .gDef( "--reposd-dir, -D <DIR>",	// translators: --reposd-dir, -D <DIR>
	 _("Use alternative repository definition file directory.") )
  .gDef( "--cache-dir, -C <DIR>",	// translators: --cache-dir, -C <DIR>
	 _("Use alternative directory for all caches.") )
  .gDef( "--raw-cache-dir <DIR>",	// translators: --raw-cache-dir <DIR>
	 _("Use alternative raw meta-data cache directory.") )
  .gDef( "--solv-cache-dir <DIR>",	// translators: --solv-cache-dir <DIR>
	 _("Use alternative solv file cache directory.") )
  .gDef( "--pkg-cache-dir <DIR>",	// translators: --pkg-cache-dir <DIR>
	 _("Use alternative package cache directory.") )

  .gSection( _("Repository Options:") )
  .gDef( "--no-gpg-checks",	// translators: --no-gpg-checks
	 _("Ignore GPG check failures and continue.") )
  .gDef( "--gpg-auto-import-keys",	// translators: --gpg-auto-import-keys
	 _("Automatically trust and import new repository signing keys.") )
  .gDef( "--plus-repo, -p <URI>",	// translators: --plus-repo, -p <URI>
	 _("Use an additional repository.") )
  .gDef( "--plus-content <TAG>",	// translators: --plus-content <TAG>
	 _("Additionally use disabled repositories providing a specific keyword. Try '--plus-content debug' to enable repos indicating to provide debug packages.") )
  .gDef( "--disable-repositories",	// translators: --disable-repositories
	 _("Do not read meta-data from repositories.") )
  .gDef( "--no-refresh",	// translators: --no-refresh
	 _("Do not refresh the repositories.") )
  .gDef( "--no-cd",	// translators: --no-cd
	 _("Ignore CD/DVD repositories.") )
  .gDef( "--no-remote",	// translators: --no-remote
	 _("Ignore remote repositories.") )
  .gDef( "--releasever",	// translators: --releasever
	 _("Set the value of $releasever in all .repo files (default: distribution version)") )

  .gSection( _("Target Options:") )
  .gDef( "--root, -R <DIR>",	// translators: --root, -R <DIR>
	 _("Operate on a different root directory.") )
  .gDef( "--installroot <DIR>",	// translators: --installroot <DIR>
	 _("Operate on a different root directory, but share repositories with the host.") )
  .gDef( "--disable-system-resolvables",	// translators: --disable-system-resolvables
	 _("Do not read installed packages.") )

  ///////////////////////////////////////////////////////////////////
  .gMainCommands()

  .gDef( "help, ?",	// translators: command summary: help, ?
	 _("Print help.") )
  .gDef( "shell, sh",	// translators: command summary: shell, sh
	 _("Accept multiple commands at once.") )

  .gSection( _("Repository Management:") )
  .gDef( "repos, lr",	// translators: command summary: repos, lr
	 _("List all defined repositories.") )
  .gDef( "addrepo, ar",	// translators: command summary: addrepo, ar
	 _("Add a new repository.") )
  .gDef( "removerepo, rr",	// translators: command summary: removerepo, rr
	 _("Remove specified repository.") )
  .gDef( "renamerepo, nr",	// translators: command summary: renamerepo, nr
	 _("Rename specified repository.") )
  .gDef( "modifyrepo, mr",	// translators: command summary: modifyrepo, mr
	 _("Modify specified repository.") )
  .gDef( "refresh, ref",	// translators: command summary: refresh, ref
	 _("Refresh all repositories.") )
  .gDef( "clean",	// translators: command summary: clean
	 _("Clean local caches.") )

  .gSection( _("Service Management:") )
  .gDef( "services, ls",	// translators: command summary: services, ls
	 _("List all defined services.") )
  .gDef( "addservice, as",	// translators: command summary: addservice, as
	 _("Add a new service.") )
  .gDef( "modifyservice, ms",	// translators: command summary: modifyservice, ms
	 _("Modify specified service.") )
  .gDef( "removeservice, rs",	// translators: command summary: removeservice, rs
	 _("Remove specified service.") )
  .gDef( "refresh-services, refs",	// translators: command summary: refresh-services, refs
	 _("Refresh all services.") )

  .gSection( _("Software Management:") )
  .gDef( "install, in",	// translators: command summary: install, in
	 _("Install packages.") )
  .gDef( "remove, rm",	// translators: command summary: remove, rm
	 _("Remove packages.") )
  .gDef( "verify, ve",	// translators: command summary: verify, ve
	 _("Verify integrity of package dependencies.") )
  .gDef( "source-install, si",	// translators: command summary: source-install, si
	 _("Install source packages and their build dependencies.") )
  .gDef( "install-new-recommends, inr",	// translators: command summary: install-new-recommends, inr
	 _("Install newly added packages recommended by installed packages.") )

  .gSection( _("Update Management:") )
  .gDef( "update, up",	// translators: command summary: update, up
	 _("Update installed packages with newer versions.") )
  .gDef( "list-updates, lu",	// translators: command summary: list-updates, lu
	 _("List available updates.") )
  .gDef( "patch",	// translators: command summary: patch
	 _("Install needed patches.") )
  .gDef( "list-patches, lp",	// translators: command summary: list-patches, lp
	 _("List needed patches.") )
  .gDef( "dist-upgrade, dup",	// translators: command summary: dist-upgrade, dup
	 _("Perform a distribution upgrade.") )
  .gDef( "patch-check, pchk",	// translators: command summary: patch-check, pchk
	 _("Check for patches.") )

  .gSection( _("Querying:") )
  .gDef( "search, se",	// translators: command summary: search, se
         _("Search for packages matching a pattern.") )
  .gDef( "info, if",	// translators: command summary: info, if
	 _("Show full information for specified packages.") )
  .gDef( "patch-info",	// translators: command summary: patch-info
	 _("Show full information for specified patches.") )
  .gDef( "pattern-info",	// translators: command summary: pattern-info
	 _("Show full information for specified patterns.") )
  .gDef( "product-info",	// translators: command summary: product-info
	 _("Show full information for specified products.") )
  .gDef( "patches, pch",	// translators: command summary: patches, pch
         _("List all available patches.") )
  .gDef( "packages, pa",	// translators: command summary: packages, pa
	 _("List all available packages.") )
  .gDef( "patterns, pt",	// translators: command summary: patterns, pt
	 _("List all available patterns.") )
  .gDef( "products, pd",	// translators: command summary: products, pd
	 _("List all available products.") )
  .gDef( "what-provides, wp",	// translators: command summary: what-provides, wp
         _("List packages providing specified capability.") )

  .gSection( _("Package Locks:") )
  .gDef( "addlock, al",	// translators: command summary: addlock, al
	 _("Add a package lock.") )
  .gDef( "removelock, rl",	// translators: command summary: removelock, rl
	 _("Remove a package lock.") )
  .gDef( "locks, ll",	// translators: command summary: locks, ll
	 _("List current package locks.") )
  .gDef( "cleanlocks, cl",	// translators: command summary: cleanlocks, cl
	 _("Remove unused locks.") )

  .gSection( _("Other Commands:") )
  .gDef( "versioncmp, vcmp",	// translators: command summary: versioncmp, vcmp
	 _("Compare two version strings.") )
  .gDef( "targetos, tos",	// translators: command summary: targetos, tos
	 _("Print the target operating system ID string.") )
  .gDef( "licenses",	// translators: command summary: licenses
	 _("Print report about licenses and EULAs of installed packages.") )
  .gDef( "download",	// translators: command summary: download
	 _("Download rpms specified on the commandline to a local directory.") )
  .gDef( "source-download",	// translators: command summary: source-download
	 _("Download source rpms for all installed packages to a local directory.") )
  .gDef( "needs-rebooting",	// translators: command summary: needs-rebooting
         _("Check if the needs-reboot flag was set.") )
  .gSection( _("Subcommands:") )
  .gDef( "subcommand",	// translators: command summary: subcommand
	 _("Lists available subcommands.") )
  ;

  zypper.out().info( globalHelp, Out::QUIET );
  print_command_help_hint( zypper );
  return;
}

void print_unknown_command_hint( Zypper & zypper, const std::string & cmd_r )
{
  zypper.out().info(
    // translators: %s is "help" or "zypper help" depending on whether
    // zypper shell is running or not
    str::Format(_("Type '%s' to get a list of global options and commands."))
    % (zypper.runningShell() ? "help" : "zypper help") );
  zypper.out().gap();
  zypper.out().info(
    // translators: %1% is the name of an (unknown) command
    // translators: %2% something providing more info (like 'zypper help subcommand')
    // translators: The word 'subcommand' also refers to a zypper command and should not be translated.
    str::Format(_("In case '%1%' is not a typo it's probably not a built-in command, but provided as a subcommand or plug-in (see '%2%').") )
    /*%1%*/ % cmd_r
    /*%2%*/ % "zypper help subcommand"
  );
  zypper.out().info(
    // translators: %1% and %2% are plug-in packages which might provide it.
    // translators: The word 'subcommand' also refers to a zypper command and should not be translated.
    str::Format(_("In this case a specific package providing the subcommand needs to be installed first. Those packages are often named '%1%' or '%2%'.") )
    /*%1%*/ % ("zypper-"+cmd_r)
    /*%2%*/ % ("zypper-"+cmd_r+"-plugin")
  );
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
  static const int indeterminate = -1;
  int optvalColor = indeterminate;

  static struct option global_options[] = {
    {"help",                       no_argument,       0, 'h'},
    {"verbose",                    no_argument,       0, 'v'},
    {"quiet",                      no_argument,       0, 'q'},
    {"version",                    no_argument,       0, 'V'},
    {"promptids",                  no_argument,       0,  0 },
    {"color",			   no_argument,	&optvalColor, 1},
    {"no-color",		   no_argument,	&optvalColor, 0},
    // rug compatibility alias for -vv
    {"debug",                      no_argument,       0,  0 },
    // rug compatibility alias for the default output level => ignored
    {"normal-output",              no_argument,       0,  0 },
    {"terse",                      no_argument,       0, 't'},
    {"no-abbrev",                  no_argument,       0, 'A'},
    {"table-style",                required_argument, 0, 's'},
    {"rug-compatible",             no_argument,       0, 'r'},	/* DEPRECATED and UNSUPPORTED SINCE SLE12 */
    {"non-interactive",            no_argument,       0, 'n'},
    {"non-interactive-include-reboot-patches", no_argument, 0, '0'},
    {"no-gpg-checks",              no_argument,       0,  0 },
    {"gpg-auto-import-keys",       no_argument,       0,  0 },
    {"root",                       required_argument, 0, 'R'},
    {"installroot",                required_argument, 0,  0 },
    {"reposd-dir",                 required_argument, 0, 'D'},
    {"cache-dir",                  required_argument, 0, 'C'},
    {"raw-cache-dir",              required_argument, 0,  0 },
    {"solv-cache-dir",             required_argument, 0,  0 },
    {"pkg-cache-dir",              required_argument, 0,  0 },
    {"opt",                        optional_argument, 0, 'o'},
    // TARGET OPTIONS
    {"disable-system-resolvables", no_argument,       0,  0 },
    // REPO OPTIONS
    {"plus-repo",                  required_argument, 0, 'p'},
    {"plus-content",               required_argument, 0,  0 },
    {"disable-repositories",       no_argument,       0,  0 },
    {"no-refresh",                 no_argument,       0,  0 },
    {"no-cd",                      no_argument,       0,  0 },
    {"no-remote",                  no_argument,       0,  0 },
    {"releasever",                 required_argument, 0,  0 },
    {"xmlout",                     no_argument,       0, 'x'},
    {"config",                     required_argument, 0, 'c'},
    {"userdata",                   required_argument, 0,  0 },
    {"ignore-unknown",             no_argument,       0, 'i'},
    {0, 0, 0, 0}
  };

  // ====== parse global options ======
  parsed_opts gopts = parse_options( _argc, _argv, global_options );
  searchPackagesHintHack::argvCmdIdx = optind;
  for ( const char * opterr : { "_unknown", "_missing_arg" } )
  {
    if ( gopts.count( opterr ) )
    {
      setExitCode( ZYPPER_EXIT_ERR_SYNTAX );
      ZYPP_THROW( ExitRequestException( std::string("global")+opterr ) );
    }
  }

  parsed_opts::const_iterator it;

  // read config from specified file or default config files
  _config.read( (it = gopts.find("config")) != gopts.end() ? it->second.front() : "" );

  // ====== output setup ======
  // depends on global options, that's we set it up here
  //! \todo create a default in the zypper constructor, recreate here.

  // determine the desired verbosity
  int iverbosity = 0;
  //// --quiet
  if ( gopts.count("quiet") )
  {
    _gopts.verbosity = iverbosity = -1;
    DBG << "Verbosity " << _gopts.verbosity << endl;
  }
  //// --verbose
  if ( (it = gopts.find("verbose")) != gopts.end() )
  {
    //! \todo if iverbosity is -1 now, say we conflict with -q
    _gopts.verbosity += iverbosity = it->second.size();
    // _gopts.verbosity += gopts["verbose"].size();
  }

  Out::Verbosity verbosity = Out::NORMAL;
  switch( iverbosity )
  {
    case -1: verbosity = Out::QUIET; break;
    case 0: verbosity = Out::NORMAL; break;
    case 1: verbosity = Out::HIGH; break;
    default: verbosity = Out::DEBUG;
  }

  //// --debug
  // rug compatibility alias for -vv
  if ( gopts.count("debug") )
    verbosity = Out::DEBUG;

  if ( gopts.count("terse") )
  {
    _gopts.machine_readable = true;
    _gopts.no_abbrev = true;
    _gopts.terse = true;
    if ( optvalColor == indeterminate )
      optvalColor = false;
  }

  // adjust --[no-]color from CLI
  if ( optvalColor != indeterminate )
    _config.do_colors = optvalColor;

  // create output object
  //// --xml-out
  if ( gopts.count("xmlout") )
  {
    _config.do_colors = false;	// no color in xml mode!
    _out_ptr = new OutXML( verbosity );
    _gopts.machine_readable = true;
    _gopts.no_abbrev = true;
  }
  else
  {
    OutNormal * p = new OutNormal( verbosity );
    p->setUseColors( _config.do_colors );
    _out_ptr = p;
  }

  out().info( str::Format(_("Verbosity: %d")) % _gopts.verbosity , Out::HIGH );
  DBG << "Verbosity " << verbosity << endl;
  DBG << "Output type " << _out_ptr->type() << endl;

  if ( gopts.count("no-abbrev") )
    _gopts.no_abbrev = true;

  if ( (it = gopts.find("table-style")) != gopts.end() )
  {
    unsigned s;
    str::strtonum( it->second.front(), s );
    if ( s < TLS_End )
      Table::defaultStyle = (TableLineStyle)s;
    else
      out().error( str::Format(_("Invalid table style %d.")) % s,
		   str::Format(_("Use an integer number from %d to %d")) % 0 % 8 );
  }

  //  ======== get command ========
  // Print and exit global opts: --version and --propmtids
  // Actually they should be turned into a command ...
  if ( gopts.count("version") )
  {
    out().info( PACKAGE " " VERSION, Out::QUIET );
    ZYPP_THROW( ExitRequestException("version shown") );
  }
  if ( gopts.count("promptids") )
  {
    #define PR_ENUML(nam, val) out().info(#nam "=" #val, Out::QUIET);
    #define PR_ENUM(nam, val) PR_ENUML(nam, val)
    #include "output/prompt.h"
    ZYPP_THROW( ExitRequestException("promptids shown") );
  }

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
  if ( gopts.count( "help" ) )
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
      print_main_help( *this );
      ZYPP_THROW( ExitRequestException("help provided") );
    }
    else if ( command() == ZypperCommand::HELP )// help on help
    {
      print_main_help( *this );
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
    shared_ptr<SubcommandOptions> myOpts( assertCommandOptions<SubcommandOptions>() );
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

  // ======== other global options ========

  if ( (it = gopts.find( "releasever" )) != gopts.end() )
  {
    ::setenv( "ZYPP_REPO_RELEASEVER", it->second.front().c_str(), 1 );
  }
  {
    const char * env = ::getenv( "ZYPP_REPO_RELEASEVER" );
    if ( env && *env )
    {
      out().warning( str::Str() << _("Enforced setting") << ": $releasever=" << env );
      WAR << "Enforced setting: $releasever=" << env << endl;
    }
  }


  if ( (it = gopts.find( "userdata" )) != gopts.end() )
  {
    if ( ! ZConfig::instance().setUserData( it->second.front() ) )
    {
      out().error(_("User data string must not contain nonprintable or newline characters!"));
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      ZYPP_THROW( ExitRequestException("userdata") );
    }
  }

  ///////////////////////////////////////////////////////////////////
  // Rug compatibility is dropped since SLE12.
  // Rug options are removed from documantation(commit#53ffd419) but
  // will stay active in code for a while.
  std::string rug_test( _argv[0] );
  if ( gopts.count("rug-compatible") || Pathname::basename( _argv[0] ) == "rug" )
  {
    out().error("************************************************************************");
    out().error("** Rug-compatible mode is no longer available. [-r,--rug-compatible]");
    out().error("************************************************************************");
    setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
    ZYPP_THROW( ExitRequestException("rug-compatible") );
  }
  ///////////////////////////////////////////////////////////////////

  if ( gopts.count("non-interactive") )
  {
    _gopts.non_interactive = true;
    out().info(_("Entering non-interactive mode."), Out::HIGH );
    MIL << "Entering non-interactive mode" << endl;
  }

  if ( gopts.count("non-interactive-include-reboot-patches") )
  {
    _gopts.reboot_req_non_interactive = true;
    out().info(_("Patches having the flag rebootSuggested set will not be treated as interactive."), Out::HIGH );
    MIL << "Patches having the flag rebootSuggested set will not be treated as interactive" << endl;
  }

  if ( gopts.count("no-gpg-checks") )
  {
    _gopts.no_gpg_checks = true;
    out().info(_("Entering 'no-gpg-checks' mode."), Out::HIGH );
    MIL << "Entering no-gpg-checks mode" << endl;
  }

  if ( gopts.count("gpg-auto-import-keys") )
  {
    _gopts.gpg_auto_import_keys = true;
    std::string warn = str::form(
      _("Turning on '%s'. New repository signing keys will be automatically imported!"),
      "--gpg-auto-import-keys");
    out().warning( warn, Out::HIGH );
    MIL << "gpg-auto-import-keys is on" << endl;
  }

  if ( (it = gopts.find("root")) != gopts.end() || (it = gopts.find("installroot")) != gopts.end() )
  {
    if ( gopts.find("root") != gopts.end() && gopts.find("installroot") != gopts.end() )
    {
      out().error( cli::errorMutuallyExclusiveOptions( "--root --installroot" ) );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      ZYPP_THROW( ExitRequestException("invalid args") );
    }
    _gopts.root_dir = it->second.front();
    _gopts.changedRoot = true;
    _gopts.is_install_root = (it->first == "installroot");

    //make sure ZConfig knows the RepoManager root is not inside the target rootfs
    if ( _gopts.is_install_root )
      ZConfig::instance().setRepoManagerRoot("/");

    Pathname tmp( _gopts.root_dir );
    if ( !tmp.absolute() )
    {
      out().error(_("The path specified in the --root option must be absolute."));
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      ZYPP_THROW( ExitRequestException("invalid args") );
    }

    DBG << "root dir = " << _gopts.root_dir << " is_install_root = " << _gopts.is_install_root << endl;
    if (_gopts.is_install_root)
      _gopts.rm_options = RepoManagerOptions("/");
    else
      _gopts.rm_options = RepoManagerOptions(_gopts.root_dir);
  }

  // on the fly check the baseproduct symlink
  {
    PathInfo pi( _gopts.root_dir + "/etc/products.d/baseproduct" );
    if ( ! pi.isFile() && PathInfo( _gopts.root_dir + "/etc/products.d" ).isDir() )
    {
      ERR << "baseproduct symlink is dangling or missing: " << pi << endl;
      out().warning(_(
	"The /etc/products.d/baseproduct symlink is dangling or missing!\n"
	"The link must point to your core products .prod file in /etc/products.d.\n"
      ));
    }
  }

  if ( (it = gopts.find("reposd-dir")) != gopts.end() )
  {
    _gopts.rm_options.knownReposPath = it->second.front();
  }

  // cache dirs

  ZConfig &zconfig = ZConfig::instance();
  if ( (it = gopts.find("cache-dir")) != gopts.end() )
  {
    zconfig.setRepoCachePath( it->second.front() );
    _gopts.rm_options.repoCachePath		= zconfig.repoCachePath();
    _gopts.rm_options.repoRawCachePath		= zconfig.repoMetadataPath();
    _gopts.rm_options.repoSolvCachePath		= zconfig.repoSolvfilesPath();
    _gopts.rm_options.repoPackagesCachePath	= zconfig.repoPackagesPath();
  }

  if ( (it = gopts.find("raw-cache-dir")) != gopts.end() ) {
    zconfig.setRepoMetadataPath( it->second.front() );
    _gopts.rm_options.repoRawCachePath = zconfig.repoMetadataPath();
  }

  if ( (it = gopts.find("solv-cache-dir")) != gopts.end() ){
    zconfig.setRepoSolvfilesPath( it->second.front() );
    _gopts.rm_options.repoSolvCachePath = zconfig.repoSolvfilesPath();
  }

  if ( (it = gopts.find("pkg-cache-dir")) != gopts.end() ){
    zconfig.setRepoPackagesPath( it->second.front() );
    _gopts.rm_options.repoPackagesCachePath = zconfig.repoPackagesPath();
  }

  DBG << "repos.d dir = " << _gopts.rm_options.knownReposPath << endl;
  DBG << "cache dir = " << _gopts.rm_options.repoCachePath << endl;
  DBG << "raw cache dir = " << _gopts.rm_options.repoRawCachePath << endl;
  DBG << "solv cache dir = " << _gopts.rm_options.repoSolvCachePath << endl;
  DBG << "package cache dir = " << _gopts.rm_options.repoPackagesCachePath << endl;

  if ( gopts.count("disable-repositories") )
  {
    MIL << "Repositories disabled, using target only." << endl;
    out().info(
      _("Repositories disabled, using the database of installed packages only."),
      Out::HIGH);
    _gopts.disable_system_sources = true;
  }
  else
  {
    MIL << "Repositories enabled" << endl;
  }

  if ( gopts.count("no-refresh") )
  {
    _gopts.no_refresh = true;
    out().info(_("Autorefresh disabled."), Out::HIGH );
    MIL << "Autorefresh disabled." << endl;
  }

  if ( gopts.count("no-cd") )
  {
    _gopts.no_cd = true;
    out().info(_("CD/DVD repositories disabled."), Out::HIGH );
    MIL << "No CD/DVD repos." << endl;
  }

  if ( gopts.count("no-remote") )
  {
    _gopts.no_remote = true;
    out().info(_("Remote repositories disabled."), Out::HIGH );
    MIL << "No remote repos." << endl;
  }

  if ( gopts.count("disable-system-resolvables") )
  {
    MIL << "System resolvables disabled" << endl;
    out().info(_("Ignoring installed resolvables."), Out::HIGH );
    _gopts.disable_system_resolvables = true;
  }

  // testing option
  if ( (it = gopts.find("opt")) != gopts.end() )
  {
    cout << "Opt arg: ";
    std::copy( it->second.begin(), it->second.end(), std::ostream_iterator<std::string>( cout, ", " ) );
    cout << endl;
  }

  // additional repositories by URL
  if ( gopts.count("plus-repo") )
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
      std::list<std::string> repos = gopts["plus-repo"];

      int count = 1;
      for ( std::list<std::string>::const_iterator it = repos.begin(); it != repos.end(); ++it )
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
	repo.setPackagesPath( Pathname::assertprefix( _gopts.root_dir, ZYPPER_RPM_CACHE_DIR ) );

        _rdata.temporary_repos.push_back( repo );
        DBG << "got additional repo: " << url << endl;
        count++;
      }
    }
    }
  }

  // additional repositories by content (keywords)
  if ( gopts.count("plus-content") )
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
      const std::list<std::string> & content( gopts["plus-content"] );
      _rdata.plusContentRepos.insert( content.begin(), content.end() );
    }
    }
  }

  if ( gopts.count("ignore-unknown") )
    _gopts.ignore_unknown = true;

  MIL << "DONE" << endl;
}


void Zypper::commandShell()
{
  MIL << "Entering the shell" << endl;

  setRunningShell( true );

  if ( _gopts.changedRoot && _gopts.root_dir != "/" )
  {
    // bnc#575096: Quick fix
    ::setenv( "ZYPP_LOCKFILE_ROOT", _gopts.root_dir.c_str(), 0 );
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
  // clear command options
  if (!_copts.empty())
    _copts.clear();
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

  struct option no_options = { 0, 0, 0, 0 };
  struct option *specific_options = &no_options;

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
        print_main_help( *this );
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

    MIL << "New Style command done " << endl;
    return;
  }

  switch ( command().toEnum() )
  {
  // print help on help and return
  // this should work for both, in shell and out of shell
  case ZypperCommand::HELP_e:
  {
    print_main_help( *this );
    ZYPP_THROW( ExitRequestException("help provided") );
  }

  case ZypperCommand::SUBCOMMAND_e:
  {
    // This is different from other commands: Executed in main;
    // here we are prepared for showing help only.
    if ( ! runningHelp() )
      setRunningHelp( true );

    static struct option options[] = {
      {0, 0, 0, 0}
    };
    specific_options = options;

    shared_ptr<SubcommandOptions> myOpts( assertCommandOptions<SubcommandOptions>() );
    myOpts->loadDetected();
    // the following will either pop up a manpage or return a string to be displayed.
    _command_help = assertCommandOptions<SubcommandOptions>()->helpString();
    break;
  }

  default:
  {
    if ( runningHelp() )
      break;

    ERR << "Unknown or unexpected command" << endl;
    out().error(_("Unexpected program flow."));
    report_a_bug( out() );
  }
  }

  // no need to parse command options if we already know we just want help
  if ( runningHelp() )
    return;

  // parse command options
  _copts = parse_options( argc(), argv(), specific_options );
  searchPackagesHintHack::argvArgIdx = optind;
  if ( _copts.count("_unknown") || _copts.count("_missing_arg") )
  {
    setExitCode( ZYPPER_EXIT_ERR_SYNTAX );
    ERR << "Unknown option or missing argument, returning." << endl;
    return;
  }

  //---------------------------------------------------------------------------
  //@TODO REMOVE THIS CODE WITH COPTS

  GlobalSettings::reset();

  // set the global dry-run setting
  DryRunSettings::instanceNoConst()._enabled = _copts.count("dry-run");

  // set the global repo setting
  parsed_opts::const_iterator it;
  auto &repoFilter = InitRepoSettings::instanceNoConst()._repoFilter;
  // --repo
  if ( ( it = copts.find("repo")) != copts.end() )
     repoFilter.insert( repoFilter.end(), it->second.begin(), it->second.end() );
  // --catalog - rug compatibility
  if ( ( it = copts.find("catalog")) != copts.end() )
     repoFilter.insert( repoFilter.end(), it->second.begin(), it->second.end() );
  //---------------------------------------------------------------------------

  // Leagcy cli translations (mostly from rug to zypper)
  legacyCLITranslate( _copts, "agree-to-third-party-licenses",	"auto-agree-with-licenses" );
  legacyCLITranslate( _copts, "sort-by-catalog",		"sort-by-repo" );
  legacyCLITranslate( _copts, "uninstalled-only",		"not-installed-only",	Out::HIGH );	// bsc#972997: Prefer --not-installed-only over misleading --uninstalled-only

  // bsc#957862: pkg/apt/yum user convenience: no-confirm  ==> --non-interactive
  if ( _copts.count("no-confirm") )
  {
    if ( ! _gopts.non_interactive )
    {
      out().info(_("Entering non-interactive mode."), Out::HIGH );
      MIL << "Entering non-interactive mode" << endl;
     _gopts.non_interactive = true;
    }
  }

  ::copts = _copts;
  MIL << "Done parsing options." << endl;

  // treat --help command option like global --help option from now on
  // i.e. when used together with command to print command specific help
  setRunningHelp( runningHelp() || copts.count("help") );

  if ( optind < argc() )
  {
    std::ostringstream s;
    s << _("Non-option program arguments: ");
    while ( optind < argc() )
    {
      std::string argument = argv()[optind++];
      s << "'" << argument << "' ";
      _arguments.push_back( argument );
    }
    out().info( s.str(), Out::HIGH );
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
      if ( _gopts.changedRoot && _gopts.root_dir != "/" )
      {
	// bnc#575096: Quick fix
	::setenv( "ZYPP_LOCKFILE_ROOT", _gopts.root_dir.c_str(), 0 );
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

  ResObject::Kind kind;
  switch( command().toEnum() )
  {

  // dummy commands
  case ZypperCommand::SUBCOMMAND_e:	// subcommands are not expected to be executed here!
  default:
    // if the program reaches this line, something went wrong
    setExitCode( ZYPPER_EXIT_ERR_BUG );
  }
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

// Local Variables:
// c-basic-offset: 2
// End:
