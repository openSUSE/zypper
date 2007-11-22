// zypper - a command line interface for libzypp
// http://en.opensuse.org/User:Mvidner

// (initially based on dmacvicar's zmart)

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
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
#include "zypp/base/Logger.h"

#include "zypp/base/UserRequestException.h"
#include "zypp/repo/RepoException.h"
#include "zypp/zypp_detail/ZYppReadOnlyHack.h"

#include "zypper-main.h"
#include "zypper.h"
#include "zypper-repos.h"
#include "zypper-misc.h"

#include "zypper-tabulator.h"
#include "zypper-search.h"
#include "zypper-info.h"
#include "zypper-getopt.h"
#include "zypper-command.h"
#include "zypper-utils.h"
#include "zypper-callbacks.h"

using namespace std;
using namespace zypp;
using namespace zypp::detail;
using namespace boost;

ZYpp::Ptr God = NULL;
RuntimeData gData;
Settings gSettings;
parsed_opts gopts; // global options
parsed_opts copts; // command options
ZypperCommand command(ZypperCommand::NONE);
bool ghelp = false;


Zypper::Zypper()
  : _argc(0), _argv(NULL),
    _command(ZypperCommand::NONE),
    _exit_code(ZYPPER_EXIT_OK), _running_shell(false)
{
  MIL << "Hi, me zypper " VERSION " built " << __DATE__ << " " <<  __TIME__ << endl;
}


Zypper::~Zypper()
{
  MIL << "Bye!" << endl;
}


int Zypper::main(int argc, char ** argv)
{
  _argc = argc;
  _argv = argv;

  // parse global options and the command
  processGlobalOptions();
  if (exitCode() != ZYPPER_EXIT_OK)
    return exitCode();

  switch(command().toEnum())
  {
  case ZypperCommand::SHELL_e:
    commandShell();
    return ZYPPER_EXIT_OK;

  case ZypperCommand::NONE_e:
  {
    if (ghelp)
      return ZYPPER_EXIT_OK;
    else
      return ZYPPER_EXIT_ERR_SYNTAX;
  }

  default:
    safeDoCommand();
    return exitCode();
  }

  WAR << "This line should never be reached." << endl;

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
  static struct option global_options[] = {
    {"help",            no_argument,       0, 'h'},
    {"verbose",         no_argument,       0, 'v'},
    {"quiet",           no_argument,       0, 'q'},
    {"version",         no_argument,       0, 'V'},
    {"terse",           no_argument,       0, 't'},
    {"table-style",     required_argument, 0, 's'},
    {"rug-compatible",  no_argument,       0, 'r'},
    {"non-interactive", no_argument,       0, 'n'},
    {"no-gpg-checks",   no_argument,       0,  0 },
    {"root",            required_argument, 0, 'R'},
    {"reposd-dir",      required_argument, 0, 'D'},
    {"cache-dir",       required_argument, 0, 'C'},
    {"raw-cache-dir",   required_argument, 0,  0 },
    {"opt",             optional_argument, 0, 'o'},
    {"disable-system-resolvables", optional_argument, 0, 'o'},
    {0, 0, 0, 0}
  };

  // parse global options
  gopts = _gopts = parse_options (_argc, _argv, global_options);
  if (gOpts().count("_unknown"))
  {
    _exit_code = ZYPPER_EXIT_ERR_SYNTAX;
    return;
  }

  static string help_global_options = _("  Options:\n"
    "\t--help, -h\t\tHelp.\n"
    "\t--version, -V\t\tOutput the version number.\n"
    "\t--quiet, -q\t\tSuppress normal output, print only error messages.\n"
    "\t--verbose, -v\t\tIncrease verbosity.\n"
    "\t--terse, -t\t\tTerse output for machine consumption.\n"
    "\t--table-style, -s\tTable style (integer).\n"
    "\t--rug-compatible, -r\tTurn on rug compatibility.\n"
    "\t--non-interactive, -n\tDon't ask anything, use default answers automatically.\n"
    "\t--no-gpg-checks\t\tIgnore GPG check failures and continue.\n"
    "\t--root, -R <dir>\tOperate on a different root directory.\n"
    "\t--reposd-dir, D <dir>\tUse alternative repository definition files directory.\n"
    "\t--cache-dir, C <dir>\tUse alternative meta-data cache database directory.\n"
    "\t--raw-cache-dir <dir>\tUse alternative raw meta-data cache directory\n"
  );

  if (gopts.count("rug-compatible"))
    gSettings.is_rug_compatible = true;

  // Help is parsed by setting the help flag for a command, which may be empty
  // $0 -h,--help
  // $0 command -h,--help
  // The help command is eaten and transformed to the help option
  // $0 help
  // $0 help command
  if (gopts.count("help"))
    ghelp = true;

  if (gopts.count("quiet")) {
    gSettings.verbosity = -1;
    DBG << "Verbosity " << gSettings.verbosity << endl;
  }

  if (gopts.count("verbose")) {
    gSettings.verbosity += gopts["verbose"].size();
    cout << format(_("Verbosity: %d")) % gSettings.verbosity << endl;
    DBG << "Verbosity " << gSettings.verbosity << endl;
  }

  if (gopts.count("non-interactive")) {
    gSettings.non_interactive = true;
    cout_n << _("Entering non-interactive mode.") << endl;
    MIL << "Entering non-interactive mode" << endl;
  }

  if (gopts.count("no-gpg-checks")) {
    gSettings.no_gpg_checks = true;
    cout_n << _("Entering no-gpg-checks mode.") << endl;
    MIL << "Entering no-gpg-checks mode" << endl;
  }

  if (gopts.count("table-style")) {
    unsigned s;
    str::strtonum (gopts["table-style"].front(), s);
    if (s < _End)
      Table::defaultStyle = (TableStyle) s;
    else
      cerr << _("Invalid table style ") << s << endl;
  }

  if (gopts.count("root")) {
    gSettings.root_dir = gopts["root"].front();
    Pathname tmp(gSettings.root_dir);
    if (!tmp.absolute())
    {
      cerr << _("The path specified in the --root option must be absolute.") << endl;
      _exit_code = ZYPPER_EXIT_ERR_INVALID_ARGS;
      return;
    }

    DBG << "root dir = " << gSettings.root_dir << endl;
    gSettings.rm_options.knownReposPath = gSettings.root_dir
      + gSettings.rm_options.knownReposPath;
    gSettings.rm_options.repoCachePath = gSettings.root_dir
      + gSettings.rm_options.repoCachePath;
    gSettings.rm_options.repoRawCachePath = gSettings.root_dir
      + gSettings.rm_options.repoRawCachePath;
  }

  if (gopts.count("reposd-dir")) {
    gSettings.rm_options.knownReposPath = gopts["reposd-dir"].front();
  }

  if (gopts.count("cache-dir")) {
    gSettings.rm_options.repoCachePath = gopts["cache-dir"].front();
  }

  if (gopts.count("raw-cache-dir")) {
    gSettings.rm_options.repoRawCachePath = gopts["raw-cache-dir"].front();
  }

  DBG << "repos.d dir = " << gSettings.rm_options.knownReposPath << endl;
  DBG << "cache dir = " << gSettings.rm_options.repoCachePath << endl;
  DBG << "raw cache dir = " << gSettings.rm_options.repoRawCachePath << endl;

  // testing option
  if (gopts.count("opt")) {
    cout << "Opt arg: ";
    std::copy (gopts["opt"].begin(), gopts["opt"].end(),
               ostream_iterator<string> (cout, ", "));
    cout << endl;
  }

  static string help_commands = _(
    "  Commands:\n"
    "\thelp, ?\t\t\tHelp\n"
    "\tshell, sh\t\tAccept multiple commands at once\n"
    "\tinstall, in\t\tInstall packages or resolvables\n"
    "\tremove, rm\t\tRemove packages or resolvables\n"
    "\tsearch, se\t\tSearch for packages matching a pattern\n"
    "\trepos, lr\t\tList all defined repositories.\n"
    "\taddrepo, ar\t\tAdd a new repository\n"
    "\tremoverepo, rr\t\tRemove specified repository\n"
    "\trenamerepo, nr\t\tRename specified repository\n"
    "\tmodifyrepo, mr\t\tModify specified repository\n"
    "\trefresh, ref\t\tRefresh all repositories\n"
    "\tpatch-check, pchk\tCheck for patches\n"
    "\tpatches, pch\t\tList patches\n"
    "\tlist-updates, lu\tList updates\n"
    "\txml-updates, xu\t\tList updates and patches in xml format\n"
    "\tupdate, up\t\tUpdate installed resolvables with newer versions.\n"
    "\tinfo, if\t\tShow full information for packages\n"
    "\tpatch-info\t\tShow full information for patches\n"
    "\tsource-install, si\tInstall a source package\n"
    "");

  // get command
  try
  {
    if (optind < _argc)
      ::command = _command = ZypperCommand(_argv[optind++]);

    if (_command == ZypperCommand::HELP)
    {
      ghelp = true;
      if (optind < _argc)
        _command = ZypperCommand(_argv[optind++]);
      else
        _command = ZypperCommand::NONE;
    }
  }
  // exception from command parsing
  catch (Exception & e)
  {
    cerr << e.msg() << endl;
    _command = ZypperCommand::NONE;
  }

  if (_command == ZypperCommand::NONE)
  {
    if (ghelp)
      cout << help_global_options << endl << help_commands;
    else if (gopts.count("version"))
      cout << PACKAGE " " VERSION << endl;
    else
    {
      cerr << _("Try -h for help.") << endl;
      _exit_code = ZYPPER_EXIT_ERR_SYNTAX;
      return;
    }
  }

  _exit_code = ZYPPER_EXIT_OK;
}


void Zypper::commandShell()
{
  _running_shell = true;

  string histfile;
  try {
    Pathname p (getenv ("HOME"));
    p /= ".zypper_history";
    histfile = p.asString ();
  } catch (...) {
    // no history
  }
  using_history ();
  if (!histfile.empty ())
    read_history (histfile.c_str ());

  bool loop = true;
  while (loop) {
    // reset globals
    ghelp = false;
    
    // read a line
    string line = readline_getline ();
    cerr_vv << "Got: " << line << endl;
    // reset optind etc
    optind = 0;
    // split it up and create sh_argc, sh_argv
    Args args(line);
    _sh_argc = args.argc();
    _sh_argv = args.argv();

    string command_str = _sh_argv[0] ? _sh_argv[0] : "";

    if (command_str == "\004") // ^D
    {
      loop = false;
      cout << endl; // print newline after ^D
    }
    else
    {
      try
      {
        ::command = _command = ZypperCommand(command_str);
        if (_command == ZypperCommand::SHELL_QUIT)
          loop = false;
        else if (_command == ZypperCommand::SHELL)
          cout << _("You already are running zypper's shell.") << endl;
        else
          safeDoCommand();
      }
      catch (Exception & e)
      {
        cerr << e.msg() <<  endl;
      }
    }
  }

  if (!histfile.empty ())
    write_history (histfile.c_str ());
}


/// process one command from the OS shell or the zypper shell
// catch unexpected exceptions and tell the user to report a bug (#224216)
void Zypper::safeDoCommand()
{
  try
  {
    doCommand();
    ::command = _command = ZypperCommand::NONE;
  }
  catch (const AbortRequestException & ex)
  {
    ZYPP_CAUGHT(ex);

    //cerr << _("User requested to abort.") << endl;
    cerr << ex.asUserString() << endl;
  }
  catch (const Exception & ex) {
    ZYPP_CAUGHT(ex);

    cerr << _("Unexpected exception.") << endl;
    cerr << ex.asUserString() << endl;
        report_a_bug(cerr);
  }

  if ( gSettings.machine_readable )
    cout << "</stream>" << endl;
}

/// process one command from the OS shell or the zypper shell
void Zypper::doCommand()
{
  // === command-specific options ===

  struct option no_options = {0, 0, 0, 0};
  struct option *specific_options = &no_options;
  string specific_help;

/*  string help_global_source_options = _(
      "  Repository options:\n"
      "\t--disable-repositories, -D\t\tDo not read data from defined repositories.\n"
      "\t--plus-repo <URI|.repo>\t\tRead additional repository\n" //! \todo additional repo
      );*/
//! \todo preserve for rug comp.  "\t--disable-system-sources, -D\t\tDo not read the system sources\n"
//! \todo preserve for rug comp.  "\t--source, -S\t\tRead additional source\n"

//  string help_global_target_options = _("  Target options:\n"
//      "\t--disable-system-resolvables, -T\t\tDo not read system installed resolvables\n"
//      );

  if ( (command() == ZypperCommand::HELP) && (argc() > 1) )
  try {
    ghelp = true;
    command() = ZypperCommand(argv()[1]);
  }
  catch (Exception & ex) {
    // in case of an unknown command specified to help, an exception is thrown
  }

  if (command() == ZypperCommand::HELP)
  {
    cout << _("Type 'zypper help' to get a list of global options and commands.") << endl;
    cout << _("Type 'zypper help <command>' to get a command-specific help.") << endl;
    setExitCode(ZYPPER_EXIT_OK);
    return;
  }
  else if (command() == ZypperCommand::INSTALL) {
    static struct option install_options[] = {
      {"repo",                      required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog",                   required_argument, 0, 'c'},
      {"type",                      required_argument, 0, 't'},
      // the default (ignored)
      {"name",                      no_argument,       0, 'n'},
      {"force",                     no_argument,       0, 'f'},
      {"capability",                no_argument,       0, 'C'},
      // rug compatibility, we have global --non-interactive
      {"no-confirm",                no_argument,       0, 'y'},
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      // rug compatibility, we have --auto-agree-with-licenses
      {"agree-to-third-party-licenses",  no_argument,  0, 0},
      {"debug-solver",              no_argument,       0, 0},
      {"force-resolution",          no_argument,       0, 'R'},
      {"help",                      no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = install_options;
    specific_help = _(
      // TranslatorExplanation don't translate the resolvable types
      // (package, patch, pattern, product) or at least leave also their
      // originals, since they are expected untranslated on the command line
      "install (in) [options] <capability> ...\n"
      "\n"
      "Install resolvables with specified capabilities. A capability is"
      " NAME[OP<VERSION>], where OP is one of <, <=, =, >=, >.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias>              Install resolvables only from repository specified by alias.\n"
      "-t, --type <type>               Type of resolvable (package, patch, pattern, product) (default: package)\n"
      "-n, --name                      Select resolvables by plain name, not by capability\n"
      "-C, --capability                Select resolvables by capability\n"
      "-f, --force                     Install even if the item is already installed (reinstall)\n"
      "-l, --auto-agree-with-licenses  Automatically say 'yes' to third party license confirmation prompt.\n"
      "                                See 'man zypper' for more details.\n"
      "    --debug-solver              Create solver test case for debugging\n"
      "-R, --force-resolution          Force the solver to find a solution (even agressive)\n"
    );
  }
  else if (command() == ZypperCommand::REMOVE) {
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
      {"force-resolution", no_argument, 0, 'R'},
      {"help",       no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = remove_options;
    specific_help = _(
      // TranslatorExplanation don't translate the resolvable types
      // (see the install command comment) 
      "remove (rm) [options] <capability> ...\n"
      "\n"
      "Remove resolvables with specified capabilities. A capability is"
      " NAME[OP<VERSION>], where OP is one of <, <=, =, >=, >.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias>     Operate only with resolvables from repository specified by alias.\n"
      "-t, --type <type>      Type of resolvable (package, patch, pattern, product) (default: package)\n"
      "-n, --name             Select resolvables by plain name, not by capability\n"
      "-C, --capability       Select resolvables by capability\n"
      "    --debug-solver     Create solver test case for debugging\n"  
      "-R, --force-resolution Force the solver to find a solution (even agressive)\n"
    );
  }
  else if (command() == ZypperCommand::SRC_INSTALL) {
    static struct option src_install_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = src_install_options;
    specific_help = _(
      "source-install (si) <name> ...\n"
      "\n"
      "Install source packages specified by their names.\n"
      "\n"
      "This command has no additional options.\n"
    );
  }
  else if (command() == ZypperCommand::ADD_REPO) {
    static struct option service_add_options[] = {
      {"type", required_argument, 0, 't'},
      {"disabled", no_argument, 0, 'd'},
      {"no-refresh", no_argument, 0, 'n'},
      {"repo", required_argument, 0, 'r'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_add_options;
    // TranslatorExplanation don't translate the repo types (well, the plaindir)
    // since they must be used in their original shape
    specific_help = _(
      "addrepo (ar) [options] <URI> <alias>\n"
      "\n"
      "Add repository specified by URI to the system and assing the specified alias to it.\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <FILE.repo>  Read the URL and alias from a file (even remote)\n"
      "-t, --type <TYPE>       Type of repository (yast2, rpm-md, or plaindir)\n"
      "-d, --disabled          Add the repository as disabled\n"
      "-n, --no-refresh        Add the repository with auto-refresh disabled\n"
    );
  }
  else if (command() == ZypperCommand::LIST_REPOS) {
    static struct option service_list_options[] = {
      {"export", required_argument, 0, 'e'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_list_options;
    specific_help = _(
      "repos (lr)\n"
      "\n"
      "List all defined repositories.\n"
      "\n"
      "  Command options:\n"
      "-e, --export <FILE.repo>  Export all defined repositories as a single local .repo file\n"
    );
  }
  else if (command() == ZypperCommand::REMOVE_REPO) {
    static struct option service_delete_options[] = {
      {"help", no_argument, 0, 'h'},
      {"loose-auth", no_argument, 0, 0},
      {"loose-query", no_argument, 0, 0},
      {0, 0, 0, 0}
    };
    specific_options = service_delete_options;
    specific_help = _(
      "removerepo (rr) [options] <alias|URL>\n"
      "\n"
      "Remove repository specified by alias or URL.\n"
      "\n"
      "  Command options:\n"
      "    --loose-auth\tIgnore user authentication data in the URL\n"
      "    --loose-query\tIgnore query string in the URL\n"
    );
  }
  else if (command() == ZypperCommand::RENAME_REPO) {
    static struct option service_rename_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_rename_options;
    specific_help = _(
      "renamerepo [options] <alias> <new-alias>\n"
      "\n"
      "Assign new alias to the repository specified by alias.\n"
      "\n"
      "This command has no additional options.\n"
    );
  }
  else if (command() == ZypperCommand::MODIFY_REPO) {
    static struct option service_modify_options[] = {
      {"help", no_argument, 0, 'h'},
      {"disable", no_argument, 0, 'd'},
      {"enable", no_argument, 0, 'e'},
      {"enable-autorefresh", no_argument, 0, 'a'},
      {"disable-autorefresh", no_argument, 0, 0},
      {0, 0, 0, 0}
    };
    specific_options = service_modify_options;
    specific_help = _(
      "modifyrepo (mr) <options> <alias>\n"
      "\n"
      "Modify properties of the repository specified by alias.\n"
      "\n"
      "  Command options:\n"
      "-d, --disable             Disable the repository (but don't remove it)\n"
      "-e, --enable              Enable a disabled repository\n"
      "-a, --enable-autorefresh  Enable auto-refresh of the repository\n"
      "    --disable-autorefresh Disable auto-refresh of the repository\n"
    );
  }
  else if (command() == ZypperCommand::REFRESH) {
    static struct option refresh_options[] = {
      {"force", no_argument, 0, 'f'},
      {"force-build", no_argument, 0, 'b'},
      {"force-download", no_argument, 0, 'd'},
      {"build-only", no_argument, 0, 'B'},
      {"download-only", no_argument, 0, 'D'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = refresh_options;
    specific_help = _(
      "refresh (ref) [alias|#] ...\n"
      "\n"
      "Refresh repositories specified by their alias or number."
      " If none are specified, all enabled repositories will be refreshed.\n"
      "\n"
      "  Command options:\n"
      "-f, --force             Force a complete refresh\n"
      "-b, --force-build       Force rebuild of the database\n"
      "-d, --force-download    Force download of raw metadata\n"
      "-B, --build-only        Only build the database, don't download metadata.\n"
      "-D, --download-only     Only download raw metadata, don't build the database\n"
    );
  }
  else if (command() == ZypperCommand::LIST_UPDATES) {
    static struct option list_updates_options[] = {
      {"repo",        required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog",     required_argument, 0, 'c'},
      {"type",        required_argument, 0, 't'},
      {"best-effort", no_argument, 0, 0 },
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = list_updates_options;
    specific_help = _(
      // TranslatorExplanation don't translate the resolvable types
      // (see the install command comment) 
      "list-updates [options]\n"
      "\n"
      "List all available updates\n"
      "\n"
      "  Command options:\n"
      "-t, --type <type>   Type of resolvable (package, patch, pattern, product) (default: patch)\n"
      "-r, --repo <alias>  List only updates from the repository specified by the alias.\n"
      "    --best-effort   Do a 'best effort' approach to update, updates to a lower than latest-and-greatest version are also acceptable.\n"
    );
  }
  else if (command() == ZypperCommand::UPDATE) {
    static struct option update_options[] = {
      {"repo",                      required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog",                   required_argument, 0, 'c'},
      {"type",		            required_argument, 0, 't'},
      // rug compatibility option, we have global --non-interactive
      // note: rug used this uption only to auto-answer the 'continue with install?' prompt.
      {"no-confirm",                no_argument,       0, 'y'},
      {"skip-interactive",          no_argument,       0, 0},
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      // rug compatibility, we have --auto-agree-with-licenses
      {"agree-to-third-party-licenses",  no_argument,  0, 0},
      {"best-effort",               no_argument,       0, 0},
      {"debug-solver",              no_argument,       0, 0},
      {"force-resolution",          no_argument,       0, 'R'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = update_options;
    specific_help = _(
      // TranslatorExplanation don't translate the resolvable types
      // (see the install command comment) 
      "update (up) [options]\n"
      "\n"
      "Update all installed resolvables with newer versions, where applicable.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-t, --type <type>               Type of resolvable (package, patch, pattern, product) (default: patch)\n"
      "-r, --repo <alias>              Limit updates to the repository specified by the alias.\n"
      "    --skip-interactive          Skip interactive updates\n"
      "-l, --auto-agree-with-licenses  Automatically say 'yes' to third party license confirmation prompt.\n"
      "                                See man zypper for more details.\n"
      "    --best-effort               Do a 'best effort' approach to update, updates to a lower than latest-and-greatest version are also acceptable\n"
      "    --debug-solver              Create solver test case for debugging\n"
      "-R, --force-resolution          Force the solver to find a solution (even agressive)\n"
    );
  }
  else if (command() == ZypperCommand::DIST_UPGRADE) {
    static struct option dupdate_options[] = {
      {"repo",                      required_argument, 0, 'r'},
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      {"debug-solver",              no_argument,       0, 0},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = dupdate_options;
    specific_help = _(
      "dist-upgrade (dup) [options]\n"
      "\n"
      "Perform a distribution upgrade.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias>              Limit the upgrade to the repository specified by the alias.\n"
      "-l, --auto-agree-with-licenses  Automatically say 'yes' to third party license confirmation prompt.\n"
      "                                See man zypper for more details.\n"
      "    --debug-solver              Create solver test case for debugging\n"
    );
  }
  else if (command() == ZypperCommand::SEARCH) {
    static struct option search_options[] = {
      {"installed-only", no_argument, 0, 'i'},
      {"uninstalled-only", no_argument, 0, 'u'},
      {"match-all", no_argument, 0, 0},
      {"match-any", no_argument, 0, 0},
      {"match-substrings", no_argument, 0, 0},
      {"match-words", no_argument, 0, 0},
      {"match-exact", no_argument, 0, 0},
      {"search-descriptions", no_argument, 0, 'd'},
      {"case-sensitive", no_argument, 0, 'c'},
      {"type",    required_argument, 0, 't'},
      {"sort-by-name", no_argument, 0, 0},
      // rug compatibility option, we have --sort-by-repo
      {"sort-by-catalog", no_argument, 0, 0},
      {"sort-by-repo", no_argument, 0, 0},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'}, //! \todo fix conflicting 'c' short option
      {"repo", required_argument, 0, 'r'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = search_options;
    specific_help = _(
      // TranslatorExplanation don't translate the resolvable types
      // (see the install command comment) 
      "search [options] [querystring...]\n"
      "\n"
      "Search for packages matching given search strings\n"
      "\n"
      "  Command options:\n"
      "    --match-all            Search for a match with all search strings (default)\n"
      "    --match-any            Search for a match with any of the search strings\n"
      "    --match-substrings     Matches with search strings may be partial words (default)\n"
      "    --match-words          Matches with search strings may only be whole words\n"
      "    --match-exact          Searches for an exact package name\n"
      "-d, --search-descriptions  Search also in package summaries and descriptions.\n"
      "-c, --case-sensitive       Perform case-sensitive search.\n"
      "-i, --installed-only       Show only packages that are already installed.\n"
      "-u, --uninstalled-only     Show only packages that are not currently installed.\n"
      "-t, --type <type>          Search only for packages of the specified type.\n"
      "-r, --repo <alias>         Search only in the repository specified by the alias.\n"
      "    --sort-by-name         Sort packages by name (default).\n"
      "    --sort-by-repo         Sort packages by repository.\n"
      "\n"
      "* and ? wildcards can also be used within search strings.\n"
    );
  }
  else if (command() == ZypperCommand::PATCH_CHECK) {
    static struct option patch_check_options[] = {
      {"repo", required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = patch_check_options;
    specific_help = _(
      "patch-check\n"
      "\n"
      "Check for available patches\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias>  Check for patches only in the repository specified by the alias.\n"
    );
  }
  else if (command() == ZypperCommand::SHOW_PATCHES) {
    static struct option patches_options[] = {
      {"repo", required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = patches_options;
    specific_help = _(
      "patches\n"
      "\n"
      "List all available patches\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias>  Check for patches only in the repository specified by the alias.\n"
    );
  }
  else if (command() == ZypperCommand::INFO) {
    static struct option info_options[] = {
      {"repo", required_argument, 0, 'r'},
      // rug compatibility option, we have --repo
      {"catalog", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = info_options;
    //! \todo -t option is missing (10.3+)
    specific_help = _(
      "info <name> ...\n"
      "\n"
      "Show full information for packages\n"
      "\n"
      "  Command options:\n"
      "\n"
      "-r, --repo <alias>  Work only with the repository specified by the alias.\n"
    );
  }
  // rug compatibility command, we have zypper info [-t <res_type>]
  else if (command() == ZypperCommand::RUG_PATCH_INFO) {
    static struct option patch_info_options[] = {
      {"catalog", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = patch_info_options;
    specific_help = _(
      "patch-info <patchname> ...\n"
      "\n"
      "Show detailed information for patches\n"
      "\n"
      "This is a rug compatibility alias for 'zypper info -t patch'\n"
    );
  }
  else if (command() == ZypperCommand::MOO) {
    static struct option moo_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = moo_options;
    specific_help = _(
      "moo\n"
      "\n"
      "Show an animal\n"
      "\n"
      "This command has no additional options.\n"
      );
  }
  else if (command() == ZypperCommand::XML_LIST_UPDATES_PATCHES) {
    static struct option xml_updates_options[] = {
      {"repo", required_argument, 0, 'r'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = xml_updates_options;
    specific_help = _(
      "xml-updates\n"
      "\n"
      "Show updates and patches in xml format\n"
      "\n"
      "  Command options:\n"
      "-r, --repo <alias>  Work only with updates from repository specified by alias.\n"
    );
  }

  vector<string> arguments;

  // parse command options
  if (!ghelp)
  {
    ::copts = _copts = parse_options (argc(), argv(), specific_options);
    if (copts.count("_unknown"))
    {
      setExitCode(ZYPPER_EXIT_ERR_SYNTAX);
      return;
    }

    // treat --help command option like global --help option from now on
    // i.e. when used together with command to print command specific help
    ghelp = ghelp || copts.count("help");
  
    if (optind < argc()) {
      cout_v << _("Non-option program arguments: ");
      while (optind < argc()) {
        string argument = argv()[optind++];
        cout_v << "'" << argument << "' ";
        arguments.push_back (argument);
      }
      cout_v << endl;
    }

    // === process options ===
  
    if (gopts.count("terse")) 
    {
      gSettings.machine_readable = true;
      cout << "<?xml version='1.0'?>" << endl;
      cout << "<stream>" << endl;
    }
  
    if (gopts.count("disable-repositories") ||
        gopts.count("disable-system-sources"))
    {
      MIL << "Repositories disabled, using target only." << endl;
      cout_n <<
          _("Repositories disabled, using the database of installed packages only.")
          << endl;
      gSettings.disable_system_sources = true;
    }
    else
    {
      MIL << "System sources enabled" << endl;
    }
  
    if (gopts.count("disable-system-resolvables"))
    {
      MIL << "System resolvables disabled" << endl;
      cout_v << _("Ignoring installed resolvables...") << endl;
      gSettings.disable_system_resolvables = true;
    }
  
    if (gopts.count("source")) {
      list<string> sources = gopts["source"];
      for (list<string>::const_iterator it = sources.begin(); it != sources.end(); ++it )
      {
        Url url = make_url (*it);
        if (!url.isValid())
  	setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
        return;
        gSettings.additional_sources.push_back(url); 
      }
    }
  
    // here come commands that need the lock
    try {
      if (command() == ZypperCommand::LIST_REPOS)
        zypp_readonly_hack::IWantIt (); // #247001, #302152
  
      God = zypp::getZYpp();
    }
    catch (Exception & excpt_r) {
      ZYPP_CAUGHT (excpt_r);
      ERR  << "A ZYpp transaction is already in progress." << endl;
      string msg = _("A ZYpp transaction is already in progress."
          " This means, there is another application using the libzypp library for"
          " package management running. All such applications must be closed before"
          " using this command.");
  
      if ( gSettings.machine_readable )
        cout << "<message type=\"error\">" << msg  << "</message>" <<  endl;
      else
        cerr << msg << endl;
  
      setExitCode(ZYPPER_EXIT_ERR_ZYPP);
      return;
    }
  }

  ResObject::Kind kind;


  // === execute command ===

  // --------------------------( moo )----------------------------------------

  if (command() == ZypperCommand::MOO)
  {
    if (ghelp) { cout << specific_help << endl; return; }

    // TranslatorExplanation this is a hedgehog, paint another animal, if you want
    cout_n << _("   \\\\\\\\\\\n  \\\\\\\\\\\\\\__o\n__\\\\\\\\\\\\\\'/_") << endl;
    return;
  }

  // --------------------------( repo list )----------------------------------
  
  else if (command() == ZypperCommand::LIST_REPOS)
  {
    if (ghelp) { cout << specific_help << endl; return; }
    // if (ghelp) display_command_help()

    list_repos();
    return;
  }

  // --------------------------( addrepo )------------------------------------
  
  else if (command() == ZypperCommand::ADD_REPO)
  {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);return;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for modifying system repositories.") << endl;
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);return;
    }

    // too many arguments
    if (arguments.size() > 2)
    {
      report_too_many_arguments(specific_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);return;
    }

    // indeterminate indicates the user has not specified the values
    tribool enabled(indeterminate); 
    tribool refresh(indeterminate);

    if (copts.count("disabled"))
      enabled = false;
    if (copts.count("no-refresh"))
      refresh = false;

    try
    {
      // add repository specified in .repo file
      if (copts.count("repo"))
      {
        setExitCode(add_repo_from_file(copts["repo"].front(), enabled, refresh));
        return;
      }
  
      // force specific repository type. Validation is done in add_repo_by_url()
      string type = copts.count("type") ? copts["type"].front() : "";
  
      // display help message if insufficient info was given
      if (arguments.size() < 2)
      {
        cerr << _("Too few arguments. At least URL and alias are required.") << endl;
        ERR << "Too few arguments. At least URL and alias are required." << endl;
        cout_n << specific_help;
        setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);return;
      }

      Url url = make_url (arguments[0]);
      if (!url.isValid())
        setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);return;

      // by default, enable the repo and set autorefresh
      if (indeterminate(enabled)) enabled = true;
      if (indeterminate(refresh)) refresh = true;

      warn_if_zmd();

      // load gpg keys
      cond_init_target ();

      setExitCode(add_repo_by_url(
          url, arguments[1]/*alias*/, type, enabled, refresh));
      return;
    }
    catch (const repo::RepoUnknownTypeException & e)
    {
      ZYPP_CAUGHT(e);
      report_problem(e,
          _("Specified type is not a valid repository type:"),
          _("See 'zypper -h addrepo' or man zypper to get a list of known repository types."));
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }
  }

  // --------------------------( delete repo )--------------------------------

  else if (command() == ZypperCommand::REMOVE_REPO)
  {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);return;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for modifying system repositories.") << endl;
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);return;
    }

    if (arguments.size() < 1)
    {
      cerr << _("Required argument missing.") << endl;
      ERR << "Required argument missing." << endl;
      cout_n << _("Usage") << ':' << endl;
      cout_n << specific_help;
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);return;
    }

    // too many arguments
    //! \todo allow to specify multiple repos to delete
    else if (arguments.size() > 1)
    {
      report_too_many_arguments(specific_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);return;
    }

    warn_if_zmd ();

    bool found = remove_repo(arguments[0]);
    if (found)
    {
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    MIL << "Repository not found by alias, trying delete by URL" << endl;
    cout_v << _("Repository not found by alias, trying delete by URL...") << endl;

    Url url;
    try { url = Url (arguments[0]); }
    catch ( const Exception & excpt_r )
    {
      ZYPP_CAUGHT( excpt_r );
      cerr_v << _("Given URL is invalid.") << endl;
      cerr_v << excpt_r.asUserString() << endl;
    }

    if (url.isValid())
    {
      url::ViewOption urlview = url::ViewOption::DEFAULTS + url::ViewOption::WITH_PASSWORD;

      if (copts.count("loose-auth"))
      {
        urlview = urlview
          - url::ViewOptions::WITH_PASSWORD
          - url::ViewOptions::WITH_USERNAME;
      }

      if (copts.count("loose-query"))
        urlview = urlview - url::ViewOptions::WITH_QUERY_STR;

      found = remove_repo(url, urlview);
    }
    else
      found = false;

    if (!found)
    {
      MIL << "Repository not found by given alias or URL." << endl;
      cerr << _("Repository not found by given alias or URL.") << endl;
    }

    setExitCode(ZYPPER_EXIT_OK);
    return;
  }

  // --------------------------( rename repo )--------------------------------

  else if (command() == ZypperCommand::RENAME_REPO)
  {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for modifying system repositories.") << endl;
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    if (arguments.size() < 2)
    {
      cerr << _("Too few arguments. At least URL and alias are required.") << endl;
      ERR << "Too few arguments. At least URL and alias are required." << endl;
      cout_n << specific_help;
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }
    // too many arguments
    else if (arguments.size() > 2)
    {
      report_too_many_arguments(specific_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

//    cond_init_target ();
    warn_if_zmd ();
    try {
      // also stores it
      rename_repo(arguments[0], arguments[1]);
    }
    catch ( const Exception & excpt_r )
    {
      cerr << excpt_r.asUserString() << endl;
      setExitCode(ZYPPER_EXIT_ERR_ZYPP);
      return;
    }

    setExitCode(ZYPPER_EXIT_OK);
    return;
  }

  // --------------------------( modify repo )--------------------------------

  else if (command() == ZypperCommand::MODIFY_REPO)
  {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for modifying system repositories.") << endl;
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    if (arguments.size() < 1)
    {
      cerr << _("Alias is a required argument.") << endl;
      ERR << "Na alias argument given." << endl;
      cout_n << specific_help;
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }
    // too many arguments
    if (arguments.size() > 1)
    {
      report_too_many_arguments(specific_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    modify_repo(arguments[0]);
  }

  // --------------------------( refresh )------------------------------------

  else if (command() == ZypperCommand::REFRESH)
  {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for refreshing system repositories.") << endl;
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    setExitCode(refresh_repos(arguments));
    return;
  }

  // --------------------------( remove/install )-----------------------------

  else if (command() == ZypperCommand::INSTALL ||
           command() == ZypperCommand::REMOVE)
  {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    if (arguments.size() < 1)
    {
      cerr << _("Too few arguments.");
      cerr << " " << _("At least one package name is required.") << endl << endl;
      cerr << specific_help;
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    if (copts.count("auto-agree-with-licenses")
        || copts.count("agree-to-third-party-licenses"))
      gSettings.license_auto_agree = true;

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for installing or uninstalling packages.") << endl;
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    // rug compatibility code
    // switch on non-interactive mode if no-confirm specified
    if (copts.count("no-confirm"))
      gSettings.non_interactive = true;


    // read resolvable type
    string skind = copts.count("type")?  copts["type"].front() : "package";
    kind = string_to_kind (skind);
    if (kind == ResObject::Kind ()) {
      cerr << format(_("Unknown resolvable type: %s")) % skind << endl;
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    int initret = init_repos();
    if (initret != ZYPPER_EXIT_OK)
    {
      setExitCode(initret);
      return;
    }

    //! \todo support temporary additional repos
    /*
    for ( std::list<Url>::const_iterator it = gSettings.additional_sources.begin(); it != gSettings.additional_sources.end(); ++it )
    {
      include_source_by_url( *it );
    }
    */

    if ( gData.repos.empty() )
    {
      cerr << _("Warning: No repositories defined."
          " Operating only with the installed resolvables."
          " Nothing can be installed.") << endl;
    }

    cond_init_target ();
    cond_load_resolvables();

    bool install_not_remove = command() == ZypperCommand::INSTALL;
    bool by_capability = false; // install by name by default
    if (copts.count("capability"))
      by_capability = true;
    for ( vector<string>::const_iterator it = arguments.begin();
          it != arguments.end(); ++it ) {
      if (by_capability)
        mark_by_capability (install_not_remove, kind, *it);
      else
        mark_by_name (install_not_remove, kind, *it);
    }

    if (copts.count("debug-solver"))
    {
      establish();
      cout_n << _("Generating solver test case...") << endl;
      if (God->resolver()->createSolverTestcase("/var/log/zypper.solverTestCase"))
        cout_n << _("Solver test case generated successfully.") << endl;
      else
      {
        cerr << _("Error creating the solver test case.") << endl;
        setExitCode(ZYPPER_EXIT_ERR_ZYPP);
        return;
      }
    }
    else
    {
      setExitCode(solve_and_commit());
      return;
    }

    setExitCode(ZYPPER_EXIT_OK);
    return;
  }

  // -------------------( source install )------------------------------------

  else if (command() == ZypperCommand::SRC_INSTALL)
  {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    if (arguments.size() < 1)
    {
      cerr << _("Source package name is a required argument.") << endl;
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    int initret = init_repos();
    if (initret != ZYPPER_EXIT_OK)
    {
      setExitCode(initret);
      return;
    }
    cond_init_target();
    // load only repo resolvables, we don't need the installed ones
    load_repo_resolvables(false /* don't load to pool */);

    setExitCode(source_install(arguments));
    return;
  }

  // --------------------------( search )-------------------------------------

  else if (command() == ZypperCommand::SEARCH)
  {
    ZyppSearchOptions options;

    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    if (gSettings.disable_system_resolvables || copts.count("uninstalled-only"))
      options.setInstalledFilter(ZyppSearchOptions::UNINSTALLED_ONLY);

    if (copts.count("installed-only")) options.setInstalledFilter(ZyppSearchOptions::INSTALLED_ONLY);
    if (copts.count("match-any")) options.setMatchAny();
    if (copts.count("match-words")) options.setMatchWords();
    if (copts.count("match-exact")) options.setMatchExact();
    if (copts.count("search-descriptions")) options.setSearchDescriptions();
    if (copts.count("case-sensitive")) options.setCaseSensitive();

    if (copts.count("type") > 0) {
      options.clearKinds();
      std::list<std::string>::const_iterator it;
      for (it = copts["type"].begin(); it != copts["type"].end(); ++it) {
	kind = string_to_kind( *it );
        if (kind == ResObject::Kind()) {
          cerr << _("Unknown resolvable type ") << *it << endl;
          setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
          return;
        }
        options.addKind( kind );
      }
    }
    else if (gSettings.is_rug_compatible) {
      options.clearKinds();
      options.addKind( ResTraits<Package>::kind );
    }

    if (copts.count("repo") > 0) {
      options.clearRepos();
      std::list<std::string>::const_iterator it;
      for (it = copts["repo"].begin(); it != copts["repo"].end(); ++it) {
        options.addRepo( *it );
      }
    }

    options.resolveConflicts();

    int initret = init_repos();
    if (initret != ZYPPER_EXIT_OK)
    {
      setExitCode(initret);
      return;
    }
    cond_init_target();         // calls ZYpp::initializeTarget("/");
    
    establish();

    Table t;
    t.style(Ascii);

    ZyppSearch search( God, options, arguments );
    FillTable callback( t, search.installedCache(), search.getQueryInstancePtr(), search.options() );

    search.doSearch( callback, callback );

    if (t.empty())
      cout << _("No resolvables found.") << endl;
    else {
      cout << endl;
      if (copts.count("sort-by-catalog") || copts.count("sort-by-repo"))
        t.sort(1);
      else
        t.sort(3); // sort by name
      cout << t;
    }

    setExitCode(ZYPPER_EXIT_OK);
    return;
  }

  // --------------------------( patch check )--------------------------------

  // TODO: rug summary
  else if (command() == ZypperCommand::PATCH_CHECK) {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    // too many arguments
    if (arguments.size() > 0)
    {
      report_too_many_arguments(specific_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    cond_init_target ();

    int initret = init_repos();
    if (initret != ZYPPER_EXIT_OK)
    {
      setExitCode(initret);
      return;
    }

    // TODO additional_sources
    // TODO warn_no_sources
    // TODO calc token?

    // now load resolvables:
    cond_load_resolvables();

    establish ();
    patch_check ();

    if (gData.security_patches_count > 0)
    {
      setExitCode(ZYPPER_EXIT_INF_SEC_UPDATE_NEEDED);
      return;
    }
    if (gData.patches_count > 0)
    {
      setExitCode(ZYPPER_EXIT_INF_UPDATE_NEEDED);
      return;
    }
    setExitCode(ZYPPER_EXIT_OK);
    return;
  }

  // --------------------------( patches )------------------------------------

  else if (command() == ZypperCommand::SHOW_PATCHES) {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    // too many arguments
    if (arguments.size() > 0)
    {
      report_too_many_arguments(specific_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    cond_init_target ();
    int initret = init_repos();
    if (initret != ZYPPER_EXIT_OK)
    {
      setExitCode(initret);
      return;
    }
    cond_load_resolvables();
    establish ();
    show_patches ();
    setExitCode(ZYPPER_EXIT_OK);
    return;
  }

  // --------------------------( list updates )-------------------------------

  else if (command() == ZypperCommand::LIST_UPDATES) {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    // too many arguments
    if (arguments.size() > 0)
    {
      report_too_many_arguments(specific_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    string skind = copts.count("type")?  copts["type"].front() :
      gSettings.is_rug_compatible? "package" : "patch";
    kind = string_to_kind (skind);
    if (kind == ResObject::Kind ()) {
      cerr << format(_("Unknown resolvable type: %s")) % skind << endl;
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    bool best_effort = copts.count( "best-effort" ); 

    if (gSettings.is_rug_compatible && best_effort) {
	best_effort = false;
	// 'rug' is the name of a program and must not be translated
	// 'best-effort' is a program parameter and can not be translated
	cerr << _("Running as 'rug', can't do 'best-effort' approach to update.") << endl;
    }
    cond_init_target ();
    int initret = init_repos();
    if (initret != ZYPPER_EXIT_OK)
    {
      setExitCode(initret);
      return;
    }
    cond_load_resolvables();
    establish ();

    list_updates( kind, best_effort );

    setExitCode(ZYPPER_EXIT_OK);
    return;
  }


  // -----------------( xml list updates and patches )------------------------

  else if (command() == ZypperCommand::XML_LIST_UPDATES_PATCHES) {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    cond_init_target ();
    int initret = init_repos();
    if (initret != ZYPPER_EXIT_OK)
    {
      setExitCode(initret);
      return;
    }
    cond_load_resolvables();
    establish ();

    cout << "<update-status version=\"0.6\">" << endl;
    cout << "<update-list>" << endl;
    if (!xml_list_patches ())	// Only list updates if no
      xml_list_updates ();	// affects-pkg-mgr patches are available
    cout << "</update-list>" << endl;
    cout << "</update-status>" << endl;

    setExitCode(ZYPPER_EXIT_OK);
    return;
  }

  // -----------------------------( update )----------------------------------

  else if (command() == ZypperCommand::UPDATE) {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for updating packages.") << endl;
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    // too many arguments
    if (arguments.size() > 0)
    {
      report_too_many_arguments(specific_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    // rug compatibility code
    // switch on non-interactive mode if no-confirm specified
    if (copts.count("no-confirm"))
      gSettings.non_interactive = true;

    if (copts.count("auto-agree-with-licenses")
        || copts.count("agree-to-third-party-licenses"))
      gSettings.license_auto_agree = true;

    string skind = copts.count("type")?  copts["type"].front() :
      gSettings.is_rug_compatible? "package" : "patch";
    kind = string_to_kind (skind);
    if (kind == ResObject::Kind ()) {
	cerr << format(_("Unknown resolvable type: %s")) % skind << endl;
	setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
	return;
    }

    bool best_effort = copts.count( "best-effort" ); 

    if (gSettings.is_rug_compatible && best_effort) {
	best_effort = false;
	// 'rug' is the name of a program and must not be translated
	// 'best-effort' is a program parameter and can not be translated
	cerr << _("Running as 'rug', can't do 'best-effort' approach to update.") << endl;
    }
    cond_init_target ();
    int initret = init_repos();
    if (initret != ZYPPER_EXIT_OK)
    {
      setExitCode(initret);
      return;
    }
    cond_load_resolvables ();
    establish ();

    bool skip_interactive = copts.count("skip-interactive") || gSettings.non_interactive;
    mark_updates( kind, skip_interactive, best_effort );


    if (copts.count("debug-solver"))
    {
      cout_n << _("Generating solver test case...") << endl;
      if (God->resolver()->createSolverTestcase("/var/log/zypper.solverTestCase"))
        cout_n << _("Solver test case generated successfully.") << endl;
      else
        cerr << _("Error creating the solver test case.") << endl;
    }
    // commit
    // returns ZYPPER_EXIT_OK, ZYPPER_EXIT_ERR_ZYPP,
    // ZYPPER_EXIT_INF_REBOOT_NEEDED, or ZYPPER_EXIT_INF_RESTART_NEEDED
    else
    {
      setExitCode(solve_and_commit());
      return;
    }

    setExitCode(ZYPPER_EXIT_OK);
    return; 
  }

  // ----------------------------( dist-upgrade )------------------------------

  else if (command() == ZypperCommand::DIST_UPGRADE) {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for performing a distribution upgrade.") << endl;
      setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
      return;
    }

    // too many arguments
    if (arguments.size() > 0)
    {
      report_too_many_arguments(specific_help);
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    if (copts.count("auto-agree-with-licenses"))
      gSettings.license_auto_agree = true;

    cond_init_target ();
    int initret = init_repos();
    if (initret != ZYPPER_EXIT_OK)
    {
      setExitCode(initret);
      return;
    }
    cond_load_resolvables ();
    establish ();
    zypp::UpgradeStatistics opt_stats;
    God->resolver()->doUpgrade(opt_stats);

    if (copts.count("debug-solver"))
    {
      cout_n << _("Generating solver test case...") << endl;
      if (God->resolver()->createSolverTestcase("/var/log/zypper.solverTestCase"))
        cout_n << _("Solver test case generated successfully.") << endl;
      else
        cerr << _("Error creating the solver test case.") << endl;
    }
    // commit
    // returns ZYPPER_EXIT_OK, ZYPPER_EXIT_ERR_ZYPP,
    // ZYPPER_EXIT_INF_REBOOT_NEEDED, or ZYPPER_EXIT_INF_RESTART_NEEDED
    else
    {
      setExitCode(solve_and_commit());
      return;
    }

    setExitCode(ZYPPER_EXIT_OK);
    return; 
  }

  // -----------------------------( info )------------------------------------

  else if (command() == ZypperCommand::INFO ||
           command() == ZypperCommand::RUG_PATCH_INFO) {
    if (ghelp)
    {
      cout << specific_help;
      setExitCode(ZYPPER_EXIT_OK);
      return;
    }

    if (arguments.size() < 1)
    {
      cerr << _("Required argument missing.") << endl;
      ERR << "Required argument missing." << endl;
      cout_n << _("Usage") << ':' << endl;
      cout_n << specific_help;
      setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    cond_init_target ();
    int initret = init_repos();
    if (initret != ZYPPER_EXIT_OK)
    {
      setExitCode(initret);
      return;
    }
    cond_load_resolvables ();
    establish ();

    printInfo(command(),arguments);

    setExitCode(ZYPPER_EXIT_OK);
    return;
  }

  // if the program reaches this line, something went wrong
  setExitCode(ZYPPER_EXIT_ERR_BUG);
}

// Local Variables:
// c-basic-offset: 2
// End:
