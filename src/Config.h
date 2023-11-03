/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_CONFIG_H_
#define ZYPPER_CONFIG_H_

#include <string>
#include <set>

#include <zypp/Url.h>
#include <zypp/Pathname.h>
#include <zypp/RepoManager.h>

#include "Command.h"
#include "utils/colors.h"
#include "utils/flags/zyppflags.h"
#include "utils/ansi.h"
#include "output/Out.h"
#include <zypp-tui/Config>

/**
 *
 */
struct Config : public ztui::Config
{
  /** Initializes the config options to defaults. */
  Config();

  std::vector<zypp::ZyppFlags::CommandGroup> cliOptions ();

  /** Reads zypper.conf and stores the result */
  void read(const std::string & file = "");

  /** Which columns to show in repo list by default (string of short options).*/
  std::string repo_list_columns;

  bool solver_installRecommends;
  std::set<ZypperCommand> solver_forceResolutionCommands;

  bool psCheckAccessDeleted;	///< do post commit 'zypper ps' check?

  /** zypper.conf: color.useColors */
  std::string color_useColors;

  zypp::TriBool color_pkglistHighlight;	// true:all; indeterminate:first; false:no
  ansi::Color   color_pkglistHighlightAttribute;

  zypp::TriBool search_runSearchPackages;	// runSearchPackages after search: always/never/ask

  /** Hackisch way so save back a search_runSearchPackages value from search-packages-hinthack. */
  void saveback_search_runSearchPackages( const zypp::TriBool & value_r );

  /** zypper.conf: obs.baseUrl */
  zypp::Url obs_baseUrl;
  /** zypper.conf: obs.platform */
  std::string obs_platform;

  /** Whether subcommands found in $PATH should be considered */
  bool seach_subcommand_in_path = true;

  //====================================== CLI config options ========================================
  //  std::list<Url> additional_sources;
  Out::Verbosity verbosity;
  bool disable_system_sources;
  bool disable_system_resolvables;
  bool non_interactive;
  bool non_interactive_skip_manual_patches;
  bool reboot_req_non_interactive;
  bool no_gpg_checks;
  bool gpg_auto_import_keys;
  bool machine_readable;
  /** Whether to disable autorefresh. */
  bool no_refresh;
  /** Whether to ignore cd/dvd repos) */
  bool no_cd;
  /** Whether to ignore remote (http, ...) repos */
  bool no_remote;
  std::string root_dir;
  bool is_install_root; /// < used when the package target rootfs is not the same as the zypper metadata rootfs
  zypp::RepoManagerOptions rm_options;
  bool no_abbrev;
  bool terse;
  bool changedRoot;
  bool ignore_unknown;
  const int	exclude_optional_patches_default;	// global default
  int		exclude_optional_patches;		// effective value (--with[out]-optional)
  bool wantHelp; ///< help was requested by CLI


  // helper variables for CLI parsing
  bool _wantXMLOut = false;
  std::vector<std::string> plusRepoFromCLI;
  std::vector<std::string> plusContentFromCLI;

private:
  zypp::Pathname _cfgSaveFile;	///< the default config file used for saving back values (--config or in $HOME)
};

#endif /* ZYPPER_CONFIG_H_ */
