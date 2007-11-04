#ifndef ZYPPER_H
#define ZYPPER_H

#include <libintl.h>

#include <zypp/base/Logger.h>
#include <zypp/ResStore.h>
#include <zypp/RepoInfo.h>
#include <zypp/RepoManager.h>

#include "zypper-command.h"

// ===== exit codes ======

#define ZYPPER_EXIT_OK                     0
// errors
#define ZYPPER_EXIT_ERR_BUG                1 // undetermined error
#define ZYPPER_EXIT_ERR_SYNTAX             2 // syntax error, e.g. zypper instal, zypper in --unknown option
#define ZYPPER_EXIT_ERR_INVALID_ARGS       3 // invalid arguments given, e.g. zypper source-add httttps://asdf.net 
#define ZYPPER_EXIT_ERR_ZYPP               4 // error indicated from within libzypp, e.g. God = zypp::getZYpp() threw an exception
#define ZYPPER_EXIT_ERR_PRIVILEGES         5 // unsufficient privileges for the operation
#define ZYPPER_EXIT_NO_REPOS               6 // no repositories defined

// info
#define ZYPPER_EXIT_INF_UPDATE_NEEDED      100 // update needed
#define ZYPPER_EXIT_INF_SEC_UPDATE_NEEDED  101 // security update needed
#define ZYPPER_EXIT_INF_REBOOT_NEEDED      102 // reboot needed after install/upgrade 
#define ZYPPER_EXIT_INF_RESTART_NEEDED     103 // restart of package manager itself needed

/**
 * Structure for holding various start-up setting.
 */
struct Settings
{
  Settings()
  :
  verbosity(0),  
  previous_code(-1),
  disable_system_sources(false),
  disable_system_resolvables(false),
  is_rug_compatible(false),
  non_interactive(false),
  no_gpg_checks(false),
  license_auto_agree(false),
  machine_readable(false),
  root_dir("/")
  {}

  std::list<zypp::Url> additional_sources;

  /**
   * Level of the amount of output.
   *
   * <ul> 
   * <li>-1 quiet</li>
   * <li> 0 normal (default)</li>
   * <li> 1 verbose</li>
   * <li> 2 debug</li>
   * </ul>
   */
  int verbosity;
  int previous_code;
  std::string command;
  bool disable_system_sources;
  bool disable_system_resolvables;
  bool is_rug_compatible;
  bool non_interactive;
  bool no_gpg_checks;
  bool license_auto_agree;
  bool machine_readable;
  std::string root_dir;
  zypp::RepoManagerOptions rm_options;
};

struct RuntimeData
{
  RuntimeData()
    : patches_count(0), security_patches_count(0), show_media_progress_hack(false)
  {}

  std::list<zypp::RepoInfo> repos;
  int patches_count;
  int security_patches_count;
  std::vector<std::string> packages_to_install;
  std::vector<std::string> packages_to_uninstall; 
  zypp::ResStore repo_resolvables;
  zypp::ResStore target_resolvables;
  zypp::RepoInfo current_repo;

  // hack to enable media progress reporting in the commit phase in normal
  // output level
  bool show_media_progress_hack;
};

extern RuntimeData gData;
extern Settings gSettings;
extern std::ostream no_stream;
extern ZypperCommand command;

#define VERBOSITY_NORMAL 0
#define VERBOSITY_MEDIUM 1
#define VERBOSITY_HIGH 2

/**
 * Macro to filter output above the current verbosity level.
 *
 * \see Output Macros
 * \see Settings::verbosity
 */
#define COND_STREAM(STREAM,LEVEL) ((gSettings.verbosity >= LEVEL) ? STREAM : no_stream)

/** \name Output Macros
 * Alway use these macros to produce output so that the verbosity options
 * like -v or --quiet are respected. Use standard cout and cerr only in
 * cases where it is desirable to ignore them (e.g. help texts (when -h is
 * used) or brief error messages must always be displayed, even if --quiet
 * has been specified).
 */
//!@{
//! normal output
#define cout_n COND_STREAM(cout, VERBOSITY_NORMAL)
//! verbose output
#define cout_v COND_STREAM(cout, VERBOSITY_MEDIUM)
//! verbose error output
#define cerr_v COND_STREAM(cerr, VERBOSITY_MEDIUM)
//! debug info output
#define cout_vv COND_STREAM(cout, VERBOSITY_HIGH)
//! debug error output (details)
#define cerr_vv COND_STREAM(cerr, VERBOSITY_HIGH)
//!@}

// undefine _ and _PL macros from libzypp
#ifdef _
#undef _
#endif
#ifdef _PL
#undef _PL
#endif

// define new macros
#define _(MSG) ::gettext(MSG)
#define _PL(MSG1,MSG2,N) ::ngettext(MSG1,MSG2,N)

#endif /*ZYPPER_H*/

// Local Variables:
// c-basic-offset: 2
// End:
