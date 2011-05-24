/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_H
#define ZYPPER_H

#include <string>
#include <vector>

#include "zypp/base/Exception.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/TriBool.h"

#include "zypp/RepoInfo.h"
#include "zypp/RepoManager.h" // for RepoManagerOptions
#include "zypp/SrcPackage.h"
#include "zypp/TmpPath.h"

#include "Config.h"
#include "Command.h"
#include "utils/getopt.h"
#include "output/Out.h"

/** directory for storing manually installed (zypper install foo.rpm) RPM files
 */
#define ZYPPER_RPM_CACHE_DIR "/var/cache/zypper/RPMS"

/**
 * Structure for holding global options.
 *
 * \deprecated To be replaced by Config
 */
struct GlobalOptions
{
  GlobalOptions()
  :
  verbosity(0),
  disable_system_sources(false),
  disable_system_resolvables(false),
  is_rug_compatible(false),
  non_interactive(false),
  reboot_req_non_interactive(false),
  no_gpg_checks(false),
  gpg_auto_import_keys(false),
  machine_readable(false),
  no_refresh(false),
  no_cd(false),
  no_remote(false),
  root_dir("/"),
  no_abbrev(false),
  terse(false),
  changedRoot(false)
  {}

//  std::list<zypp::Url> additional_sources;

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
  bool disable_system_sources;
  bool disable_system_resolvables;
  bool is_rug_compatible;
  bool non_interactive;
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
  zypp::RepoManagerOptions rm_options;
  bool no_abbrev;
  bool terse;
  bool changedRoot;
};

/**
 * \bug The RepoInfo lists kept herein may lack housekeeping data added by the
 * zypp::RepoManager. Consider using your own RepoInfos only for those not
 * maintained by zypp::RepoManager. (bnc #544432)
*/
struct RuntimeData
{
  RuntimeData()
    : patches_count(0), security_patches_count(0)
    , show_media_progress_hack(false)
    , force_resolution(zypp::indeterminate)
    , solve_before_commit(true)
    , commit_pkgs_total(0)
    , commit_pkg_current(0)
    , seen_verify_hint(false)
    , action_rpm_download(false)
    , waiting_for_input(false)
  {}

  std::list<zypp::RepoInfo> repos;
  std::list<zypp::RepoInfo> additional_repos;
  int patches_count;
  int security_patches_count;
  /**
   * Used by requestMedia callback
   * \todo but now it uses label, remove this variable?
   */
  zypp::RepoInfo current_repo;

  std::list<zypp::SrcPackage::constPtr> srcpkgs_to_install;

  // hack to enable media progress reporting in the commit phase in normal
  // output level
  bool show_media_progress_hack;

  // Indicates an ongoing raw meta-data refresh.
  // If not empty call zypper.out().progress(
  //  "raw-refresh", raw_refresh_progress_label) in the media download
  // progress callback.
  //! \todo better way to do this would be to propagate the download progress
  // all the way up from the media back-end through fetcher and downloader
  // into the RepoManager::refreshMetadata(), so that we get a combined percentage
  std::string raw_refresh_progress_label;

  /** Used to override the command line option */
  zypp::TriBool force_resolution;

  /**
   * Set to <tt>false</tt> to avoid calling of the solver
   * in \ref solve_and_commit(). Needed after Resolver::doUpdate()
   */
  bool solve_before_commit;

  unsigned int commit_pkgs_total;
  unsigned int commit_pkg_current;

  bool seen_verify_hint;
  bool action_rpm_download;

  //! \todo move this to a separate Status struct
  bool waiting_for_input;

  //! Temporary directory for any use. Used e.g. as packagesPath of TMP_RPM_REPO_ALIAS repository.
  zypp::filesystem::TmpDir tmpdir;
};

typedef zypp::shared_ptr<zypp::RepoManager> RepoManager_Ptr;

class Zypper : private zypp::base::NonCopyable
{
public:
  typedef zypp::RW_pointer<Zypper,zypp::rw_pointer::Scoped<Zypper> > Ptr;
  typedef std::vector<std::string>  ArgList;

  static Ptr & instance();

  int main(int argc, char ** argv);

  // setters & getters
  Out & out();
  void setOutputWriter(Out * out) { _out_ptr = out; }
  Config & config() { return _config; }
  const GlobalOptions & globalOpts() const { return _gopts; }
  GlobalOptions & globalOptsNoConst() { return _gopts; }
  const parsed_opts & cOpts() const { return _copts; }
  const ZypperCommand & command() const { return _command; }
  const std::string & commandHelp() const { return _command_help; }
  const ArgList & arguments() const { return _arguments; }
  RuntimeData & runtimeData() { return _rdata; }

  zypp::RepoManager & repoManager()
  { if (!_rm) _rm.reset(new zypp::RepoManager(_gopts.rm_options)); return *_rm; }

  void initRepoManager()
  { _rm.reset(new zypp::RepoManager(_gopts.rm_options)); }

  int exitCode() const { return _exit_code; }
  void setExitCode(int exit) { _exit_code = exit; }
  bool runningShell() const { return _running_shell; }
  bool runningHelp() const { return _running_help; }
  bool exitRequested() const { return _exit_requested; }
  void requestExit(bool do_exit = true) { _exit_requested = do_exit; }

  int argc() { return _running_shell ? _sh_argc : _argc; }
  char ** argv() { return _running_shell ? _sh_argv : _argv; }

  void cleanup();

public:
  ~Zypper();
private:
  Zypper();

  void processGlobalOptions();
  void processCommandOptions();
  void commandShell();
  void shellCleanup();
  void safeDoCommand();
  void doCommand();

  void setCommand(const ZypperCommand & command) { _command = command; }
  void setRunningShell(bool value = true) { _running_shell = value; }
  void setRunningHelp(bool value = true) { _running_help = value; }

private:

  int     _argc;
  char ** _argv;

  Out * _out_ptr;
  Config _config;
  GlobalOptions _gopts;
  parsed_opts   _copts;
  ZypperCommand _command;
  ArgList _arguments;
  std::string _command_help;

  int   _exit_code;
  bool  _running_shell;
  bool  _running_help;
  bool  _exit_requested;

  RuntimeData _rdata;

  RepoManager_Ptr   _rm;

  int _sh_argc;
  char **_sh_argv;
};

void print_main_help(const Zypper & zypper);
void print_unknown_command_hint(Zypper & zypper);
void print_command_help_hint(Zypper & zypper);


class ExitRequestException : public zypp::Exception
{
public:
  ExitRequestException(const std::string & msg = "") : zypp::Exception(msg) {}
};

#endif /*ZYPPER_H*/

// Local Variables:
// c-basic-offset: 2
// End:
