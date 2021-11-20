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
#include "output/Out.h"

/**
 *
 */
struct Config
{
  /** Initializes the config options to defaults. */
  Config();

  std::vector<ZyppFlags::CommandGroup> cliOptions ();

  /** Reads zypper.conf and stores the result */
  void read(const std::string & file = "");

  /** Which columns to show in repo list by default (string of short options).*/
  std::string repo_list_columns;

  bool solver_installRecommends;
  std::set<ZypperCommand> solver_forceResolutionCommands;

  bool psCheckAccessDeleted;	///< do post commit 'zypper ps' check?

  /**
   * True unless output is a dumb tty or file. In this case we should not use
   * any ANSI Escape sequences (at least those moving the cursor; color may
   * be explicitly enabled 'zypper --color ..| less'
   */
  bool do_ttyout;

  /**
   * Whether to colorize the output. This is evaluated according to
   * color_useColors and do_ttyout.
   */
  bool do_colors;

  /** zypper.conf: color.useColors */
  std::string color_useColors;

  ansi::Color color_result;
  ansi::Color color_msgStatus;
  ansi::Color color_msgError;
  ansi::Color color_msgWarning;
  ansi::Color color_prompt;
  ansi::Color color_promptOption;
  ansi::Color color_positive;
  ansi::Color color_change;
  ansi::Color color_negative;
  ansi::Color color_highlight;
  ansi::Color color_lowlight;
  ansi::Color color_osdebug;

  TriBool color_pkglistHighlight;	// true:all; indeterminate:first; false:no
  ansi::Color   color_pkglistHighlightAttribute;

  TriBool search_runSearchPackages;	// runSearchPackages after search: always/never/ask

  /** Hackisch way so save back a search_runSearchPackages value from search-packages-hinthack. */
  void saveback_search_runSearchPackages( const TriBool & value_r );

  /** zypper.conf: obs.baseUrl */
  Url obs_baseUrl;
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
  bool reboot_req_non_interactive;
  bool no_gpg_checks;
  std::vector<std::string> gpg_auto_import_keys;
  bool machine_readable;
  /** Whether to disable autorefresh. */
  bool no_refresh;
  /** Whether to ignore cd/dvd repos) */
  bool no_cd;
  /** Whether to ignore remote (http, ...) repos */
  bool no_remote;
  std::string root_dir;
  bool is_install_root; /// < used when the package target rootfs is not the same as the zypper metadata rootfs
  RepoManagerOptions rm_options;
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
  Pathname _cfgSaveFile;	///< the default config file used for saving back values (--config or in $HOME)
};

#endif /* ZYPPER_CONFIG_H_ */
