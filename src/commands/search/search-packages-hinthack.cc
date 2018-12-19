/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <zypp/base/LogTools.h>
#include <zypp/target/rpm/librpmDb.h>
#include <zypp/ui/Selectable.h>
#include <zypp/ResPool.h>

#include "search-packages-hinthack.h"

#include "Zypper.h"
#include "commands/subcommand.h"

using namespace zypp;

#undef	ENABLE_COULD_BE_INSTALLED_HINT // let's see if we need it
#undef	ENABLE_DISABLED_IN_CONFIG_HINT // let's see if we need it

///////////////////////////////////////////////////////////////////
namespace searchPackagesHintHack
{
  ///////////////////////////////////////////////////////////////////
  namespace
  {
    static const std::string ZSPP_PackageName { "zypper-search-packages-plugin" };
    static const Pathname    ZSPP_BinaryPath  { "/usr/lib/zypper/commands/zypper-search-packages" };
    static const Capability  ZSPP_CliFwdCap   { "supports-zypper-cli-forwarding" };
    /** Whether hinting/running search-packages should be considered at all. */
    bool maySearchPackagesAtAll( Zypper & zypper_r )
    {
      if ( ! zypper_r.config().do_ttyout )
	return false;	// no terminal output, i.e. no user in front

      Out & out( zypper_r.out() );
      if ( ! out.typeNORMAL() || out.verbosity() < Out::NORMAL )
	return false;	// xml output or quiet mode

      if ( zypper_r.runningShell() )
	return false;	// we don't support subcommands in shell

      if ( zypper_r.config().non_interactive )
	return false;	// Never forward in non-interactive mode (search-packages itself relies on this!)

      if ( zypper_r.config().changedRoot )
	return false;	// we're chrooted

      return true;
    }

    /** Whether an installed search-packages subcommand provides \c supports-zypper-cli-forwarding.
     * FATE#325599: Introduced with SLE15-SP1 and forces us to even call the plugin.
     */
    bool searchPackagesSupportsZypperCliForwarding()
    {
      using target::rpm::librpmDb;
      // RpmDb access is blocked while the Target is not initialized.
      // Launching the Target just for this query would be an overkill.
      struct TmpUnblock {
	TmpUnblock()
	: _wasBlocked( librpmDb::isBlocked() )
	{ if ( _wasBlocked ) librpmDb::unblockAccess(); }
	~TmpUnblock()
	{ if ( _wasBlocked ) librpmDb::blockAccess(); }
      private:
	bool _wasBlocked;
      } tmpUnblock;

      librpmDb::db_const_iterator it;
      return( it.findPackage( ZSPP_PackageName ) && it->tag_provides().count( ZSPP_CliFwdCap ) );
    }

    /** Print a hint about an available search-packages subcommand. */
    void printSearchPackagesHint( Zypper & zypper_r, bool plgInstalled_r )
    {
      str::Str msg;
      // translator: %1% denotes a zypper command to execute. Like 'zypper search-packages'.
      msg << str::Format(_("For an extended search including not yet activated remote resources please use '%1%'.")) % "zypper search-packages";

      if ( ! plgInstalled_r )
	// translator: %1% denotes a zypper command to execute. Like 'zypper search-packages'.
	msg << ' ' << str::Format(_("The package providing this subcommand is currently not installed. You can install it by calling '%1%'.")) % "zypper in zypper-search-packages-plugin";

      zypper_r.out().notePar( 4, msg );
    }

    /** We could run search-packages, but shall we?  */
    bool userDecissionToCallSearchPackages( Zypper & zypper_r )
    {
      tribool callSP = zypper_r.config().search_runSearchPackages;

      zypper_r.out().gap();
      if ( indeterminate(callSP) )
      {
	HIGHLIGHTString tag { "Undecided in zypper.conf:" };
	std::string hint { "[search] runSearchPackages = ask" };
	zypper_r.out().infoLRHint( text::join( tag, str::Format(_("For an extended search including not yet activated remote resources you may now run '%1%'.")) % "zypper search-packages" ),
				   hint );
	std::string question { str::Format(_("Do you want to run '%1%' now?")) % "zypper search-packages" };
        callSP = read_bool_answer( PROMPT_YN_GPG_UNSIGNED_FILE_ACCEPT, question, false );
      }
#ifdef ENABLE_DISABLED_IN_CONFIG_HINT
      else
#else
      else if ( callSP )
#endif
      {
	HIGHLIGHTString tag { callSP ? "Enabled in zypper.conf:" : "Disabled in zypper.conf:" };
	std::string hint { callSP ? "[search] runSearchPackages = always" : "[search] runSearchPackages = never" };
	zypper_r.out().infoLRHint( text::join( tag, str::Format(_("Run '%1%' to search in not yet activated remote resources.")) % "zypper search-packages" ),
				   hint );
      }
      zypper_r.out().gap();

      return( callSP == true );
    }

    /** Forward the search request to search-packages subcommand. */
    void callSearchPackages( Zypper & zypper_r )
    {
      shared_ptr<SubcommandOptions> plgOptions { new SubcommandOptions };

      plgOptions->_detected._cmd = "search-packages";
      plgOptions->_detected._name = ZSPP_BinaryPath.basename();
      plgOptions->_detected._path = ZSPP_BinaryPath.dirname();
      // Slightly adjust the commandline and forward it to the subcommand
      {
	SubcommandOptions::Arglist args { zypper_r.argv(), zypper_r.argv()+zypper_r.argc() };
	args[0] = "search-packages";
	// indicate it's called from zypper; replacing the command name it also separates global and command opts
	args[argvCmdIdx] = "--no-query-local";
	// explicitly insert "--" in case the plugin does not know whether the last option takes an argument
	args.insert( args.begin()+argvArgIdx, "--" );

	plgOptions->args( std::move(args) );
      }

      SubCmd cmd ( { plgOptions->_detected._cmd }, plgOptions );
      cmd.runCmd( zypper_r );
    }


  } // namespace
  ///////////////////////////////////////////////////////////////////

  // NOTE: Dumb approach remembering the argv indices while (old style) parsing.
  // Does not work with new style parser nor in shell mode (Zypper _sh_argv vs. _argv)
  int argvCmdIdx = 0;
  int argvArgIdx = 0;

  void callOrNotify( Zypper & zypper_r )
  {
    if ( ! maySearchPackagesAtAll( zypper_r ) )
      return;

    // NOTE: We can't rely on the ResPool content (might be empty in help, or
    // incomplete due to --installed-only, --not-installed-only, --repo, etc.
    // Authority for the subcommand being installed is the binary!
    bool plgInstalled = PathInfo( ZSPP_BinaryPath ).isFile();
    if ( ! plgInstalled )
    {
#ifdef ENABLE_COULD_BE_INSTALLED_HINT
      // Be quiet unless we happen to know that the subcommand could be installed and hint.
      ui::Selectable::Ptr plg { ui::Selectable::get( ResKind::package, ZSPP_PackageName ) };
      if ( ! plg || plg->availableEmpty() )
	return;
#else
      return;
#endif
    }

    if ( zypper_r.runningHelp() || ! plgInstalled || ! searchPackagesSupportsZypperCliForwarding() )
    {
      printSearchPackagesHint( zypper_r, plgInstalled );
      return;
    }

    // Here we're in 'search' AND could execute the subcommand(supports-zypper-cli-forwarding")...
    if ( userDecissionToCallSearchPackages( zypper_r ) )
      callSearchPackages( zypper_r );
  }
} // searchPackagesHintHack
///////////////////////////////////////////////////////////////////

