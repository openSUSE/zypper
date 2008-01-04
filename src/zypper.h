#ifndef ZYPPER_H
#define ZYPPER_H

#include <string>
#include <vector>

#include "zypp/base/Exception.h"
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/ResStore.h"
#include "zypp/RepoInfo.h"
#include "zypp/RepoManager.h"

#include "zypper-command.h"
#include "zypper-getopt.h"

/**
 * Structure for holding global options.
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
  no_gpg_checks(false),
  machine_readable(false),
  root_dir("/")
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
  bool no_gpg_checks;
  bool machine_readable;
  std::string root_dir;
  zypp::RepoManagerOptions rm_options;
};


struct CommandOptions
{
  CommandOptions()
    :
  license_auto_agree(false)
  {}

  bool license_auto_agree;
};


DEFINE_PTR_TYPE(Zypper);

class Zypper : public zypp::base::ReferenceCounted, private zypp::base::NonCopyable
{
public:
  static Zypper_Ptr instance(); 

  int main(int argc, char ** argv);

  // setters & getters
  const GlobalOptions & globalOpts() const { return _gopts; }
  const CommandOptions & cmdOpts() const { return _cmdopts; }
  const parsed_opts & cOpts() const { return _copts; }
  const ZypperCommand & command() const { return _command; }
  const std::string & commandHelp() const { return _command_help; }
  const std::vector<std::string> & arguments() const { return _arguments; }
  int exitCode() const { return _exit_code; }
  void setExitCode(int exit) { _exit_code = exit; } 
  bool runningShell() const { return _running_shell; }
  bool runningHelp() const { return _running_help; }

  int argc() { return _running_shell ? _sh_argc : _argc; } 
  char ** argv() { return _running_shell ? _sh_argv : _argv; }
  
private:
  Zypper();
  ~Zypper();

  void processGlobalOptions();
  void processCommandOptions();
  void commandShell();
  void shellCleanup();
  void safeDoCommand();
  void doCommand();
  void cleanup();

  void setCommand(const ZypperCommand & command) { _command = command; }
  void setRunningShell(bool value = true) { _running_shell = value; }
  void setRunningHelp(bool value = true) { _running_help = value; }

private:

  int     _argc;
  char ** _argv;

  GlobalOptions _gopts;
  CommandOptions _cmdopts;
  parsed_opts   _copts;
  ZypperCommand _command;
  std::vector<std::string> _arguments;
  std::string _command_help;

  int   _exit_code;
  bool  _running_shell;
  bool  _running_help;

  int _sh_argc;
  char **_sh_argv;
};

void print_main_help(const Zypper & zypper);
void print_unknown_command_hint(const Zypper & zypper);
void print_command_help_hint(const Zypper & zypper);

struct RuntimeData
{
  RuntimeData()
    : patches_count(0), security_patches_count(0), show_media_progress_hack(false)
  {}

  std::list<zypp::RepoInfo> repos;
  std::list<zypp::RepoInfo> additional_repos;
  int patches_count;
  int security_patches_count;
  zypp::ResStore repo_resolvables;
  zypp::ResStore target_resolvables;
  zypp::RepoInfo current_repo;

  // hack to enable media progress reporting in the commit phase in normal
  // output level
  bool show_media_progress_hack;
};

extern RuntimeData gData;
extern std::ostream no_stream;

class ExitRequestException : public zypp::Exception
{
public:
  ExitRequestException(const std::string & msg = "") : zypp::Exception(msg) {}
//  ExitRequestException(const std::string & msg = "") : _msg(msg) {}

//  const std::string & msg() const { return _msg; }
private:
//  std::string _msg;
};

#endif /*ZYPPER_H*/

// Local Variables:
// c-basic-offset: 2
// End:
