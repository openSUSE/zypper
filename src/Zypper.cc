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
#include <sys/vfs.h>
#include <sys/statvfs.h>
#include <linux/magic.h>

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
#include "locks.h"
#include "search.h"
#include "info.h"
#include "ps.h"
#include "download.h"
#include "source-download.h"
#include "configtest.h"
#include "subcommand.h"

#include "output/OutNormal.h"
#include "output/OutXML.h"

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

static void rug_list_resolvables(Zypper & zypper);

///////////////////////////////////////////////////////////////////
namespace {

  /** Hack for bsc#1089994: SLE15 hinting to the zypper-search-packages-plugin subcommand. */
  inline void SLE15_SearchPackagesHintHack( Zypper & zypper )
  {
    if ( ! zypper.config().do_ttyout )
      return;

    Out & out( zypper.out() );
    if ( !out.typeNORMAL() || out.verbosity() < Out::NORMAL )
      return;

    // Don't hint to a subcommand if --root is used (subcommands do not support global opts)
    if ( zypper.globalOpts().changedRoot )
      return;

    ui::Selectable::Ptr plg;
    if ( ResPool::instance().empty() )
    {
      // No pool - maybe 'zypper help search': Hint if plugin script is installed
      if ( !PathInfo( "/usr/lib/zypper/commands/zypper-search-packages" ).isFile() )
	return;
    }
    else
    {
      // Hint if package is in pool
      plg = ui::Selectable::get( ResKind::package, "zypper-search-packages-plugin" );
      if ( !plg )
	return;
    }

    // So write out the hint....
    str::Str msg;
    // translator: %1% denotes a zypper command to execute. Like 'zypper search-packages'.
    msg << str::Format(_("For an extended search including not yet activated remote resources please use '%1%'.")) % "zypper search-packages";
    if ( plg && plg->installedEmpty() )
      // translator: %1% denotes a zypper command to execute. Like 'zypper search-packages'.
      msg << ' ' << str::Format(_("The package providing this subcommand is currently not installed. You can install it by calling '%1%'.")) % "zypper in zypper-search-packages-plugin";

    out.notePar( msg );
  }

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

  enum LegacyCLIMsgType {
    Local,
    Global,
    Ignored
  };

  inline std::string legacyCLIStr( const std::string & old_r, const std::string & new_r, LegacyCLIMsgType type_r )
  {
    switch (type_r) {
    case Local:
    case Global:
      return str::Format( type_r == Global
         ? _("Legacy commandline option %1% detected. Please use global option %2% instead.")
         : _("Legacy commandline option %1% detected. Please use %2% instead.") )
         % NEGATIVEString(dashdash(old_r))
         % POSITIVEString(dashdash(new_r));
      break;
    case Ignored:
      return str::Format(
         _("Legacy commandline option %1% detected. This option is ignored."))
         % NEGATIVEString(dashdash(old_r));
      break;
    }
    return std::string();
  }

  inline void legacyCLITranslate( parsed_opts & copts_r, const std::string & old_r, const std::string & new_r, Out::Verbosity verbosity_r = Out::NORMAL, LegacyCLIMsgType type = Local )
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
      case ZypperCommand::INSTALL_e:
      case ZypperCommand::SRC_INSTALL_e:
      case ZypperCommand::PATCH_e:
      case ZypperCommand::DIST_UPGRADE_e:
      case ZypperCommand::INSTALL_NEW_RECOMMENDS_e:
      case ZypperCommand::VERIFY_e:
      case ZypperCommand::REMOVE_e:
      case ZypperCommand::ADD_REPO_e:
      case ZypperCommand::REMOVE_REPO_e:
      case ZypperCommand::MODIFY_REPO_e:
      case ZypperCommand::RENAME_REPO_e:
      case ZypperCommand::ADD_SERVICE_e:
      case ZypperCommand::REMOVE_SERVICE_e:
      case ZypperCommand::MODIFY_SERVICE_e: {

        if ( zypper.cOpts().count("dry-run") )
          return true;

        struct statfs fsinfo;
        memset( &fsinfo, 0, sizeof(struct statfs) );

        int err = 0;
        do {
          err = statfs( gopts_r.root_dir.c_str(), &fsinfo );
        } while ( err == -1 && errno == EINTR );

        if ( !err ) {
          if ( fsinfo.f_flags & ST_RDONLY ) {

            bool isTransactionalServer = ( fsinfo.f_type == BTRFS_SUPER_MAGIC && PathInfo( "/usr/sbin/transactional-update" ).isFile() );

            std::string msg;
            if ( isTransactionalServer && !gopts_r.changedRoot ) {
              msg = _("This is a transactional-server, please use transactional-update to update or modify the system.");
            } else {
              msg = _("The target filesystem is mounted as read-only. Please make sure the target filesystem is writeable.");
            }
            zypper.out().errorPar( msg );
            ERR << msg << endl;
            return false;
          }
        } else {
          WAR << "Checking if " << gopts_r.root_dir << " is mounted read only failed with errno : " << errno << std::endl;
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
namespace env {
  /** XDG_CACHE_HOME: base directory relative to which user specific non-essential data files should be stored.
   * http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
   */
  inline Pathname XDG_CACHE_HOME()
  {
    Pathname ret;
    const char * envp = getenv( "XDG_CACHE_HOME" );
    if ( envp && *envp )
      ret = envp;
    else
    {
      ret = getenv( "HOME" );
      ret /= ".cache";
    }
    return ret;
  }
} //namespace env
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace
{
  ///////////////////////////////////////////////////////////////////
  /// \class CommandHelpFormater
  /// \brief Class for command help formating
  ///////////////////////////////////////////////////////////////////
  struct CommandHelpFormater
  {
    CommandHelpFormater()
    : _mww( _str, Zypper::instance().out().defaultFormatWidth( 100 ) )
    {}

    /** Allow using the underlying steam directly. */
    template<class Tp_>
    CommandHelpFormater & operator<<( Tp_ && val )
    { _str << std::forward<Tp_>(val); return *this; }

    /** Conversion to std::string */
    operator std::string() const { return _str.str(); }

    /** An empty line */
    CommandHelpFormater & gap()
    { _mww.gotoNextPar(); return *this; }

    /** Synopsis
     * \code
     * "<singleline text_r>"
     * \endcode
     */
    CommandHelpFormater & synopsis( boost::string_ref text_r )
    { _mww.writePar( text_r ); return *this; }
    /** \overload const char * text */
    CommandHelpFormater & synopsis( const char * text_r )
    { return synopsis( boost::string_ref(text_r) ); }
    /** \overload std::string text */
    CommandHelpFormater & synopsis( const std::string & text_r )
    { return synopsis( boost::string_ref(text_r) ); }
    /** \overload str::Format text */
    CommandHelpFormater & synopsis( const str::Format & text_r )
    { return synopsis( boost::string_ref(text_r.str()) ); }


    /** Description block with leading gap
     * \code
     *
     * "<multiline text_r>"
     * \endcode
     */
    CommandHelpFormater & description( boost::string_ref text_r )
    { _mww.gotoNextPar(); _mww.writePar( text_r ); return *this; }
    /** \overload const char * text */
    CommandHelpFormater & description( const char * text_r )
    { return description( boost::string_ref(text_r) ); }
    /** \overload std::string text */
    CommandHelpFormater & description( const std::string & text_r )
    { return description( boost::string_ref(text_r) ); }
    /** \overload str::Format text */
    CommandHelpFormater & description( const str::Format & text_r )
    { return description( boost::string_ref(text_r.str()) ); }

    /** Option section title
     * \code
     * ""
     * "  <text_r:>"
     * ""
     * \endcode
     */
    CommandHelpFormater & optionSection( boost::string_ref text_r )
    { _mww.gotoNextPar(); _mww.writePar( text_r, 2 ); _mww.gotoNextPar(); return *this; }

    CommandHelpFormater & optionSectionCommandOptions()
    { return optionSection(_("Command options:") ); }

    CommandHelpFormater & optionSectionSolverOptions()
    { return optionSection(_("Solver options:") ); }

    CommandHelpFormater & optionSectionExpertOptions()
    { return optionSection(_("Expert options:") ); }

    CommandHelpFormater & noOptionSection()
    { return optionSection(_("This command has no additional options.") ); }

    CommandHelpFormater & legacyOptionSection()
    { return optionSection(_("Legacy options:") ); }

    CommandHelpFormater & legacyOption( boost::string_ref old_r, boost::string_ref new_r )
    { // translator: '-r             The same as -f.
      return option( old_r, str::Format(_("The same as %1%.")) % new_r ); }


    /** Option definition
     * \code
     * "123456789012345678901234567890123456789
     * "-o, --long-name             <text_r> starts on 29 maybe on next line"
     * "                            if long-name is too long.
     * \endcode
     */
    CommandHelpFormater & option( boost::string_ref option_r, boost::string_ref text_r )
    { _mww.writeDefinition( option_r , text_r, (option_r.starts_with( "--" )?4:0), 28 ); return *this; }
    /** \overload const char * text */
    CommandHelpFormater & option( boost::string_ref option_r, const char * text_r )
    { return option( option_r, boost::string_ref(text_r) ); }
    /** \overload std::string text */
    CommandHelpFormater & option( boost::string_ref option_r, const std::string & text_r )
    { return option( option_r, boost::string_ref(text_r) ); }
    /** \overload str::Format text */
    CommandHelpFormater & option( boost::string_ref option_r, const str::Format & text_r )
    { return option( option_r, boost::string_ref(text_r.str()) ); }
    /** \overload "option\ntext_r" */
    CommandHelpFormater & option( boost::string_ref allinone_r )
    {
      std::string::size_type sep = allinone_r.find( '\n' );
      if ( sep != std::string::npos )
	_mww.writeDefinition( allinone_r.substr( 0, sep ), allinone_r.substr( sep+1 ), (allinone_r.starts_with( "--" )?4:0), 28 );
      else
	_mww.writeDefinition( allinone_r , "", (allinone_r.starts_with( "--" )?4:0), 28 );
      return *this;
    }

    /** \todo eliminate legacy indentation */
    CommandHelpFormater & option26( boost::string_ref option_r, boost::string_ref text_r )
    { _mww.writeDefinition( option_r , text_r, (option_r.starts_with( "--" )?4:0), 26 ); return *this; }

  private:
    std::ostringstream   _str;
    mbs::MbsWriteWrapped _mww;
  };
} //namespace
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
    % (zypper.runningShell() ? "help <command>" : "zypper help <command>") );
}

/// \todo use it in all commands!
int Zypper::defaultLoadSystem( LoadSystemFlags flags_r )
{
  DBG << "FLAGS:" << flags_r << endl;
  if ( ! flags_r.testFlag( NO_POOL ) )
  {
    init_target( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return exitCode();

    if ( ! flags_r.testFlag( NO_REPOS ) )
    {
      init_repos(*this);
      if ( exitCode() != ZYPPER_EXIT_OK )
	return exitCode();
    }

    DtorReset _tmp( _gopts.disable_system_resolvables );
    if ( flags_r.testFlag( NO_TARGET ) )
    {
      _gopts.disable_system_resolvables = true;
    }
    load_resolvables( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return exitCode();

    if ( ! ( flags_r & NO_POOL ) )
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

  case ZypperCommand::INSTALL_e:
  {
    shared_ptr<InstallOptions> myOpts( new InstallOptions );
    _commandOptions = myOpts;
    static struct option install_options[] = {
      {"repo",                      required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog",                   required_argument, 0, 'c'},
      {"from",                      required_argument, 0,  0 },
      {"type",                      required_argument, 0, 't'},
      // the default (ignored)
      {"name",                      no_argument,       0, 'n'},
      {"force",                     no_argument,       0, 'f'},
      {"oldpackage",                no_argument,       0,  0 },
      {"replacefiles",              no_argument,       0,  0 },
      {"capability",                no_argument,       0, 'C'},
      {"no-confirm",                no_argument,       0, 'y'},	// pkg/apt/yum user convenience ==> --non-interactive
      {"allow-unsigned-rpm",        no_argument,       0,  0 },	// disable gpg checks for directly passed rpms
      ARG_License_Agreement,
      {"agree-to-third-party-licenses",  no_argument,  0,  0 },	// rug compatibility, we have --auto-agree-with-licenses
      ARG_Solver_Flags_Common,
      ARG_Solver_Flags_Recommends,
      ARG_Solver_Flags_Installs,
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
      // rug compatibility - will mark all packages for installation (like 'in *')
      {"entire-catalog",            required_argument, 0,  0 },
      {"help",                      no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = install_options;
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      // translators: the first %s = "package, patch, pattern, product",
      // second %s = "package",
      // and the third %s = "only, in-advance, in-heaps, as-needed"
      "install (in) [options] <capability|rpm_file_uri> ...\n"
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
    .option( "-y, --no-confirm",	_("Don't require user interaction. Alias for the --non-interactive global option.") )
    .option( "--allow-unsigned-rpm", _("Silently install unsigned rpm packages given as commandline parameters.") )
    .optionSectionSolverOptions()
    .option_Solver_Flags_Common
    .option_Solver_Flags_Recommends
    .optionSectionExpertOptions()
    .option_Solver_Flags_Installs
    ;
    break;
  }

  case ZypperCommand::REMOVE_e:
  {
    static struct option remove_options[] = {
      {"repo",       required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog",    required_argument, 0, 'c'},
      {"type",       required_argument, 0, 't'},
      // the default (ignored)
      {"name",       no_argument,       0, 'n'},
      {"capability", no_argument,       0, 'C'},
      {"no-confirm", no_argument,       0, 'y'},		// pkg/apt/yum user convenience ==> --non-interactive
      ARG_Solver_Flags_Common,
      {"clean-deps", no_argument,       0, 'u'},
      {"no-clean-deps", no_argument,    0, 'U'},
      {"dry-run",    no_argument,       0, 'D'},
      {"details",		    no_argument,       0,  0 },
      // rug uses -N shorthand
      {"dry-run",    no_argument,       0, 'N'},
      {"help",       no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = remove_options;
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      // TranslatorExplanation the first %s = "package, patch, pattern, product"
      //  and the second %s = "package"
      "remove (rm) [options] <capability> ...\n"
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
    .option( "-y, --no-confirm",	_("Don't require user interaction. Alias for the --non-interactive global option.") )

    .optionSectionSolverOptions()
    .option_Solver_Flags_Common
    ;
    break;
  }

  case ZypperCommand::SRC_INSTALL_e:
  {
    static struct option src_install_options[] = {
      {"build-deps-only", no_argument, 0, 'd'},
      {"no-build-deps", no_argument, 0, 'D'},
      {"download-only", no_argument, 0, 0},
      {"repo", required_argument, 0, 'r'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = src_install_options;
    _command_help = ( CommandHelpFormater()
    << _(
      "source-install (si) [options] <name> ...\n"
      "\n"
      "Install specified source packages and their build dependencies.\n"
      "\n"
      "  Command options:\n"
      "-d, --build-deps-only    Install only build dependencies of specified packages.\n"
      "-D, --no-build-deps      Don't install build dependencies.\n"
      "-r, --repo <alias|#|URI> Install packages only from specified repositories.\n"
      "    --download-only      Only download the packages, do not install.\n"
    ) )
    .description(
      (str::Format(_("The default location where rpm installs source packages to is '%1%', but the value can be changed in your local rpm configuration. In case of doubt try executing '%2%'."))
      % "/usr/src/packages/{SPECS,SOURCES}"
      % "rpm --eval \"%{_specdir} and %{_sourcedir}\""
      ).str()
    )
    ;
    break;
  }

  case ZypperCommand::VERIFY_e:
  {
    shared_ptr<VerifyOptions> myOpts( new VerifyOptions );
    _commandOptions = myOpts;
    static struct option verify_options[] = {
      {"no-confirm", no_argument, 0, 'y'},			// pkg/apt/yum user convenience ==> --non-interactive
      {"dry-run", no_argument, 0, 'D'},
      // rug uses -N shorthand
      {"dry-run", no_argument, 0, 'N'},
      {"details",		    no_argument,       0,  0 },
      {"download",                  required_argument, 0,  0 },
      // aliases for --download
      // in --download-only, -d must be kept for backward and rug compatibility
      {"download-only",             no_argument,       0, 'd'},
      {"download-in-advance",       no_argument,       0,  0 },
      {"download-in-heaps",         no_argument,       0,  0 },
      {"download-as-needed",        no_argument,       0,  0 },
      {"repo",                      required_argument, 0, 'r'},
      ARG_Solver_Flags_Common,
      ARG_Solver_Flags_Recommends,
      ARG_Solver_Flags_Installs,
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = verify_options;
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      "verify (ve) [options]\n"
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
    .option( "-y, --no-confirm",	_("Don't require user interaction. Alias for the --non-interactive global option.") )

    .optionSectionSolverOptions()
    .option_Solver_Flags_Common
    .option_Solver_Flags_Recommends
    .optionSectionExpertOptions()
    .option_Solver_Flags_Installs
    ;
    break;
  }

  case ZypperCommand::INSTALL_NEW_RECOMMENDS_e:
  {
    shared_ptr<InrOptions> myOpts( new InrOptions );
    _commandOptions = myOpts;
    static struct option options[] = {
      {"dry-run", no_argument, 0, 'D'},
      {"details",		    no_argument,       0,  0 },
      {"download",                  required_argument, 0,  0 },
      // aliases for --download
      // in --download-only, -d must be kept for backward and rug compatibility
      {"download-only",             no_argument,       0, 'd'},
      {"download-in-advance",       no_argument,       0,  0 },
      {"download-in-heaps",         no_argument,       0,  0 },
      {"download-as-needed",        no_argument,       0,  0 },
      {"repo", required_argument, 0, 'r'},
      ARG_Solver_Flags_Common,
      ARG_Solver_Flags_Installs,
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      "install-new-recommends (inr) [options]\n"
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

    .optionSectionSolverOptions()
    .option_Solver_Flags_Common
    .optionSectionExpertOptions()
    .option_Solver_Flags_Installs
    ;
    break;
  }

  case ZypperCommand::ADD_SERVICE_e:
  {
    static struct option service_add_options[] = {
      {"type", required_argument, 0, 't'},
      {"help", no_argument, 0, 'h'},
      ARG_SERVICE_PROP,
      {0, 0, 0, 0}
    };
#if 0
    _(
      // translators: the %s = "ris" (the only service type currently supported)
      "addservice (as) [options] <URI> <alias>\n"
      "\n"
      "Add a repository index service to the system.\n"
      "\n"
      "  Command options:\n"
      "-t, --type <type>       Type of the service (%s).\n"
      "-d, --disable           Add the service as disabled.\n"
      "-n, --name <name>       Specify descriptive name for the service.\n"
    )
#endif
    specific_options = service_add_options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("addservice (as) [OPTIONS] <URI> <ALIAS>")
    )
    .description(// translators: command description
    _("Add a repository index service to the system.")
    )
    .optionSectionCommandOptions()
    .option_SERVICE_PROP
    .legacyOptionSection()
    .option( "-t, --type <TYPE>",	( str::Format(_("The type of service is always autodetected. This option is ignored.") ) ).str() )	// FIXME: leagcy, actually autodetected but check libzypp
    ;
    break;
  }

  case ZypperCommand::REMOVE_SERVICE_e:
  {
    static struct option options[] = {
      {"help", no_argument, 0, 'h'},
      {"loose-auth", no_argument, 0, 0},
      {"loose-query", no_argument, 0, 0},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      // TranslatorExplanation the %s = "yast2, rpm-md, plaindir"
      "removeservice (rs) [options] <alias|#|URI>\n"
      "\n"
      "Remove specified repository index service from the system..\n"
      "\n"
      "  Command options:\n"
      "    --loose-auth   Ignore user authentication data in the URI.\n"
      "    --loose-query  Ignore query string in the URI.\n"
    );
    break;
  }

  case ZypperCommand::MODIFY_SERVICE_e:
  {
    static struct option service_modify_options[] = {
      {"help", no_argument, 0, 'h'},
      ARG_SERVICE_PROP,
      /* LEGACY(ARG_SERVICE_PROP) prefers -f */	{"refresh",	no_argument,	0, 'r'},
      /* LEGACY(ARG_SERVICE_PROP) prefers -F */	{"no-refresh",	no_argument,	0, 'R'},
      ARG_REPO_SERVICE_COMMON_AGGREGATE,
      {"ar-to-enable",  required_argument, 0, 'i'},
      {"ar-to-disable", required_argument, 0, 'I'},
      {"rr-to-enable",  required_argument, 0, 'j'},
      {"rr-to-disable", required_argument, 0, 'J'},
      {"cl-to-enable",  no_argument, 0, 'k'},
      {"cl-to-disable", no_argument, 0, 'K'},
      {0, 0, 0, 0}
    };
#if 0
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
#endif
    specific_options = service_modify_options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("modifyservice (ms) <OPTIONS> <ALIAS|#|URI>")
    )
    .synopsis( str::Format(	// translators: command synopsis; do not translate lowercase words
    _("modifyservice (ms) <OPTIONS> <%1%>") ) % "--all|--remote|--local|--medium-type"
    )
    .description( str::Format(// translators: command description
    _("Modify properties of services specified by alias, number, or URI, or by the '%1%' aggregate options.") ) % "--all, --remote, --local, --medium-type"
    )
    .optionSectionCommandOptions()
    .option_SERVICE_PROP
    .gap()
    .option( "-a, --all",			_("Apply changes to all services.") )
    .option( "-l, --local",			_("Apply changes to all local services.") )
    .option( "-t, --remote",			_("Apply changes to all remote services.") )
    .option( "-m, --medium-type <TYPE>",	_("Apply changes to services of specified type.") )
    .gap()
    .option( "-i, --ar-to-enable <ALIAS>",	_("Add a RIS service repository to enable.") )
    .option( "-I, --ar-to-disable <ALIAS>",	_("Add a RIS service repository to disable.") )
    .option( "-j, --rr-to-enable <ALIAS>",	_("Remove a RIS service repository to enable.") )
    .option( "-J, --rr-to-disable <ALIAS>",	_("Remove a RIS service repository to disable.") )
    .option( "-k, --cl-to-enable",		_("Clear the list of RIS repositories to enable.") )
    .option( "-K, --cl-to-disable",		_("Clear the list of RIS repositories to disable.") )
    // Legacy Options:
    .legacyOptionSection()
    .legacyOption( "-r", "-f" )
    .legacyOption( "-R", "-F" )
    ;
    break;
  }

  case ZypperCommand::LIST_SERVICES_e:
  {
    static struct option options[] =
    {
      {"help",			no_argument,	0, 'h'},
      {"uri",			no_argument,	0, 'u'},
      {"url",			no_argument,	0,  0 },
      {"priority",		no_argument,	0, 'p'},
      {"details",		no_argument,	0, 'd'},
      {"with-repos",		no_argument,	0, 'r'},
      {"show-enabled-only",	no_argument,	0, 'E'},
      {"sort-by-uri",		no_argument,	0, 'U'},
      {"sort-by-name",		no_argument,	0, 'N'},
      {"sort-by-priority",	no_argument,	0, 'P'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "services (ls) [options]\n"
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
    break;
  }

  case ZypperCommand::REFRESH_SERVICES_e:
  {
    static struct option options[] = {
      {"force",			no_argument,	0, 'f'},
      {"help",			no_argument,	0, 'h'},
      {"with-repos",		no_argument,	0, 'r'},
      {"restore-status",	no_argument,	0, 'R'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "refresh-services (refs) [options]\n"
      "\n"
      "Refresh defined repository index services.\n"
      "\n"
      "  Command options:\n"
      "-f, --force           Force a complete refresh.\n"
      "-r, --with-repos      Refresh also the service repositories.\n"
      "-R, --restore-status  Also restore service repositories enabled/disabled state.\n"
    );
    break;
  }

  case ZypperCommand::ADD_REPO_e:
  {
    static struct option service_add_options[] = {
      {"type", required_argument, 0, 't'},
      {"repo", 			required_argument, 	0, 'r'},	// :( conflicted with '-r --refresh', so ARG_REPO_PROP now uses -f/F
      {"help", no_argument, 0, 'h'},
      {"check", no_argument, 0, 'c'},
      {"no-check", no_argument, 0, 'C'},
      ARG_REPO_PROP,
      {0, 0, 0, 0}
    };
#if 0
    _(
      // translators: the %s = "yast2, rpm-md, plaindir"
      "addrepo (ar) [options] <URI> <alias>\n"
      "addrepo (ar) [options] <file.repo>\n"
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
    specific_options = service_add_options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("addrepo (ar) [OPTIONS] <URI> <ALIAS>")
    )
     .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("addrepo (ar) [OPTIONS] <FILE.repo>")
    )
    .description(// translators: command description
    _("Add a repository to the system. The repository can be specified by its URI or can be read from specified .repo file (even remote).")
    )
    .optionSectionCommandOptions()
    .option( "-r, --repo <FILE.repo>",	_("Just another means to specify a .repo file to read.") )
    .option( "-c, --check",		_("Probe URI.") )
    .option( "-C, --no-check",		_("Don't probe URI, probe later during refresh.") )
    .gap()
    .option_REPO_PROP
    .legacyOptionSection()
    .option( "-t, --type <TYPE>",	str::Format(_("The repository type is always autodetected. This option is ignored.") ) )
    ;
    break;
  }

  case ZypperCommand::LIST_REPOS_e:
  {
    static struct option service_list_options[] = {
      {"export",		required_argument,	0, 'e'},
      {"alias",			no_argument,		0, 'a'},
      {"name",			no_argument,		0, 'n'},
      {"refresh",		no_argument,		0, 'r'},
      {"uri",			no_argument,		0, 'u'},
      {"url",			no_argument,		0,  0 },
      {"priority",		no_argument,		0, 'p'},
      {"details",		no_argument,		0, 'd'},
      {"show-enabled-only",	no_argument,		0, 'E'},
      {"sort-by-priority",	no_argument,		0, 'P'},
      {"sort-by-uri",		no_argument,		0, 'U'},
      {"sort-by-alias",		no_argument,		0, 'A'},
      {"sort-by-name",		no_argument,		0, 'N'},
      {"service",		no_argument,		0, 's'},
      {"help",			no_argument,		0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_list_options;

    _command_help = _(
      "repos (lr) [options] [repo] ...\n"
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
    break;
  }

  case ZypperCommand::REMOVE_REPO_e:
  {
    static struct option service_delete_options[] = {
      {"help", no_argument, 0, 'h'},
      {"loose-auth", no_argument, 0, 0},
      {"loose-query", no_argument, 0, 0},
      ARG_REPO_SERVICE_COMMON_AGGREGATE,
      {0, 0, 0, 0}
    };
    specific_options = service_delete_options;
    _command_help = ( CommandHelpFormater() << _(
      "removerepo (rr) [options] <alias|#|URI>\n"
      "\n"
      "Remove repository specified by alias, number or URI.\n"
      "\n"
      "  Command options:\n"
      "    --loose-auth   Ignore user authentication data in the URI.\n"
      "    --loose-query  Ignore query string in the URI.\n"
    ))
    .gap()
    .option_REPO_AGGREGATES;
    break;
  }

  case ZypperCommand::RENAME_REPO_e:
  {
    static struct option service_rename_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_rename_options;
    _command_help = _(
      "renamerepo (nr) [options] <alias|#|URI> <new-alias>\n"
      "\n"
      "Assign new alias to the repository specified by alias, number or URI.\n"
      "\n"
      "This command has no additional options.\n"
    );
    break;
  }

  case ZypperCommand::MODIFY_REPO_e:
  {
    static struct option service_modify_options[] = {
      {"help", no_argument, 0, 'h'},
      ARG_REPO_PROP,
      /* LEGACY(ARG_REPO_PROP) prefers -f */	{"refresh",	no_argument,	0, 'r'},
      /* LEGACY(ARG_REPO_PROP) prefers -F */	{"no-refresh",	no_argument,	0, 'R'},
      ARG_REPO_SERVICE_COMMON_AGGREGATE,
      {0, 0, 0, 0}
    };
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
    specific_options = service_modify_options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate lowercase words
    _("modifyrepo (mr) <OPTIONS> <ALIAS|#|URI>")
    )
    .synopsis( str::Format(	// translators: command synopsis; do not translate lowercase words
    _("modifyrepo (mr) <OPTIONS> <%1%>") ) % "--all|--remote|--local|--medium-type"
    )
    .description( str::Format(	// translators: command description
    _("Modify properties of repositories specified by alias, number, or URI, or by the '%1%' aggregate options.") ) % "--all, --remote, --local, --medium-type"
    )
    .optionSectionCommandOptions()
    .option_REPO_PROP
    .gap()
    .option_REPO_AGGREGATES
    // Legacy Options:
    .legacyOptionSection()
    .legacyOption( "-r", "-f" )
    .legacyOption( "-R", "-F" )
    ;
    break;
  }

  case ZypperCommand::REFRESH_e:
  {
    static struct option refresh_options[] = {
      {"force", no_argument, 0, 'f'},
      {"force-build", no_argument, 0, 'b'},
      {"force-download", no_argument, 0, 'd'},
      {"build-only", no_argument, 0, 'B'},
      {"download-only", no_argument, 0, 'D'},
      {"repo", required_argument, 0, 'r'},
      {"services", no_argument, 0, 's'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = refresh_options;
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
    break;
  }

  case ZypperCommand::CLEAN_e:
  {
    static struct option service_list_options[] = {
      {"help", no_argument, 0, 'h'},
      {"repo", required_argument, 0, 'r'},
      {"metadata", no_argument, 0, 'm'},
      {"raw-metadata", no_argument, 0, 'M'},
      {"all", no_argument, 0, 'a'},
      {0, 0, 0, 0}
    };
    specific_options = service_list_options;
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
    break;
  }

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
    _command_help = str::form(_(
      // TranslatorExplanation the first %s = "package, patch, pattern, product"
      //  and the second %s = "patch"
      "list-updates (lu) [options]\n"
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
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      // translators: the first %s = "package, patch, pattern, product",
      // the second %s = "patch",
      // and the third %s = "only, in-avance, in-heaps, as-needed"
      "update (up) [options] [packagename] ...\n"
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
    .option( "-y, --no-confirm",	_("Don't require user interaction. Alias for the --non-interactive global option.") )

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
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      "patch [options]\n"
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
    .option(_("-b, --bugzilla[=#]"		"\n"	"List applicable patches for Bugzilla issues."))
    .option(_(    "--cve[=#]"			"\n"	"List applicable patches for CVE issues."))
    .option(_(    "--issues[=STRING]"		"\n"	"Look for issues matching the specified string."))
    .option(_(    "--date <YYYY-MM-DD>"		"\n"	"List only patches issued up to, but not including, the specified date."))
    .option(_("-g, --category <CATEGORY>"	"\n"	"List only patches with this category."))
    .option(_(    "--severity <SEVERITY>"	"\n"	"List only patches with this severity."))
    .option(_("-a, --all"			"\n"	"List all patches, not only applicable ones."))
    .option_WITHout_OPTIONAL
    .option(_("-r, --repo <ALIAS|#|URI>"	"\n"	"List only patches from the specified repository."))
    ;
    break;
  }

  case ZypperCommand::DIST_UPGRADE_e:
  {
    shared_ptr<DupOptions> myOpts( new DupOptions );
    _commandOptions = myOpts;
    static struct option dupdate_options[] = {
      {"no-confirm",                no_argument,       0, 'y'},	// pkg/apt/yum user convenience ==> --non-interactive
      {"repo",                      required_argument, 0, 'r'},
      {"from",                      required_argument, 0,  0 },
      {"replacefiles",              no_argument,       0,  0 },
      ARG_License_Agreement,
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
      // solver flags
      ARG_Solver_Flags_Common,
      ARG_Solver_Flags_Recommends,
      ARG_Solver_Flags_Installs,
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = dupdate_options;
    _command_help = ( CommandHelpFormater()
      << str::form(_(
      "dist-upgrade (dup) [options]\n"
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
      .option( "-y, --no-confirm",		_("Don't require user interaction. Alias for the --non-interactive global option.") )

      .optionSectionSolverOptions()
      .option_Solver_Flags_Common
      .option_Solver_Flags_Recommends
      .optionSectionExpertOptions()
      .option_Solver_Flags_Installs

      ;
    break;
  }

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
    _command_help = ( CommandHelpFormater() << _(
      "search (se) [options] [querystring] ...\n"
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
        .gap()
        .option("--supplements", _("Search for packages which supplement the search strings."));
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
    .option(_("-r, --repo <ALIAS|#|URI>"	"\n"	"Check for patches only in the specified repository."))
    .option(_("--updatestack-only"		"\n"	"Check only for patches which affect the package management itself."))
    .option_WITHout_OPTIONAL
    ;
    break;
  }

  case ZypperCommand::PATCHES_e:
  {
    static struct option patches_options[] = {
      {"repo", required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = patches_options;
    _command_help = _(
      "patches (pch) [repository] ...\n"
      "\n"
      "List all patches available in specified repositories.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
    );
    break;
  }

  case ZypperCommand::PACKAGES_e:
  {
    static struct option options[] = {
      {"repo",			required_argument,	0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog",		required_argument,	0, 'c'},
      ARG_not_INSTALLED_ONLY,
      {"orphaned",		no_argument,		0,  0 },
      {"suggested",		no_argument,		0,  0 },
      {"recommended",		no_argument,		0,  0 },
      {"unneeded",		no_argument,		0,  0 },
      {"sort-by-name",		no_argument,		0, 'N'},
      {"sort-by-repo",		no_argument,		0, 'R'},
      {"sort-by-catalog",	no_argument,		0,  0 },	// TRANSLATED into sort-by-repo
      {"help",			no_argument,		0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "packages (pa) [options] [repository] ...\n"
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
    break;
  }

  case ZypperCommand::PATTERNS_e:
  {
    static struct option options[] = {
      {"repo", required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      ARG_not_INSTALLED_ONLY,
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "patterns (pt) [options] [repository] ...\n"
      "\n"
      "List all patterns available in specified repositories.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
      "-i, --installed-only      Show only installed patterns.\n"
      "-u, --not-installed-only  Show only patterns which are not installed.\n"
    );
    break;
  }

  case ZypperCommand::PRODUCTS_e:
  {
    static struct option options[] = {
      {"repo", required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      ARG_not_INSTALLED_ONLY,
      {"xmlfwd",		required_argument,	0,  0 },
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = ( CommandHelpFormater()
      << _(
      "products (pd) [options] [repository] ...\n"
      "\n"
      "List all products available in specified repositories.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
      "-i, --installed-only      Show only installed products.\n"
      "-u, --not-installed-only  Show only products which are not installed.\n") )
    .option26( "--xmlfwd <tag>",	_("XML output only: Literally forward the XML tags found in a product file.") )
      ;
    break;
  }

  case ZypperCommand::INFO_e:
  {
    static struct option info_options[] = {
      {"match-substrings", no_argument, 0, 's'},
      {"type", required_argument, 0, 't'},
      {"repo", required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      {"provides", no_argument, 0, 0},
      {"requires", no_argument, 0, 0},
      {"conflicts", no_argument, 0, 0},
      {"obsoletes", no_argument, 0, 0},
      {"recommends", no_argument, 0, 0},
      {"supplements", no_argument, 0, 0},
      {"suggests", no_argument, 0, 0},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = info_options;
    _command_help = ( CommandHelpFormater() << str::form(_(
        "info (if) [options] <name> ...\n"
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
      .option("--supplements", _("Show supplements."));

    break;
  }

  // rug compatibility command, we have zypper info [-t <res_type>]
  case ZypperCommand::RUG_PATCH_INFO_e:
  {
    static struct option patch_info_options[] = {
      {"catalog", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = patch_info_options;
    _command_help = str::form(_(
      "patch-info <patchname> ...\n"
      "\n"
      "Show detailed information for patches.\n"
      "\n"
      "This is an alias for '%s'.\n"
    ), "zypper info -t patch");
    break;
  }

  // rug compatibility command, we have zypper info [-t <res_type>]
  case ZypperCommand::RUG_PATTERN_INFO_e:
  {
    static struct option options[] = {
      {"catalog", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = str::form(_(
      "pattern-info <pattern_name> ...\n"
      "\n"
      "Show detailed information for patterns.\n"
      "\n"
      "This is an alias for '%s'.\n"
    ), "zypper info -t pattern");
    break;
  }

  // rug compatibility command, we have zypper info [-t <res_type>]
  case ZypperCommand::RUG_PRODUCT_INFO_e:
  {
    static struct option options[] = {
      {"catalog", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = str::form(_(
      "product-info <product_name> ...\n"
      "\n"
      "Show detailed information for products.\n"
      "\n"
      "This is an alias for '%s'.\n"
    ), "zypper info -t product");
    break;
  }

  case ZypperCommand::WHAT_PROVIDES_e:
  {
    static struct option options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "what-provides (wp) <capability>\n"
      "\n"
      "List all packages providing the specified capability.\n"
      "\n"
      "This command has no additional options.\n"
    );
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
    _command_help = _(
      "moo\n"
      "\n"
      "Show an animal.\n"
      "\n"
      "This command has no additional options.\n"
      );
    break;
  }

  case ZypperCommand::ADD_LOCK_e:
  {
    static struct option options[] =
    {
      {"type", required_argument, 0, 't'},
      {"repo", required_argument, 0, 'r'},
      // rug compatibility (although rug does not seem to support this)
      {"catalog", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = str::form(_(
      "addlock (al) [options] <packagename> ...\n"
      "\n"
      "Add a package lock. Specify packages to lock by exact name or by a"
      " glob pattern using '*' and '?' wildcard characters.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI>  Restrict the lock to the specified repository.\n"
      "-t, --type <type>         Type of package (%s).\n"
      "                          Default: %s.\n"
    ), "package, patch, pattern, product", "package");

    break;
  }

  case ZypperCommand::REMOVE_LOCK_e:
  {
    static struct option options[] =
    {
      {"type", required_argument, 0, 't'},
      {"repo", required_argument, 0, 'r'},
      // rug compatibility (although rug does not seem to support this)
      {"catalog", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = str::form(_(
      "removelock (rl) [options] <lock-number|packagename> ...\n"
      "\n"
      "Remove a package lock. Specify the lock to remove by its number obtained"
      " with '%s' or by package name.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI>  Remove only locks with specified repository.\n"
      "-t, --type <type>         Type of package (%s).\n"
      "                          Default: %s.\n"
    ), "zypper locks", "package, patch, pattern, product", "package");
    break;
  }

  case ZypperCommand::LIST_LOCKS_e:
  {
    shared_ptr<ListLocksOptions> myOpts( new ListLocksOptions() );
    _commandOptions = myOpts;
    static struct option options[] =
    {
      {"help",			no_argument,		0, 'h'},
      {"matches",		no_argument,		0, 'm'},
      {"solvables",		no_argument,		0, 's'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate the command 'name (abbreviations)' or '-option' names
      _("locks (ll) [options]")
    )
    .description(	// translators: command description
      _("List current package locks.")
    )
    .optionSectionCommandOptions()
    .option( "-m, --matches",	// translators: -m, --matches
	     _("Show the number of resolvables matched by each lock.") )
    .option( "-s, --solvables",	// translators: -s, --solvables
	     _("List the resolvables matched by each lock.") )
    ;
    break;
  }

  case ZypperCommand::CLEAN_LOCKS_e:
  {
    static struct option options[] =
    {
      {"help", no_argument, 0, 'h'},
      {"only-duplicates", no_argument, 0, 'd' },
      {"only-empty", no_argument, 0, 'e' },
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = (_(
      "cleanlocks (cl)\n"
      "\n"
      "Remove useless locks.\n"
      "\n"
      "  Command options:\n"
      "-d, --only-duplicates     Clean only duplicate locks.\n"
      "-e, --only-empty          Clean only locks which doesn't lock anything.\n"
    ));
    break;
  }

  case ZypperCommand::TARGET_OS_e:
  {
    static struct option options[] =
    {
      {"help",  no_argument, 0, 'h'},
      {"label", no_argument, 0, 'l'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "targetos (tos) [options]\n"
      "\n"
      "Show various information about the target operating system.\n"
      "By default, an ID string is shown.\n"
      "\n"
      "  Command options:\n"
      "-l, --label                 Show the operating system label.\n"
    );
    break;
  }

  case ZypperCommand::VERSION_CMP_e:
  {
    static struct option options[] =
    {
      {"help", no_argument, 0, 'h'},
      {"match", no_argument, 0, 'm'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "versioncmp (vcmp) <version1> <version2>\n"
      "\n"
      "Compare the versions supplied as arguments.\n"
      "\n"
      "  Command options:\n"
      "-m, --match  Takes missing release number as any release.\n"
    );
    break;
  }

  case ZypperCommand::LICENSES_e:
  {
    static struct option options[] =
    {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "licenses\n"
      "\n"
      "Report licenses and EULAs of currently installed software packages.\n"
      "\n"
      "This command has no additional options.\n"
    );
    break;
  }


  case ZypperCommand::PS_e:
  {
    shared_ptr<PsOptions> myOpts( new PsOptions() );
    _commandOptions = myOpts;
    static struct option options[] =
    {
      {"help",		no_argument,		0, 'h'},
      {"short",		no_argument,		0, 's'},
      {"print",		required_argument,	0,  0 },
      {"debugFile",		required_argument,	0,  'd' },
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = CommandHelpFormater()
    .synopsis(	// translators: command synopsis; do not translate the command 'name (abbreviations)' or '-option' names
      _("ps [options]")
    )
    .description(	// translators: command description
      _("List running processes which might still use files and libraries deleted by recent upgrades.")
    )
    .optionSectionCommandOptions()
    .option( "-s, --short",	// translators: -s, --short
	     _("Create a short table not showing the deleted files. Given twice, show only processes which are associated with a system service. Given three times, list the associated system service names only.") )
    .option( "--print <format>",	// translators: --print <format>
	     _("For each associated system service print <format> on the standard output, followed by a newline. Any '%s' directive in <format> is replaced by the system service name.") )
    .option("-d, --debugFile <path>", // translators: -d, --debugFile <path>
       _("Write debug output to file <path>."))
    ;
    break;
  }


  case ZypperCommand::DOWNLOAD_e:
  {
    shared_ptr<DownloadOptions> myOpts( new DownloadOptions() );
    _commandOptions = myOpts;
    static struct option options[] =
    {
      {"help",			no_argument,		0, 'h'},
      {"all-matches",		no_argument,		&myOpts->_allmatches, 1},
      {"dry-run",		no_argument,		&myOpts->_dryrun, 1},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "download [options] <packages>...\n"
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
    break;
  }


  case ZypperCommand::SOURCE_DOWNLOAD_e:
  {
    shared_ptr<SourceDownloadOptions> myOpts( new SourceDownloadOptions() );
    _commandOptions = myOpts;
    static struct option options[] =
    {
      {"help",			no_argument, 0, 'h'},
      {"directory",		required_argument, 0, 'd'},
//       {"manifest",		no_argument, &myOpts->_manifest, 1},
//       {"no-manifest",		no_argument, &myOpts->_manifest, 0},
      {"delete",		no_argument, &myOpts->_delete, 1},
      {"no-delete",		no_argument, &myOpts->_delete, 0},
      {"status",		no_argument, &myOpts->_dryrun, 1},
      {0, 0, 0, 0}
    };
    specific_options = options;
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
    break;
  }


  case ZypperCommand::SHELL_QUIT_e:
  {
    static struct option quit_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = quit_options;
    _command_help = _(
      "quit (exit, ^D)\n"
      "\n"
      "Quit the current zypper shell.\n"
      "\n"
      "This command has no additional options.\n"
    );
    break;
  }

  case ZypperCommand::SHELL_e:
  {
    static struct option quit_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = quit_options;
    _command_help = _(
      "shell (sh)\n"
      "\n"
      "Enter the zypper command shell.\n"
      "\n"
      "This command has no additional options.\n"
    );
    break;
  }

  case ZypperCommand::RUG_SERVICE_TYPES_e:
  {
    static struct option options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      // translators: this is just a legacy command
      "service-types (st)\n"
      "\n"
      "List available service types.\n"
    );
    break;
  }

  case ZypperCommand::RUG_LIST_RESOLVABLES_e:
  {
    static struct option options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      // translators: this is just a legacy command
      "list-resolvables (lr)\n"
      "\n"
      "List available resolvable types.\n"
    );
    break;
  }

  case ZypperCommand::RUG_MOUNT_e:
  {
    static struct option options[] = {
      {"alias", required_argument, 0, 'a'},
      {"name", required_argument, 0, 'n'},
      // dummy for now - always recurse
      {"recurse", required_argument, 0, 'r'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      // trunslators: this is a rug-compatibility command (equivalent of
      // 'zypper addrepo -t plaindir URI'). You can refer to rug's translations
      // for how to translate specific terms like channel or service if in doubt.
      "mount\n"
      "\n"
      "Mount directory with RPMs as a channel.\n"
      "\n"
      "  Command options:\n"
      "-a, --alias <alias>  Use given string as service alias.\n"
      "-n, --name <name>    Use given string as service name.\n"
      "-r, --recurse        Dive into subdirectories.\n"
    );
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
    _command_help = str::form(_(
      "patch-search [options] [querystring...]\n"
      "\n"
      "Search for patches matching given search strings. This is an alias for '%s'.\n"
    ), "zypper -r search -t patch --detail ...");
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
    _command_help = _(
      // translators: this is just a legacy command
      "ping [options]\n"
      "\n"
      "This command has dummy implementation which always returns 0.\n"
    );
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
  if ( _copts.count("_unknown") || _copts.count("_missing_arg") )
  {
    setExitCode( ZYPPER_EXIT_ERR_SYNTAX );
    ERR << "Unknown option or missing argument, returning." << endl;
    return;
  }

  // Leagcy cli translations (mostly from rug to zypper)
  legacyCLITranslate( _copts, "agree-to-third-party-licenses",	"auto-agree-with-licenses" );
  legacyCLITranslate( _copts, "sort-by-catalog",		"sort-by-repo" );
  legacyCLITranslate( _copts, "uninstalled-only",		"not-installed-only",	Out::HIGH );	// bsc#972997: Prefer --not-installed-only over misleading --uninstalled-only

  if ( command().toEnum() == ZypperCommand::ADD_REPO_e  || command().toEnum() == ZypperCommand::ADD_SERVICE_e ) {
    legacyCLITranslate( _copts, "type",	"",  Out::NORMAL, LegacyCLIMsgType::Ignored);
  }

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
      SLE15_SearchPackagesHintHack( *this );
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

  // --------------------------( service list )-------------------------------

  case ZypperCommand::LIST_SERVICES_e:
  {
    initRepoManager();
    if ( copts.count( "with-repos" ) )
      checkIfToRefreshPluginServices( *this );
    list_services( *this );

    break;
  }

  // --------------------------( service refresh )-----------------------------

  case ZypperCommand::REFRESH_SERVICES_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for refreshing services.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    // needed to be able to retrieve target distribution
    init_target(*this);
    _gopts.rm_options.servicesTargetDistro =
      God->target()->targetDistribution();

    initRepoManager();

    refresh_services( *this );

    break;
  }

  // --------------------------( add service )---------------------------------

  case ZypperCommand::ADD_SERVICE_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for modifying system services.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    // too many arguments
    if ( _arguments.size() > 2 )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    // missing arguments
    if ( _arguments.size() < 2 )
    {
      report_required_arg_missing( out(), _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    Url url = make_url( _arguments[0] );
    if ( !url.isValid() )
    {
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    initRepoManager();
    // force specific repository type.
    std::string type = copts.count("type") ? copts["type"].front() : "";

    // check for valid service type
    bool isservice = false;
    if ( type.empty() )
    {
      // zypper does not access net when adding repos/services, thus for zypper
      // the URI is always service unless --type is explicitely specified.
      isservice = true;
    }
    else
    {
      try
      {
	repo::ServiceType stype(type);
	isservice = true;
      }
      catch ( const repo::RepoUnknownTypeException & e )
      {}
    }

    if ( isservice )
      add_service_by_url( *this, url, _arguments[1] );
    else
    {
      try
      {
        add_repo_by_url( *this, url, _arguments[1]);
      }
      catch ( const repo::RepoUnknownTypeException & e )
      {
        ZYPP_CAUGHT( e );
        out().error(
            str::form(_("'%s' is not a valid service type."), type.c_str()),
            str::form(
                _("See '%s' or '%s' to get a list of known service types."),
                "zypper help addservice", "man zypper"));
        setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      }
    }

    break;
  }

  case ZypperCommand::MODIFY_SERVICE_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for modifying system services.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    bool non_alias = copts.count("all") || copts.count("local") || copts.count("remote") || copts.count("medium-type");

    if ( _arguments.size() < 1 && !non_alias )
    {
      // translators: aggregate option is e.g. "--all". This message will be
      // followed by ms command help text which will explain it
      out().error(_("Alias or an aggregate option is required."));
      ERR << "No alias argument given." << endl;
      out().info( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }
    // too many arguments
    if ( _arguments.size() > 1 || ( _arguments.size() > 0 && non_alias ) )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    initRepoManager();

    if ( non_alias )
    {
      modify_services_by_option( *this );
    }
    else
    {
      repo::RepoInfoBase_Ptr srv;
      if ( match_service( *this, _arguments[0], srv ) )
      {
        if ( dynamic_pointer_cast<ServiceInfo>(srv) )
          modify_service( *this, srv->alias() );
        else
          modify_repo( *this, srv->alias() );
      }
      else
      {
        out().error( str::Format(_("Service '%s' not found.")) % _arguments[0] );
        ERR << "Service " << _arguments[0] << " not found" << endl;
      }
    }

    break;
  }

  // --------------------------( repo list )----------------------------------

  case ZypperCommand::LIST_REPOS_e:
  {
    initRepoManager();
    checkIfToRefreshPluginServices( *this );
    list_repos( *this );

    break;
  }

  // --------------------------( addrepo )------------------------------------

  case ZypperCommand::ADD_REPO_e:
  case ZypperCommand::RUG_MOUNT_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error( _("Root privileges are required for modifying system repositories.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    // too many arguments
    if ( _arguments.size() > 2 )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    try
    {
      // add repository specified in .repo file
      if ( copts.count("repo") )
      {
        add_repo_from_file( *this,copts["repo"].front() );
        return;
      }

      switch ( _arguments.size() )
      {
      // display help message if insufficient info was given
      case 0:
        out().error(_("Too few arguments."));
        ERR << "Too few arguments." << endl;
        out().info( _command_help );
        setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
        return;
      case 1:
        if ( command() == ZypperCommand::RUG_MOUNT )
        {
          std::string alias;
          parsed_opts::const_iterator it = _copts.find("alias");
          if ( it != _copts.end() )
            alias = it->second.front();
          // get the last component of the path
          if ( alias.empty() )
          {
            Pathname path( _arguments[0] );
            alias = path.basename();
          }
          _arguments.push_back( alias );
          // continue to case 2:
        }
        else if( !isRepoFile( _arguments[0] ) )
        {
          out().error(_("If only one argument is used, it must be a URI pointing to a .repo file."));
          ERR << "Not a repo file." << endl;
          out().info( _command_help );
          setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
          return;
        }
        else
        {
          initRepoManager();
          add_repo_from_file( *this,_arguments[0] );
          break;
        }
      case 2:
	Url url;
	if ( _arguments[0].find("obs") == 0 )
	  url = make_obs_url( _arguments[0], config().obs_baseUrl, config().obs_platform );
	else
	  url = make_url( _arguments[0] );
        if ( !url.isValid() )
        {
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }

        if ( copts.count("check") )
        {
          if ( !copts.count("no-check") )
            _gopts.rm_options.probe = true;
          else
            out().warning(str::form(
              _("Cannot use %s together with %s. Using the %s setting."),
              "--check", "--no-check", "zypp.conf")
                ,Out::QUIET );
        }
        else if ( copts.count("no-check") )
          _gopts.rm_options.probe = false;

        initRepoManager();

        // load gpg keys
        init_target( *this );

        add_repo_by_url( *this, url, _arguments[1]/*alias*/ );
        return;
      }
    }
    catch ( const repo::RepoUnknownTypeException & e )
    {
      ZYPP_CAUGHT( e );
      out().error( e, _("Specified type is not a valid repository type:"),
		   str::form( _("See '%s' or '%s' to get a list of known repository types."),
			      "zypper help addrepo", "man zypper" ) );
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    }

    break;
  }

  // --------------------------( delete repo )--------------------------------

  case ZypperCommand::REMOVE_SERVICE_e:
  case ZypperCommand::REMOVE_REPO_e:
  {
    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        command() == ZypperCommand::REMOVE_REPO ?
          _("Root privileges are required for modifying system repositories.") :
          _("Root privileges are required for modifying system services.") );
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    if (command() == ZypperCommand::REMOVE_REPO)
    {
      bool aggregate = copts.count("all") || copts.count("local") || copts.count("remote") || copts.count("medium-type");

      if ( _arguments.size() < 1 && !aggregate )
      {
        report_alias_or_aggregate_required ( out(), _command_help );
        ERR << "No alias argument given." << endl;
        setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
        return;
      }

      // too many arguments
      if ( _arguments.size() && aggregate )
      {
        report_too_many_arguments( out(), _command_help );
        setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
        return;
      }

      initRepoManager();
      if ( aggregate )
      {
        remove_repos_by_option( *this );
      }
      else
      {
        // must store repository before remove to ensure correct match number
        std::set<RepoInfo,RepoInfoAliasComparator> repo_to_remove;
        for_(it, _arguments.begin(), _arguments.end())
        {
          RepoInfo repo;
          if (match_repo(*this,*it,&repo))
          {
            repo_to_remove.insert(repo);
          }
          else
          {
            MIL << "Repository not found by given alias, number or URI." << endl;
            // translators: %s is the supplied command line argument which
            // for which no repository counterpart was found
            out().error( str::Format(_("Repository '%s' not found by alias, number or URI.")) % *it );
          }
        }

        for_(it, repo_to_remove.begin(), repo_to_remove.end())
          remove_repo(*this,*it);
      }
    }
    else
    {
      if (_arguments.size() < 1)
      {
        ERR << "Required argument missing." << endl;
        report_required_arg_missing( out(), _command_help );
        setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
        return;
      }

      initRepoManager();

      std::set<repo::RepoInfoBase_Ptr, ServiceAliasComparator> to_remove;
      for_(it, _arguments.begin(), _arguments.end())
      {
        repo::RepoInfoBase_Ptr s;
        if (match_service(*this, *it, s))
        {
          to_remove.insert(s);
        }
        else
        {
          MIL << "Service not found by given alias, number or URI." << endl;
	  // translators: %s is the supplied command line argument which
	  // for which no service counterpart was found
	  out().error( str::Format(_("Service '%s' not found by alias, number or URI.")) % *it );
        }
      }

      for_(it, to_remove.begin(), to_remove.end())
      {
        RepoInfo_Ptr repo_ptr = dynamic_pointer_cast<RepoInfo>(*it);
        if (repo_ptr)
          remove_repo(*this, *repo_ptr);
        else
          remove_service(*this, *dynamic_pointer_cast<ServiceInfo>(*it));
      }
    }

    break;
  }

  // --------------------------( rename repo )--------------------------------

  case ZypperCommand::RENAME_REPO_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for modifying system repositories.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    if ( _arguments.size() < 2 )
    {
      out().error(_("Too few arguments. At least URI and alias are required.") );
      ERR << "Too few arguments. At least URI and alias are required." << endl;
      out().info( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }
    // too many arguments
    else if ( _arguments.size() > 2 )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    initRepoManager();
    try
    {
      RepoInfo repo;
      if ( match_repo( *this,_arguments[0], &repo  ))
      {
	rename_repo( *this, repo.alias(), _arguments[1] );
      }
      else
      {
	 out().error( str::Format(_("Repository '%s' not found.")) % _arguments[0] );
         ERR << "Repo " << _arguments[0] << " not found" << endl;
      }
    }
    catch ( const Exception & excpt_r )
    {
      out().error( excpt_r.asUserString() );
      setExitCode( ZYPPER_EXIT_ERR_ZYPP );
      return;
    }

    return;
  }

  // --------------------------( modify repo )--------------------------------

  case ZypperCommand::MODIFY_REPO_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for modifying system repositories.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    bool aggregate = copts.count("all") || copts.count("local") || copts.count("remote") || copts.count("medium-type");

    if ( _arguments.size() < 1 && !aggregate )
    {
      report_alias_or_aggregate_required ( out(), _command_help );
      ERR << "No alias argument given." << endl;
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    // too many arguments
    if ( _arguments.size() && aggregate )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    initRepoManager();
    if ( aggregate )
    {
      modify_repos_by_option( *this );
    }
    else
    {
      for_( arg,_arguments.begin(),_arguments.end() )
      {
        RepoInfo r;
        if ( match_repo(*this,*arg,&r) )
        {
          modify_repo( *this, r.alias() );
        }
        else
        {
          out().error( str::Format(_("Repository %s not found.")) % *arg );
          ERR << "Repo " << *arg << " not found" << endl;
          setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
        }
      }
    }

    break;
  }

  // --------------------------( refresh )------------------------------------

  case ZypperCommand::REFRESH_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for refreshing system repositories.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    if ( globalOpts().no_refresh )
      out().warning( str::Format(_("The '%s' global option has no effect here.")) % "--no-refresh" );

    // by default refresh only repositories
    initRepoManager();
    if ( copts.count("services") )
    {
      if ( !_arguments.empty() )
      {
        out().error(str::form(_("Arguments are not allowed if '%s' is used."), "--services"));
        setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
        return;
      }
      // needed to be able to retrieve target distribution
      init_target( *this );
      _gopts.rm_options.servicesTargetDistro = God->target()->targetDistribution();
      refresh_services( *this );
    }
    else
    {
      checkIfToRefreshPluginServices( *this );
    }
    refresh_repos( *this );
    break;
  }

  // --------------------------( clean )------------------------------------

  case ZypperCommand::CLEAN_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for cleaning local caches.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    initRepoManager();
    clean_repos( *this );
    break;
  }

  // --------------------------( remove/install )-----------------------------

  case ZypperCommand::INSTALL_e:
  case ZypperCommand::REMOVE_e:
  {
    if ( _arguments.size() < 1 && !_copts.count("entire-catalog") )
    {
      out().error(
          _("Too few arguments."),
          _("At least one package name is required."));
      out().info( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for installing or uninstalling packages.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    // rug compatibility code
    parsed_opts::const_iterator optit;
    if ( (optit = _copts.find("entire-catalog")) != _copts.end() )
    {
      if ( !_arguments.empty() )
        // translators: rug related message, shown if
        // 'zypper in --entire-catalog foorepo someargument' is specified
        out().warning(_("Ignoring arguments, marking the entire repository."));
      _arguments.clear();
      _arguments.push_back("*");
      _copts["from"] = _copts["entire-catalog"];
    }

    // read resolvable type
    std::string skind = copts.count("type")?  copts["type"].front() : "package";
    kind = string_to_kind( skind );
    if ( kind == ResObject::Kind() )
    {
      out().error( str::Format(_("Unknown package type: %s")) % skind );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    bool install_not_remove = command() == ZypperCommand::INSTALL;

    // can't remove patch
    if ( kind == ResKind::patch && !install_not_remove )
    {
      out().error( _("Cannot uninstall patches."),
		   _("Installed status of a patch is determined solely based on its dependencies.\n"
		     "Patches are not installed in sense of copied files, database records,\n"
		     "or similar.") );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      ZYPP_THROW( ExitRequestException("not implemented") );
    }

     // can't remove source package
    if ( kind == ResKind::srcpackage && !install_not_remove )
    {
      out().error(_("Uninstallation of a source package not defined and implemented."));
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      ZYPP_THROW( ExitRequestException("not implemented") );
    }

    // parse the download options to check for errors
    get_download_option( *this );

    initRepoManager();

    // check for rpm files among the arguments
    ArgList rpms_files_caps;
    Pathname cliRPMCache;	// temporary plaindir repo (if needed)
    if ( install_not_remove )
    {
      for ( std::vector<std::string>::iterator it = _arguments.begin(); it != _arguments.end(); )
      {
        if ( looks_like_rpm_file( *it ) )
        {
          DBG << *it << " looks like rpm file" << endl;
          out().info( str::Format(_("'%s' looks like an RPM file. Will try to download it.")) % *it,
		      Out::HIGH );

          // download the rpm into the temp cache
	  if ( cliRPMCache.empty() )
	    cliRPMCache = runtimeData().tmpdir / TMP_RPM_REPO_ALIAS / "%CLI%";
	  Pathname rpmpath = cache_rpm( *it, cliRPMCache );
          if ( rpmpath.empty() )
          {
            out().error( str::Format(_("Problem with the RPM file specified as '%s', skipping.")) % *it );
          }
          else
          {
            using target::rpm::RpmHeader;
            // rpm header (need name-version-release)
            RpmHeader::constPtr header = RpmHeader::readPackage( rpmpath, RpmHeader::NOSIGNATURE );
            if ( header )
            {
              std::string nvrcap =
                TMP_RPM_REPO_ALIAS ":" +
                header->tag_name() + "=" +
                str::numstring(header->tag_epoch()) + ":" +
                header->tag_version() + "-" +
                header->tag_release();
              DBG << "rpm package capability: " << nvrcap << endl;

              // store the rpm file capability string (name=version-release)
              rpms_files_caps.push_back( nvrcap );
            }
            else
            {
              out().error( str::Format(_("Problem reading the RPM header of %s. Is it an RPM file?")) % *it );
            }
          }

          // remove this rpm argument
          it = _arguments.erase( it );
        }
        else
          ++it;
      }
    }

    // If there were some rpm files, add the rpm cache as a temporary plaindir repo.
    // Set up as temp repo, but redirect PackagesPath to ZYPPER_RPM_CACHE_DIR. This
    // way downloaded packages (e.g. --download-only) are accessible until they get
    // installed (unless .keeppackages)
    if ( !rpms_files_caps.empty() )
    {
      // add a plaindir repo
      RepoInfo repo;
      repo.setAlias( TMP_RPM_REPO_ALIAS );
      repo.setName(_("Plain RPM files cache") );
      repo.setBaseUrl( cliRPMCache.asDirUrl() );
      repo.setMetadataPath( runtimeData().tmpdir / TMP_RPM_REPO_ALIAS / "%AUTO%" );
      repo.setPackagesPath( Pathname::assertprefix( _gopts.root_dir, ZYPPER_RPM_CACHE_DIR ) );
      repo.setType( repo::RepoType::RPMPLAINDIR );
      repo.setEnabled( true );
      repo.setAutorefresh( true );
      repo.setKeepPackages( false );

      if ( _copts.count("allow-unsigned-rpm") )
      {
        repo.setGpgCheck(RepoInfo::GpgCheck::AllowUnsignedPackage);
      }

      // shut up zypper
      SCOPED_VERBOSITY( out(), Out::QUIET );
      refresh_repo( *this, repo );
      runtimeData().temporary_repos.push_back( repo );
    }
    // no rpms and no other arguments either
    else if ( _arguments.empty() )
    {
      out().error(_("No valid arguments specified.") );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    //! \todo quit here if the argument list remains empty after founding only invalid rpm args

    // prepare repositories
    // bsc#606220: don't load repos when removing
    if ( install_not_remove )
    {
      init_repos( *this );
      if ( exitCode() != ZYPPER_EXIT_OK )
	return;

      if ( _rdata.repos.empty() )
      {
	out().warning(_("No repositories defined. Operating only with the installed resolvables. Nothing can be installed.") );
	if ( command() == ZypperCommand::INSTALL )
	{
	  setExitCode( ZYPPER_EXIT_NO_REPOS );
	  return;
	}
      }
    }

    // prepare target
    init_target( *this );
    // load metadata
    load_resolvables( *this );
    // needed to compute status of PPP
    resolve( *this );

    // parse package arguments
    PackageArgs::Options argopts;
    if ( !install_not_remove )
      argopts.do_by_default = false;
    PackageArgs args( kind, argopts );

    // tell the solver what we want

    SolverRequester::Options sropts;
    if ( copts.find("force") != copts.end() )
      sropts.force = true;
    if ( copts.find("oldpackage") != copts.end() )
      sropts.oldpackage = true;
    sropts.force_by_cap  = copts.find("capability") != copts.end();
    sropts.force_by_name = copts.find("name") != copts.end();
    if ( sropts.force )
      sropts.force_by_name = true;

    if ( sropts.force_by_cap && sropts.force_by_name )
    {
      // translators: meaning --capability contradicts --force/--name
      out().error( str::form(_("%s contradicts %s"), "--capability", (sropts.force ? "--force" : "--name") ) );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      ZYPP_THROW( ExitRequestException("invalid args") );
    }

    if ( install_not_remove && sropts.force_by_cap && sropts.force )
    {
      // translators: meaning --force with --capability
      out().error( str::form(_("%s cannot currently be used with %s"), "--force", "--capability" ) );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      ZYPP_THROW( ExitRequestException("invalid args") );
    }

    if ( install_not_remove && (optit = copts.find("from")) != copts.end() )
      repo_specs_to_aliases( *this, optit->second, sropts.from_repos );

    SolverRequester sr( sropts );
    if ( install_not_remove )
      sr.install( args );
    else
      sr.remove( args );
    PackageArgs rpm_args( rpms_files_caps );
    sr.install( rpm_args );

    sr.printFeedback(out());

    if ( !globalOpts().ignore_unknown
      && ( sr.hasFeedback( SolverRequester::Feedback::NOT_FOUND_NAME )
        || sr.hasFeedback( SolverRequester::Feedback::NOT_FOUND_CAP ) ) )
    {
      setExitCode( ZYPPER_EXIT_INF_CAP_NOT_FOUND );
      if ( command () != ZypperCommand::REMOVE && globalOpts().non_interactive )	// bsc#980263: relax if removing packages
        ZYPP_THROW( ExitRequestException("name or capability not found") );
    }

    // give user feedback from package selection
    // TODO feedback goes here

    solve_and_commit( *this );

    break;
  }

  // -------------------( source install )------------------------------------

  case ZypperCommand::SRC_INSTALL_e:
  {
    if ( _arguments.size() < 1 )
    {
      out().error(_("Source package name is a required argument.") );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for installing or uninstalling packages.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    initRepoManager();
    init_target( *this );
    init_repos( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return;
    // if ( !copts.count("no-build-deps") ) // if target resolvables are not read, solver produces a weird result
    load_target_resolvables( *this );
    load_repo_resolvables( *this );

    if ( copts.count("no-build-deps") )
      mark_src_pkgs( *this);
    else
      build_deps_install( *this );

    solve_and_commit( *this );
    break;
  }

  case ZypperCommand::VERIFY_e:
  case ZypperCommand::INSTALL_NEW_RECOMMENDS_e:
  {
    // too many arguments
    if ( _arguments.size() > 0 )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for installing or uninstalling packages.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    // parse the download options to check for errors
    get_download_option( *this );

    initRepoManager();

    init_repos( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return;

    if ( _rdata.repos.empty() )
    {
      out().warning(_("No repositories defined. Operating only with the installed resolvables. Nothing can be installed.") );
    }

    // prepare target
    init_target( *this );
    // load metadata
    load_resolvables( *this );
    solve_and_commit( *this );

    break;
  }

  // --------------------------( search )-------------------------------------

  case ZypperCommand::SEARCH_e:
  case ZypperCommand::RUG_PATCH_SEARCH_e:
  {
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

    SLE15_SearchPackagesHintHack( *this );
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

  case ZypperCommand::PATCHES_e:
  case ZypperCommand::PATTERNS_e:
  case ZypperCommand::PACKAGES_e:
  case ZypperCommand::PRODUCTS_e:
  {
    for ( auto & repo : _arguments )
    {
      // see todo at ::copts
      copts["repo"].push_back( repo );	// convert arguments to '-r repo'
      _copts["repo"].push_back( repo );	// convert arguments to '-r repo'
    }

    initRepoManager();

    init_target( *this);
    init_repos( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return;
    load_resolvables( *this );
    // needed to compute status of PPP
    // Currently CleandepsOnRemove adds information about user selected packages,
    // which enhances the computation of unneeded packages. Might be superfluous in the future.
    AutoDispose<bool> restoreCleandepsOnRemove( God->resolver()->cleandepsOnRemove(),
						bind( &Resolver::setCleandepsOnRemove, God->resolver(), _1 ) );
    God->resolver()->setCleandepsOnRemove( true );
    resolve( *this );

    switch ( command().toEnum() )
    {
    case ZypperCommand::PATCHES_e:
      list_patches( *this );
      break;
    case ZypperCommand::PATTERNS_e:
      list_patterns( *this );
      break;
    case ZypperCommand::PACKAGES_e:
      list_packages( *this );
      break;
    case ZypperCommand::PRODUCTS_e:
      if ( copts.count("xmlfwd") && out().type() != Out::TYPE_XML )
      {
	out().warning( str::Format(_("Option %1% has no effect without the %2% global option.")) % "--xmlfwd" % "--xmlout" );
      }
      list_products( *this );
      break;
    default:;
    }

    break;
  }

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

  // ----------------------------( dist-upgrade )------------------------------

  case ZypperCommand::DIST_UPGRADE_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for performing a distribution upgrade.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    // too many arguments
    if (_arguments.size() > 0)
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    // parse the download options to check for errors
    get_download_option( *this );

    initRepoManager();

    if ( !copts.count("repo") && !copts.count("from") && repoManager().knownRepositories().size() > 1 )
      out().warning( str::form(
	_("You are about to do a distribution upgrade with all enabled"
        " repositories. Make sure these repositories are compatible before you"
	" continue. See '%s' for more information about this command."),
	"man zypper") );

    init_target( *this );
    init_repos( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return;
    load_resolvables( *this );

    solve_and_commit( *this );

    break;
  }

  // -----------------------------( info )------------------------------------

  case ZypperCommand::INFO_e:
  case ZypperCommand::RUG_PATCH_INFO_e:
  case ZypperCommand::RUG_PATTERN_INFO_e:
  case ZypperCommand::RUG_PRODUCT_INFO_e:
  {
    if ( _arguments.size() < 1 )
    {
      out().error(_("Required argument missing.") );
      ERR << "Required argument missing." << endl;
      std::ostringstream s;
      s << _("Usage") << ':' << endl;
      s << _command_help;
      out().info( s.str() );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    ResKindSet kinds;
    switch ( command().toEnum() )
    {
    case ZypperCommand::RUG_PATCH_INFO_e:
      kinds.insert( ResKind::patch );
      break;
    case ZypperCommand::RUG_PATTERN_INFO_e:
      kinds.insert( ResKind::pattern );
      break;
    case ZypperCommand::RUG_PRODUCT_INFO_e:
      kinds.insert( ResKind::product );
      break;
    default:
    case ZypperCommand::INFO_e:
      if ( copts.count("type") )
      {
	for ( auto kindstr : copts["type"] )
	{
	  ResKind kind( string_to_kind( kindstr ) );
	  if ( kind )
	    kinds.insert( kind );
	  else
	  {
	    out().error( str::Format(_("Unknown package type '%s'.")) % kindstr );
	    setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
	    return;
	  }
	}
      }
      break;
    }

    initRepoManager();
    init_target( *this );
    init_repos( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return;
    load_resolvables( *this );
    // needed to compute status of PPP
    resolve( *this );

    printInfo( *this, std::move(kinds) );

    return;
  }

  // -----------------------------( locks )------------------------------------

  case ZypperCommand::ADD_LOCK_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for adding of package locks.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    // too few arguments
    if ( _arguments.empty() )
    {
      report_required_arg_missing( out(), _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

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
        kinds.insert( kind );
      }
    }
    //else
    //  let add_locks determine the appropriate type (bnc #551956)

    add_locks( *this, _arguments, kinds );

    break;
  }

  case ZypperCommand::REMOVE_LOCK_e:
  {
    // check root user
    if ( geteuid() != 0 && !globalOpts().changedRoot )
    {
      out().error(_("Root privileges are required for adding of package locks.") );
      setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
      return;
    }

    if ( _arguments.empty() )
    {
      report_required_arg_missing( out(), _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

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
        kinds.insert( kind );
      }
    }
    //else
    //  let remove_locks determine the appropriate type

    remove_locks( *this, _arguments, kinds );

    break;
  }

  case ZypperCommand::LIST_LOCKS_e:
  {
    shared_ptr<ListLocksOptions> listLocksOptions = commandOptionsAs<ListLocksOptions>();
    if ( !listLocksOptions )
      throw( Out::Error( ZYPPER_EXIT_ERR_BUG, "Wrong or missing options struct." ) );

    bool needLoadSystem = false;

    if ( copts.count("matches") )
    {
      listLocksOptions->_withMatches = true;
      needLoadSystem = true;
    }
    if ( copts.count("solvables") )
    {
      listLocksOptions->_withSolvables = true;
      needLoadSystem = true;
    }

    if ( needLoadSystem )
      defaultLoadSystem();

    list_locks( *this );

    break;
  }

  case ZypperCommand::CLEAN_LOCKS_e:
  {
    initRepoManager();
    init_target( *this );
    init_repos( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return;
    load_resolvables( *this );

    Locks::instance().read();
    Locks::size_type start = Locks::instance().size();
    if ( !copts.count("only-duplicate") )
      Locks::instance().removeEmpty();
    if ( !copts.count("only-empty") )
      Locks::instance().removeDuplicates();

    Locks::instance().save();

    Locks::size_type diff = start - Locks::instance().size();
    out().info( str::form( PL_("Removed %lu lock.","Removed %lu locks.", diff), (long unsigned)diff ) );

    break;
  }

  // ----------------------------(utils/others)--------------------------------

  case ZypperCommand::TARGET_OS_e:
  {
    if (out().type() == Out::TYPE_XML)
    {
      out().error(_("XML output not implemented for this command.") );
      break;
    }

    if ( copts.find("label") != copts.end() )
    {
      if ( globalOpts().terse )
      {
        cout << "labelLong\t" << str::escape(Target::distributionLabel( globalOpts().root_dir ).summary, '\t') << endl;
        cout << "labelShort\t" << str::escape(Target::distributionLabel( globalOpts().root_dir ).shortName, '\t') << endl;
      }
      else
      {
        out().info( str::form(_("Distribution Label: %s"), Target::distributionLabel( globalOpts().root_dir ).summary.c_str() ) );
        out().info( str::form(_("Short Label: %s"), Target::distributionLabel( globalOpts().root_dir ).shortName.c_str() ) );
      }
    }
    else
      out().info( Target::targetDistribution( globalOpts().root_dir ) );

    break;
  }

  case ZypperCommand::VERSION_CMP_e:
  {
    if ( _arguments.size() < 2 )
    {
      report_required_arg_missing( out(), _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }
    else if ( _arguments.size() > 2 )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    Edition lhs( _arguments[0] );
    Edition rhs( _arguments[1] );
    int result;
    if ( _copts.count("match") )
      result = lhs.match( rhs );
    else
      result = lhs.compare( rhs );

    // be terse when talking to machines
    if ( _gopts.terse )
    {
      out().info( str::numstring(result) );
      break;
    }

    // tell a human
    if (result == 0)
      out().info( str::form(_("%s matches %s"), lhs.asString().c_str(), rhs.asString().c_str() ) );
    else if ( result > 0 )
      out().info( str::form(_("%s is newer than %s"), lhs.asString().c_str(), rhs.asString().c_str() ) );
    else
      out().info( str::form(_("%s is older than %s"), lhs.asString().c_str(), rhs.asString().c_str() ) );

    break;
  }

  case ZypperCommand::LICENSES_e:
  {
    if ( !_arguments.empty() )
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
    // now load resolvables:
    load_resolvables( *this );
    // needed to compute status of PPP
    resolve( *this );

    report_licenses( *this );

    break;
  }


  case ZypperCommand::PS_e:
  {
    if ( !_arguments.empty() )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    shared_ptr<PsOptions> myOpts( assertCommandOptions<PsOptions>() );
    if ( _copts.count( "print" ) )
    {
      // implies -sss
      myOpts->_shortness = 3;
      myOpts->_format = _copts["print"].back();	// last wins
    }
    else if ( _copts.count( "short" ) )
    {
      myOpts->_shortness = _copts["short"].size();
    }
    else if ( _copts.count( "debugFile" ) )
    {
      myOpts->_debugFile = _copts["debugFile"].front();
    }

    ps( *this );
    break;
  }


  case ZypperCommand::DOWNLOAD_e:
  {
    if ( _arguments.empty() )
    {
      report_required_arg_missing( out(), _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    // Check for a usable pkg-cache-dir
    if ( geteuid() != 0 )
    {
      bool mayuse = userMayUseDir( _gopts.rm_options.repoPackagesCachePath );
      if ( ! mayuse && /* is the default path: */
	   _gopts.rm_options.repoPackagesCachePath == RepoManagerOptions( _gopts.root_dir ).repoPackagesCachePath )
      {
	_gopts.rm_options.repoPackagesCachePath = env::XDG_CACHE_HOME() / "zypp/packages";
	mayuse = userMayUseDir( _gopts.rm_options.repoPackagesCachePath );
      }

      if ( ! mayuse )
      {
	out().error( str::Format(_("Insufficient privileges to use download directory '%s'.")) % _gopts.rm_options.repoPackagesCachePath );
	setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
	return;
      }
    }

    // go
    init_target( *this );
    initRepoManager();
    init_repos( *this );
    if ( exitCode() != ZYPPER_EXIT_OK )
      return;
    // now load resolvables:
    load_resolvables( *this );

    shared_ptr<DownloadOptions> myOpts( assertCommandOptions<DownloadOptions>() );

    if ( _copts.count( "dry-run" ) )
      myOpts->_dryrun = true;

    download( *this );
    break;
  }


  case ZypperCommand::SOURCE_DOWNLOAD_e:
  {
    if ( !_arguments.empty() )
    {
      report_too_many_arguments( _command_help );
      setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      return;
    }

    shared_ptr<SourceDownloadOptions> myOpts( assertCommandOptions<SourceDownloadOptions>() );

    if ( _copts.count( "directory" ) )
      myOpts->_directory = _copts["directory"].back();	// last wins

    if ( _copts.count( "dry-run" ) )
      myOpts->_dryrun = true;

    sourceDownload( *this );

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

  case ZypperCommand::RUG_SERVICE_TYPES_e:
  {
    Table t;
    t << ( TableHeader() << _("Alias") << _("Name") << _("Description") );

    t << ( TableRow() << "yum" << "YUM" << "YUM server service" );	// rpm-md
    t << ( TableRow() << "yast" << "YaST2" << "YaST2 repository" );
    t << ( TableRow() << "zypp" << "ZYPP" << "ZYpp installation repository" );
    t << ( TableRow() << "mount" << "Mount" << "Mount a directory of RPMs" );
    t << ( TableRow() << "plaindir" << "Plaindir" << "Mount a directory of RPMs" );
    t << ( TableRow() << "nu" << "NU" << "Novell Updates service" );	// ris

    cout << t;

    break;
  }

  case ZypperCommand::RUG_LIST_RESOLVABLES_e:
  {
    rug_list_resolvables( *this );
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

void rug_list_resolvables( Zypper & zypper )
{
  Table t;
  t << ( TableHeader() << _("Resolvable Type") );

  t << ( TableRow() << "package" );
  t << ( TableRow() << "patch" );
  t << ( TableRow() << "pattern" );
  t << ( TableRow() << "product" );

  cout << t;
}


// Local Variables:
// c-basic-offset: 2
// End:
