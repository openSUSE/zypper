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

#include <boost/logic/tribool.hpp>
#include <boost/format.hpp>

#include "zypp/ZYppFactory.h"
#include "zypp/zypp_detail/ZYppReadOnlyHack.h"

#include "zypp/base/Logger.h"
#include "zypp/base/Algorithm.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/sat/SolvAttr.h"
#include "zypp/PoolQuery.h"
#include "zypp/Locks.h"

#include "zypp/target/rpm/RpmHeader.h" // for install <.rpmURI>

#include "main.h"
#include "Zypper.h"
#include "Command.h"
#include "SolverRequester.h"

#include "Table.h"
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

#include "output/OutNormal.h"
#include "output/OutXML.h"

using namespace std;
using namespace zypp;
using namespace boost;

ZYpp::Ptr God = NULL;
parsed_opts copts; // command options

static void rug_list_resolvables(Zypper & zypper);

Zypper::Zypper()
  : _argc(0), _argv(NULL), _out_ptr(NULL),
    _command(ZypperCommand::NONE),
    _exit_code(ZYPPER_EXIT_OK),
    _running_shell(false), _running_help(false), _exit_requested(false),
    _sh_argc(0), _sh_argv(NULL)
{
  MIL << "Zypper instance created." << endl;
}


Zypper::~Zypper()
{
  delete _out_ptr;
  MIL << "Zypper instance destroyed. Bye!" << endl;
}


Zypper::Ptr & Zypper::instance()
{
  static Zypper::Ptr _instance;

  if (!_instance)
    _instance.reset(new Zypper());
  else
    XXX << "Got an existing instance." << endl;

  return _instance;
}


int Zypper::main(int argc, char ** argv)
{
  _argc = argc;
  _argv = argv;

  // parse global options and the command
  try {
    processGlobalOptions();
  }
  catch (const ExitRequestException & e)
  {
    MIL << "Caught exit request:" << endl << e.msg() << endl;
    return exitCode();
  }

  if (runningHelp())
  {
    safeDoCommand();
    return exitCode();
  }

  switch(command().toEnum())
  {
  case ZypperCommand::SHELL_e:
    commandShell();
    cleanup();
    return exitCode();

  case ZypperCommand::NONE_e:
    return ZYPPER_EXIT_ERR_SYNTAX;

  default:
    safeDoCommand();
    cleanup();
    return exitCode();
  }

  WAR << "This line should never be reached." << endl;

  return exitCode();
}

Out & Zypper::out()
{
  if (_out_ptr)
    return *_out_ptr;

  cerr << "uninitialized output writer" << endl;
  ZYPP_THROW(ExitRequestException("no output writer"));
}


void print_main_help(Zypper & zypper)
{
  static string help_global_options = _("  Global Options:\n"
    "\t--help, -h\t\tHelp.\n"
    "\t--version, -V\t\tOutput the version number.\n"
    "\t--promptids\t\tOutput a list of zypper's user prompts.\n"
    "\t--config, -c <file>\tUse specified config file instead of the default.\n"
    "\t--quiet, -q\t\tSuppress normal output, print only error\n"
    "\t\t\t\tmessages.\n"
    "\t--verbose, -v\t\tIncrease verbosity.\n"
    "\t--no-abbrev, -A\t\tDo not abbreviate text in tables.\n"
    "\t--table-style, -s\tTable style (integer).\n"
    "\t--rug-compatible, -r\tTurn on rug compatibility.\n"
    "\t--non-interactive, -n\tDo not ask anything, use default answers\n"
    "\t\t\t\tautomatically.\n"
    "\t--non-interactive-include-reboot-patches\n"
    "\t\t\t\tDo not treat patches as interactive, which have\n"
    "\t\t\t\tthe rebootSuggested-flag set.\n"
    "\t--xmlout, -x\t\tSwitch to XML output.\n"
  );

  static string repo_manager_options = _(
    "\t--reposd-dir, -D <dir>\tUse alternative repository definition file\n"
    "\t\t\t\tdirectory.\n"
    "\t--cache-dir, -C <dir>\tUse alternative directory for all caches.\n"
    "\t--raw-cache-dir <dir>\tUse alternative raw meta-data cache directory.\n"
    "\t--solv-cache-dir <dir>\tUse alternative solv file cache directory.\n"
    "\t--pkg-cache-dir <dir>\tUse alternative package cache directory.\n"
  );

  static string help_global_repo_options = _("     Repository Options:\n"
    "\t--no-gpg-checks\t\tIgnore GPG check failures and continue.\n"
    "\t--gpg-auto-import-keys\tAutomatically trust and import new repository\n"
      "\t\t\t\tsigning keys.\n"
    "\t--plus-repo, -p <URI>\tUse an additional repository.\n"
    "\t--disable-repositories\tDo not read meta-data from repositories.\n"
    "\t--no-refresh\t\tDo not refresh the repositories.\n"
    "\t--no-cd\t\t\tIgnore CD/DVD repositories.\n"
    "\t--no-remote\t\tIgnore remote repositories.\n"
  );

  static string help_global_target_options = _("     Target Options:\n"
    "\t--root, -R <dir>\tOperate on a different root directory.\n"
    "\t--disable-system-resolvables\n"
    "\t\t\t\tDo not read installed packages.\n"
  );

  static string help_commands = _(
    "  Commands:\n"
    "\thelp, ?\t\t\tPrint help.\n"
    "\tshell, sh\t\tAccept multiple commands at once.\n"
  );

  static string help_repo_commands = _("     Repository Management:\n"
    "\trepos, lr\t\tList all defined repositories.\n"
    "\taddrepo, ar\t\tAdd a new repository.\n"
    "\tremoverepo, rr\t\tRemove specified repository.\n"
    "\trenamerepo, nr\t\tRename specified repository.\n"
    "\tmodifyrepo, mr\t\tModify specified repository.\n"
    "\trefresh, ref\t\tRefresh all repositories.\n"
    "\tclean\t\t\tClean local caches.\n"
  );

  static string help_service_commands = _("     Service Management:\n"
    "\tservices, ls\t\tList all defined services.\n"
    "\taddservice, as\t\tAdd a new service.\n"
    "\tmodifyservice, ms\tModify specified service.\n"
    "\tremoveservice, rs\tRemove specified service.\n"
    "\trefresh-services, refs\tRefresh all services.\n"
  );

  static string help_package_commands = _("     Software Management:\n"
    "\tinstall, in\t\tInstall packages.\n"
    "\tremove, rm\t\tRemove packages.\n"
    "\tverify, ve\t\tVerify integrity of package dependencies.\n"
    "\tsource-install, si\tInstall source packages and their build\n"
    "\t\t\t\tdependencies.\n"
    "\tinstall-new-recommends, inr\n"
    "\t\t\t\tInstall newly added packages recommended\n"
    "\t\t\t\tby installed packages.\n"
  );

  static string help_update_commands = _("     Update Management:\n"
    "\tupdate, up\t\tUpdate installed packages with newer versions.\n"
    "\tlist-updates, lu\tList available updates.\n"
    "\tpatch\t\t\tInstall needed patches.\n"
    "\tlist-patches, lp\tList needed patches.\n"
    "\tdist-upgrade, dup\tPerform a distribution upgrade.\n"
    "\tpatch-check, pchk\tCheck for patches.\n"
  );

  static string help_query_commands = _("     Querying:\n"
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

  static string help_lock_commands = _("     Package Locks:\n"
    "\taddlock, al\t\tAdd a package lock.\n"
    "\tremovelock, rl\t\tRemove a package lock.\n"
    "\tlocks, ll\t\tList current package locks.\n"
    "\tcleanlocks, cl\t\tRemove unused locks.\n"
  );

  static string help_other_commands = _("     Other Commands:\n"
    "\tversioncmp, vcmp\tCompare two version strings.\n"
    "\ttargetos, tos\t\tPrint the target operating system ID string.\n"
    "\tlicenses\t\tPrint report about licenses and EULAs of\n"
    "\t\t\t\tinstalled packages.\n"
  );

  static string help_usage = _(
    "  Usage:\n"
    "\tzypper [--global-options] <command> [--command-options] [arguments]\n"
  );

  zypper.out().info(help_usage, Out::QUIET);
  zypper.out().info(help_global_options, Out::QUIET);
  zypper.out().info(repo_manager_options, Out::QUIET);
  zypper.out().info(help_global_repo_options, Out::QUIET);
  zypper.out().info(help_global_target_options, Out::QUIET);
  zypper.out().info(help_commands, Out::QUIET);
  zypper.out().info(help_repo_commands, Out::QUIET);
  zypper.out().info(help_service_commands, Out::QUIET);
  zypper.out().info(help_package_commands, Out::QUIET);
  zypper.out().info(help_update_commands, Out::QUIET);
  zypper.out().info(help_query_commands, Out::QUIET);
  zypper.out().info(help_lock_commands, Out::QUIET);
  zypper.out().info(help_other_commands, Out::QUIET);

  print_command_help_hint(zypper);
}

void print_unknown_command_hint(Zypper & zypper)
{
  zypper.out().info(boost::str(format(
    // translators: %s is "help" or "zypper help" depending on whether
    // zypper shell is running or not
    _("Type '%s' to get a list of global options and commands."))
      % (zypper.runningShell() ? "help" : "zypper help")));
}

void print_command_help_hint(Zypper & zypper)
{
  zypper.out().info(boost::str(format(
    // translators: %s is "help" or "zypper help" depending on whether
    // zypper shell is running or not
    _("Type '%s' to get command-specific help."))
      % (zypper.runningShell() ? "help <command>" : "zypper help <command>")));
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

  static struct option global_options[] = {
    {"help",                       no_argument,       0, 'h'},
    {"verbose",                    no_argument,       0, 'v'},
    {"quiet",                      no_argument,       0, 'q'},
    {"version",                    no_argument,       0, 'V'},
    {"promptids",                  no_argument,       0,  0 },
    // rug compatibility alias for -vv
    {"debug",                      no_argument,       0,  0 },
    // rug compatibility alias for the default output level => ignored
    {"normal-output",              no_argument,       0,  0 },
    // not implemented currently => ignored
    {"terse",                      no_argument,       0, 't'},
    {"no-abbrev",                  no_argument,       0, 'A'},
    {"table-style",                required_argument, 0, 's'},
    {"rug-compatible",             no_argument,       0, 'r'},
    {"non-interactive",            no_argument,       0, 'n'},
    {"non-interactive-include-reboot-patches", no_argument, 0, '0'},
    {"no-gpg-checks",              no_argument,       0,  0 },
    {"gpg-auto-import-keys",       no_argument,       0,  0 },
    {"root",                       required_argument, 0, 'R'},
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
    {"disable-repositories",       no_argument,       0,  0 },
    {"no-refresh",                 no_argument,       0,  0 },
    {"no-cd",                      no_argument,       0,  0 },
    {"no-remote",                  no_argument,       0,  0 },
    {"xmlout",                     no_argument,       0, 'x'},
    {"config",                     required_argument, 0, 'c'},
    {0, 0, 0, 0}
  };

  // parse global options
  parsed_opts gopts = parse_options (_argc, _argv, global_options);
  if (gopts.count("_unknown") || gopts.count("_missing_arg"))
  {
    setExitCode(ZYPPER_EXIT_ERR_SYNTAX);
    return;
  }

  parsed_opts::const_iterator it;

  // read config from specified file or default config files
  _config.read(
      (it = gopts.find("config")) != gopts.end() ? it->second.front() : "");

  // ====== output setup ======
  // depends on global options, that's we set it up here
  //! \todo create a default in the zypper constructor, recreate here.

  // determine the desired verbosity
  int iverbosity = 0;
  //// --quiet
  if (gopts.count("quiet"))
  {
    _gopts.verbosity = iverbosity = -1;
    DBG << "Verbosity " << _gopts.verbosity << endl;
  }
  //// --verbose
  if ((it = gopts.find("verbose")) != gopts.end())
  {
    //! \todo if iverbosity is -1 now, say we conflict with -q
    _gopts.verbosity += iverbosity = it->second.size();
    // _gopts.verbosity += gopts["verbose"].size();
  }

  Out::Verbosity verbosity = Out::NORMAL;
  switch(iverbosity)
  {
    case -1: verbosity = Out::QUIET; break;
    case 0: verbosity = Out::NORMAL; break;
    case 1: verbosity = Out::HIGH; break;
    default: verbosity = Out::DEBUG;
  }

  //// --debug
  // rug compatibility alias for -vv
  if (gopts.count("debug"))
    verbosity = Out::DEBUG;

  // create output object

  //// --xml-out
  if (gopts.count("xmlout"))
  {
    _out_ptr = new OutXML(verbosity);
    _gopts.machine_readable = true;
    _gopts.no_abbrev = true;
  }
  else
  {
    OutNormal * p = new OutNormal(verbosity);
    p->setUseColors(_config.do_colors);
    _out_ptr = p;
  }

  out().info(boost::str(format(_("Verbosity: %d")) % _gopts.verbosity), Out::HIGH);
  DBG << "Verbosity " << verbosity << endl;
  DBG << "Output type " << _out_ptr->type() << endl;

  if (gopts.count("no-abbrev"))
    _gopts.no_abbrev = true;

  if ((it = gopts.find("table-style")) != gopts.end())
  {
    unsigned s;
    str::strtonum (it->second.front(), s);
    if (s < _End)
      Table::defaultStyle = (TableLineStyle) s;
    else
      out().error(str::form(_("Invalid table style %d."), s),
          str::form(_("Use an integer number from %d to %d"), 0, 8));
  }

  if (gopts.count("terse"))
  {
    _gopts.machine_readable = true;
    _gopts.no_abbrev = true;
    _gopts.terse = true;
    //out().error("--terse is not implemented, does nothing");
  }

  // ======== other global options ========

  string rug_test(_argv[0]);
  if (gopts.count("rug-compatible") || rug_test.rfind("rug") == rug_test.size()-3 )
  {
    _gopts.is_rug_compatible = true;
    out().info("Switching to rug-compatible mode.", Out::DEBUG);
    DBG << "Switching to rug-compatible mode." << endl;
  }

  // Help is parsed by setting the help flag for a command, which may be empty
  // $0 -h,--help
  // $0 command -h,--help
  // The help command is eaten and transformed to the help option
  // $0 help
  // $0 help command
  if (gopts.count("help"))
    setRunningHelp(true);

  if (gopts.count("non-interactive")) {
    _gopts.non_interactive = true;
    out().info(_("Entering non-interactive mode."), Out::HIGH);
    MIL << "Entering non-interactive mode" << endl;
  }

  if (gopts.count("non-interactive-include-reboot-patches")) {
    _gopts.reboot_req_non_interactive = true;
    out().info(_("Patches having the flag rebootSuggested set will not be treated as interactive."), Out::HIGH);
    MIL << "Patches having the flag rebootSuggested set will not be treated as interactive" << endl;
  }

  if (gopts.count("no-gpg-checks")) {
    _gopts.no_gpg_checks = true;
    out().info(_("Entering 'no-gpg-checks' mode."), Out::HIGH);
    MIL << "Entering no-gpg-checks mode" << endl;
  }

  if (gopts.count("gpg-auto-import-keys")) {
    _gopts.gpg_auto_import_keys = true;
    string warn = str::form(
      _("Turning on '%s'. New repository signing keys will be automatically imported!"),
      "--gpg-auto-import-keys");
    out().warning(warn, Out::HIGH);
    MIL << "gpg-auto-import-keys is on" << endl;
  }

  if ((it = gopts.find("root")) != gopts.end()) {
    _gopts.root_dir = it->second.front();
    _gopts.changedRoot = true;
    Pathname tmp(_gopts.root_dir);
    if (!tmp.absolute())
    {
      out().error(
        _("The path specified in the --root option must be absolute."));
      _exit_code = ZYPPER_EXIT_ERR_INVALID_ARGS;
      return;
    }

    DBG << "root dir = " << _gopts.root_dir << endl;
    _gopts.rm_options = RepoManagerOptions(_gopts.root_dir);
  }

  if ((it = gopts.find("reposd-dir")) != gopts.end()) {
    _gopts.rm_options.knownReposPath = it->second.front();
  }

  // cache dirs

  if ((it = gopts.find("cache-dir")) != gopts.end())
    _gopts.rm_options.repoCachePath = it->second.front();

  if ((it = gopts.find("raw-cache-dir")) != gopts.end())
    _gopts.rm_options.repoRawCachePath = it->second.front();
  else
    _gopts.rm_options.repoRawCachePath =
      _gopts.rm_options.repoCachePath / "raw";

  if ((it = gopts.find("solv-cache-dir")) != gopts.end())
    _gopts.rm_options.repoSolvCachePath = it->second.front();
  else
    _gopts.rm_options.repoSolvCachePath =
      _gopts.rm_options.repoCachePath / "solv";

  if ((it = gopts.find("pkg-cache-dir")) != gopts.end())
    _gopts.rm_options.repoPackagesCachePath = it->second.front();
  else
    _gopts.rm_options.repoPackagesCachePath =
      _gopts.rm_options.repoCachePath / "packages";

  DBG << "repos.d dir = " << _gopts.rm_options.knownReposPath << endl;
  DBG << "cache dir = " << _gopts.rm_options.repoCachePath << endl;
  DBG << "raw cache dir = " << _gopts.rm_options.repoRawCachePath << endl;
  DBG << "solv cache dir = " << _gopts.rm_options.repoSolvCachePath << endl;
  DBG << "package cache dir = " << _gopts.rm_options.repoPackagesCachePath << endl;

  if (gopts.count("disable-repositories"))
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

  if (gopts.count("no-refresh"))
  {
    _gopts.no_refresh = true;
    out().info(_("Autorefresh disabled."), Out::HIGH);
    MIL << "Autorefresh disabled." << endl;
  }

  if (gopts.count("no-cd"))
  {
    _gopts.no_cd = true;
    out().info(_("CD/DVD repositories disabled."), Out::HIGH);
    MIL << "No CD/DVD repos." << endl;
  }

  if (gopts.count("no-remote"))
  {
    _gopts.no_remote = true;
    out().info(_("Remote repositories disabled."), Out::HIGH);
    MIL << "No remote repos." << endl;
  }

  if (gopts.count("disable-system-resolvables"))
  {
    MIL << "System resolvables disabled" << endl;
    out().info(_("Ignoring installed resolvables."), Out::HIGH);
    _gopts.disable_system_resolvables = true;
  }

  // testing option
  if ((it = gopts.find("opt")) != gopts.end()) {
    cout << "Opt arg: ";
    std::copy (it->second.begin(), it->second.end(),
               ostream_iterator<string> (cout, ", "));
    cout << endl;
  }

  // get command
  if (optind < _argc)
  {
    try { setCommand(ZypperCommand(_argv[optind++])); }
    // exception from command parsing
    catch (Exception & e)
    {
      out().error(e.asUserString());
      if (runningHelp())
      {
        print_unknown_command_hint(*this);
        ZYPP_THROW(ExitRequestException("help provided"));
      }
    }
  }
  else if (!(gopts.count("version") || gopts.count("promptids")))
    setRunningHelp();

  if (command() == ZypperCommand::HELP)
  {
    // -h already present, print help on help and quit
    if (runningHelp())
    {
      print_unknown_command_hint(*this);
      print_command_help_hint(*this);
      ZYPP_THROW(ExitRequestException("help provided"));
    }

    setRunningHelp(true);
    if (optind < _argc)
    {
      // set the next argument as command (to provide help later)
      string arg = _argv[optind++];
      try { setCommand(ZypperCommand(arg)); }
      catch (Exception e)
      {
        // unknown command, print hint and exit
        if (!arg.empty() && arg != "-h" && arg != "--help")
        {
          out().info(e.asUserString(), Out::QUIET);
          print_unknown_command_hint(*this);
          ZYPP_THROW(ExitRequestException("help provided"));
        }
      }
    }
    // plain help command, print main help and quit
    else
    {
      print_main_help(*this);
      ZYPP_THROW(ExitRequestException("help provided"));
    }
  }
  else if (command() == ZypperCommand::NONE)
  {
    if (runningHelp())
      print_main_help(*this);
    else if (gopts.count("version"))
    {
      out().info(PACKAGE " " VERSION, Out::QUIET);
      ZYPP_THROW(ExitRequestException("version shown"));
    }
    else if (gopts.count("promptids"))
    {
      #define PR_ENUML(nam, val) out().info(#nam "=" #val, Out::QUIET);
      #define PR_ENUM(nam, val) PR_ENUML(nam, val)
      #include "output/prompt.h"
      ZYPP_THROW(ExitRequestException("promptids shown"));
    }
    else
    {
      print_unknown_command_hint(*this);
      setExitCode(ZYPPER_EXIT_ERR_SYNTAX);
    }
  }
  else if (command() == ZypperCommand::SHELL && optind < _argc)
  {
    string arg = _argv[optind++];
    if (!arg.empty())
    {
      if (arg == "-h" || arg == "--help")
        setRunningHelp(true);
      else
      {
        report_too_many_arguments("shell\n");
        setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
        ZYPP_THROW(ExitRequestException("help provided"));
      }
    }
  }

  // additional repositories
  if (gopts.count("plus-repo"))
  {
    switch (command().toEnum())
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
      out().warning(boost::str(format(
        // TranslatorExplanation The %s is "--plus-repo"
        _("The %s option has no effect here, ignoring."))
        % "--plus-repo"));
      break;
    }
    default:
    {
      list<string> repos = gopts["plus-repo"];

      int count = 1;
      for (list<string>::const_iterator it = repos.begin();
          it != repos.end(); ++it)
      {
        Url url = make_url (*it);
        if (!url.isValid())
        {
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }

        RepoInfo repo;
        repo.addBaseUrl(url);
        repo.setEnabled(true);
        repo.setAutorefresh(true);
        repo.setAlias(boost::str(format("tmp%d") % count));
        repo.setName(url.asString());

        _rdata.additional_repos.push_back(repo);
        DBG << "got additional repo: " << url << endl;
        count++;
      }
    }
    }
  }

  MIL << "DONE" << endl;
}


void Zypper::commandShell()
{
  MIL << "Entering the shell" << endl;

  setRunningShell(true);

  string histfile;
  try {
    const char * env = getenv ("HOME");
    if ( env )
    {
      Pathname p( env );
      p /= ".zypper_history";
      histfile = p.asString ();
    }
  } catch (...) {
    // no history
  }
  using_history ();
  if (!histfile.empty ())
    read_history (histfile.c_str ());

  while (true) {
    // read a line
    string line = readline_getline ();
    out().info(boost::str(format("Got: %s") % line), Out::DEBUG);
    // reset optind etc
    optind = 0;
    // split it up and create sh_argc, sh_argv
    Args args(line);
    _sh_argc = args.argc();
    _sh_argv = args.argv();

    string command_str = _sh_argv[0] ? _sh_argv[0] : "";

    if (command_str == "\004") // ^D
    {
      cout << endl; // print newline after ^D
      break;
    }

    try
    {
      setCommand(ZypperCommand(command_str));
      if (command() == ZypperCommand::SHELL_QUIT)
        break;
      else if (command() == ZypperCommand::NONE)
        print_unknown_command_hint(*this);
      else
        safeDoCommand();
    }
    catch (const Exception & e)
    {
      out().error(e.msg());
      print_unknown_command_hint(*this);
    }

    shellCleanup();
  }

  if (!histfile.empty ())
    write_history (histfile.c_str ());

  MIL << "Leaving the shell" << endl;
  setRunningShell(false);
}

void Zypper::shellCleanup()
{
  MIL << "Cleaning up for the next command." << endl;

  switch(command().toEnum())
  {
  case ZypperCommand::INSTALL_e:
  case ZypperCommand::REMOVE_e:
  case ZypperCommand::UPDATE_e:
  case ZypperCommand::PATCH_e:
  {
    remove_selections(*this);
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
  setRunningHelp(false);
  // ... and the exit code
  setExitCode(ZYPPER_EXIT_OK);

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
    if (command() == ZypperCommand::NONE || exitCode())
      return;
    doCommand();
  }
  catch (const AbortRequestException & ex)
  {
    ZYPP_CAUGHT(ex);

    // << _("User requested to abort.") << endl;
    out().error(ex.asUserString());
  }
  catch (const ExitRequestException & e)
  {
    MIL << "Caught exit request:" << endl << e.msg() << endl;
  }
  catch (const Exception & ex)
  {
    ZYPP_CAUGHT(ex);

    Out::Verbosity tmp = out().verbosity();
    out().setVerbosity(Out::DEBUG);
    out().error(ex, _("Unexpected exception."));
    out().setVerbosity(tmp);

    report_a_bug(out());
  }
}

// === command-specific options ===
void Zypper::processCommandOptions()
{
  MIL << "START" << endl;

  struct option no_options = {0, 0, 0, 0};
  struct option *specific_options = &no_options;

  if (command() == ZypperCommand::HELP)
  {
    // in shell, check next argument to see if command-specific help is wanted
    if (runningShell())
    {
      if (argc() > 1)
      {
        string cmd = argv()[1];
        try
        {
          setRunningHelp(true);
          setCommand(ZypperCommand(cmd));
        }
        catch (Exception & ex) {
          // unknown command. Known command will be handled in the switch
          // and doCommand()
          if (!cmd.empty() && cmd != "-h" && cmd != "--help")
          {
            out().info(ex.asUserString(), Out::QUIET);
            print_unknown_command_hint(*this);
            ZYPP_THROW(ExitRequestException("help provided"));
          }
        }
      }
      // if no command is requested, show main help
      else
      {
        print_main_help(*this);
        ZYPP_THROW(ExitRequestException("help provided"));
      }
    }
  }

  switch (command().toEnum())
  {
  // print help on help and return
  // this should work for both, in shell and out of shell
  case ZypperCommand::HELP_e:
  {
    print_unknown_command_hint(*this);
    print_command_help_hint(*this);
    ZYPP_THROW(ExitRequestException("help provided"));
  }

  //! \todo all option descriptions in help texts should start at 29th character
  //! and should wrap at 79th column (bnc #423007)

  case ZypperCommand::INSTALL_e:
  {
    static struct option install_options[] = {
      {"repo",                      required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog",                   required_argument, 0, 'c'},
      {"from",                      required_argument, 0,  0 },
      {"type",                      required_argument, 0, 't'},
      // the default (ignored)
      {"name",                      no_argument,       0, 'n'},
      {"force",                     no_argument,       0, 'f'},
      {"capability",                no_argument,       0, 'C'},
      // rug compatibility, we have global --non-interactive
      {"no-confirm",                no_argument,       0, 'y'},
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      // rug compatibility, we have --auto-agree-with-licenses
      {"agree-to-third-party-licenses",  no_argument,  0,  0 },
      {"debug-solver",              no_argument,       0,  0 },
      {"no-force-resolution",       no_argument,       0, 'R'},
      {"force-resolution",          no_argument,       0,  0 },
      {"dry-run",                   no_argument,       0, 'D'},
      // rug uses -N shorthand
      {"dry-run",                   no_argument,       0, 'N'},
      {"no-recommends",             no_argument,       0,  0 },
      {"recommends",                no_argument,       0,  0 },
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
    _command_help = str::form(_(
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
      "-f, --force                 Reinstall the package if the exact version is\n"
      "                            available in repositories.\n"
      "-l, --auto-agree-with-licenses\n"
      "                            Automatically say 'yes' to third party license\n"
      "                            confirmation prompt.\n"
      "                            See 'man zypper' for more details.\n"
      "    --debug-solver          Create solver test case for debugging.\n"
      "    --no-recommends         Do not install recommended packages, only required.\n"
      "    --recommends            Install also recommended packages in addition\n"
      "                            to the required.\n"
      "-R, --no-force-resolution   Do not force the solver to find solution,\n"
      "                            let it ask.\n"
      "    --force-resolution      Force the solver to find a solution (even\n"
      "                            an aggressive one).\n"
      "-D, --dry-run               Test the installation, do not actually install.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
    ), "package, patch, pattern, product, srcpackage",
       "package",
       "only, in-advance, in-heaps, as-needed");
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
      // rug compatibility, we have global --non-interactive
      {"no-confirm", no_argument,       0, 'y'},
      {"debug-solver", no_argument,     0, 0},
      {"no-force-resolution", no_argument, 0, 'R'},
      {"force-resolution", no_argument, 0,  0 },
      {"clean-deps", no_argument,       0, 'u'},
      {"no-clean-deps", no_argument,    0, 'U'},
      {"dry-run",    no_argument,       0, 'D'},
      // rug uses -N shorthand
      {"dry-run",    no_argument,       0, 'N'},
      {"help",       no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = remove_options;
    _command_help = str::form(_(
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
      "    --debug-solver          Create solver test case for debugging.\n"
      "-R, --no-force-resolution   Do not force the solver to find solution,\n"
      "                            let it ask.\n"
      "    --force-resolution      Force the solver to find a solution (even\n"
      "                            an aggressive one).\n"
      "-u, --clean-deps            Automatically remove unneeded dependencies.\n"
      "-U, --no-clean-deps         No automatic removal of unneeded dependencies.\n"
      "-D, --dry-run               Test the removal, do not actually remove.\n"
    ), "package, patch, pattern, product", "package");
    break;
  }

  case ZypperCommand::SRC_INSTALL_e:
  {
    static struct option src_install_options[] = {
      {"build-deps-only", no_argument, 0, 'd'},
      {"no-build-deps", no_argument, 0, 'D'},
      {"repo", required_argument, 0, 'r'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = src_install_options;
    _command_help = _(
      "source-install (si) [options] <name> ...\n"
      "\n"
      "Install specified source packages and their build dependencies.\n"
      "\n"
      "  Command options:\n"
      "-d, --build-deps-only    Install only build dependencies of specified packages.\n"
      "-D, --no-build-deps      Don't install build dependencies.\n"
      "-r, --repo <alias|#|URI> Install packages only from specified repositories.\n"
    );
    break;
  }

  case ZypperCommand::VERIFY_e:
  {
    static struct option verify_options[] = {
      // rug compatibility option, we have global --non-interactive
      {"no-confirm", no_argument, 0, 'y'},
      {"dry-run", no_argument, 0, 'D'},
      // rug uses -N shorthand
      {"dry-run", no_argument, 0, 'N'},
      {"download",                  required_argument, 0,  0 },
      // aliases for --download
      // in --download-only, -d must be kept for backward and rug compatibility
      {"download-only",             no_argument,       0, 'd'},
      {"download-in-advance",       no_argument,       0,  0 },
      {"download-in-heaps",         no_argument,       0,  0 },
      {"download-as-needed",        no_argument,       0,  0 },
      {"repo", required_argument, 0, 'r'},
      {"no-recommends", no_argument, 0, 0},
      {"help", no_argument, 0, 'h'},
      {"debug-solver", no_argument, 0, 0},
      {0, 0, 0, 0}
    };
    specific_options = verify_options;
    _command_help = str::form(_(
      "verify (ve) [options]\n"
      "\n"
      "Check whether dependencies of installed packages are satisfied"
      " and repair eventual dependency problems.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "    --no-recommends         Do not install recommended packages, only required.\n"
      "    --recommends            Install also recommended packages in addition\n"
      "                            to the required.\n"
      "-D, --dry-run               Test the repair, do not actually do anything to\n"
      "                            the system.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
    ), "only, in-advance, in-heaps, as-needed");
    break;
  }

  case ZypperCommand::INSTALL_NEW_RECOMMENDS_e:
  {
    static struct option options[] = {
      {"dry-run", no_argument, 0, 'D'},
      {"download",                  required_argument, 0,  0 },
      // aliases for --download
      // in --download-only, -d must be kept for backward and rug compatibility
      {"download-only",             no_argument,       0, 'd'},
      {"download-in-advance",       no_argument,       0,  0 },
      {"download-in-heaps",         no_argument,       0,  0 },
      {"download-as-needed",        no_argument,       0,  0 },
      {"repo", required_argument, 0, 'r'},
      {"debug-solver", no_argument, 0, 0},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = str::form(_(
      "install-new-recommends (inr) [options]\n"
      "\n"
      "Install newly added packages recommended by already installed packages."
      " This can typically be used to install new language packages or drivers"
      " for newly added hardware.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias|#|URI>    Load only the specified repositories.\n"
      "-D, --dry-run               Test the installation, do not actually install.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
      "    --debug-solver          Create solver test case for debugging.\n"
    ), "only, in-advance, in-heaps, as-needed");
    break;
  }

  case ZypperCommand::ADD_SERVICE_e:
  {
    static struct option service_add_options[] = {
      {"type", required_argument, 0, 't'},
      {"disable", no_argument, 0, 'd'},
      {"name", required_argument, 0, 'n'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_add_options;
    _command_help = str::form(_(
      // translators: the %s = "ris" (the only service type currently supported)
      "addservice (as) [options] <URI> <alias>\n"
      "\n"
      "Add a repository index service to the system.\n"
      "\n"
      "  Command options:\n"
      "-t, --type <type>       Type of the service (%s).\n"
      "-d, --disable           Add the service as disabled.\n"
      "-n, --name <alias>      Specify descriptive name for the service.\n"
    ), "ris");
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
      "Remove specified repository index service from the sytem..\n"
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
      {"disable", no_argument, 0, 'd'},
      {"enable", no_argument, 0, 'e'},
      {"refresh", no_argument, 0, 'r'},
      {"no-refresh", no_argument, 0, 'R'},
      {"name", required_argument, 0, 'n'},
      {"ar-to-enable",  required_argument, 0, 'i'},
      {"ar-to-disable", required_argument, 0, 'I'},
      {"rr-to-enable",  required_argument, 0, 'j'},
      {"rr-to-disable", required_argument, 0, 'J'},
      {"cl-to-enable",  no_argument, 0, 'k'},
      {"cl-to-disable", no_argument, 0, 'K'},
      // aggregates
      {"all", no_argument, 0, 'a' },
      {"local", no_argument, 0, 'l' },
      {"remote", no_argument, 0, 't' },
      {"medium-type", required_argument, 0, 'm' },
      {0, 0, 0, 0}
    };
    specific_options = service_modify_options;
    _command_help = str::form(_(
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
      "-n, --name                     Set a descriptive name for the service.\n"
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
    ), "--all|--remote|--local|--medium-type"
     , "--all, --remote, --local, --medium-type");
    // ---------|---------|---------|---------|---------|---------|---------|---------
    break;
  }

  case ZypperCommand::LIST_SERVICES_e:
  {
    static struct option options[] =
    {
      {"help", no_argument, 0, 'h'},
      {"uri", no_argument, 0, 'u'},
      {"url", no_argument, 0, 'u'},
      {"priority", no_argument, 0, 'p'},
      {"details", no_argument, 0, 'd'},
//      {"type", required_argument, 0, 't'},
      {"with-repos", no_argument, 0, 'r'},
      {"sort-by-uri", no_argument, 0, 'U'},
      {"sort-by-name", no_argument, 0, 'N'},
      {"sort-by-priority", no_argument, 0, 'P'},
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
//      "-t, --type                List only services of specified type.\n"
      "-r, --with-repos          Show also repositories belonging to the services.\n"
      "-P, --sort-by-priority    Sort the list by repository priority.\n"
      "-U, --sort-by-uri         Sort the list by URI.\n"
      "-N, --sort-by-name        Sort the list by name.\n"
    );
    break;
  }

  case ZypperCommand::REFRESH_SERVICES_e:
  {
    static struct option options[] = {
      {"help", no_argument, 0, 'h'},
      {"with-repos", no_argument, 0, 'r'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "refresh-services (refs) [options]\n"
      "\n"
      "Refresh defined repository index services.\n"
      "\n"
      "  Command options:\n"
      "-r, --with-repos      Refresh also repositories.\n"
    );
    break;
  }

  case ZypperCommand::ADD_REPO_e:
  {
    static struct option service_add_options[] = {
      {"type", required_argument, 0, 't'},
      {"disable", no_argument, 0, 'd'},
      {"repo", required_argument, 0, 'r'},
      {"help", no_argument, 0, 'h'},
      {"check", no_argument, 0, 'c'},
      {"no-check", no_argument, 0, 'C'},
      {"name", required_argument, 0, 'n'},
      {"keep-packages", no_argument, 0, 'k'},
      {"no-keep-packages", no_argument, 0, 'K'},
      {"gpgcheck", no_argument, 0, 'g'},
      {"no-gpgcheck", no_argument, 0, 'G'},
      {"refresh", no_argument, 0, 'f'},
      {0, 0, 0, 0}
    };
    specific_options = service_add_options;
    _command_help = str::form(_(
      // translators: the %s = "yast2, rpm-md, plaindir"
      "addrepo (ar) [options] <URI> <alias>\n"
      "addrepo (ar) [options] <file.repo>\n"
      "\n"
      "Add a repository to the sytem. The repository can be specified by its URI"
      " or can be read from specified .repo file (even remote).\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <file.repo>  Just another means to specify a .repo file to read.\n"
      "-t, --type <type>       Type of repository (%s).\n"
      "-d, --disable           Add the repository as disabled.\n"
      "-c, --check             Probe URI.\n"
      "-C, --no-check          Don't probe URI, probe later during refresh.\n"
      "-n, --name <name>       Specify descriptive name for the repository.\n"
      "-k, --keep-packages     Enable RPM files caching.\n"
      "-K, --no-keep-packages  Disable RPM files caching.\n"
      "-g, --gpgcheck          Enable GPG check for this repository.\n"
      "-G, --no-gpgcheck       Disable GPG check for this repository.\n"
      "-f, --refresh           Enable autorefresh of the repository.\n"
    ), "yast2, rpm-md, plaindir");
    break;
  }

  case ZypperCommand::LIST_REPOS_e:
  {
    static struct option service_list_options[] = {
      {"export", required_argument, 0, 'e'},
      {"alias", no_argument, 0, 'a'},
      {"name", no_argument, 0, 'n'},
      {"refresh", no_argument, 0, 'r'},
      {"uri", no_argument, 0, 'u'},
      {"url", no_argument, 0, 'u'},
      {"priority", no_argument, 0, 'p'},
      {"details", no_argument, 0, 'd'},
      {"sort-by-priority", no_argument, 0, 'P'},
      {"sort-by-uri", no_argument, 0, 'U'},
      {"sort-by-alias", no_argument, 0, 'A'},
      {"sort-by-name", no_argument, 0, 'N'},
      {"service", no_argument, 0, 's'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_list_options;

    // handle the conflicting rug's lr here:
    if (_gopts.is_rug_compatible)
    {
      static struct option options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
      };
      specific_options = options;

      _command_help = _(
        // translators: this is just a rug compatiblity command
        "list-resolvables (lr)\n"
        "\n"
        "List available resolvable types.\n"
      );
      break;
    }

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
      {0, 0, 0, 0}
    };
    specific_options = service_delete_options;
    _command_help = _(
      "removerepo (rr) [options] <alias|#|URI>\n"
      "\n"
      "Remove repository specified by alias, number or URI.\n"
      "\n"
      "  Command options:\n"
      "    --loose-auth   Ignore user authentication data in the URI.\n"
      "    --loose-query  Ignore query string in the URI.\n"
    );
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
      {"disable", no_argument, 0, 'd'},
      {"enable", no_argument, 0, 'e'},
      {"refresh", no_argument, 0, 'r'},
      {"no-refresh", no_argument, 0, 'R'},
      {"name", required_argument, 0, 'n'},
      {"priority", required_argument, 0, 'p'},
      {"keep-packages", no_argument, 0, 'k'},
      {"no-keep-packages", no_argument, 0, 'K'},
      {"gpgcheck", no_argument, 0, 'g'},
      {"no-gpgcheck", no_argument, 0, 'G'},
      {"all", no_argument, 0, 'a' },
      {"local", no_argument, 0, 'l' },
      {"remote", no_argument, 0, 't' },
      {"medium-type", required_argument, 0, 'm' },
      {0, 0, 0, 0}
    };
    specific_options = service_modify_options;
    _command_help = str::form(_(
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
      "-n, --name                Set a descriptive name for the repository.\n"
      "-p, --priority <integer>  Set priority of the repository.\n"
      "-k, --keep-packages       Enable RPM files caching.\n"
      "-K, --no-keep-packages    Disable RPM files caching.\n"
      "-g, --gpgcheck            Enable GPG check for this repository.\n"
      "-G, --no-gpgcheck         Disable GPG check for this repository.\n"
      "\n"
      "-a, --all                 Apply changes to all repositories.\n"
      "-l, --local               Apply changes to all local repositories.\n"
      "-t, --remote              Apply changes to all remote repositories.\n"
      "-m, --medium-type <type>  Apply changes to repositories of specified type.\n"
    ), "--all|--remote|--local|--medium-type"
     , "--all, --remote, --local, --medium-type");
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
    static struct option update_options[] = {
      {"repo",                      required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog",                   required_argument, 0, 'c'},
      {"type",                      required_argument, 0, 't'},
      // rug compatibility option, we have global --non-interactive
      // note: rug used this uption only to auto-answer the 'continue with install?' prompt.
      {"no-confirm",                no_argument,       0, 'y'},
      {"skip-interactive",          no_argument,       0, 0},
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      // rug compatibility, we have --auto-agree-with-licenses
      {"agree-to-third-party-licenses",  no_argument,  0, 0},
      {"best-effort",               no_argument,       0, 0},
      {"debug-solver",              no_argument,       0, 0},
      {"no-force-resolution",       no_argument,       0, 'R'},
      {"force-resolution",          no_argument,       0,  0 },
      {"no-recommends",             no_argument,       0,  0 },
      {"recommends",                no_argument,       0,  0 },
      {"dry-run",                   no_argument,       0, 'D'},
      // rug uses -N shorthand
      {"dry-run",                   no_argument,       0, 'N'},
      {"download",                  required_argument, 0,  0 },
      // aliases for --download
      // in --download-only, -d must be kept for backward and rug compatibility
      {"download-only",             no_argument,       0, 'd'},
      {"download-in-advance",       no_argument,       0,  0 },
      {"download-in-heaps",         no_argument,       0,  0 },
      {"download-as-needed",        no_argument,       0,  0 },
      // rug-compatibility - dummy for now
      //! \todo category can now be implemented in 'patch' using PoolQuery
      {"category",                  no_argument,       0, 'g'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = update_options;
    _command_help = str::form(_(
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
      "-l, --auto-agree-with-licenses\n"
      "                            Automatically say 'yes' to third party license\n"
      "                            confirmation prompt.\n"
      "                            See man zypper for more details.\n"
      "    --best-effort           Do a 'best effort' approach to update. Updates\n"
      "                            to a lower than the latest version are\n"
      "                            also acceptable.\n"
      "    --debug-solver          Create solver test case for debugging.\n"
      "    --no-recommends         Do not install recommended packages, only required.\n"
      "    --recommends            Install also recommended packages in addition\n"
      "                            to the required.\n"
      "-R, --no-force-resolution   Do not force the solver to find solution,\n"
      "                            let it ask.\n"
      "    --force-resolution      Force the solver to find a solution (even\n"
      "                            an aggressive one).\n"
      "-D, --dry-run               Test the update, do not actually update.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
    ), "package, patch, pattern, product, srcpackage",
       "package",
       "only, in-advance, in-heaps, as-needed");
    break;
  }

  case ZypperCommand::PATCH_e:
  {
    static struct option update_options[] = {
      {"repo",                      required_argument, 0, 'r'},
      {"skip-interactive",          no_argument,       0,  0 },
      {"with-interactive",          no_argument,       0,  0 },
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      {"debug-solver",              no_argument,       0,  0 },
      {"no-recommends",             no_argument,       0,  0 },
      {"recommends",                no_argument,       0,  0 },
      {"dry-run",                   no_argument,       0, 'D'},
      {"download",                  required_argument, 0,  0 },
      // aliases for --download
      // in --download-only, -d must be kept for backward and rug compatibility
      {"download-only",             no_argument,       0, 'd'},
      {"download-in-advance",       no_argument,       0,  0 },
      {"download-in-heaps",         no_argument,       0,  0 },
      {"download-as-needed",        no_argument,       0,  0 },
      {"bz",                        required_argument, 0, 'b'},
      {"bugzilla",                  required_argument, 0, 'b'},
      {"cve",                       required_argument, 0,  0 },
      {"category",                  required_argument, 0, 'g'},
      {"date",                      required_argument, 0,  0 },
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = update_options;
    _command_help = str::form(_(
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
      "-g  --category <category>   Install all patches in this category.\n"
     "    --date <YYYY-MM-DD>      Install patches issued until the specified date\n"
      "    --debug-solver          Create solver test case for debugging.\n"
      "    --no-recommends         Do not install recommended packages, only required.\n"
      "    --recommends            Install also recommended packages in addition\n"
      "                            to the required.\n"
      "-r, --repo <alias|#|URI>    Load only the specified repository.\n"
      "-D, --dry-run               Test the update, do not actually update.\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
    ), "only, in-advance, in-heaps, as-needed");
    break;
  }

  case ZypperCommand::LIST_PATCHES_e:
  {
    static struct option list_updates_options[] = {
      {"repo",        required_argument, 0, 'r'},
      {"bz",          optional_argument, 0, 'b'},
      {"bugzilla",    optional_argument, 0, 'b'},
      {"cve",         optional_argument, 0,  0 },
      {"category",    required_argument, 0, 'g'},
      {"date",        required_argument, 0,  0 },
      {"issues",      optional_argument, 0,  0 },
      {"all",         no_argument,       0, 'a'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = list_updates_options;
    _command_help = _(
      "list-patches (lp) [options]\n"
      "\n"
      "List all available needed patches.\n"
      "\n"
      "  Command options:\n"
      "-b, --bugzilla[=#]         List needed patches for Bugzilla issues.\n"
      "    --cve[=#]              List needed patches for CVE issues.\n"
      "-g  --category <category>  List all patches in this category.\n"
      "    --issues[=string]      Look for issues matching the specified string.\n"
      "-a, --all                  List all patches, not only the needed ones.\n"
      "-r, --repo <alias|#|URI>   List only patches from the specified repository.\n"
      "    --date <YYYY-MM-DD>    List patches issued up to the specified date\n"
    );
    break;
  }

  case ZypperCommand::DIST_UPGRADE_e:
  {
    static struct option dupdate_options[] = {
      {"repo",                      required_argument, 0, 'r'},
      {"from",                      required_argument, 0,  0 },
      {"no-recommends",             no_argument,       0,  0 },
      {"recommends",                no_argument,       0,  0 },
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      {"debug-solver",              no_argument,       0,  0 },
      {"dry-run",                   no_argument,       0, 'D'},
      // rug uses -N shorthand
      {"dry-run",                   no_argument,       0, 'N'},
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
    specific_options = dupdate_options;
    _command_help = str::form(_(
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
      "    --debug-solver          Create solver test case for debugging\n"
      "    --no-recommends         Do not install recommended packages, only required.\n"
      "    --recommends            Install also recommended packages in addition\n"
      "                            to the required.\n"
      "-D, --dry-run               Test the upgrade, do not actually upgrade\n"
      "    --download              Set the download-install mode. Available modes:\n"
      "                            %s\n"
      "-d, --download-only         Only download the packages, do not install.\n"
    ), "only, in-advance, in-heaps, as-needed");
    break;
  }

  case ZypperCommand::SEARCH_e:
  {
    static struct option search_options[] = {
      {"installed-only", no_argument, 0, 'i'},
      {"uninstalled-only", no_argument, 0, 'u'},
      {"match-all", no_argument, 0, 0},
      {"match-any", no_argument, 0, 0},
      {"match-substrings", no_argument, 0, 0},
      {"match-words", no_argument, 0, 0},
      {"match-exact", no_argument, 0, 0},
      {"search-descriptions", no_argument, 0, 'd'},
      {"case-sensitive", no_argument, 0, 'C'},
      {"type",    required_argument, 0, 't'},
      {"sort-by-name", no_argument, 0, 0},
      // rug compatibility option, we have --sort-by-repo
      {"sort-by-catalog", no_argument, 0, 0},
      {"sort-by-repo", no_argument, 0, 0},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      {"repo", required_argument, 0, 'r'},
      {"details", no_argument, 0, 's'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = search_options;
    _command_help = _(
      "search (se) [options] [querystring] ...\n"
      "\n"
      "Search for packages matching given search strings.\n"
      "\n"
      "  Command options:\n"
      "    --match-all            Search for a match with all search strings (default).\n"
      "    --match-any            Search for a match with any of the search strings.\n"
      "    --match-substrings     Search for a match to partial words (default).\n"
      "    --match-words          Search for a match to whole words only.\n"
      "    --match-exact          Searches for an exact package name.\n"
      "-d, --search-descriptions  Search also in package summaries and descriptions.\n"
      "-C, --case-sensitive       Perform case-sensitive search.\n"
      "-i, --installed-only       Show only packages that are already installed.\n"
      "-u, --uninstalled-only     Show only packages that are not currently installed.\n"
      "-t, --type <type>          Search only for packages of the specified type.\n"
      "-r, --repo <alias|#|URI>   Search only in the specified repository.\n"
      "    --sort-by-name         Sort packages by name (default).\n"
      "    --sort-by-repo         Sort packages by repository.\n"
      "-s, --details              Show each available version in each repository\n"
      "                           on a separate line.\n"
      "\n"
      "* and ? wildcards can also be used within search strings.\n"
    );
    break;
  }

  case ZypperCommand::PATCH_CHECK_e:
  {
    static struct option patch_check_options[] = {
      {"repo", required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = patch_check_options;
    _command_help = _(
      "patch-check (pchk) [options]\n"
      "\n"
      "Check for available patches.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias|#|URI>  Check for patches only in the specified repository.\n"
    );
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
      {"repo", required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      {"installed-only", no_argument, 0, 'i'},
      {"uninstalled-only", no_argument, 0, 'u'},
      {"sort-by-name", no_argument, 0, 'N'},
      {"sort-by-repo", no_argument, 0, 'R'},
      {"sort-by-catalog", no_argument, 0, 0},
      {"help", no_argument, 0, 'h'},
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
      "-u, --uninstalled-only    Show only packages which are not installed.\n"
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
      {"installed-only", no_argument, 0, 'i'},
      {"uninstalled-only", no_argument, 0, 'u'},
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
      "-u, --uninstalled-only    Show only patterns which are not installed.\n"
    );
    break;
  }

  case ZypperCommand::PRODUCTS_e:
  {
    static struct option options[] = {
      {"repo", required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      {"installed-only", no_argument, 0, 'i'},
      {"uninstalled-only", no_argument, 0, 'u'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "products (pd) [options] [repository] ...\n"
      "\n"
      "List all products available in specified repositories.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias|#|URI>  Just another means to specify repository.\n"
      "-i, --installed-only      Show only installed products.\n"
      "-u, --uninstalled-only    Show only products which are not installed.\n"
    );
    break;
  }

  case ZypperCommand::INFO_e:
  {
    static struct option info_options[] = {
      {"type", required_argument, 0, 't'},
      {"repo", required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      {"requires", no_argument, 0, 0},
      {"recommends", no_argument, 0, 0},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = info_options;
    _command_help = str::form(_(
        "info (if) [options] <name> ...\n"
        "\n"
        "Show detailed information for specified packages.\n"
        "\n"
        "  Command options:\n"
        "-r, --repo <alias|#|URI>  Work only with the specified repository.\n"
        "-t, --type <type>         Type of package (%s).\n"
        "                          Default: %s.\n"
        "    --requires            Show also requires and prerequires.\n"
        "    --recommends          Show also recommends."
      ), "package, patch, pattern, product", "package");

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
      "This is a rug compatibility alias for '%s'.\n"
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
      "This is a rug compatibility alias for '%s'.\n"
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
      "This is a rug compatibility alias for '%s'.\n"
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
      // rug compatiblity (although rug does not seem to support this)
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
      {"repo", required_argument, 0, 'r'},
      // rug compatiblity (although rug does not seem to support this)
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
    ), "zypper locks");
    break;
  }

  case ZypperCommand::LIST_LOCKS_e:
  {
    static struct option options[] =
    {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "locks (ll)\n"
      "\n"
      "List current package locks.\n"
      "\n"
      "This command has no additional options.\n"
    );
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
    static struct option options[] =
    {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = options;
    _command_help = _(
      "ps\n"
      "\n"
      "List running processes which use files deleted by recent upgrades.\n"
      "\n"
      "This command has no additional options.\n"
    );
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
      // translators: this is just a rug-compatiblity command
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
      // translators: this is just a rug-compatiblity command
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
      {"installed-only", no_argument, 0, 'i'},
      {"uninstalled-only", no_argument, 0, 'u'},
      {"match-all", no_argument, 0, 0},
      {"match-any", no_argument, 0, 0},
      {"match-substrings", no_argument, 0, 0},
      {"match-words", no_argument, 0, 0},
      {"match-exact", no_argument, 0, 0},
      {"search-descriptions", no_argument, 0, 'd'},
      {"case-sensitive", no_argument, 0, 'C'},
      {"sort-by-name", no_argument, 0, 0},
      {"sort-by-catalog", no_argument, 0, 0},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = search_options;
    _command_help = str::form(_(
      "patch-search [options] [querystring...]\n"
      "\n"
      "Search for patches matching given search strings. This is a"
      " rug-compatibility alias for '%s'. See zypper's manual page for details.\n"
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
      // translators: this is just a rug-compatiblity command
      "ping [options]\n"
      "\n"
      "This command has dummy implementation which always returns 0.\n"
      "It is provided for compatibility with rug.\n"
    );
    break;
  }

  default:
  {
    if (runningHelp())
      break;

    ERR << "Unknown or unexpected command" << endl;
    out().error(_("Unexpected program flow."));
    report_a_bug(out());
  }
  }

  // no need to parse command options if we already know we just want help
  if (runningHelp())
    return;

  // parse command options
  ::copts = _copts = parse_options (argc(), argv(), specific_options);
  if (copts.count("_unknown") || copts.count("_missing_arg"))
  {
    setExitCode(ZYPPER_EXIT_ERR_SYNTAX);
    ERR << "Unknown option or missing argument, returning." << endl;
    return;
  }

  MIL << "Done parsing options." << endl;

  // treat --help command option like global --help option from now on
  // i.e. when used together with command to print command specific help
  setRunningHelp(runningHelp() || copts.count("help"));

  if (optind < argc())
  {
    ostringstream s;
    s << _("Non-option program arguments: ");
    while (optind < argc())
    {
      string argument = argv()[optind++];
      s << "'" << argument << "' ";
      _arguments.push_back (argument);
    }
    out().info(s.str(), Out::HIGH);
  }

  MIL << "Done " << endl;
}

/// process one command from the OS shell or the zypper shell
void Zypper::doCommand()
{
  if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

  // === ZYpp lock ===

  try
  {
    const char *roh = getenv("ZYPP_READONLY_HACK");
    if (roh != NULL && roh[0] == '1')
      zypp_readonly_hack::IWantIt ();

    else if (   command() == ZypperCommand::LIST_REPOS
             || command() == ZypperCommand::LIST_SERVICES
             || command() == ZypperCommand::TARGET_OS )
      zypp_readonly_hack::IWantIt (); // #247001, #302152

    God = zypp::getZYpp();
  }
  catch (ZYppFactoryException & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);

    bool still_locked = true;
    // check for packagekit (bnc #580513)
    if (excpt_r.lockerName().find("packagekitd") != string::npos)
    {
      // ask user wheter to tell it to quit
      out().info(_(
        "PackageKit is blocking zypper. This happens if you have an"
        " updater applet or other software management application using"
        " PackageKit running."
      ));

      bool reply = read_bool_answer(
          PROMPT_PACKAGEKIT_QUIT, _("Tell PackageKit to quit?"), false);

      // tell it to quit
      while (reply && still_locked)
      {
        packagekit_suggest_quit();
        ::sleep(2);
        if (packagekit_running())
        {
          out().info(_("PackageKit is still running (probably busy)."));
          reply = read_bool_answer(
              PROMPT_PACKAGEKIT_QUIT, _("Try again?"), false);
        }
        else
          still_locked = false;
      }
    }

    if (still_locked)
    {
      ERR  << "A ZYpp transaction is already in progress." << endl;
      out().error(excpt_r.asString());

      setExitCode(ZYPPER_EXIT_ERR_ZYPP);
      throw (ExitRequestException("ZYpp locked"));
    }
    else
    {
      // try to get the lock again
      try { God = zypp::getZYpp(); }
      catch (ZYppFactoryException & e)
      {
        // this should happen only rarely, so no special handling here
        ERR  << "still locked." << endl;
        out().error(e.asString());

        setExitCode(ZYPPER_EXIT_ERR_ZYPP);
        throw (ExitRequestException("ZYpp locked"));
      }
    }
  }
  catch (Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
    out().error(excpt_r.msg());
    setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    throw (ExitRequestException("ZYpp locked"));
  }

  // === execute command ===

  MIL << "Going to process command " << command().toEnum() << endl;
  ResObject::Kind kind;

  switch(command().toEnum())
  {

  // --------------------------( moo )----------------------------------------

  case ZypperCommand::MOO_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // TranslatorExplanation this is a hedgehog, paint another animal, if you want
    out().info(_("   \\\\\\\\\\\n  \\\\\\\\\\\\\\__o\n__\\\\\\\\\\\\\\'/_"));
    break;
  }

  // --------------------------( service list )-------------------------------

  case ZypperCommand::LIST_SERVICES_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    initRepoManager();

    list_services(*this);

    break;
  }

  // --------------------------( service refresh )-----------------------------

  case ZypperCommand::REFRESH_SERVICES_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for refreshing services."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    // needed to be able to retrieve target distribution
    init_target(*this);
    this->_gopts.rm_options.servicesTargetDistro =
      God->target()->targetDistribution();

    initRepoManager();

    refresh_services(*this);

    break;
  }

  // --------------------------( add service )---------------------------------

  case ZypperCommand::ADD_SERVICE_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for modifying system services."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    // too many arguments
    if (_arguments.size() > 2)
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    // missing arguments
    if (_arguments.size() < 2)
    {
      report_required_arg_missing(out(), _command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    // indeterminate indicates the user has not specified the values
    tribool enabled(indeterminate);

    if (copts.count("disable"))
      enabled = false;

    // enable by default
    if (indeterminate(enabled)) enabled = true;

    Url url = make_url (_arguments[0]);
    if (!url.isValid())
    {
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    initRepoManager();
    RepoManager & rm = repoManager();

    // force specific repository type.
    string type = copts.count("type") ? copts["type"].front() : "";

    // check for valid service type
    bool isservice = false;
    if (type.empty())
    {
      // rug does not make difference between services and repos, so service-add
      // must be able to handle them both (bnc #429620)
      if (globalOpts().is_rug_compatible)
      {
        repo::ServiceType stype = rm.probeService(url);
        // is it a service?
        if (stype != repo::ServiceType::NONE)
        {
          isservice = true;
          type = stype.asString();
        }
        // is it a repo?
        else
        {
          repo::RepoType rtype = rm.probe(url);
          if (rtype != repo::RepoType::NONE)
            type = rtype.asString();
          // complain & exit
          else
          {
            out().error(
              _("Can't find a valid repository at given location:"),
              _("Could not determine the type of the repository."
                " Check if the specified URI points to a valid repository."));
            setExitCode(ZYPPER_EXIT_ERR_ZYPP);
            return;
          }
        }
      }
      // zypper does not access net when adding repos/services, thus for zypper
      // the URI is always service unless --type is explicitely specified.
      else
       isservice = true;
    }
    else
    {
      try { repo::ServiceType stype(type); isservice = true; }
      catch (const repo::RepoUnknownTypeException & e) {}
    }

    warn_if_zmd();

    if (isservice)
      add_service_by_url(*this, url, _arguments[1], type, enabled);
    else
    {
      try
      {
        add_repo_by_url(*this, url, _arguments[1], type, enabled);
      }
      catch (const repo::RepoUnknownTypeException & e)
      {
        ZYPP_CAUGHT(e);
        out().error(
            str::form(_("'%s' is not a valid service type."), type.c_str()),
            str::form(
                _("See '%s' or '%s' to get a list of known service types."),
                "zypper help addservice", "man zypper"));
        setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      }
    }

    break;
  }

  case ZypperCommand::MODIFY_SERVICE_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for modifying system services."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    bool non_alias = copts.count("all") || copts.count("local") ||
        copts.count("remote") || copts.count("medium-type");

    if (_arguments.size() < 1 && !non_alias)
    {
      // translators: aggregate option is e.g. "--all". This message will be
      // followed by ms command help text which will explain it
      out().error(_("Alias or an aggregate option is required."));
      ERR << "No alias argument given." << endl;
      out().info(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }
    // too many arguments
    if (_arguments.size() > 1
       || (_arguments.size() > 0 && non_alias))
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    initRepoManager();

    if (non_alias)
    {
      modify_services_by_option(*this);
    }
    else
    {
      repo::RepoInfoBase_Ptr srv;
      if (match_service(*this, _arguments[0], srv))
      {
        if (dynamic_pointer_cast<ServiceInfo>(srv))
          modify_service(*this, srv->alias());
        else
          modify_repo(*this, srv->alias());
      }
      else
      {
        out().error(
          boost::str(format(_("Service '%s' not found.")) % _arguments[0]));
        ERR << "Service " << _arguments[0] << " not found" << endl;
      }
    }

    break;
  }

  // --------------------------( repo list )----------------------------------

  case ZypperCommand::LIST_REPOS_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    initRepoManager();

    //! \todo this conflicts with other 'lr' aliases
    //if (_gopts.is_rug_compatible)
    //  rug_list_resolvables(*this);
    //else
      list_repos(*this);

    break;
  }

  // --------------------------( addrepo )------------------------------------

  case ZypperCommand::ADD_REPO_e:
  case ZypperCommand::RUG_MOUNT_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for modifying system repositories."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    // too many arguments
    if (_arguments.size() > 2)
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    // indeterminate indicates the user has not specified the values

    TriBool enabled = indeterminate;
    if (copts.count("disable"))
      enabled = false;

    TriBool autorefresh = indeterminate;
    if (copts.count("refresh"))
      autorefresh = true;

    TriBool keep_pkgs;
    if (copts.count("keep-packages"))
      keep_pkgs = true;
    else if (copts.count("no-keep-packages"))
      keep_pkgs = false;

    TriBool gpgCheck;
    if (copts.count("gpgcheck"))
      gpgCheck = true;
    else if (copts.count("no-gpgcheck"))
      gpgCheck = false;

    try
    {
      // add repository specified in .repo file
      if (copts.count("repo"))
      {
        add_repo_from_file(*this,copts["repo"].front(), enabled, autorefresh, keep_pkgs, gpgCheck);
        return;
      }

      // force specific repository type. Validation is done in add_repo_by_url()
      string type = copts.count("type") ? copts["type"].front() : "";
      if (command() == ZypperCommand::RUG_MOUNT || type == "mount")
        type = "plaindir";
      else if (type == "zypp")
        type = "";

      switch (_arguments.size())
      {
      // display help message if insufficient info was given
      case 0:
        out().error(_("Too few arguments."));
        ERR << "Too few arguments." << endl;
        out().info(_command_help);
        setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
        return;
      case 1:
        if (command() == ZypperCommand::RUG_MOUNT)
        {
          string alias;
          parsed_opts::const_iterator it = _copts.find("alias");
          if (it != _copts.end())
            alias = it->second.front();
          // get the last component of the path
          if (alias.empty())
          {
            Pathname path(_arguments[0]);
            alias = path.basename();
          }
          _arguments.push_back(alias);
          // continue to case 2:
        }
        else if( !isRepoFile(_arguments[0] ))
        {
          out().error(
            _("If only one argument is used, it must be a URI pointing to a .repo file."));
          ERR << "Not a repo file." << endl;
          out().info(_command_help);
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }
        else
        {
          initRepoManager();
          add_repo_from_file(*this,_arguments[0], enabled, autorefresh, keep_pkgs, gpgCheck);
          break;
        }
      case 2:
	Url url;
	if (_arguments[0].find("obs") == 0)
	  url = make_obs_url(_arguments[0], config().obs_baseUrl, config().obs_platform);
	else
	  url = make_url(_arguments[0]);
        if (!url.isValid())
        {
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }

        if (copts.count("check"))
        {
          if (!copts.count("no-check"))
            this->_gopts.rm_options.probe = true;
          else
            this->out().warning(str::form(
              _("Cannot use %s together with %s. Using the %s setting."),
              "--check", "--no-check", "zypp.conf")
                ,Out::QUIET);
        }
        else if (copts.count("no-check"))
          this->_gopts.rm_options.probe = false;

        warn_if_zmd();

        initRepoManager();

        // load gpg keys
        init_target(*this);

        add_repo_by_url(
	    *this, url, _arguments[1]/*alias*/, type, enabled, autorefresh, keep_pkgs, gpgCheck);
        return;
      }
    }
    catch (const repo::RepoUnknownTypeException & e)
    {
      ZYPP_CAUGHT(e);
      out().error(e,
          _("Specified type is not a valid repository type:"),
          str::form(
              _("See '%s' or '%s' to get a list of known repository types."),
              "zypper help addrepo", "man zypper"));
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    }

    break;
  }

  // --------------------------( delete repo )--------------------------------

  case ZypperCommand::REMOVE_SERVICE_e:
  case ZypperCommand::REMOVE_REPO_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        command() == ZypperCommand::REMOVE_REPO ?
          _("Root privileges are required for modifying system repositories.") :
          _("Root privileges are required for modifying system services."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    if (_arguments.size() < 1)
    {
      out().error(_("Required argument missing."));
      ERR << "Required argument missing." << endl;
      print_usage(out(), _command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    warn_if_zmd ();

    initRepoManager();

    if (command() == ZypperCommand::REMOVE_REPO)
    {
      // must store repository before remove to ensure correct match number
      set<RepoInfo,RepoInfoAliasComparator> repo_to_remove;
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
          out().error(boost::str(format(
            // translators: %s is the supplied command line argument which
            // for which no repository counterpart was found
            _("Repository '%s' not found by alias, number or URI.")) % *it));
        }
      }

      for_(it, repo_to_remove.begin(), repo_to_remove.end())
        remove_repo(*this,*it);
    }
    else
    {
      set<repo::RepoInfoBase_Ptr, ServiceAliasComparator> to_remove;
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
          out().error(boost::str(format(
            // translators: %s is the supplied command line argument which
            // for which no service counterpart was found
            _("Service '%s' not found by alias, number or URI.")) % *it));
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
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for modifying system repositories."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    if (_arguments.size() < 2)
    {
      out().error(_("Too few arguments. At least URI and alias are required."));
      ERR << "Too few arguments. At least URI and alias are required." << endl;
      out().info(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }
    // too many arguments
    else if (_arguments.size() > 2)
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    warn_if_zmd ();
    initRepoManager();
    try {
      RepoInfo repo;
      if (match_repo(*this,_arguments[0], &repo))
      {
	rename_repo(*this, repo.alias(), _arguments[1]);
      }
      else
      {
	 out().error(boost::str(format(
           _("Repository '%s' not found.")) % _arguments[0]));
         ERR << "Repo " << _arguments[0] << " not found" << endl;
      }
    }
    catch ( const Exception & excpt_r )
    {
      out().error(excpt_r.asUserString());
      setExitCode(ZYPPER_EXIT_ERR_ZYPP);
      return;
    }

    return;
  }

  // --------------------------( modify repo )--------------------------------

  case ZypperCommand::MODIFY_REPO_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for modifying system repositories."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    bool aggregate =
        copts.count("all") || copts.count("local") ||
        copts.count("remote") || copts.count("medium-type");

    if (_arguments.size() < 1 && !aggregate)
    {
      // translators: aggregate option is e.g. "--all". This message will be
      // followed by mr command help text which will explain it
      out().error(_("Alias or an aggregate option is required."));
      ERR << "No alias argument given." << endl;
      out().info(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    // too many arguments
    if (_arguments.size() && aggregate)
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }


    initRepoManager();
    if (aggregate)
    {
      modify_repos_by_option(*this);
    }
    else
    {
      for_(arg,_arguments.begin(),_arguments.end())
      {
        RepoInfo r;
        if (match_repo(*this,*arg,&r))
        {
          modify_repo(*this, r.alias());
        }
        else
        {
          out().error(
            boost::str(format(_("Repository %s not found.")) % *arg));
          ERR << "Repo " << *arg << " not found" << endl;
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
        }
      }
    }

    break;
  }

  // --------------------------( refresh )------------------------------------

  case ZypperCommand::REFRESH_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for refreshing system repositories."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    if (globalOpts().no_refresh)
      out().warning(boost::str(format(
        _("The '%s' global option has no effect here.")) % "--no-refresh"));

    // by default refresh only repositories
    if (copts.count("services"))
    {
      if (!_arguments.empty())
      {
        out().error(str::form(
            _("Arguments are not allowed if '%s' is used."), "--services"));
        setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
        return;
      }
      // needed to be able to retrieve target distribution
      init_target(*this);
      this->_gopts.rm_options.servicesTargetDistro =
            God->target()->targetDistribution();
      initRepoManager();
      refresh_services(*this);
    }
    initRepoManager();
    refresh_repos(*this);
    break;
  }

  // --------------------------( clean )------------------------------------

  case ZypperCommand::CLEAN_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for cleaning local caches."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    initRepoManager();
    clean_repos(*this);
    break;
  }

  // --------------------------( remove/install )-----------------------------

  case ZypperCommand::INSTALL_e:
  case ZypperCommand::REMOVE_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    if (_arguments.size() < 1 && !_copts.count("entire-catalog"))
    {
      out().error(
          _("Too few arguments."),
          _("At least one package name is required."));
      out().info(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for installing or uninstalling packages."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    // rug compatibility code
    // switch on non-interactive mode if no-confirm specified
    if (copts.count("no-confirm"))
      _gopts.non_interactive = true;

    // rug compatibility code
    parsed_opts::const_iterator optit;
    if ((optit = _copts.find("entire-catalog")) != _copts.end())
    {
      if (!_arguments.empty())
        // translators: rug related message, shown if
        // 'zypper in --entire-catalog foorepo someargument' is specified
        out().warning(_("Ignoring arguments, marking the entire repository."));
      _arguments.clear();
      _arguments.push_back("*");
      _copts["from"] = _copts["entire-catalog"];
    }

    // read resolvable type
    string skind = copts.count("type")?  copts["type"].front() : "package";
    kind = string_to_kind (skind);
    if (kind == ResObject::Kind ()) {
      out().error(boost::str(format(_("Unknown package type: %s")) % skind));
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    bool install_not_remove = command() == ZypperCommand::INSTALL;

    // can't remove patch
    if (kind == ResKind::patch && !install_not_remove)
    {
      out().error(
          _("Cannot uninstall patches."),
          _("Installed status of a patch is determined solely based on its dependencies.\n"
            "Patches are not installed in sense of copied files, database records,\n"
            "or similar."));
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      throw ExitRequestException("not implemented");
    }

    // can't remove pattern (for now)
    if (kind == ResKind::pattern && !install_not_remove)
    {
      //! \todo define and implement pattern removal (bnc #407040)
      out().error(
          _("Uninstallation of a pattern is currently not defined and implemented."));
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      throw ExitRequestException("not implemented");
    }

    // parse the download options to check for errors
    get_download_option(*this);

    initRepoManager();

    // check for rpm files among the arguments
    ArgList rpms_files_caps;
    if (install_not_remove)
    {
      for (vector<string>::iterator it = _arguments.begin();
            it != _arguments.end(); )
      {
        if (looks_like_rpm_file(*it))
        {
          DBG << *it << " looks like rpm file" << endl;
          out().info(boost::str(format(
            _("'%s' looks like an RPM file. Will try to download it.")) % *it),
            Out::HIGH);

          // download the rpm into the cache
          //! \todo do we want this or a tmp dir? What about the files cached before?
          //! \todo optimize: don't mount the same media multiple times for each rpm
          Pathname rpmpath = cache_rpm(*it,
              (_gopts.root_dir != "/" ? _gopts.root_dir : "")
              + ZYPPER_RPM_CACHE_DIR);

          if (rpmpath.empty())
          {
            out().error(boost::str(format(
              _("Problem with the RPM file specified as '%s', skipping."))
              % *it));
          }
          else
          {
            using target::rpm::RpmHeader;
            // rpm header (need name-version-release)
            RpmHeader::constPtr header =
              RpmHeader::readPackage(rpmpath, RpmHeader::NOSIGNATURE);
            if (header)
            {
              string nvrcap =
                TMP_RPM_REPO_ALIAS ":" +
                header->tag_name() + "=" +
                str::numstring(header->tag_epoch()) + ":" +
                header->tag_version() + "-" +
                header->tag_release();
              DBG << "rpm package capability: " << nvrcap << endl;

              // store the rpm file capability string (name=version-release)
              rpms_files_caps.push_back(nvrcap);
            }
            else
            {
              out().error(boost::str(format(
                _("Problem reading the RPM header of %s. Is it an RPM file?"))
                  % *it));
            }
          }

          // remove this rpm argument
          it = _arguments.erase(it);
        }
        else
          ++it;
      }
    }

    // if there were some rpm files, add the rpm cache as a temporary plaindir repo
    if (!rpms_files_caps.empty())
    {
      // add a plaindir repo
      RepoInfo repo;
      repo.setType(repo::RepoType::RPMPLAINDIR);
      repo.addBaseUrl(Url("dir://"
          + (_gopts.root_dir != "/" ? _gopts.root_dir : "")
          + ZYPPER_RPM_CACHE_DIR));
      repo.setEnabled(true);
      repo.setAutorefresh(true);
      repo.setAlias(TMP_RPM_REPO_ALIAS);
      repo.setName(_("Plain RPM files cache"));
      repo.setKeepPackages(false);
      // empty packages path would cause unwanted removal of installed rpms
      // in current working directory (bnc #445504)
      // OTOH packages path == ZYPPER_RPM_CACHE_DIR (the same as repo URI)
      // causes cp file thesamefile, which fails silently. This may be worth
      // fixing in libzypp.
      repo.setPackagesPath(runtimeData().tmpdir);

      // shut up zypper
      Out::Verbosity tmp = out().verbosity();
      out().setVerbosity(Out::QUIET);

      add_repo(*this, repo);
      refresh_repo(*this, repo);

      out().setVerbosity(tmp);
    }
    // no rpms and no other arguments either
    else if (_arguments.empty())
    {
      out().error(_("No valid arguments specified."));
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    //! \todo quit here if the argument list remains empty after founding only invalid rpm args

    // prepare repositories
    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;

    if ( _rdata.repos.empty() )
    {
      out().error(_("Warning: No repositories defined."
          " Operating only with the installed resolvables."
          " Nothing can be installed."));
    }

    // prepare target
    init_target(*this);
    // load metadata
    load_resolvables(*this);
    // needed to compute status of PPP
    resolve(*this);

    // parse package arguments
    PackageArgs::Options argopts;
    if (!install_not_remove)
      argopts.do_by_default = false;
    PackageArgs args(kind, argopts);

    // tell the solver what we want

    SolverRequester::Options sropts;
    if (copts.find("force") != copts.end())
      sropts.force = true;
    sropts.force_by_cap  = copts.find("capability") != copts.end();
    sropts.force_by_name = copts.find("name") != copts.end();
    if (sropts.force)
      sropts.force_by_name = true;

    if (sropts.force_by_cap && sropts.force_by_name)
    {
      // translators: meaning --capability contradicts --force/--name
      out().error(str::form(_("%s contradicts %s"),
          "--capability", (sropts.force ? "--force" : "--name")));
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      ZYPP_THROW(ExitRequestException());
    }

    if (install_not_remove && sropts.force_by_cap && sropts.force)
    {
      // translators: meaning --force with --capability
      out().error(str::form(_("%s cannot currently be used with %s"),
          "--force", "--capability"));
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      ZYPP_THROW(ExitRequestException());
    }

    if (install_not_remove && (optit = copts.find("from")) != copts.end())
      repo_specs_to_aliases(*this, optit->second, sropts.from_repos);

    SolverRequester sr(sropts);
    if (install_not_remove)
      sr.install(args);
    else
      sr.remove(args);
    PackageArgs rpm_args(rpms_files_caps);
    sr.install(rpm_args);

    sr.printFeedback(out());

    if (sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME) ||
        sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_CAP))
    {
      setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
      if (globalOpts().non_interactive)
        ZYPP_THROW(ExitRequestException());
    }

    // give user feedback from package selection
    // TODO feedback goes here

    solve_and_commit(*this);

    break;
  }

  // -------------------( source install )------------------------------------

  case ZypperCommand::SRC_INSTALL_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    if (_arguments.size() < 1)
    {
      out().error(_("Source package name is a required argument."));
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    initRepoManager();

    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;
    init_target(*this);
    // if (!copts.count("no-build-deps")) // if target resolvables are not read, solver produces a weird result
    load_target_resolvables(*this);
    load_repo_resolvables(*this);

    if (copts.count("no-build-deps"))
      mark_src_pkgs(*this);
    else
      build_deps_install(*this);

    solve_and_commit(*this);
    break;
  }

  case ZypperCommand::VERIFY_e:
  case ZypperCommand::INSTALL_NEW_RECOMMENDS_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // too many arguments
    if (_arguments.size() > 0)
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    // rug compatibility code
    // switch on non-interactive mode if no-confirm specified
    if (copts.count("no-confirm"))
      _gopts.non_interactive = true;

    // parse the download options to check for errors
    get_download_option(*this);

    initRepoManager();

    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;

    if ( _rdata.repos.empty() )
    {
      out().error(_("Warning: No repositories defined."
          " Operating only with the installed resolvables."
          " Nothing can be installed."));
    }

    // prepare target
    init_target(*this);
    // load metadata
    load_resolvables(*this);

    solve_and_commit(*this);

    break;
  }

  // --------------------------( search )-------------------------------------

  case ZypperCommand::SEARCH_e:
  case ZypperCommand::RUG_PATCH_SEARCH_e:
  {
    if (command() == ZypperCommand::RUG_PATCH_SEARCH)
      _gopts.is_rug_compatible = true;

    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    zypp::PoolQuery query;

    TriBool inst_notinst = indeterminate;
    if (globalOpts().disable_system_resolvables || copts.count("uninstalled-only"))
    {
      query.setUninstalledOnly(); // beware: this is not all to it, look at zypper-search, _only_not_installed
      inst_notinst = false;
    }
    if (copts.count("installed-only"))
      inst_notinst = true;
    //  query.setInstalledOnly();
    //if (copts.count("match-any")) options.setMatchAny();
    if (copts.count("match-words"))
      query.setMatchWord();
    if (copts.count("match-exact"))
      query.setMatchExact();
    if (copts.count("case-sensitive"))
      query.setCaseSensitive();

    if (command() == ZypperCommand::RUG_PATCH_SEARCH)
      query.addKind(ResKind::patch);
    else if (globalOpts().is_rug_compatible)
      query.addKind(ResKind::package);
    else if (copts.count("type") > 0)
    {
      std::list<std::string>::const_iterator it;
      for (it = copts["type"].begin(); it != copts["type"].end(); ++it)
      {
        kind = string_to_kind( *it );
        if (kind == ResObject::Kind())
        {
          out().error(boost::str(format(
            _("Unknown package type '%s'.")) % *it));
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }
        query.addKind( kind );
      }
    }

    initRepoManager();

    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;

    // add available repos to query
    if (cOpts().count("repo"))
    {
      std::list<zypp::RepoInfo>::const_iterator repo_it;
      for (repo_it = _rdata.repos.begin();repo_it != _rdata.repos.end();++repo_it){
        query.addRepo( repo_it->alias());
        if (! repo_it->enabled())
        {
          out().warning(boost::str(format(
            _("Specified repository '%s' is disabled."))
              % (config().show_alias ? repo_it->alias() : repo_it->name())));
        }
      }
    }

    for(vector<string>::const_iterator it = _arguments.begin();
        it != _arguments.end(); ++it)
    {
      query.addString(*it);
      if (!query.matchGlob() && it->find_first_of("?*") != string::npos)
        query.setMatchGlob();
    }
    query.addAttribute(sat::SolvAttr::name);
    if (cOpts().count("search-descriptions"))
    {
      query.addAttribute(sat::SolvAttr::summary);
      query.addAttribute(sat::SolvAttr::description);
    }

    init_target(*this);

    // now load resolvables:
    load_resolvables(*this);
    // needed to compute status of PPP
    resolve(*this);

    Table t;
    t.lineStyle(Ascii);

    try
    {
      if (command() == ZypperCommand::RUG_PATCH_SEARCH)
      {
        FillPatchesTable callback(t, inst_notinst);
        invokeOnEach(query.poolItemBegin(), query.poolItemEnd(), callback);
      }
      else if (_gopts.is_rug_compatible || _copts.count("details"))
      {
        FillSearchTableSolvable callback(t, inst_notinst);
        invokeOnEach(query.selectableBegin(), query.selectableEnd(), callback);
      }
      else
      {
        FillSearchTableSelectable callback(t, inst_notinst);
        invokeOnEach(query.selectableBegin(), query.selectableEnd(), callback);
      }

      if (t.empty())
        out().info(_("No packages found."), Out::QUIET);
      else
      {
        cout << endl; //! \todo  out().separator()?

        if (command() == ZypperCommand::RUG_PATCH_SEARCH)
        {
          if (copts.count("sort-by-catalog") || copts.count("sort-by-repo"))
            t.sort(1);
          else
            t.sort(3); // sort by name
        }
        else if (_gopts.is_rug_compatible)
        {
          if (copts.count("sort-by-catalog") || copts.count("sort-by-repo"))
            t.sort(1);
          else
            t.sort(3); // sort by name
        }
        else if (_copts.count("details"))
        {
          if (copts.count("sort-by-catalog") || copts.count("sort-by-repo"))
            t.sort(5);
          else
            t.sort(1); // sort by name
        }
        else
        {
          // sort by name (can't sort by repo)
          t.sort(1);
          if (!globalOpts().no_abbrev)
            t.allowAbbrev(2);
        }

	//cout << t; //! \todo out().table()?
	out().searchResult( t );
      }
    }
    catch (const Exception & e)
    {
      out().error(e,
        _("Problem occurred initializing or executing the search query") + string(":"),
        string(_("See the above message for a hint.")) + " " +
          _("Running 'zypper refresh' as root might resolve the problem."));
      setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    }

    break;
  }

  // --------------------------( patch check )--------------------------------

  // TODO: rug summary
  case ZypperCommand::PATCH_CHECK_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // too many arguments
    if (_arguments.size() > 0)
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    initRepoManager();

    init_target(*this);
    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;

    // TODO calc token?

    // now load resolvables:
    load_resolvables(*this);
    // needed to compute status of PPP
    resolve(*this);

    patch_check();

    if (_rdata.security_patches_count > 0)
    {
      setExitCode(ZYPPER_EXIT_INF_SEC_UPDATE_NEEDED);
      return;
    }
    if (_rdata.patches_count > 0)
    {
      setExitCode(ZYPPER_EXIT_INF_UPDATE_NEEDED);
      return;
    }

    break;
  }

  // --------------------------( misc queries )--------------------------------

  case ZypperCommand::PATCHES_e:
  case ZypperCommand::PATTERNS_e:
  case ZypperCommand::PACKAGES_e:
  case ZypperCommand::PRODUCTS_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    initRepoManager();

    init_target(*this);
    init_repos(*this, _arguments);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;
    load_resolvables(*this);
    // needed to compute status of PPP
    resolve(*this);

    switch (command().toEnum())
    {
    case ZypperCommand::PATCHES_e:
      list_patches(*this);
      break;
    case ZypperCommand::PATTERNS_e:
      list_patterns(*this);
      break;
    case ZypperCommand::PACKAGES_e:
      list_packages(*this);
      break;
    case ZypperCommand::PRODUCTS_e:
      list_products(*this);
      break;
    default:;
    }

    break;
  }

  case ZypperCommand::WHAT_PROVIDES_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    if (_arguments.empty())
    {
      report_required_arg_missing(out(), _command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }
    else if (_arguments.size() > 1)
    {
      report_too_many_arguments(out(), _command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    initRepoManager();

    init_target(*this);
    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;
    load_resolvables(*this);

    switch (command().toEnum())
    {
    case ZypperCommand::WHAT_PROVIDES_e:
      list_what_provides(*this, _arguments[0]);
      break;
    default:;
    }

    break;
  }

  // --------------------------( list updates )-------------------------------

  case ZypperCommand::LIST_UPDATES_e:
  case ZypperCommand::LIST_PATCHES_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // too many arguments
    if (_arguments.size() > 0)
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    ResKindSet kinds;
    if (copts.count("type"))
    {
      std::list<std::string>::const_iterator it;
      for (it = copts["type"].begin(); it != copts["type"].end(); ++it)
      {
        kind = string_to_kind( *it );
        if (kind == ResObject::Kind())
        {
          out().error(boost::str(format(
            _("Unknown package type '%s'.")) % *it));
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }
        kinds.insert(kind);
      }
    }
    else if (command() == ZypperCommand::LIST_PATCHES)
      kinds.insert(ResKind::patch);
    else
      kinds.insert(ResKind::package);

    //! \todo drop this option - it's the default for packages now, irrelevant
    //! for patches; just test with products and patterns
    bool best_effort = copts.count( "best-effort" );

    if (globalOpts().is_rug_compatible && best_effort)
    {
      best_effort = false;
      // translators: Running as 'rug', cannot use 'best-effort' option.
      out().info(str::form(
        _("Running as '%s', cannot use '%s' option."), "rug", "best-effort"),
        Out::HIGH);
    }

    if ((copts.count("bugzilla") || copts.count("bz") || copts.count("cve")) &&
        copts.count("issues"))
    {
      out().error(str::form(
        _("Cannot use %s together with %s."), "--issues", "--bz, --cve"));
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    initRepoManager();
    init_target(*this);
    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;
    load_resolvables(*this);
    resolve(*this);

    if (copts.count("bugzilla") || copts.count("bz")
        || copts.count("cve") || copts.count("issues"))
      list_patches_by_issue(*this);
    else
      list_updates(*this, kinds, best_effort);

    break;
  }

  // -----------------------------( update )----------------------------------

  case ZypperCommand::UPDATE_e:
  case ZypperCommand::PATCH_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for updating packages."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    // too many arguments
    if (!_arguments.empty() && command() == ZypperCommand::PATCH)
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    // rug compatibility code
    // switch on non-interactive mode if no-confirm specified
    if (copts.count("no-confirm"))
      _gopts.non_interactive = true;

    bool skip_interactive = false;
    if (copts.count("skip-interactive"))
    {
      if (copts.count("with-interactive"))
      {
        out().error(str::form(_("%s contradicts %s"), "--with-interactive", "--skip-interactive"));
        setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
        return;
      }
      skip_interactive = true;
    }
    // bnc #497711
    else if (globalOpts().non_interactive && !copts.count("with-interactive"))
      skip_interactive = true;
    MIL << "Skipping interactive patches: " << (skip_interactive ? "yes" : "no") << endl;

    ResKindSet kinds;
    if (copts.count("type"))
    {
      std::list<std::string>::const_iterator it;
      for (it = copts["type"].begin(); it != copts["type"].end(); ++it)
      {
        kind = string_to_kind( *it );
        if (kind == ResObject::Kind())
        {
          out().error(boost::str(format(
            _("Unknown package type '%s'.")) % *it));
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }

        if (kind == ResKind::product)
        {
          out().error(_("Operation not supported."),
              str::form(_("To update installed products use '%s'."),
                  "zypper dup [--from <repo>]"));
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }
        else if (kind == ResKind::srcpackage)
        {
          out().error(_("Operation not supported."),
              str::form(
                  _("Zypper does not keep track of installed source"
                    " packages. To install the latest source package and"
                    " its build dependencies, use '%s'."),
                  "zypper dup [--from <repo>]"));
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }

        kinds.insert(kind);
      }
    }
    else if (command() == ZypperCommand::PATCH)
      kinds.insert(ResKind::patch);
    else
      kinds.insert(ResKind::package);

    if (!arguments().empty() && kinds.size() > 1)
    {
      out().error(_("Cannot use multiple types when specific packages are given as arguments."));
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    bool best_effort = copts.count( "best-effort" );
    if (globalOpts().is_rug_compatible && best_effort)
    {
      best_effort = false;
      out().info(str::form(
        // translators: Running as 'rug', cannot use 'best-effort' option.
        _("Running as '%s', cannot use '%s' option."), "rug", "best-effort"),
        Out::HIGH);
    }

    // parse the download options to check for errors
    get_download_option(*this);

    init_target(*this);
    initRepoManager();

    // rug compatibility - treat arguments as repos
    if (_gopts.is_rug_compatible && !_arguments.empty())
      init_repos(*this, _arguments);
    else
      init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;

    load_resolvables(*this);
    resolve(*this); // needed to compute status of PPP


    // patch --bugzilla/--cve
    if (copts.count("bugzilla") || copts.count("bz") || copts.count("cve"))
      mark_updates_by_issue(*this);
    // update without arguments
    else
    {
      SolverRequester::Options sropts;
      if (copts.find("force") !=
copts.end())
        sropts.force = true;
      sropts.best_effort = best_effort;
      sropts.skip_interactive = skip_interactive; // bcn #647214

      // if --category is specified
      {
	parsed_opts::const_iterator optit;
	optit = copts.find("category");
	if (optit != copts.end())
	{
	  for_(i, optit->second.begin(), optit->second.end())
	  {
	    sropts.category = *i;
	    break;
	  }
	}
      }

      // if --date is specified
      {
        parsed_opts::const_iterator optit;
        optit = copts.find("date");
        if (optit != copts.end())
        {
          for_(i, optit->second.begin(), optit->second.end())
          {
            // ISO 8601 format
            sropts.date_limit = Date(*i, "%F");
            break;
          }
        }

      }

      SolverRequester sr(sropts);
      if (arguments().empty())
      {
        for_(kit, kinds.begin(), kinds.end())
        {
          if (*kit == ResKind::package)
          {
            MIL << "Computing package update..." << endl;
            // this will do a complete pacakge update as far as possible
            // while respecting solver policies
            zypp::getZYpp()->resolver()->doUpdate();
            // no need to call Resolver::resolvePool() afterwards
            runtimeData().solve_before_commit = false;
          }
          // update -t patch; patch
          else if (*kit == ResKind::patch)
            sr.updatePatches();
          else if (*kit == ResKind::pattern)
            sr.updatePatterns();
          // should not get here (see above kind parsing code), but just in case
          else
          {
            out().error(_("Operation not supported."));
            setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
            return;
          }
        }
      }
      // update with arguments
      else
      {
        PackageArgs args(*kinds.begin());
        sr.update(args);
      }

      sr.printFeedback(out());

      if (sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME) ||
          sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_CAP))
      {
        setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
        if (globalOpts().non_interactive)
          ZYPP_THROW(ExitRequestException());
      }
    }

    solve_and_commit(*this);

    break;
  }

  // ----------------------------( dist-upgrade )------------------------------

  case ZypperCommand::DIST_UPGRADE_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for performing a distribution upgrade."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    // too many arguments
    if (_arguments.size() > 0)
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    // parse the download options to check for errors
    get_download_option(*this);

    initRepoManager();

    if (!copts.count("repo") && !copts.count("from")
        && repoManager().knownRepositories().size() > 1)
      this->out().warning(str::form(_(
        "You are about to do a distribution upgrade with all enabled"
        " repositories. Make sure these repositories are compatible before you"
        " continue. See '%s' for more information about this command."),
        "man zypper"));

    init_target(*this);
    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;
    load_resolvables(*this);

    solve_and_commit(*this);

    break;
  }

  // -----------------------------( info )------------------------------------

  case ZypperCommand::INFO_e:
  case ZypperCommand::RUG_PATCH_INFO_e:
  case ZypperCommand::RUG_PATTERN_INFO_e:
  case ZypperCommand::RUG_PRODUCT_INFO_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    if (_arguments.size() < 1)
    {
      out().error(_("Required argument missing."));
      ERR << "Required argument missing." << endl;
      ostringstream s;
      s << _("Usage") << ':' << endl;
      s << _command_help;
      out().info(s.str());
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    switch (command().toEnum())
    {
    case ZypperCommand::RUG_PATCH_INFO_e:
      kind =  ResKind::patch;
      break;
    case ZypperCommand::RUG_PATTERN_INFO_e:
      kind =  ResKind::pattern;
      break;
    case ZypperCommand::RUG_PRODUCT_INFO_e:
      kind =  ResKind::product;
      break;
    default:
    case ZypperCommand::INFO_e:
      // read resolvable type
      string skind = copts.count("type")?  copts["type"].front() : "package";
      kind = string_to_kind (skind);
      if (kind == ResObject::Kind ()) {
        out().error(boost::str(format(
          _("Unknown package type '%s'.")) % skind));
        setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
        return;
      }
    }

    // XXX: would requires/recommends make sense for pattern etc.?
    if (copts.count("requires") || copts.count("recommends"))
    {
      if (kind != ResKind::package && kind != ResKind::patch)
      {
        out().error(boost::str(format(
          _("Type '%s' does not support %s.")) % kind % "--requires/--recommends"));
        setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
        return;
      }
    }

    initRepoManager();
    init_target(*this);
    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;
    load_resolvables(*this);
    // needed to compute status of PPP
    resolve(*this);

    printInfo(*this, kind);

    return;
  }

  // -----------------------------( locks )------------------------------------

  case ZypperCommand::ADD_LOCK_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for adding of package locks."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    // too few arguments
    if (_arguments.empty())
    {
      report_required_arg_missing(out(), _command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }
    // too many arguments
    //TODO rug compatibility
/*    else if (_arguments.size() > 1)
    {
      // rug compatibility
      if (_gopts.is_rug_compatible)
        // translators: 'zypper addlock foo' takes only one argument.
        out().warning(_("Only the first command argument considered. Zypper currently does not support versioned locks."));
      else
      {
        report_too_many_arguments(_command_help);
        setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
        return;
      }
    }*/

    ResKindSet kinds;
    if (copts.count("type"))
    {
      std::list<std::string>::const_iterator it;
      for (it = copts["type"].begin(); it != copts["type"].end(); ++it)
      {
        kind = string_to_kind( *it );
        if (kind == ResObject::Kind())
        {
          out().error(boost::str(format(
            _("Unknown package type '%s'.")) % *it));
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }
        kinds.insert(kind);
      }
    }
    //else
    //  let add_locks determine the appropriate type (bnc #551956)

    add_locks(*this, _arguments, kinds);

    break;
  }

  case ZypperCommand::REMOVE_LOCK_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    // check root user
    if (geteuid() != 0 && !globalOpts().changedRoot)
    {
      out().error(
        _("Root privileges are required for adding of package locks."));
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    if (_arguments.size() < 1)
    {
      report_required_arg_missing(out(), _command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    initRepoManager();
    init_target(*this);
    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;
    load_resolvables(*this);

    remove_locks(*this, _arguments);

    break;
  }

  case ZypperCommand::LIST_LOCKS_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    list_locks(*this);

    break;
  }

  case ZypperCommand::CLEAN_LOCKS_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    initRepoManager();
    init_target(*this);
    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;
    load_resolvables(*this);

    Locks::instance().read();
    Locks::size_type start = Locks::instance().size();
    if ( !copts.count("only-duplicate") )
      Locks::instance().removeEmpty();
    if ( !copts.count("only-empty") )
      Locks::instance().removeDuplicates();

    Locks::instance().save();

    Locks::size_type diff = start - Locks::instance().size();
    out().info(str::form(
        _PL("Removed %lu lock.","Removed %lu locks.", diff),
        (long unsigned) diff));

    break;
  }

  // ----------------------------(utils/others)--------------------------------

  case ZypperCommand::TARGET_OS_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    if (out().type() == Out::TYPE_XML)
    {
      out().error("XML output not implemented for this command.");
      break;
    }

    if (copts.find("label") != copts.end())
    {
      if (globalOpts().terse)
      {
        cout << "labelLong\t" << str::escape(Target::distributionLabel(globalOpts().root_dir).summary, '\t') << endl;
        cout << "labelShort\t" << str::escape(Target::distributionLabel(globalOpts().root_dir).shortName, '\t') << endl;
      }
      else
      {
        out().info(str::form(_("Distribution Label: %s"),
          Target::distributionLabel(globalOpts().root_dir).summary.c_str()));
        out().info(str::form(_("Short Label: %s"),
          Target::distributionLabel(globalOpts().root_dir).shortName.c_str()));
      }
    }
    else
      out().info(Target::targetDistribution(globalOpts().root_dir));

    break;
  }

  case ZypperCommand::VERSION_CMP_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    if (_arguments.size() < 2)
    {
      report_required_arg_missing(out(), _command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }
    else if (_arguments.size() > 2)
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    Edition lhs(_arguments[0]);
    Edition rhs(_arguments[1]);

    int result;

    if (_copts.count("match"))
      result = lhs.match(rhs);
    else
      result = lhs.compare(rhs);

    // be terse when talking to machines
    if (_gopts.terse)
    {
      out().info(str::numstring(result));
      break;
    }

    // tell a human
    if (result == 0)
      out().info(str::form(_("%s matches %s"), lhs.asString().c_str(), rhs.asString().c_str()));
    else if (result > 0)
      out().info(str::form(_("%s is newer than %s"), lhs.asString().c_str(), rhs.asString().c_str()));
    else
      out().info(str::form(_("%s is older than %s"), lhs.asString().c_str(), rhs.asString().c_str()));

    break;
  }

  case ZypperCommand::LICENSES_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    if (!_arguments.empty())
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    initRepoManager();
    init_repos(*this);
    if (exitCode() != ZYPPER_EXIT_OK)
      return;
    init_target(*this);
    // now load resolvables:
    load_resolvables(*this);
    // needed to compute status of PPP
    resolve(*this);

    report_licenses(*this);

    break;
  }


  case ZypperCommand::PS_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    if (!_arguments.empty())
    {
      report_too_many_arguments(_command_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    list_processes_using_deleted_files(*this);

    break;
  }


  // -----------------------------( shell )------------------------------------

  case ZypperCommand::SHELL_QUIT_e:
  {
    if (runningHelp())
      out().info(_command_help, Out::QUIET);
    else if (!runningShell())
      out().warning(
        _("This command only makes sense in the zypper shell."), Out::QUIET);
    else
      out().error("oops, you wanted to quit, didn't you?"); // should not happen

    break;
  }

  case ZypperCommand::SHELL_e:
  {
    if (runningHelp())
      out().info(_command_help, Out::QUIET);
    else if (runningShell())
      out().info(_("You already are running zypper's shell."));
    else
    {
      out().error(_("Unexpected program flow."));
      report_a_bug(out());
    }

    break;
  }

  case ZypperCommand::RUG_SERVICE_TYPES_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }

    Table t;

    TableHeader th;
    th << _("Alias") << _("Name") << _("Description");
    t << th;

    { TableRow tr; tr << "yum" << "YUM" << "YUM server service"; t << tr; } // rpm-md
    { TableRow tr; tr << "yast" << "YaST2" << "YaST2 repository"; t << tr; }
    { TableRow tr; tr << "zypp" << "ZYPP" << "ZYpp installation repository"; t << tr; }
    { TableRow tr; tr << "mount" << "Mount" << "Mount a directory of RPMs"; t << tr; }
    { TableRow tr; tr << "plaindir" << "Plaindir" << "Mount a directory of RPMs"; t << tr; }
    { TableRow tr; tr << "nu" << "NU" << "Novell Updates service"; t << tr; } // ris

    cout << t;

    break;
  }

  case ZypperCommand::RUG_LIST_RESOLVABLES_e:
  {
    if (runningHelp()) { out().info(_command_help, Out::QUIET); return; }
    rug_list_resolvables(*this);
    break;
  }

  // dummy commands
  case ZypperCommand::RUG_PING_e:
  {
    break;
  }

  default:
    // if the program reaches this line, something went wrong
    setExitCode(ZYPPER_EXIT_ERR_BUG);
  }
}

void Zypper::cleanup()
{
  MIL << "START" << endl;

  // remove the additional repositories specified by --plus-repo
  for (list<RepoInfo>::const_iterator it = _rdata.additional_repos.begin();
         it != _rdata.additional_repos.end(); ++it)
    remove_repo(*this, *it);

  // remove tmprpm cache repo
  for_(it, _rdata.repos.begin(), _rdata.repos.end())
    if (it->alias() == TMP_RPM_REPO_ALIAS)
    {
      // shut up zypper
      Out::Verbosity tmp = out().verbosity();
      out().setVerbosity(Out::QUIET);

      remove_repo(*this, *it);

      out().setVerbosity(tmp);
      break;
    }
}

void rug_list_resolvables(Zypper & zypper)
{
  Table t;

  TableHeader th;
  th << _("Resolvable Type");
  t << th;

  { TableRow tr; tr << "package"; t << tr; }
  { TableRow tr; tr << "patch"; t << tr; }
  { TableRow tr; tr << "pattern"; t << tr; }
  { TableRow tr; tr << "product"; t << tr; }

  cout << t;
}


// Local Variables:
// c-basic-offset: 2
// End:
