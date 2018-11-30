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
#include "configtest.h"
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
using namespace zypp;

bool sigExitOnce = true;	// Flag to prevent nested calls to Zypper::immediateExit

///////////////////////////////////////////////////////////////////
// for now use some defines to have consistent definition of args
// used across multiple commands

// Common modify Repo/Service aggregate options (argdef only)
#define ARG_REPO_SERVICE_COMMON_AGGREGATE	\
    {"all",		no_argument,		0, 'a' },	\
    {"local",		no_argument,		0, 'l' },	\
    {"remote",		no_argument,		0, 't' },	\
    {"medium-type",	required_argument,	0, 'm' }


// Common Repo/Service properties (argdef only)
// LEGACY: --refresh short option was -f in ADD_REPO, -r in all other Repo/Service commands.
//         Unfortunately -r is already --repo in ADD_REPO, so switching all Repo/Service commands
//         to prefer -f/F.
#define ARG_REPO_SERVICE_COMMON_PROP	\
    {"name",		required_argument,	0, 'n'},	\
    {"enable",		no_argument,		0, 'e'},	\
    {"disable",		no_argument,		0, 'd'},	\
    {"refresh",		no_argument,		0, 'f'},	\
    {"no-refresh",	no_argument,		0, 'F'}


// Add/Mod Service property settings
#define ARG_SERVICE_PROP	\
    ARG_REPO_SERVICE_COMMON_PROP

#define option_SERVICE_PROP	\
     option( "-n, --name <NAME>",	_("Set a descriptive name for the service.") )		\
    .option( "-e, --enable",	        _("Enable a disabled service.") )			\
    .option( "-d, --disable",	        _("Disable the service (but don't remove it).") )	\
    .option( "-f, --refresh",	        _("Enable auto-refresh of the service.") )		\
    .option( "-F, --no-refresh",	_("Disable auto-refresh of the service.") )


// Add/Mod Repo property settings
#define ARG_REPO_PROP	\
    ARG_REPO_SERVICE_COMMON_PROP,	\
    {"priority", 			required_argument,	0, 'p'},	\
    {"keep-packages",			no_argument,		0, 'k'},	\
    {"no-keep-packages",		no_argument,		0, 'K'},	\
    {"gpgcheck",			no_argument,		0, 'g'},	\
    {"gpgcheck-strict",			no_argument,		0,  0 },	\
    {"gpgcheck-allow-unsigned",		no_argument,		0,  0 },	\
    {"gpgcheck-allow-unsigned-repo",	no_argument,		0,  0 },	\
    {"gpgcheck-allow-unsigned-package",	no_argument,		0,  0 },	\
    {"no-gpgcheck",			no_argument,		0, 'G'},	\
    {"default-gpgcheck",		no_argument,		0,  0 }

#define option_REPO_PROP	\
     option( "-n, --name <NAME>",	_("Set a descriptive name for the repository.") )	\
    .option( "-e, --enable",	        _("Enable a disabled repository.") )			\
    .option( "-d, --disable",	        _("Disable the repository (but don't remove it).") )	\
    .option( "-f, --refresh",	        _("Enable auto-refresh of the repository.") )		\
    .option( "-F, --no-refresh",	_("Disable auto-refresh of the repository.") )		\
    .option( "-p, --priority <INTEGER>",_("Set priority of the repository.") )			\
    .option( "-k, --keep-packages",	_("Enable RPM files caching.") )			\
    .option( "-K, --no-keep-packages",	_("Disable RPM files caching.") )			\
    .option( "-g, --gpgcheck",			_("Enable GPG check for this repository.") )	\
    .option( "--gpgcheck-strict",		_("Enable strict GPG check for this repository.") )	\
    .option( "--gpgcheck-allow-unsigned",	str::Format(_("Short hand for '%1%'.") ) % "--gpgcheck-allow-unsigned-repo --gpgcheck-allow-unsigned-package" )	\
    .option( "--gpgcheck-allow-unsigned-repo",	_("Enable GPG check but allow the repository metadata to be unsigned.") )	\
    .option( "--gpgcheck-allow-unsigned-package",_("Enable GPG check but allow installing unsigned packages from this repository.") )	\
    .option( "-G, --no-gpgcheck",		_("Disable GPG check for this repository.") )	\
    .option( "--default-gpgcheck",		_("Use the global GPG check setting defined in /etc/zypp/zypp.conf. This is the default.") )	\

#define option_REPO_AGGREGATES \
   option( "-a, --all",			_("Apply changes to all repositories.") ) \
  .option( "-l, --local",		_("Apply changes to all local repositories.") ) \
  .option( "-t, --remote",		_("Apply changes to all remote repositories.") ) \
  .option( "-m, --medium-type <TYPE>",	_("Apply changes to repositories of specified type.") )


// bsc#972997: Prefer --not-installed-only over misleading --uninstalled-only
#define ARG_not_INSTALLED_ONLY	\
    {"installed-only",		no_argument, 0, 'i'},	\
    {"not-installed-only",	no_argument, 0, 'u'},	\
    {"uninstalled-only",	no_argument, 0,  0 }

// Solver flag options common to all solving commands
// (install remove update dup patch verify inr)
#define ARG_Solver_Flags_Common	\
    {"debug-solver",		no_argument, 0,  0 },	\
    {"force-resolution",	no_argument, 0,  0 },	\
    {"no-force-resolution",	no_argument, 0, 'R'}

#define option_Solver_Flags_Common	\
     option( "--debug-solver",		_("Create a solver test case for debugging.") )	\
    .option( "--force-resolution",	_("Force the solver to find a solution (even an aggressive one) rather than asking.") )	\
    .option( "--no-force-resolution",	_("Do not force the solver to find solution, let it ask.") )

// Solver flag with/without recommends
// (not used in remove and inr)
#define ARG_Solver_Flags_Recommends	\
    {"recommends",		no_argument, 0,  0 },	\
    {"no-recommends",		no_argument, 0,  0 }

#define option_Solver_Flags_Recommends	\
     option( "--recommends",		_("Install also recommended packages in addition to the required ones.") )	\
    .option( "--no-recommends	",	_("Do not install recommended packages, only required ones.") )

// auto license agreements
#define ARG_License_Agreement	\
    {"auto-agree-with-licenses",	no_argument,	0, 'l' },	\
    {"auto-agree-with-product-licenses",no_argument,	0,  0  }	// Mainly for SUSEConnect, not (yet) documented

#define option_License_Agreement	\
     option( "-l, --auto-agree-with-licenses",	_("Automatically say 'yes' to third party license confirmation prompt. See 'man zypper' for more details.") ),	\
    .option( "--auto-agree-with-product-licenses",_("Automatically accept product licenses only. See 'man zypper' for more details.") )

// with/without optional patches
#define ARG_WITHout_OPTIONAL	\
    {"with-optional",			no_argument,		&_gopts.exclude_optional_patches, 0 },	\
    {"without-optional",		no_argument,		&_gopts.exclude_optional_patches, 1 }

#define option_WITHout_OPTIONAL	\
     option( "--with-optional" )	\
    .option( "--without-optional",	_("Whether applicable optional patches should be treated as needed or be excluded.")	\
    + std::string(" ")					\
    + ( _gopts.exclude_optional_patches_default		\
    ? _("The default is to exclude optional patches.")	\
    : _("The default is to include optional patches.") ))

#define ARG_Solver_Flags_Installs \
  {"allow-downgrade",           no_argument,       &myOpts->_allowDowngrade, 1 },   \
  {"no-allow-downgrade",        no_argument,       &myOpts->_allowDowngrade, 0 },   \
  {"allow-name-change",         no_argument,       &myOpts->_allowNameChange, 1 },  \
  {"no-allow-name-change",      no_argument,       &myOpts->_allowNameChange, 0 },  \
  {"allow-arch-change",         no_argument,       &myOpts->_allowArchChange, 1 },  \
  {"no-allow-arch-change",      no_argument,       &myOpts->_allowArchChange, 0 },  \
  {"allow-vendor-change",       no_argument,       &myOpts->_allowVendorChange, 1 },\
  {"no-allow-vendor-change",    no_argument,       &myOpts->_allowVendorChange, 0 }

#define option_Solver_Flags_Installs \
   option( "--allow-downgrade" )  \
  .option( "--no-allow-downgrade",		_("Whether to allow downgrading installed resolvables.") ) \
  .option( "--allow-name-change" ) \
  .option( "--no-allow-name-change",	_("Whether to allow changing the names of installed resolvables.") ) \
  .option( "--allow-arch-change" ) \
  .option( "--no-allow-arch-change",	_("Whether to allow changing the architecture of installed resolvables.") ) \
  .option( "--allow-vendor-change" ) \
  .option( "--no-allow-vendor-change",	_("Whether to allow changing the vendor of installed resolvables.") )

///////////////////////////////////////////////////////////////////
namespace cli
{
  inline std::string errorMutuallyExclusiveOptions( const std::string & options_r )
  {
    // translator: %1% is a list of command line option names
    return str::Format(_("These options are mutually exclusive: %1%")) % options_r;
  }

  RepoInfo::GpgCheck gpgCheck( Zypper & zypper )
  {
    RepoInfo::GpgCheck ret = RepoInfo::GpgCheck::indeterminate;
    bool	fail = false;
    std::string	failDetail;

    typedef std::pair<RepoInfo::GpgCheck,const char *> Pair;
    for ( const Pair & p : {
      Pair{ RepoInfo::GpgCheck::On,			"gpgcheck"				},
      Pair{ RepoInfo::GpgCheck::Strict,			"gpgcheck-strict"			},
      Pair{ RepoInfo::GpgCheck::AllowUnsigned,		"gpgcheck-allow-unsigned"		},
      Pair{ RepoInfo::GpgCheck::AllowUnsignedRepo,	"gpgcheck-allow-unsigned-repo"		},
      Pair{ RepoInfo::GpgCheck::AllowUnsignedPackage,	"gpgcheck-allow-unsigned-package"	},
      Pair{ RepoInfo::GpgCheck::Default,		"default-gpgcheck"			},
      Pair{ RepoInfo::GpgCheck::Off,			"no-gpgcheck"				},
    } )
    {
      if ( copts.count( p.second ) )
      {
	if ( ret == RepoInfo::GpgCheck::indeterminate )
	{
	  ret = p.first;
	  failDetail = p.second;
	}
	else
	{
	  fail = true;
	  failDetail += " --";
	  failDetail += p.second;
	}
      }
    }
    if ( fail )
    {
      zypper.out().error( errorMutuallyExclusiveOptions( dashdash(failDetail) ) );
      zypper.setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      ZYPP_THROW( ExitRequestException("invalid args") );
    }
    return ret;
  }

  /** Make sure only one of \a args_r options is specified on the command line.
   * \throw ExitRequestException if multiple args are specified
   * \return the cli arg or \c nullptr
   */
  const char * assertMutuallyExclusiveArgs( Zypper & zypper, const std::initializer_list<const char *> & args_r  )
  {
    const char * ret = nullptr;
    for ( const char * arg : args_r )
    {
      if ( copts.count(arg) )
      {
	if ( ret )
	{
	  zypper.out().error( errorMutuallyExclusiveOptions( text::join( dashdash(ret), dashdash(arg) ) ) );
	  zypper.setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
	  ZYPP_THROW( ExitRequestException("invalid args") );
	}
	ret = arg;
      }
    }
    return ret;
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

  // search helper
  inline bool poolExpectMatchFor( const std::string & name_r, const Edition & edition_r )
  {
    for ( const auto & pi : ResPool::instance().byName( name_r ) )
    {
      if ( Edition::match( pi.edition(), edition_r ) == 0 )
	return true;
    }
    return false;
  }

  bool checkRequiredCapabilities ( Zypper & zypper, GlobalOptions &gopts_r )
  {
    switch ( zypper.command().toEnum() ) {
      case ZypperCommand::UPDATE_e:
      case ZypperCommand::PATCH_e:
#if 0
      // bsc#1108999: Quick fix: Allow repo commands on transactional-server (/etc is rw)
      case ZypperCommand::ADD_REPO_e:
      case ZypperCommand::REMOVE_REPO_e:
      case ZypperCommand::MODIFY_REPO_e:
      case ZypperCommand::RENAME_REPO_e:
      case ZypperCommand::ADD_SERVICE_e:
      case ZypperCommand::REMOVE_SERVICE_e:
      case ZypperCommand::MODIFY_SERVICE_e:
#endif
    {
        std::string msg;
        NeedsWritableRoot rt;
        if ( rt.check( msg ) != ZYPPER_EXIT_OK ) {
          zypper.out().errorPar( msg );
          return false;
        }
        break;
      }
      default:
        break;
    }
    return true;
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

#if 0
  static std::string help_global_options = _("  Global Options:\n"
    "\t--help, -h\t\tHelp.\n"
    "\t--version, -V\t\tOutput the version number.\n"
    "\t--promptids\t\tOutput a list of zypper's user prompts.\n"
    "\t--config, -c <file>\tUse specified config file instead of the default.\n"
    "\t--userdata <string>\tUser defined transaction id used in history and plugins.\n"
    "\t--quiet, -q\t\tSuppress normal output, print only error\n"
    "\t\t\t\tmessages.\n"
    "\t--verbose, -v\t\tIncrease verbosity.\n"
    "\t--color\n"
    "\t--no-color\t\tWhether to use colors in output if tty supports it.\n"
    "\t--no-abbrev, -A\t\tDo not abbreviate text in tables.\n"
    "\t--table-style, -s\tTable style (integer).\n"
    "\t--non-interactive, -n\tDo not ask anything, use default answers\n"
    "\t\t\t\tautomatically.\n"
    "\t--non-interactive-include-reboot-patches\n"
    "\t\t\t\tDo not treat patches as interactive, which have\n"
    "\t\t\t\tthe rebootSuggested-flag set.\n"
    "\t--xmlout, -x\t\tSwitch to XML output.\n"
    "\t--ignore-unknown, -i\tIgnore unknown packages.\n"
  );

  static std::string repo_manager_options = _(
    "\t--reposd-dir, -D <dir>\tUse alternative repository definition file\n"
    "\t\t\t\tdirectory.\n"
    "\t--cache-dir, -C <dir>\tUse alternative directory for all caches.\n"
    "\t--raw-cache-dir <dir>\tUse alternative raw meta-data cache directory.\n"
    "\t--solv-cache-dir <dir>\tUse alternative solv file cache directory.\n"
    "\t--pkg-cache-dir <dir>\tUse alternative package cache directory.\n"
  );

  static std::string help_global_repo_options = _("     Repository Options:\n"
    "\t--no-gpg-checks\t\tIgnore GPG check failures and continue.\n"
    "\t--gpg-auto-import-keys\tAutomatically trust and import new repository\n"
    "\t\t\t\tsigning keys.\n"
    "\t--plus-repo, -p <URI>\tUse an additional repository.\n"
    "\t--plus-content <tag>\tAdditionally use disabled repositories providing a specific keyword.\n"
    "\t\t\t\tTry '--plus-content debug' to enable repos indicating to provide debug packages.\n"
    "\t--disable-repositories\tDo not read meta-data from repositories.\n"
    "\t--no-refresh\t\tDo not refresh the repositories.\n"
    "\t--no-cd\t\t\tIgnore CD/DVD repositories.\n"
    "\t--no-remote\t\tIgnore remote repositories.\n"
    "\t--releasever\t\tSet the value of $releasever in all .repo files (default: distribution version)\n"
  );

  static std::string help_global_target_options = _("     Target Options:\n"
    "\t--root, -R <dir>\tOperate on a different root directory.\n"
    "\t--disable-system-resolvables\n"
    "\t\t\t\tDo not read installed packages.\n"
  );
  help_global_target_options += str::Format(
    "\t--installroot <dir>\t%1%\n" ) % _("Operate on a different root directory, but share repositories with the host."
  );

  static std::string help_commands = _(
    "  Commands:\n"
    "\thelp, ?\t\t\tPrint help.\n"
    "\tshell, sh\t\tAccept multiple commands at once.\n"
  );

  static std::string help_repo_commands = _("     Repository Management:\n"
    "\trepos, lr\t\tList all defined repositories.\n"
    "\taddrepo, ar\t\tAdd a new repository.\n"
    "\tremoverepo, rr\t\tRemove specified repository.\n"
    "\trenamerepo, nr\t\tRename specified repository.\n"
    "\tmodifyrepo, mr\t\tModify specified repository.\n"
    "\trefresh, ref\t\tRefresh all repositories.\n"
    "\tclean\t\t\tClean local caches.\n"
  );

  static std::string help_service_commands = _("     Service Management:\n"
    "\tservices, ls\t\tList all defined services.\n"
    "\taddservice, as\t\tAdd a new service.\n"
    "\tmodifyservice, ms\tModify specified service.\n"
    "\tremoveservice, rs\tRemove specified service.\n"
    "\trefresh-services, refs\tRefresh all services.\n"
  );

  static std::string help_package_commands = _("     Software Management:\n"
    "\tinstall, in\t\tInstall packages.\n"
    "\tremove, rm\t\tRemove packages.\n"
    "\tverify, ve\t\tVerify integrity of package dependencies.\n"
    "\tsource-install, si\tInstall source packages and their build\n"
    "\t\t\t\tdependencies.\n"
    "\tinstall-new-recommends, inr\n"
    "\t\t\t\tInstall newly added packages recommended\n"
    "\t\t\t\tby installed packages.\n"
  );

  static std::string help_update_commands = _("     Update Management:\n"
    "\tupdate, up\t\tUpdate installed packages with newer versions.\n"
    "\tlist-updates, lu\tList available updates.\n"
    "\tpatch\t\t\tInstall needed patches.\n"
    "\tlist-patches, lp\tList needed patches.\n"
    "\tdist-upgrade, dup\tPerform a distribution upgrade.\n"
    "\tpatch-check, pchk\tCheck for patches.\n"
  );

  static std::string help_query_commands = _("     Querying:\n"
    "\tsearch, se\t\tSearch for packages matching a pattern.\n"
    "\tinfo, if\t\tShow full information for specified packages.\n"
    "\tpatch-info\t\tShow full information for specified patches.\n"
    "\tpattern-info\t\tShow full information for specified patterns.\n"
    "\tproduct-info\t\tShow full information for specified products.\n"
    "\tpatches, pch\t\tList all available patches.\n"
    "\tpackages, pa\t\tList all available packages.\n"
    "\tpatterns, pt\t\tList all available patterns.\n"
    "\tproducts, pd\t\tList all available products.\n"
    "\twhat-provides, wp\tList packages providing specified capability.\n"
    //"\twhat-requires, wr\tList packages requiring specified capability.\n"
    //"\twhat-conflicts, wc\tList packages conflicting with specified capability.\n"
  );

  static std::string help_lock_commands = _("     Package Locks:\n"
    "\taddlock, al\t\tAdd a package lock.\n"
    "\tremovelock, rl\t\tRemove a package lock.\n"
    "\tlocks, ll\t\tList current package locks.\n"
    "\tcleanlocks, cl\t\tRemove unused locks.\n"
  );

  static std::string help_other_commands = _("     Other Commands:\n"
    "\tversioncmp, vcmp\tCompare two version strings.\n"
    "\ttargetos, tos\t\tPrint the target operating system ID string.\n"
    "\tlicenses\t\tPrint report about licenses and EULAs of\n"
    "\t\t\t\tinstalled packages.\n"
    "\tdownload\t\tDownload rpms specified on the commandline to a local directory.\n"
    "\tsource-download\t\tDownload source rpms for all installed packages\n"
    "\t\t\t\tto a local directory.\n"
  );

  static std::string help_subcommands = _("     Subcommands:\n"
    "\tsubcommand\t\tLists available subcommands.\n"
  );

  static std::string help_usage = _(
    "  Usage:\n"
    "\tzypper [--global-options] <command> [--command-options] [arguments]\n"
    "\tzypper <subcommand> [--command-options] [arguments]\n"
  );

  zypper.out().info( help_usage, Out::QUIET );
  zypper.out().info( help_global_options, Out::QUIET );
  zypper.out().info( repo_manager_options, Out::QUIET );
  zypper.out().info( help_global_repo_options, Out::QUIET );
  zypper.out().info( help_global_target_options, Out::QUIET );
  zypper.out().info( help_commands, Out::QUIET );
  zypper.out().info( help_repo_commands, Out::QUIET );
  zypper.out().info( help_service_commands, Out::QUIET );
  zypper.out().info( help_package_commands, Out::QUIET );
  zypper.out().info( help_update_commands, Out::QUIET );
  zypper.out().info( help_query_commands, Out::QUIET );
  zypper.out().info( help_lock_commands, Out::QUIET );
  zypper.out().info( help_other_commands, Out::QUIET );
  zypper.out().info( help_subcommands, Out::QUIET );

  print_command_help_hint( zypper );
#endif
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

/// \todo use it in all commands!
int Zypper::defaultLoadSystem( LoadSystemFlags flags_r )
{
  DBG << "FLAGS:" << flags_r << endl;

  if ( ! flags_r.testFlag( NoPool ) )
  {
    init_target( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return exitCode();

    if ( ! flags_r.testFlag( NoRepos ) )
    {
      init_repos(*this);
      if ( exitCode() != ZYPPER_EXIT_OK )
	return exitCode();
    }

    DtorReset _tmp( _gopts.disable_system_resolvables );
    if ( flags_r.testFlag( NoTarget ) )
    {
      _gopts.disable_system_resolvables = true;
    }
    load_resolvables( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return exitCode();

    if ( ! ( flags_r & NoPool ) )
    {
      // have REPOS and TARGET
      // compute status of PPP
      resolve(*this);
    }
  }
  return exitCode();
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

    // "what-provides" is obsolete, functionality is provided by "search"
    if ( command() == ZypperCommand::WHAT_PROVIDES_e )
    {
      out().info( str::Format(_("Command '%s' is replaced by '%s'.")) % "what-provides" % "search --provides --match-exact" );
      out().info( str::Format(_("See '%s' for all available options.")) % "help search" );
      setCommand( ZypperCommand::SEARCH_e );
      _copts["provides"].push_back( "" );
      _copts["match-exact"].push_back( "" );
      ::copts = _copts;
    }

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
      optind = newStyleCmd->parseArguments( *this, optind );

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

  //! \todo all option descriptions in help texts should start at 29th character
  //! and should wrap at 79th column (bnc #423007)
#if 0
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      // translators: the first %s = "package, patch, pattern, product",
      // second %s = "package",
      // and the third %s = "only, in-advance, in-heaps, as-needed"
      "install (in) [OPTIONS] <capability|rpm_file_uri> ...\n"
      "\n"
      "Install packages with specified capabilities or RPM files with specified\n"
      "location. A capability is NAME[.ARCH][OP<VERSION>], where OP is one\n"
      "of <, <=, =, >=, >.\n"
      "\n"
      "  Command options:\n"
      "    --from <alias|#|URI>    Select packages from the specified repository.\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "-t, --type <type>           Type of package (%s).\n"
      "                            Default: %s.\n"
      "-n, --name                  Select packages by plain name, not by capability.\n"
      "-C, --capability            Select packages by capability.\n"
      "-f, --force                 Install even if the item is already installed (reinstall),\n"
      "                            downgraded or changes vendor or architecture.\n"
      "    --oldpackage            Allow to replace a newer item with an older one.\n"
      "                            Handy if you are doing a rollback. Unlike --force\n"
      "                            it will not enforce a reinstall.\n"
      "    --replacefiles          Install the packages even if they replace files from other,\n"
      "                            already installed, packages. Default is to treat file conflicts\n"
      "                            as an error. --download-as-needed disables the fileconflict check.\n"
      "-l, --auto-agree-with-licenses\n"
      "                            Automatically say 'yes' to third party license\n"
      "                            confirmation prompt.\n"
      "                            See 'man zypper' for more details.\n"
      "-D, --dry-run               Test the installation, do not actually install.\n"
      "    --details               Show the detailed installation summary.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
    ), "package, patch, pattern, product, srcpackage",
       "package",
       "only, in-advance, in-heaps, as-needed") )
#endif

#if 0
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      // TranslatorExplanation the first %s = "package, patch, pattern, product"
      //  and the second %s = "package"
      "remove (rm) [OPTIONS] <capability> ...\n"
      "\n"
      "Remove packages with specified capabilities.\n"
      "A capability is NAME[.ARCH][OP<VERSION>], where OP is one\n"
      "of <, <=, =, >=, >.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "-t, --type <type>           Type of package (%s).\n"
      "                            Default: %s.\n"
      "-n, --name                  Select packages by plain name, not by capability.\n"
      "-C, --capability            Select packages by capability.\n"
      "-u, --clean-deps            Automatically remove unneeded dependencies.\n"
      "-U, --no-clean-deps         No automatic removal of unneeded dependencies.\n"
      "-D, --dry-run               Test the removal, do not actually remove.\n"
      "    --details               Show the detailed installation summary.\n"
      ), "package, patch, pattern, product", "package") )

    _command_help = ( CommandHelpFormater()
    << _(
      "source-install (si) [OPTIONS] <name> ...\n"
      "\n"
      "Install specified source packages and their build dependencies.\n"
      "\n"
      "  Command options:\n"
      "-d, --build-deps-only    Install only build dependencies of specified packages.\n"
      "-D, --no-build-deps      Don't install build dependencies.\n"
      "-r, --repo <alias|#|URI> Install packages only from specified repositories.\n"
      "    --download-only      Only download the packages, do not install.\n"
    ) )

    _command_help = ( CommandHelpFormater()
      << str::form(_(
      "verify (ve) [OPTIONS]\n"
      "\n"
      "Check whether dependencies of installed packages are satisfied"
      " and suggest to install or remove packages in order to repair the"
      " dependency problems.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "-D, --dry-run               Test the repair, do not actually do anything to\n"
      "                            the system.\n"
      "    --details               Show the detailed installation summary.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
      ), "only, in-advance, in-heaps, as-needed") )

    _command_help = ( CommandHelpFormater()
      << str::form(_(
      "install-new-recommends (inr) [OPTIONS]\n"
      "\n"
      "Install newly added packages recommended by already installed packages."
      " This can typically be used to install new language packages or drivers"
      " for newly added hardware.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI>    Load only the specified repositories.\n"
      "-D, --dry-run               Test the installation, do not actually install.\n"
      "    --details               Show the detailed installation summary.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
    ), "only, in-advance, in-heaps, as-needed") )

    _(
      // translators: the %s = "ris" (the only service type currently supported)
      "addservice (as) [OPTIONS] <URI> <alias>\n"
      "\n"
      "Add a repository index service to the system.\n"
      "\n"
      "  Command options:\n"
      "-t, --type <type>       Type of the service (%s).\n"
      "-d, --disable           Add the service as disabled.\n"
      "-n, --name <name>       Specify descriptive name for the service.\n"
    )

    _command_help = _(
      // TranslatorExplanation the %s = "yast2, rpm-md, plaindir"
      "removeservice (rs) [OPTIONS] <alias|#|URI>\n"
      "\n"
      "Remove specified repository index service from the system..\n"
      "\n"
      "  Command options:\n"
      "    --loose-auth   Ignore user authentication data in the URI.\n"
      "    --loose-query  Ignore query string in the URI.\n"
    );

    _(
      // translators: %s is "--all" and "--all"
      "modifyservice (ms) <options> <alias|#|URI>\n"
      "modifyservice (ms) <options> <%s>\n"
      "\n"
      "Modify properties of services specified by alias, number, or URI, or by the\n"
      "'%s' aggregate options.\n"
      "\n"
      "  Command options:\n"
      "-d, --disable                  Disable the service (but don't remove it).\n"
      "-e, --enable                   Enable a disabled service.\n"
      "-r, --refresh                  Enable auto-refresh of the service.\n"
      "-R, --no-refresh               Disable auto-refresh of the service.\n"
      "-n, --name <name>              Set a descriptive name for the service.\n"
      "\n"
      "-i, --ar-to-enable <alias>     Add a RIS service repository to enable.\n"
      "-I, --ar-to-disable <alias>    Add a RIS service repository to disable.\n"
      "-j, --rr-to-enable <alias>     Remove a RIS service repository to enable.\n"
      "-J, --rr-to-disable <alias>    Remove a RIS service repository to disable.\n"
      "-k, --cl-to-enable             Clear the list of RIS repositories to enable.\n"
      "-K, --cl-to-disable            Clear the list of RIS repositories to disable.\n"
      "\n"
      "-a, --all                      Apply changes to all services.\n"
      "-l, --local                    Apply changes to all local services.\n"
      "-t, --remote                   Apply changes to all remote services.\n"
      "-m, --medium-type <type>       Apply changes to services of specified type.\n"
    )

    ZypperCommand::LIST_SERVICES_e:
    _command_help = _(
      "services (ls) [OPTIONS]\n"
      "\n"
      "List defined services.\n"
      "\n"
      "  Command options:\n"
      "-u, --uri                 Show also base URI of repositories.\n"
      "-p, --priority            Show also repository priority.\n"
      "-d, --details             Show more information like URI, priority, type.\n"
      "-r, --with-repos          Show also repositories belonging to the services.\n"
      "-E, --show-enabled-only   Show enabled repos only.\n"
      "-P, --sort-by-priority    Sort the list by repository priority.\n"
      "-U, --sort-by-uri         Sort the list by URI.\n"
      "-N, --sort-by-name        Sort the list by name.\n"
    );

    //ZypperCommand::REFRESH_SERVICES_e
    _command_help = _(
      "refresh-services (refs) [OPTIONS]\n"
      "\n"
      "Refresh defined repository index services.\n"
      "\n"
      "  Command options:\n"
      "-f, --force           Force a complete refresh.\n"
      "-r, --with-repos      Refresh also the service repositories.\n"
      "-R, --restore-status  Also restore service repositories enabled/disabled state.\n"
    );
#endif

#if 0
    _(
      // translators: the %s = "yast2, rpm-md, plaindir"
      "addrepo (ar) [OPTIONS] <URI> <alias>\n"
      "addrepo (ar) [OPTIONS] <file.repo>\n"
      "\n"
      "Add a repository to the system. The repository can be specified by its URI"
      " or can be read from specified .repo file (even remote).\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <file.repo>    Just another means to specify a .repo file to read.\n"
      "-t, --type <type>         Type of repository (%s).\n"
      "-d, --disable             Add the repository as disabled.\n"
      "-c, --check               Probe URI.\n"
      "-C, --no-check            Don't probe URI, probe later during refresh.\n"
      "-n, --name <name>         Specify descriptive name for the repository.\n"
      "-p, --priority <integer>  Set priority of the repository.\n"
      "-k, --keep-packages       Enable RPM files caching.\n"
      "-K, --no-keep-packages    Disable RPM files caching.\n"
      "-f, --refresh             Enable autorefresh of the repository.\n"
    )
#endif

#if 0
    _command_help = _(
      "repos (lr) [OPTIONS] [repo] ...\n"
      "\n"
      "List all defined repositories.\n"
      "\n"
      "  Command options:\n"
      "-e, --export <FILE.repo>  Export all defined repositories as a single local .repo file.\n"
      "-a, --alias               Show also repository alias.\n"
      "-n, --name                Show also repository name.\n"
      "-u, --uri                 Show also base URI of repositories.\n"
      "-p, --priority            Show also repository priority.\n"
      "-r, --refresh             Show also the autorefresh flag.\n"
      "-d, --details             Show more information like URI, priority, type.\n"
      "-s, --service             Show also alias of parent service.\n"
      "-E, --show-enabled-only   Show enabled repos only.\n"
      "-U, --sort-by-uri         Sort the list by URI.\n"
      "-P, --sort-by-priority    Sort the list by repository priority.\n"
      "-A, --sort-by-alias       Sort the list by alias.\n"
      "-N, --sort-by-name        Sort the list by name.\n"
    );
#endif

#if 0
  _command_help = ( CommandHelpFormater() << _(
    "removerepo (rr) [OPTIONS] <alias|#|URI>\n"
    "\n"
    "Remove repository specified by alias, number or URI.\n"
    "\n"
    "  Command options:\n"
    "    --loose-auth   Ignore user authentication data in the URI.\n"
    "    --loose-query  Ignore query string in the URI.\n"
  ))
#endif

#if 0
    _command_help = _(
      "renamerepo (nr) [OPTIONS] <alias|#|URI> <new-alias>\n"
      "\n"
      "Assign new alias to the repository specified by alias, number or URI.\n"
      "\n"
      "This command has no additional options.\n"
    );
#endif

#if 0
    _(
      // translators: %s is "--all|--remote|--local|--medium-type"
      // and "--all, --remote, --local, --medium-type"
      "modifyrepo (mr) <options> <alias|#|URI> ...\n"
      "modifyrepo (mr) <options> <%s>\n"
      "\n"
      "Modify properties of repositories specified by alias, number, or URI, or by the\n"
      "'%s' aggregate options.\n"
      "\n"
      "  Command options:\n"
      "-d, --disable             Disable the repository (but don't remove it).\n"
      "-e, --enable              Enable a disabled repository.\n"
      "-r, --refresh             Enable auto-refresh of the repository.\n"
      "-R, --no-refresh          Disable auto-refresh of the repository.\n"
      "-n, --name <name>         Set a descriptive name for the repository.\n"
      "-p, --priority <integer>  Set priority of the repository.\n"
      "-k, --keep-packages       Enable RPM files caching.\n"
      "-K, --no-keep-packages    Disable RPM files caching.\n"
    )
    _(
      "-a, --all                 Apply changes to all repositories.\n"
      "-l, --local               Apply changes to all local repositories.\n"
      "-t, --remote              Apply changes to all remote repositories.\n"
      "-m, --medium-type <type>  Apply changes to repositories of specified type.\n"
    )
#endif

#if 0
    _command_help = _(
      "refresh (ref) [alias|#|URI] ...\n"
      "\n"
      "Refresh repositories specified by their alias, number or URI."
      " If none are specified, all enabled repositories will be refreshed.\n"
      "\n"
      "  Command options:\n"
      "-f, --force              Force a complete refresh.\n"
      "-b, --force-build        Force rebuild of the database.\n"
      "-d, --force-download     Force download of raw metadata.\n"
      "-B, --build-only         Only build the database, don't download metadata.\n"
      "-D, --download-only      Only download raw metadata, don't build the database.\n"
      "-r, --repo <alias|#|URI> Refresh only specified repositories.\n"
      "-s, --services           Refresh also services before refreshing repos.\n"
    );
#endif
#if 0
    _command_help = _(
      "clean (cc) [alias|#|URI] ...\n"
      "\n"
      "Clean local caches.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI> Clean only specified repositories.\n"
      "-m, --metadata           Clean metadata cache.\n"
      "-M, --raw-metadata       Clean raw metadata cache.\n"
      "-a, --all                Clean both metadata and package caches.\n"
    );
#endif

  case ZypperCommand::LIST_UPDATES_e:
  {
    static struct option list_updates_options[] = {
      {"repo",        required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog",     required_argument, 0, 'c'},
      {"type",        required_argument, 0, 't'},
      {"all",         no_argument,       0, 'a'},
      {"best-effort", no_argument,       0,  0 },
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = list_updates_options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("list-updates (lu) [OPTIONS]")
    )
    .description(	// translators: command description
    _("List all available updates.")
    )
    .optionSectionCommandOptions()
    .option( "-t, --type <TYPE>",	// translators: -t, --type <TYPE>
             str::Format(_("Type of package (%1%).") ) % "package, patch, pattern, product" )
    .option( "-r, --repo <ALIAS|#|URI>",	// translators: -r, --repo <ALIAS|#|URI>
             _("List only updates from the specified repository.") )
    .option( "--best-effort",	// translators: --best-effort
             _("Do a 'best effort' approach to update. Updates to a lower than the latest version are also acceptable.") )
    .option( "-a, --all",	// translators: -a, --all
             _("List all packages for which newer versions are available, regardless whether they are installable or not.") )
    ;
#if 0
    _command_help = str::form(_(
      // TranslatorExplanation the first %s = "package, patch, pattern, product"
      //  and the second %s = "patch"
      "list-updates (lu) [OPTIONS]\n"
      "\n"
      "List all available updates.\n"
      "\n"
      "  Command options:\n"
      "-t, --type <type>             Type of package (%s).\n"
      "                              Default: %s.\n"
      "-r, --repo <alias|#|URI>      List only updates from the specified repository.\n"
      "    --best-effort             Do a 'best effort' approach to update. Updates\n"
      "                              to a lower than the latest version are\n"
      "                              also acceptable.\n"
      "-a, --all                     List all packages for which newer versions are\n"
      "                              available, regardless whether they are\n"
      "                              installable or not.\n"
    ), "package, patch, pattern, product", "package");
#endif
    break;
  }

  case ZypperCommand::UPDATE_e:
  {
    shared_ptr<UpdateOptions> myOpts( new UpdateOptions );
    _commandOptions = myOpts;
    static struct option update_options[] = {
      {"repo",                      required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog",                   required_argument, 0, 'c'},
      {"type",                      required_argument, 0, 't'},
      {"no-confirm",                no_argument,       0, 'y'},	// pkg/apt/yum user convenience ==> --non-interactive
      {"skip-interactive",          no_argument,       0,  0 },
      {"with-interactive",          no_argument,       0,  0 },
      ARG_License_Agreement,
      {"agree-to-third-party-licenses",  no_argument,  0, 0},	// rug compatibility, we have --auto-agree-with-licenses
      {"best-effort",               no_argument,       0, 0},
      ARG_Solver_Flags_Common,
      ARG_Solver_Flags_Recommends,
      ARG_Solver_Flags_Installs,
      {"replacefiles",              no_argument,       0,  0 },
      {"dry-run",                   no_argument,       0, 'D'},
      // rug uses -N shorthand
      {"dry-run",                   no_argument,       0, 'N'},
      {"details",		    no_argument,       0,  0 },
      {"download",                  required_argument, 0,  0 },
      // aliases for --download
      // in --download-only, -d must be kept for backward and rug compatibility
      {"download-only",             no_argument,       0, 'd'},
      {"download-in-advance",       no_argument,       0,  0 },
      {"download-in-heaps",         no_argument,       0,  0 },
      {"download-as-needed",        no_argument,       0,  0 },
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = update_options;
#if 0
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      // translators: the first %s = "package, patch, pattern, product",
      // the second %s = "patch",
      // and the third %s = "only, in-avance, in-heaps, as-needed"
      "update (up) [OPTIONS] [packagename] ...\n"
      "\n"
      "Update all or specified installed packages with newer versions, if possible.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-t, --type <type>           Type of package (%s).\n"
      "                            Default: %s.\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "    --skip-interactive      Skip interactive updates.\n"
      "    --with-interactive      Do not skip interactive updates.\n"
      "-l, --auto-agree-with-licenses\n"
      "                            Automatically say 'yes' to third party license\n"
      "                            confirmation prompt.\n"
      "                            See man zypper for more details.\n"
      "    --best-effort           Do a 'best effort' approach to update. Updates\n"
      "                            to a lower than the latest version are\n"
      "                            also acceptable.\n"
      "    --replacefiles          Install the packages even if they replace files from other,\n"
      "                            already installed, packages. Default is to treat file conflicts\n"
      "                            as an error. --download-as-needed disables the fileconflict check.\n"
      "-D, --dry-run               Test the update, do not actually update.\n"
      "    --details               Show the detailed installation summary.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
      ), "package, patch, pattern, product, srcpackage",
         "package",
         "only, in-advance, in-heaps, as-needed") )
#endif
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("update (up) [OPTIONS] [PACKAGENAME] ...")
    )
    .description(	// translators: command description
    _("Update all or specified installed packages with newer versions, if possible.")
    )
    .optionSectionCommandOptions()
    .option( "-t, --type <TYPE>",	// translators: -t, --type <TYPE>
             str::Format(_("Type of package (%1%).") ) % "package, patch, pattern, product" )
    .option( "-r, --repo <ALIAS|#|URI>",	// translators: -r, --repo <ALIAS|#|URI>
             _("Load only the specified repository.") )
    .option( "--skip-interactive",	// translators: --skip-interactive
             _("Skip interactive updates.") )
    .option( "--with-interactive",	// translators: --with-interactive
             _("Do not skip interactive updates.") )
    .option( "-l, --auto-agree-with-licenses",	// translators: -l, --auto-agree-with-licenses
             _("Automatically say 'yes' to third party license confirmation prompt. See man zypper for more details.") )
    .option( "--best-effort",	// translators: --best-effort
             _("Do a 'best effort' approach to update. Updates to a lower than the latest version are also acceptable.") )
    .option( "--replacefiles",	// translators: --replacefiles
             _("Install the packages even if they replace files from other, already installed, packages. Default is to treat file conflicts as an error. --download-as-needed disables the fileconflict check.") )
    .option( "-D, --dry-run",	// translators: -D, --dry-run
             _("Test the update, do not actually update.") )
    .option( "--details",	// translators: --details
             _("Show the detailed installation summary.") )
    .option( "--download",	// translators: --download
             str::Format(_("Set the download-install mode. Available modes: %s") ) % "only, in-advance, in-heaps, as-needed" )
    .option( "-d, --download-only",	// translators: -d, --download-only
             _("Only download the packages, do not install.") )
    .option( "-y, --no-confirm",	// translators: -y, --no-confirm
	     _("Don't require user interaction. Alias for the --non-interactive global option.") )

    .optionSectionSolverOptions()
    .option_Solver_Flags_Common
    .option_Solver_Flags_Recommends
    .optionSectionExpertOptions()
    .option_Solver_Flags_Installs
    ;
    break;
  }

  case ZypperCommand::PATCH_e:
  {
    shared_ptr<PatchOptions> myOpts( new PatchOptions );
    _commandOptions = myOpts;
    static struct option update_options[] = {
      {"repo",                      required_argument, 0, 'r'},
      {"updatestack-only",	    no_argument,       0,  0 },
      {"with-update",		    no_argument,       0,  0 },
      {"no-confirm",                no_argument,       0, 'y'},	// pkg/apt/yum user convenience ==> --non-interactive
      {"skip-interactive",          no_argument,       0,  0 },
      {"with-interactive",          no_argument,       0,  0 },
      ARG_License_Agreement,
      ARG_Solver_Flags_Common,
      ARG_Solver_Flags_Recommends,
      ARG_Solver_Flags_Installs,
      {"replacefiles",              no_argument,       0,  0 },
      {"dry-run",                   no_argument,       0, 'D'},
      {"details",		    no_argument,       0,  0 },
      {"download",                  required_argument, 0,  0 },
      // aliases for --download
      // in --download-only, -d must be kept for backward and rug compatibility
      {"download-only",             no_argument,       0, 'd'},
      {"download-in-advance",       no_argument,       0,  0 },
      {"download-in-heaps",         no_argument,       0,  0 },
      {"download-as-needed",        no_argument,       0,  0 },
      {"bugzilla",                  required_argument, 0, 'b'},
      {"bz",                        required_argument, 0,  0 },
      {"cve",                       required_argument, 0,  0 },
      {"category",                  required_argument, 0, 'g'},
      {"severity",                  required_argument, 0,  0 },
      {"date",                      required_argument, 0,  0 },
      ARG_WITHout_OPTIONAL,
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = update_options;
#if 0
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      "patch [OPTIONS]\n"
      "\n"
      "Install all available needed patches.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "    --skip-interactive      Skip interactive patches.\n"
      "    --with-interactive      Do not skip interactive patches.\n"
      "-l, --auto-agree-with-licenses\n"
      "                            Automatically say 'yes' to third party license\n"
      "                            confirmation prompt.\n"
      "                            See man zypper for more details.\n"
      "-b, --bugzilla #            Install patch fixing the specified bugzilla issue.\n"
      "    --cve #                 Install patch fixing the specified CVE issue.\n"
      "-g  --category <category>   Install only patches with this category.\n"
      "    --severity <severity>   Install only patches with this severity.\n"
      "    --date <YYYY-MM-DD>     Install only patches issued up to, but not including, the specified date\n"
      "    --replacefiles          Install the packages even if they replace files from other,\n"
      "                            already installed, packages. Default is to treat file conflicts\n"
      "                            as an error. --download-as-needed disables the fileconflict check.\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "-D, --dry-run               Test the update, do not actually update.\n"
      "    --details               Show the detailed installation summary.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
      ), "only, in-advance, in-heaps, as-needed") )
#endif
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("patch [OPTIONS]")
    )
    .description(	// translators: command description
    _("Install all available needed patches.")
    )
    .optionSectionCommandOptions()
    .option( "--skip-interactive",	// translators: --skip-interactive
             _("Skip interactive patches.") )
    .option( "--with-interactive",	// translators: --with-interactive
             _("Do not skip interactive patches.") )
    .option( "-l, --auto-agree-with-licenses",	// translators: -l, --auto-agree-with-licenses
             _("Automatically say 'yes' to third party license confirmation prompt. See man zypper for more details.") )
    .option( "-b, --bugzilla",	// translators: -b, --bugzilla
             _("#            Install patch fixing the specified bugzilla issue.") )
    .option( "--cve",	// translators: --cve
             _("#                 Install patch fixing the specified CVE issue.") )
    .option( "-g, --category <CATEGORY>",	// translators: -g, --category <CATEGORY>
             _("Install only patches with this category.") )
    .option( "--severity <SEVERITY>",	// translators: --severity <SEVERITY>
             _("Install only patches with this severity.") )
    .option( "--date <YYYY-MM-DD>",	// translators: --date <YYYY-MM-DD>
             _("Install only patches issued up to, but not including, the specified date") )
    .option( "--replacefiles",	// translators: --replacefiles
             _("Install the packages even if they replace files from other, already installed, packages. Default is to treat file conflicts as an error. --download-as-needed disables the fileconflict check.") )
    .option( "-r, --repo <ALIAS|#|URI>",	// translators: -r, --repo <ALIAS|#|URI>
             _("Load only the specified repository.") )
    .option( "-D, --dry-run",	// translators: -D, --dry-run
             _("Test the update, do not actually update.") )
    .option( "--details",	// translators: --details
             _("Show the detailed installation summary.") )
    .option( "--download",	// translators: --download
             str::Format(_("Set the download-install mode. Available modes: %s") ) % "only, in-advance, in-heaps, as-needed" )
    .option( "-d, --download-only",	// translators: -d, --download-only
             _("Only download the packages, do not install.") )
    .option("--updatestack-only",	_("Install only patches which affect the package management itself.") )
    .option("--with-update",		_("Additionally try to update all packages not covered by patches. The option is ignored, if the patch command must update the update stack first. Can not be combined with --updatestack-only.") )
    .option_WITHout_OPTIONAL
    .option( "-y, --no-confirm",	_("Don't require user interaction. Alias for the --non-interactive global option.") )

    .optionSectionSolverOptions()
    .option_Solver_Flags_Common
    .option_Solver_Flags_Recommends
    .optionSectionExpertOptions()
    .option_Solver_Flags_Installs
    ;
    break;
  }

  case ZypperCommand::LIST_PATCHES_e:
  {
    static struct option list_updates_options[] = {
      {"repo",        required_argument, 0, 'r'},
      {"bugzilla",    optional_argument, 0, 'b'},
      {"bz",          optional_argument, 0,  0 },
      {"cve",         optional_argument, 0,  0 },
      {"category",    required_argument, 0, 'g'},
      {"severity",    required_argument, 0,  0 },
      {"date",        required_argument, 0,  0 },
      {"issues",      optional_argument, 0,  0 },
      {"all",         no_argument,       0, 'a'},
      ARG_WITHout_OPTIONAL,
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = list_updates_options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("list-patches (lp) [OPTIONS]")
    )
    .description(// translators: command description
    _("List all applicable patches.")
    )
    .optionSectionCommandOptions()
    .option( "-b, --bugzilla[=#]",	// translators: -b, --bugzilla[=#]
             _("List applicable patches for Bugzilla issues.") )
    .option( "--cve[=#]",	// translators: --cve[=#]
             _("List applicable patches for CVE issues.") )
    .option( "--issues[=STRING]",	// translators: --issues[=STRING]
             _("Look for issues matching the specified string.") )
    .option( "--date <YYYY-MM-DD>",	// translators: --date <YYYY-MM-DD>
             _("List only patches issued up to, but not including, the specified date.") )
    .option( "-g, --category <CATEGORY>",	// translators: -g, --category <CATEGORY>
             _("List only patches with this category.") )
    .option( "--severity <SEVERITY>",	// translators: --severity <SEVERITY>
             _("List only patches with this severity.") )
    .option( "-a, --all",	// translators: -a, --all
             _("List all patches, not only applicable ones.") )
    .option_WITHout_OPTIONAL
    .option( "-r, --repo <ALIAS|#|URI>",	// translators: -r, --repo <ALIAS|#|URI>
             _("List only patches from the specified repository.") )
    ;
#if 0
    .option(_("-b, --bugzilla[=#]"		"\n"	"List applicable patches for Bugzilla issues."))
    .option(_(    "--cve[=#]"			"\n"	"List applicable patches for CVE issues."))
    .option(_(    "--issues[=STRING]"		"\n"	"Look for issues matching the specified string."))
    .option(_(    "--date <YYYY-MM-DD>"		"\n"	"List only patches issued up to, but not including, the specified date."))
    .option(_("-g, --category <CATEGORY>"	"\n"	"List only patches with this category."))
    .option(_(    "--severity <SEVERITY>"	"\n"	"List only patches with this severity."))
    .option(_("-a, --all"			"\n"	"List all patches, not only applicable ones."))
    .option_WITHout_OPTIONAL
    .option(_("-r, --repo <ALIAS|#|URI>"	"\n"	"List only patches from the specified repository."))
#endif
    break;
  }

#if 0
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      "dist-upgrade (dup) [OPTIONS]\n"
      "\n"
      "Perform a distribution upgrade.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "    --from <alias|#|URI>    Restrict upgrade to specified repository.\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "-l, --auto-agree-with-licenses\n"
      "                            Automatically say 'yes' to third party license\n"
      "                            confirmation prompt.\n"
      "                            See man zypper for more details.\n"
      "    --replacefiles          Install the packages even if they replace files from other,\n"
      "                            already installed, packages. Default is to treat file conflicts\n"
      "                            as an error. --download-as-needed disables the fileconflict check.\n"
      "-D, --dry-run               Test the upgrade, do not actually upgrade\n"
      "    --details               Show the detailed installation summary.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
      ), "only, in-advance, in-heaps, as-needed") )
#endif

  case ZypperCommand::SEARCH_e:
  {
    static struct option search_options[] = {
      ARG_not_INSTALLED_ONLY,
      {"match-substrings", no_argument, 0, 0},
      {"match-words", no_argument, 0, 0},
      {"match-exact", no_argument, 0, 'x'},
      {"provides", no_argument, 0, 0},
      {"requires", no_argument, 0, 0},
      {"recommends", no_argument, 0, 0},
      {"supplements", no_argument, 0, 0},
      {"conflicts", no_argument, 0, 0},
      {"obsoletes", no_argument, 0, 0},
      {"suggests", no_argument, 0, 0},
      {"name", no_argument, 0, 'n'},
      {"file-list", no_argument, 0, 'f'},
      {"search-descriptions", no_argument, 0, 'd'},
      {"case-sensitive", no_argument, 0, 'C'},
      {"type",    required_argument, 0, 't'},
      {"sort-by-name", no_argument, 0, 0},
      // rug compatibility option, we have --sort-by-repo
      {"sort-by-catalog", no_argument, 0, 0},		// TRANSLATED into sort-by-repo
      {"sort-by-repo", no_argument, 0, 0},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      {"repo", required_argument, 0, 'r'},
      {"details", no_argument, 0, 's'},
      {"verbose", no_argument, 0, 'v'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = search_options;
#if 0
    _command_help = ( CommandHelpFormater() << _(
      "search (se) [OPTIONS] [querystring] ...\n"
      "\n"
      "Search for packages matching any of the given search strings.\n"
      "\n"
      "  Command options:\n"
      "    --match-substrings     Search for a match to partial words (default).\n"
      "    --match-words          Search for a match to whole words only.\n"
      "-x, --match-exact          Searches for an exact match of the search strings.\n"
      "    --provides             Search for packages which provide the search strings.\n"
      "    --recommends           Search for packages which recommend the search strings.\n"
      "    --requires             Search for packages which require the search strings.\n"
      "    --suggests             Search for packages which suggest the search strings.\n"
      "    --conflicts            Search packages conflicting with search strings.\n"
      "    --obsoletes            Search for packages which obsolete the search strings.\n"
      "-n, --name                 Useful together with dependency options, otherwise\n"
      "                           searching in package name is default.\n"
      "-f, --file-list            Search for a match in the file list of packages.\n"
      "-d, --search-descriptions  Search also in package summaries and descriptions.\n"
      "-C, --case-sensitive       Perform case-sensitive search.\n"
      "-i, --installed-only       Show only installed packages.\n"
      "-u, --not-installed-only   Show only packages which are not installed.\n"
      "-t, --type <type>          Search only for packages of the specified type.\n"
      "-r, --repo <alias|#|URI>   Search only in the specified repository.\n"
      "    --sort-by-name         Sort packages by name (default).\n"
      "    --sort-by-repo         Sort packages by repository.\n"
      "-s, --details              Show each available version in each repository\n"
      "                           on a separate line.\n"
      "-v, --verbose              Like --details, with additional information where the\n"
      "                           search has matched (useful for search in dependencies).\n"
      "\n"
      "* and ? wildcards can also be used within search strings.\n"
      "If a search string is enclosed in '/', it's interpreted as a regular expression.\n"
    ))
#endif
    // *******************************************************
    // @Benjamin:
    // DO NOT CONVERT SEARCH TO NEW STYLE COMMANDS
    // while https://bugzilla.suse.com/show_bug.cgi?id=1099982
    // is unfixed. PR is pending....
    // *******************************************************
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("search (se) [OPTIONS] [QUERYSTRING] ...")
    )
    .description(	// translators: command description
    _("Search for packages matching any of the given search strings.")
    )
    .description(	// translators: command description
    _("* and ? wildcards can also be used within search strings. If a search string is enclosed in '/', it's interpreted as a regular expression.")
    )
    .optionSectionCommandOptions()
    .option( "--match-substrings",	// translators: --match-substrings
             _("Search for a match to partial words (default).") )
    .option( "--match-words",	// translators: --match-words
             _("Search for a match to whole words only.") )
    .option( "-x, --match-exact",	// translators: -x, --match-exact
             _("Searches for an exact match of the search strings.") )
    .option( "--provides",	// translators: --provides
             _("Search for packages which provide the search strings.") )
    .option( "--recommends",	// translators: --recommends
             _("Search for packages which recommend the search strings.") )
    .option( "--requires",	// translators: --requires
             _("Search for packages which require the search strings.") )
    .option( "--suggests",	// translators: --suggests
             _("Search for packages which suggest the search strings.") )
    .option( "--supplements", 	// translators: --supplements
	     _("Search for packages which supplement the search strings.") )
    .option( "--conflicts",	// translators: --conflicts
             _("Search packages conflicting with search strings.") )
    .option( "--obsoletes",	// translators: --obsoletes
             _("Search for packages which obsolete the search strings.") )
    .option( "-n, --name",	// translators: -n, --name
             _("Useful together with dependency options, otherwise searching in package name is default.") )
    .option( "-f, --file-list",	// translators: -f, --file-list
             _("Search for a match in the file list of packages.") )
    .option( "-d, --search-descriptions",	// translators: -d, --search-descriptions
             _("Search also in package summaries and descriptions.") )
    .option( "-C, --case-sensitive",	// translators: -C, --case-sensitive
             _("Perform case-sensitive search.") )
    .option( "-i, --installed-only",	// translators: -i, --installed-only
             _("Show only installed packages.") )
    .option( "-u, --not-installed-only",	// translators: -u, --not-installed-only
             _("Show only packages which are not installed.") )
    .option( "-t, --type <TYPE>",	// translators: -t, --type <TYPE>
             _("Search only for packages of the specified type.") )
    .option( "-r, --repo <ALIAS|#|URI>",	// translators: -r, --repo <ALIAS|#|URI>
             _("Search only in the specified repository.") )
    .option( "--sort-by-name",	// translators: --sort-by-name
             _("Sort packages by name (default).") )
    .option( "--sort-by-repo",	// translators: --sort-by-repo
             _("Sort packages by repository.") )
    .option( "-s, --details",	// translators: -s, --details
             _("Show each available version in each repository on a separate line.") )
    .option( "-v, --verbose",	// translators: -v, --verbose
             _("Like --details, with additional information where the search has matched (useful for search in dependencies).") )
    ;
    break;
  }

  case ZypperCommand::PATCH_CHECK_e:
  {
    static struct option patch_check_options[] = {
      {"repo",				required_argument,	0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog",			required_argument,	0, 'c'},
      {"updatestack-only",		no_argument,		0,  0 },
      ARG_WITHout_OPTIONAL,
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = patch_check_options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("patch-check (pchk) [OPTIONS]")
    )
    .description(// translators: command description
    _("Display stats about applicable patches. The command returns 100 if needed patches were found, 101 if there is at least one needed security patch.")
    )
    .optionSectionCommandOptions()
    .option( "-r, --repo <ALIAS|#|URI>",	// translators: -r, --repo <ALIAS|#|URI>
             _("Check for patches only in the specified repository.") )
    .option( "--updatestack-only",	// translators: --updatestack-only
             _("Check only for patches which affect the package management itself.") )
    .option_WITHout_OPTIONAL
    ;
#if 0
    .option(_("-r, --repo <ALIAS|#|URI>"	"\n"	"Check for patches only in the specified repository."))
    .option(_("--updatestack-only"		"\n"	"Check only for patches which affect the package management itself."))
#endif
    break;
  }

#if 0
    _command_help = _(
      "patches (pch) [repository] ...\n"
      "\n"
      "List all patches available in specified repositories.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
    );
#endif

#if 0
    _command_help = _(
      "packages (pa) [OPTIONS] [repository] ...\n"
      "\n"
      "List all packages available in specified repositories.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
      "-i, --installed-only      Show only installed packages.\n"
      "-u, --not-installed-only  Show only packages which are not installed.\n"
      "    --orphaned            Show packages which are orphaned (without repository).\n"
      "    --suggested           Show packages which are suggested.\n"
      "    --recommended         Show packages which are recommended.\n"
      "    --unneeded            Show packages which are unneeded.\n"
      "-N, --sort-by-name        Sort the list by package name.\n"
      "-R, --sort-by-repo        Sort the list by repository.\n"
    );
#endif
#if 0
    _command_help = _(
      "patterns (pt) [OPTIONS] [repository] ...\n"
      "\n"
      "List all patterns available in specified repositories.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
      "-i, --installed-only      Show only installed patterns.\n"
      "-u, --not-installed-only  Show only patterns which are not installed.\n"
    );
#endif

#if 0
    _command_help = ( CommandHelpFormater()
      << _(
      "products (pd) [OPTIONS] [repository] ...\n"
      "\n"
      "List all products available in specified repositories.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
      "-i, --installed-only      Show only installed products.\n"
      "-u, --not-installed-only  Show only products which are not installed.\n") )
#endif
#if 0
    _command_help = ( CommandHelpFormater() << str::form(_(
        "info (if) [OPTIONS] <name> ...\n"
        "\n"
        "Show detailed information for specified packages.\n"
        "By default the packages which match exactly the given names are shown.\n"
        "To get also packages partially matching use option '--match-substrings'\n"
        "or use wildcards (*?) in name.\n"
        "\n"
        "  Command options:\n"
        "-s, --match-substrings    Print information for packages partially matching name.\n"
        "-r, --repo <alias|#|URI>  Work only with the specified repository.\n"
        "-t, --type <type>         Type of package (%s).\n"
        "                          Default: %s.\n"
        "    --provides            Show provides.\n"
        "    --requires            Show requires and prerequires.\n"
        "    --conflicts           Show conflicts.\n"
        "    --obsoletes           Show obsoletes.\n"
        "    --recommends          Show recommends.\n"
        "    --suggests            Show suggests.\n"
      ), "package, patch, pattern, product", "package"))
#endif
#if 0
    _command_help = str::form(_(
      "patch-info <patchname> ...\n"
      "\n"
      "Show detailed information for patches.\n"
      "\n"
      "This is an alias for '%s'.\n"
    ), "zypper info -t patch");
#endif
#if 0
    _command_help = str::form(_(
      "pattern-info <pattern_name> ...\n"
      "\n"
      "Show detailed information for patterns.\n"
      "\n"
      "This is an alias for '%s'.\n"
    ), "zypper info -t pattern");
#endif
#if 0
    _command_help = str::form(_(
      "product-info <product_name> ...\n"
      "\n"
      "Show detailed information for products.\n"
      "\n"
      "This is an alias for '%s'.\n"
    ), "zypper info -t product");
#endif

  case ZypperCommand::WHAT_PROVIDES_e:
  {
    static struct option options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("what-provides (wp) <CAPABILITY>")
    )
    .description(	// translators: command description
    _("List all packages providing the specified capability.")
    )
    .noOptionSection()
    ;
#if 0
    _command_help = _(
      "what-provides (wp) <capability>\n"
      "\n"
      "List all packages providing the specified capability.\n"
      "\n"
      "This command has no additional options.\n"
    );
#endif
    break;
  }
/*
  case ZypperCommand::WHAT_REQUIRES_e:
  {
    static struct option options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "what-requires (wr) <capability>\n"
      "\n"
      "List all packages requiring the specified capability.\n"
      "\n"
      "This command has no additional options.\n"
    );
    break;
  }

  case ZypperCommand::WHAT_CONFLICTS_e:
  {
    static struct option options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "what-conflicts (wc) <capability>\n"
      "\n"
      "List all packages conflicting with the specified capability.\n"
      "\n"
      "This command has no additional options.\n"
    );
    break;
  }
*/
  case ZypperCommand::MOO_e:
  {
    static struct option moo_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = moo_options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("moo")
    )
    .description(	// translators: command description
    _("Show an animal.")
    )
    .noOptionSection()
    ;
#if 0
    _command_help = _(
      "moo\n"
      "\n"
      "Show an animal.\n"
      "\n"
      "This command has no additional options.\n"
      );
#endif
    break;
  }

#if 0
    _command_help = _(
      "targetos (tos) [OPTIONS]\n"
      "\n"
      "Show various information about the target operating system.\n"
      "By default, an ID string is shown.\n"
      "\n"
      "  Command options:\n"
      "-l, --label                 Show the operating system label.\n"
    );
#endif

#if 0
    _command_help = _(
      "versioncmp (vcmp) <version1> <version2>\n"
      "\n"
      "Compare the versions supplied as arguments.\n"
      "\n"
      "  Command options:\n"
      "-m, --match  Takes missing release number as any release.\n"
    );
#endif
#if 0
    _command_help = _(
      "licenses\n"
      "\n"
      "Report licenses and EULAs of currently installed software packages.\n"
      "\n"
      "This command has no additional options.\n"
    );
#endif
#if 0
    _command_help = _(
      "download [OPTIONS] <PACKAGES>...\n"
      "\n"
      "Download rpms specified on the commandline to a local directory.\n"
      "Per default packages are downloaded to the libzypp package cache\n"
      "(/var/cache/zypp/packages; for non-root users $XDG_CACHE_HOME/zypp/packages),\n"
      "but this can be changed by using the global --pkg-cache-dir option.\n"
      "\n"
      "In XML output a <download-result> node is written for each\n"
      "package zypper tried to download. Upon success the local path is\n"
      "is found in 'download-result/localpath@path'.\n"
      "\n"
      "  Command options:\n"
      "--all-matches        Download all versions matching the commandline\n"
      "                     arguments. Otherwise only the best version of\n"
      "                     each matching package is downloaded.\n"
      "--dry-run            Don't download any package, just report what\n"
      "                     would be done.\n"
    );
#endif
#if 0
    _command_help = _(
      "source-download\n"
      "\n"
      "Download source rpms for all installed packages to a local directory.\n"
      "\n"
      "  Command options:\n"
      "-d, --directory <dir>\n"
      "                     Download all source rpms to this directory.\n"
      "                     Default: /var/cache/zypper/source-download\n"
      "--delete             Delete extraneous source rpms in the local directory.\n"
      "--no-delete          Do not delete extraneous source rpms.\n"
      "--status             Don't download any source rpms,\n"
      "                     but show which source rpms are missing or extraneous.\n"
    );
//       "--manifest           Write MANIFEST of packages and coresponding source rpms.\n"
//       "--no-manifest        Do not write MANIFEST.\n"
#endif


  case ZypperCommand::SHELL_QUIT_e:
  {
    static struct option quit_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = quit_options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("quit (exit, ^D)")
    )
    .description(	// translators: command description
    _("Quit the current zypper shell.")
    )
    .noOptionSection()
    ;
#if 0
    _command_help = _(
      "quit (exit, ^D)\n"
      "\n"
      "Quit the current zypper shell.\n"
      "\n"
      "This command has no additional options.\n"
    );
#endif
    break;
  }

  case ZypperCommand::SHELL_e:
  {
    static struct option quit_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = quit_options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("shell (sh)")
    )
    .description(	// translators: command description
    _("Enter the zypper command shell.")
    )
    .noOptionSection()
    ;
#if 0
    _command_help = _(
      "shell (sh)\n"
      "\n"
      "Enter the zypper command shell.\n"
      "\n"
      "This command has no additional options.\n"
    );
#endif
    break;
  }

  case ZypperCommand::RUG_PATCH_SEARCH_e:
  {
    static struct option search_options[] = {
      ARG_not_INSTALLED_ONLY,
      {"match-substrings", no_argument, 0, 0},
      {"match-words", no_argument, 0, 0},
      {"match-exact", no_argument, 0, 0},
      {"search-descriptions", no_argument, 0, 'd'},
      {"case-sensitive", no_argument, 0, 'C'},
      {"sort-by-name", no_argument, 0, 0},
      {"sort-by-catalog", no_argument, 0, 0},	// TRANSLATED into sort-by-repo
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = search_options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("patch-search [OPTIONS] [QUERYSTRING...]")
    )
    .description(	// translators: command description
    _("Search for patches matching given search strings.")
    )
    .descriptionAliasCmd( "zypper -r search -t patch --detail ..." )
    ;
#if 0
    _command_help = str::form(_(
      "patch-search [OPTIONS] [querystring...]\n"
      "\n"
      "Search for patches matching given search strings. This is an alias for '%s'.\n"
    ), "zypper -r search -t patch --detail ...");
#endif
    break;
  }

  case ZypperCommand::RUG_PING_e:
  {
    static struct option options[] = {
      {"help", no_argument, 0, 'h'},
      {"if-active", no_argument, 0, 'a'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("ping [OPTIONS]")
    )
    .description(	// translators: command description
    _("This command has dummy implementation which always returns 0.")
    )
    .noOptionSection()
    ;
#if 0
    _command_help = _(
      // translators: this is just a legacy command
      "ping [OPTIONS]\n"
      "\n"
      "This command has dummy implementation which always returns 0.\n"
    );
#endif
    break;
  }

  case ZypperCommand::CONFIGTEST_e:
  {
    static struct option options[] = {
      {"help",	no_argument,	 0, 'h' },
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = "This command is for debuging purposes only.";
    break;
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

  if ( !checkRequiredCapabilities( *this, _gopts ) ) {
    setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
    return;
  }

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

  // --------------------------( moo )----------------------------------------

  case ZypperCommand::MOO_e:
  {
    // TranslatorExplanation this is a hedgehog, paint another animal, if you want
    out().info(_("   \\\\\\\\\\\n  \\\\\\\\\\\\\\__o\n__\\\\\\\\\\\\\\'/_"));
    break;
  }

  // --------------------------( search )-------------------------------------

  case ZypperCommand::SEARCH_e:
  case ZypperCommand::RUG_PATCH_SEARCH_e:
  {
    // *******************************************************
    // @Benjamin:
    // DO NOT CONVERT SEARCH TO NEW STYLE COMMANDS
    // while https://bugzilla.suse.com/show_bug.cgi?id=1099982
    // is unfixed. PR is pending....
    // *******************************************************

    // check args...
    PoolQuery query;
    TriBool inst_notinst = indeterminate;

    if ( globalOpts().disable_system_resolvables || copts.count("not-installed-only") )
    {
      query.setUninstalledOnly(); // beware: this is not all to it, look at zypper-search, _only_not_installed
      inst_notinst = false;
    }
    if ( copts.count("installed-only") )
      inst_notinst = true;
    //  query.setInstalledOnly();


    // matchmode : either explicitly set or nullptr (we choose some default)
    const char * climatchmode = cli::assertMutuallyExclusiveArgs( *this, { "match-substrings", "match-words", "match-exact" } );
    if ( climatchmode )
    {
      if ( !strcmp( climatchmode, "match-substrings" ) )
	query.setMatchSubstring();	// this is also the PoolQuery default
      else if ( !strcmp( climatchmode, "match-words" ) )
	query.setMatchWord();
      else if ( !strcmp( climatchmode, "match-exact" ) )
	query.setMatchExact();
      else
	ZYPP_THROW( Out::Error( ZYPPER_EXIT_ERR_BUG, text::join( "Unhandled option", climatchmode ) ) );
    }

    if ( copts.count("case-sensitive") )
      query.setCaseSensitive();

    if ( command() == ZypperCommand::RUG_PATCH_SEARCH )
      query.addKind( ResKind::patch );
    else if ( copts.count("type") > 0 )
    {
      for_( it, copts["type"].begin(), copts["type"].end() )
      {
        kind = string_to_kind( *it );
        if ( kind == ResObject::Kind() )
        {
          out().error( str::Format(_("Unknown package type '%s'.")) % *it );
          setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
          return;
        }
        query.addKind( kind );
      }
    }

    if ( cOpts().count("installed-only") )
      globalOptsNoConst().no_refresh = true;

    // load system data...
    if ( defaultLoadSystem() != ZYPPER_EXIT_OK )
      return;

    // build query...

    // add available repos to query
    if ( cOpts().count("repo") )
    {
      for_(repo_it, _rdata.repos.begin(), _rdata.repos.end() )
      {
        query.addRepo( repo_it->alias() );
        if ( !repo_it->enabled() )
        {
          out().warning( str::Format(_("Specified repository '%s' is disabled.")) % repo_it->asUserString() );
        }
      }
    }

    bool details = _copts.count("details") || _copts.count("verbose");
    // add argument strings and attributes to query
    for_( it, _arguments.begin(), _arguments.end() )
    {
      Capability cap( *it );
      CapDetail detail( cap.detail() );
      std::string name = cap.detail().name().asString();

      if ( cap.detail().isVersioned() )
        details = true;	// show details if any search string includes an edition

      // Default Match::OTHER indicates to merge name into the global search string and mode.
      Match::Mode matchmode = Match::OTHER;
      if ( !climatchmode )
      {
	if ( name.size() >= 2 && *name.begin() == '/' && *name.rbegin() == '/' )
	{
	  name = name.substr( 1, name.size()-2 );
	  matchmode = Match::REGEX;
	}
	else if ( name.find_first_of("?*") != std::string::npos )
	  matchmode = Match::GLOB;
      }
      // else: match mode explicitly requested by cli arg

      // NOTE: We use the  addDependency  overload taking a  matchmode  argument for ALL
      // kinds of attributes, not only for dependencies. A constraint on 'op version'
      // will automatically be applied to match a matching dependency or to match
      // the matching solvables version, depending on the kind of attribute.
      sat::SolvAttr attr = sat::SolvAttr::name;
      if ( copts.count("provides") )
      {
        attr =  sat::SolvAttr::provides;
        query.addDependency( attr , name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
        if ( str::regex_match( name.c_str(), std::string("^/") ) )
        {
          // in case of path names also search in file list
          attr = sat::SolvAttr::filelist;
          query.setFilesMatchFullPath( true );
          query.addDependency( attr , name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
        }
      }
      if ( copts.count("requires") )
      {
        attr =  sat::SolvAttr::requires;
        query.addDependency( attr , name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
      }
      if ( copts.count("recommends") )
      {
        attr = sat::SolvAttr::recommends;
        query.addDependency( attr , name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
      }
      if ( copts.count("suggests") )
      {
        attr =  sat::SolvAttr::suggests;
        query.addDependency( attr , name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
      }
      if ( copts.count("conflicts") )
      {
        attr = sat::SolvAttr::conflicts;
        query.addDependency( attr , name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
      }
      if ( copts.count("obsoletes") )
      {
        attr = sat::SolvAttr::obsoletes;
        query.addDependency( attr , name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
      }
      if ( copts.count("supplements") )
      {
        attr = sat::SolvAttr::supplements;
        query.addDependency( attr , name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
      }
      if ( copts.count("file-list") )
      {
        attr = sat::SolvAttr::filelist;
	query.setFilesMatchFullPath( true );
        query.addDependency( attr , name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
      }
      if ( attr == sat::SolvAttr::name || copts.count("name") )
      {
        query.addDependency( sat::SolvAttr::name, name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );

	if ( matchmode == Match::OTHER && cap.detail().isNamed() )
	{
	  // ARG did not require a specific matchmode.
	  // Handle "N-V" and "N-V-R" cases. Name must match exact,
	  // Version/Release must not be empty. If versioned matches are
	  // found, don't forget to show details.
	  std::string::size_type pos = name.find_last_of( "-" );
	  if ( pos != std::string::npos && pos != 0 && pos != name.size()-1 )
	  {
	    std::string n( name.substr(0,pos) );
	    std::string r( name.substr(pos+1) );
	    Edition e( r );
	    query.addDependency( sat::SolvAttr::name, n, Rel::EQ, e, Arch(cap.detail().arch()), Match::STRING );
	    if ( poolExpectMatchFor( n, e ) )
	      details = true;	// show details if any search string includes an edition

	    std::string::size_type pos2 = name.find_last_of( "-", pos-1 );
	    if ( pos2 != std::string::npos && pos2 != 0 &&  pos2 != pos-1)
	    {
	      n = name.substr(0,pos2);
	      e = Edition( name.substr(pos2+1,pos-pos2-1), r );
	      query.addDependency( sat::SolvAttr::name, n, Rel::EQ, e, Arch(cap.detail().arch()), Match::STRING );
	      if ( poolExpectMatchFor( n, e ) )
		details = true;	// show details if any search string includes an edition
	    }
	  }
	}
      }
      if ( cOpts().count("search-descriptions") )
      {
	query.addDependency( sat::SolvAttr::summary, name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
	query.addDependency( sat::SolvAttr::description, name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
      }
    }

    // Output query result...
    Table t;
    try
    {
      if ( command() == ZypperCommand::RUG_PATCH_SEARCH )
      {
        FillPatchesTable callback( t, inst_notinst );
        invokeOnEach( query.poolItemBegin(), query.poolItemEnd(), callback );
      }
      else if ( details )
      {
        FillSearchTableSolvable callback( t, inst_notinst );
	if ( _copts.count("verbose") )
	{
	  // Option 'verbose' shows where (e.g. in 'requires', 'name') the search has matched.
	  // Info is available from PoolQuery::const_iterator.
	  for_( it, query.begin(), query.end() )
	    callback( it );
	}
	else
	{
	  for ( const auto & slv : query )
	    callback( slv );
	}
      }
      else
      {
        FillSearchTableSelectable callback( t, inst_notinst );
        invokeOnEach( query.selectableBegin(), query.selectableEnd(), callback );
      }

      if ( t.empty() )
      {
	// translators: empty search result message
        out().info(_("No matching items found."), Out::QUIET );
        setExitCode( ZYPPER_EXIT_INF_CAP_NOT_FOUND );
      }
      else
      {
        cout << endl; //! \todo  out().separator()?

        if ( command() == ZypperCommand::RUG_PATCH_SEARCH )
        {
          if ( copts.count("sort-by-repo") )
            t.sort( { 0, 1, Table::UserData } );
          else
            t.sort( { 1, Table::UserData } ); // sort by name
        }
        else if ( _copts.count("details") )
        {
          if ( copts.count("sort-by-repo") )
            t.sort( { 5, 1, Table::UserData } );
          else
            t.sort( { 1, Table::UserData } ); // sort by name
        }
        else
        {
          // sort by name (can't sort by repo)
          t.sort( 1 );
          if ( !globalOpts().no_abbrev )
            t.allowAbbrev( 2 );
        }

	//cout << t; //! \todo out().table()?
	out().searchResult( t );
      }
    }
    catch ( const Exception & e )
    {
      out().error( e, _("Problem occurred initializing or executing the search query") + std::string(":"),
		   std::string(_("See the above message for a hint.")) + " "
		   + _("Running 'zypper refresh' as root might resolve the problem.") );
      setExitCode( ZYPPER_EXIT_ERR_ZYPP );
    }

    searchPackagesHintHack::callOrNotify( *this );
    break;
  }

  // --------------------------( patch check )--------------------------------

  // TODO: rug summary
  case ZypperCommand::PATCH_CHECK_e:
  {
    // too many arguments
    if ( _arguments.size() > 0 )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    initRepoManager();

    init_target( *this );
    init_repos( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return;

    // TODO calc token?

    // now load resolvables:
    load_resolvables( *this );
    // needed to compute status of PPP
    resolve( *this );
    patch_check();
    break;
  }

  // --------------------------( misc queries )--------------------------------

  case ZypperCommand::WHAT_PROVIDES_e:
  {
    // The "what-provides" now is included in "search" command, e.g.
    // zypper what-provides 'zypper>1.6'
    // zypper se --match-exact --provides 'zypper>1.6'
    setExitCode( ZYPPER_EXIT_ERR_BUG );
    break;
  }

  // --------------------------( list updates )-------------------------------

  case ZypperCommand::LIST_UPDATES_e:
  case ZypperCommand::LIST_PATCHES_e:
  {
    // too many arguments
    if ( _arguments.size() > 0 )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    ResKindSet kinds;
    if ( copts.count("type") )
    {
      for_( it, copts["type"].begin(), copts["type"].end())
      {
        kind = string_to_kind( *it );
        if ( kind == ResObject::Kind() )
        {
          out().error( str::Format(_("Unknown package type '%s'.")) % *it );
          setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
          return;
        }
        kinds.insert( kind );
      }
    }
    else if ( command() == ZypperCommand::LIST_PATCHES )
      kinds.insert( ResKind::patch );
    else
      kinds.insert( ResKind::package );

    //! \todo drop this option - it's the default for packages now, irrelevant
    //! for patches; just test with products and patterns
    bool best_effort = copts.count( "best-effort" );

    if ( ( copts.count("bugzilla") || copts.count("bz") || copts.count("cve") )
      && copts.count("issues") )
    {
      out().error(str::form( _("Cannot use %s together with %s."), "--issues", "--bz, --cve") );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    initRepoManager();
    init_target( *this );
    init_repos( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return;
    load_resolvables( *this );
    resolve( *this );

    if ( copts.count("bugzilla") || copts.count("bz") || copts.count("cve") || copts.count("issues") )
      list_patches_by_issue( *this );
    else
      list_updates( *this, kinds, best_effort );

    break;
  }

  // -----------------------------( update )----------------------------------

  case ZypperCommand::UPDATE_e:
  case ZypperCommand::PATCH_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for updating packages.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    // too many arguments
    if ( !_arguments.empty() && command() == ZypperCommand::PATCH )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    if ( copts.count("updatestack-only") )
    {
      for ( const char * opt : { "bugzilla", "bz", "cve", "with-update" } )
      {
	if ( copts.count( opt ) )
	{
	  out().error( str::form(_("Cannot use %s together with %s."),
				 dashdash("updatestack-only").c_str(),
				 dashdash(opt).c_str() ) );
	  setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
	  return;
	}
      }
    }

    bool skip_interactive = false;
    if ( copts.count("skip-interactive") )
    {
      if ( copts.count("with-interactive") )
      {
        out().error( str::form(_("%s contradicts %s"), "--with-interactive", "--skip-interactive" ) );
        setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
        return;
      }
      skip_interactive = true;
    }
    // bnc #497711
    else if ( globalOpts().non_interactive && !copts.count("with-interactive") )
      skip_interactive = true;
    MIL << "Skipping interactive patches: " << (skip_interactive ? "yes" : "no") << endl;

    ResKindSet kinds;
    if ( copts.count("type") )
    {
      for_( it, copts["type"].begin(), copts["type"].end() )
      {
        kind = string_to_kind( *it );
        if ( kind == ResObject::Kind() )
        {
          out().error( str::Format(_("Unknown package type '%s'.")) % *it );
          setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
          return;
        }

        if ( kind == ResKind::product )
        {
          out().error(_("Operation not supported."),
		      str::form(_("To update installed products use '%s'."),
				"zypper dup [--from <repo>]") );
          setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
          return;
        }
        else if ( kind == ResKind::srcpackage )
        {
	  out().error(_("Operation not supported."),
		      str::form(_("Zypper does not keep track of installed source packages. To install the latest source package and its build dependencies, use '%s'."),
				"zypper si"));
          setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
          return;
        }

        kinds.insert( kind );
      }
    }
    else if ( command() == ZypperCommand::PATCH )
      kinds.insert( ResKind::patch );
    else
      kinds.insert( ResKind::package );

    if ( !arguments().empty() && kinds.size() > 1 )
    {
      out().error(_("Cannot use multiple types when specific packages are given as arguments.") );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    bool best_effort = copts.count( "best-effort" );

    // parse the download options to check for errors
    get_download_option( *this );

    init_target( *this );
    initRepoManager();
    init_repos( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return;

    load_resolvables( *this );
    resolve( *this ); // needed to compute status of PPP
    // Beware: While zypper calls resolve() once just to compute the PPP status,
    // solve_with_update must be false until the command passed the initialization!
    // Reset to false when leaving the block in case we are in shell mode!
    DtorReset guard( runtimeData().solve_with_update );
    if ( copts.count( "with-update" ) )
      runtimeData().solve_with_update = true;

    // patch --bugzilla/--cve
    if ( copts.count("bugzilla") || copts.count("bz") || copts.count("cve") )
      mark_updates_by_issue( *this );
    // update without arguments
    else
    {
      SolverRequester::Options sropts;
      if ( copts.find("force") != copts.end() )
        sropts.force = true;
      sropts.best_effort = best_effort;
      sropts.skip_interactive = skip_interactive; // bcn #647214
      sropts.skip_optional_patches = arguments().empty() && globalOpts().exclude_optional_patches;	// without args follow --with[out]-optional
      sropts.cliMatchPatch = CliMatchPatch( *this );

      SolverRequester sr(sropts);
      if ( arguments().empty() )
      {
        for_( kit, kinds.begin(), kinds.end() )
        {
          if ( *kit == ResKind::package )
          {
            MIL << "Computing package update..." << endl;
            // this will do a complete package update as far as possible
            // while respecting solver policies
            getZYpp()->resolver()->doUpdate();
            // no need to call Resolver::resolvePool() afterwards
            runtimeData().solve_before_commit = false;
          }
          // update -t patch; patch
          else if ( *kit == ResKind::patch )
	  {
	    runtimeData().plain_patch_command = true;
	    sr.updatePatches();
	  }
          else if ( *kit == ResKind::pattern )
            sr.updatePatterns();
          // should not get here (see above kind parsing code), but just in case
          else
          {
            out().error(_("Operation not supported.") );
            setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
            return;
          }
        }
      }
      // update with arguments
      else
      {
        PackageArgs args( *kinds.begin() );
	sr.update( args );
      }

      sr.printFeedback( out() );

      if ( !globalOpts().ignore_unknown
	&& ( sr.hasFeedback( SolverRequester::Feedback::NOT_FOUND_NAME )
	  || sr.hasFeedback( SolverRequester::Feedback::NOT_FOUND_CAP ) ) )
      {
        setExitCode( ZYPPER_EXIT_INF_CAP_NOT_FOUND );
        if ( globalOpts().non_interactive )
          ZYPP_THROW( ExitRequestException("name or capability not found") );
      }
    }
    solve_and_commit( *this );

    break;
  }

  // -----------------------------( shell )------------------------------------

  case ZypperCommand::SHELL_QUIT_e:
  {
    if ( !runningShell() )
      out().warning(_("This command only makes sense in the zypper shell."), Out::QUIET );
    else
      out().error( "oops, you wanted to quit, didn't you?" ); // should not happen

    break;
  }

  case ZypperCommand::SHELL_e:
  {
    if ( runningShell() )
      out().info(_("You already are running zypper's shell.") );
    else
    {
      out().error(_("Unexpected program flow.") );
      report_a_bug( out() );
    }

    break;
  }


  // dummy commands
  case ZypperCommand::RUG_PING_e:
  {
    break;
  }

  // Configtest debug command
  case ZypperCommand::CONFIGTEST_e:
  {
    configtest( *this );
    break;
  }

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
