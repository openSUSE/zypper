#ifndef ZYPPER_H
#define ZYPPER_H

#include "zypp/ResStore.h"
#include "zypp/RepoInfo.h"
#include "zypp/RepoManager.h"

#include "zypper-command.h"

/**
 * Structure for holding various start-up setting.
 */
struct Settings
{
  Settings()
  :
  verbosity(0),  
  disable_system_sources(false),
  disable_system_resolvables(false),
  is_rug_compatible(false),
  non_interactive(false),
  no_gpg_checks(false),
  license_auto_agree(false),
  machine_readable(false),
  root_dir("/"),
  in_shell(false)
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
  bool in_shell;
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
extern bool ghelp;

void command_shell ();
int safe_one_command(int argc, char **argv);
int process_globals(int argc, char **argv);

#endif /*ZYPPER_H*/

// Local Variables:
// c-basic-offset: 2
// End:
