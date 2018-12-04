/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "help.h"
#include "utils/messages.h"
#include "commands/commandhelpformatter.h"

HelpCmd::HelpCmd ( const std::vector<std::string> &commandAliases_r ) :
  ZypperBaseCommand (
    commandAliases_r,
    "help",
    _("Print zypper help"),
    _("Print zypper help"),
    DisableAll
  )
{ }


zypp::ZyppFlags::CommandGroup HelpCmd::cmdOptions() const
{
  return {};
}

void HelpCmd::doReset()
{ }

void HelpCmd::printMainHelp( Zypper & zypper )
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

int HelpCmd::execute( Zypper &zypper, const std::vector<std::string> & )
{
  printMainHelp ( zypper );
  return ZYPPER_EXIT_OK;
}
