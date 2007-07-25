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
#include <readline/readline.h>
#include <readline/history.h>

#include <boost/logic/tribool.hpp>

#include <zypp/base/Logger.h>
#include <zypp/zypp_detail/ZYppReadOnlyHack.h>
#include <zypp/repo/RepoException.h>

#include "zypper.h"
#include "zypper-sources.h"
#include "zypper-misc.h"

#include "zypper-rpm-callbacks.h"
#include "zypper-keyring-callbacks.h"
#include "zypper-source-callbacks.h"
#include "zypper-media-callbacks.h"
#include "zypper-tabulator.h"
#include "zypper-search.h"
#include "zypper-info.h"
#include "zypper-getopt.h"
#include "zypper-command.h"

using namespace std;
using namespace zypp;
using namespace zypp::detail;
using namespace boost;

ZYpp::Ptr God = NULL;
RuntimeData gData;
Settings gSettings;

ostream no_stream(NULL);

RpmCallbacks rpm_callbacks;
SourceCallbacks source_callbacks;
MediaCallbacks media_callbacks;
KeyRingCallbacks keyring_callbacks;
DigestCallbacks digest_callbacks;

static struct option global_options[] = {
  {"help",            no_argument,       0, 'h'},
  {"verbose",         no_argument,       0, 'v'},
  {"quiet",           no_argument,       0, 'q'},
  {"version",         no_argument,       0, 'V'},
  {"terse",           no_argument,       0, 't'},
  {"table-style",     required_argument, 0, 's'},
  {"rug-compatible",  no_argument,       0, 'r'},
  {"non-interactive", no_argument,       0, 0},
  {"no-gpg-checks",   no_argument,       0, 0},
  {"root",            required_argument, 0, 'R'},
  {"opt",             optional_argument, 0, 'o'},
  {0, 0, 0, 0}
};

/**
 * Constructor wrapper catching exceptions,
 * returning an empty one on error.
 */
Url make_url (const string & url_s) {
  Url u;
  try {
    u = Url (url_s);
  }
  catch ( const Exception & excpt_r ) {
    ZYPP_CAUGHT( excpt_r );
    cerr << _("Given URL is invalid.") << endl;
    cerr << excpt_r.asUserString() << endl;
  }
  return u;
}

string help_commands = _(
  "  Commands:\n"
  "\thelp\t\t\tHelp\n"
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
  "");

// global options
parsed_opts gopts;
bool ghelp = false;

/*
 * parses global options, returns the command
 * 
 * \returns ZypperCommand object representing the command or ZypperCommand::NONE
 *          if an unknown command has been given. 
 */
ZypperCommand process_globals(int argc, char **argv)
{
  // global options
  gopts = parse_options (argc, argv, global_options);
  if (gopts.count("_unknown"))
    return ZypperCommand::NONE;
    //return "_unknown";

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

  string help_global_options = _("  Options:\n"
    "\t--help, -h\t\tHelp.\n"
    "\t--version, -V\t\tOutput the version number.\n"
    "\t--quiet, -q\t\tSuppress normal output, print only error messages.\n"
    "\t--verbose, -v\t\tIncrease verbosity.\n"
    "\t--terse, -t\t\tTerse output for machine consumption.\n"
    "\t--table-style, -s\tTable style (integer).\n"
    "\t--rug-compatible, -r\tTurn on rug compatibility.\n"
    "\t--non-interactive\tDon't ask anything, use default answers automatically.\n"
    "\t--no-gpg-checks\t\tIgnore GPG check failures and continue.\n"
    "\t--root, -R <dir>\tOperate on a different root directory.\n");
    ;
  
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
    cout_n << _("Entering non-interactive mode.");
    MIL << "Entering non-interactive mode" << endl;
  }

  if (gopts.count("no-gpg-checks")) {
    gSettings.no_gpg_checks = true;
    cout_n << _("Entering no-gpg-checks mode.");
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
  }


  // testing option
  if (gopts.count("opt")) {
    cout << "Opt arg: ";
    std::copy (gopts["opt"].begin(), gopts["opt"].end(),
	       ostream_iterator<string> (cout, ", "));
    cout << endl;
  }

  // get command

  ZypperCommand command(ZypperCommand::NONE_e);
  try
  {
    if (optind < argc)
      command = ZypperCommand(argv[optind++]);

    if (command == ZypperCommand::HELP)
    {
      ghelp = true;
      if (optind < argc)
        command = ZypperCommand(argv[optind++]);
      else
        command = ZypperCommand::NONE;
    }
  }
  // exception from command parsing
  catch (Exception & e)
  {
    cerr << e.msg() << endl;
    command = ZypperCommand::NONE;
  }

  if (command == ZypperCommand::NONE)
  {
    if (ghelp)
      cout << help_global_options << endl << help_commands;
    else if (gopts.count("version"))
      cout << PACKAGE " " VERSION << endl;
    else
      cerr << _("Try -h for help.") << endl;
  }

  //cerr_vv << "COMMAND: " << command << endl;
  return command;
}

/// process one command from the OS shell or the zypper shell
int one_command(const ZypperCommand & command, int argc, char **argv)
{
  // === command-specific options ===

  struct option no_options = {0, 0, 0, 0};
  struct option *specific_options = &no_options;
  string specific_help;

  string help_global_source_options = _(
      "  Repository options:\n"
      "\t--disable-repositories, -D\t\tDo not read data from defined repositories.\n"
      "\t--repo <URI|.repo>\t\tRead additional repository\n"
      );
//! \todo preserve for rug comp.  "\t--disable-system-sources, -D\t\tDo not read the system sources\n"
//! \todo preserve for rug comp.  "\t--source, -S\t\tRead additional source\n"

  string help_global_target_options = _("  Target options:\n"
      "\t--disable-system-resolvables, -T\t\tDo not read system installed resolvables\n"
      );

  if (command == ZypperCommand::HELP)
  {
    //cout << help_global_options << endl << help_commands;
  }
  else if (command == ZypperCommand::INSTALL) {
    static struct option install_options[] = {
      {"catalog",	            required_argument, 0, 'c'},
      {"type",	                    required_argument, 0, 't'},
      {"no-confirm",                no_argument,       0, 'y'},
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      {"machine-readable",					no_argument,       0, 'm'},
      {"help",                      no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = install_options;
    specific_help = _(
      "install [options] <name> ...\n"
      "\n"
      "'install' - Install resolvabe(s) with specified name(s).\n"
      "\n"
      "  Command options:\n"
      "\t--catalog,-c\t\t\tOnly from this catalog (under development)\n"
      "\t--type,-t <resolvable_type>\tType of resolvable (default: package)\n"
      "\t--no-confirm,-y\t\t\tDo not require user confirmation to proceed with installation\n"
      "\t--machine-readable,-m\t\t\tGenerate machine readable output\n"
      "\t--auto-agree-with-licenses,-l\tAutomatically say 'yes' to third party license confirmation prompt.\n"
      "\t\t\t\t\tSee man zypper for more details.\n"
      );
  }
  else if (command == ZypperCommand::REMOVE) {
    static struct option remove_options[] = {
      {"type",       required_argument, 0, 't'},
      {"no-confirm", no_argument,       0, 'y'},
      {"help",       no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = remove_options;
    specific_help = _(
      "remove [options] <name> ...\n"
      "\n"
      "'remove' - Remove resolvabe(s) with specified name(s).\n"
      "\n"
      "  Command options:\n"
      "\t--type,-t <resolvable_type>\tType of resolvable (default: package)\n"
      "\t--no-confirm,-y\t\t\tDo not require user confirmation\n"
      );
  }
  else if (command == ZypperCommand::ADD_REPO) {
    static struct option service_add_options[] = {
      {"type", required_argument, 0, 't'},
      {"disabled", no_argument, 0, 'd'},
      {"no-refresh", no_argument, 0, 'n'},
      {"repo", required_argument, 0, 'r'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_add_options;
    specific_help = _(
      "addrepo (ar) [options] <URI> <alias>\n"
      "\n"
      "Add repository specified by URI to the system and assing specified alias to it."
      "\n"
      "  Command options:\n"
      "\t--repo,-r <FILE.repo>\tRead the URL and alias from a file\n"
      "\t\t\t\t(even remote)\n"
      "\t--type,-t <TYPE>\tType of repository (YaST, YUM, or Plaindir)\n"
      "\t--disabled,-d\t\tAdd the repository as disabled\n"
      "\t--no-refresh,-n\t\tAdd the repository with auto-refresh disabled\n"
      );
  }
  else if (command == ZypperCommand::LIST_REPOS) {
    static struct option service_list_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_list_options;
    specific_help = _(
      "repos (lr)\n"
      "\n"
      "List all defined repositories."
      "\n"
      "This command has no additional options.\n"
      );
  }
  else if (command == ZypperCommand::REMOVE_REPO) {
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
      "Remove repository specified by alias or URL."
      "\n"
      "  Command options:\n"
      "\t--loose-auth\tIgnore user authentication data in the URL\n"
      "\t--loose-query\tIgnore query string in the URL\n"
      );
  }
  else if (command == ZypperCommand::RENAME_REPO) {
    static struct option service_rename_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_rename_options;
    specific_help = _(
      "renamerepo [options] <alias> <new-alias>\n"
      "\n"
      "Assign new alias to the repository specified by alias."
      "\n"
      "This command has no additional options.\n"
      );
  }
  else if (command == ZypperCommand::MODIFY_REPO) {
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
      "Modify properties of the repository specified by alias."
      "\n"
      "\t--disable,-d\t\tDisable the repository (but don't remove it)\n"
      "\t--enable,-e\t\tEnable a disabled repository\n"
      "\t--enable-autorefresh,-a\tEnable auto-refresh of the repository\n"
      "\t--disable-autorefresh\tDisable auto-refresh of the repository\n"
    );
  }
  else if (command == ZypperCommand::REFRESH) {
    static struct option refresh_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = refresh_options;
    specific_help = _(
      "refresh\n"
      "\n"
      "Refresh all installation sources found in the system.\n"
      "\n"
      "This command has no additional options.\n"
      );
  }
  else if (command == ZypperCommand::LIST_UPDATES) {
    static struct option list_updates_options[] = {
      {"type",		required_argument, 0, 't'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = list_updates_options;
    specific_help = _(
      "list-updates [options]\n"
      "\n"
      "List all available updates\n"
      "\n"
      "  Command options:\n"
      "\t--type,-t <resolvable_type>\tType of resolvable (default: patch)\n"
      );
  }
  else if (command == ZypperCommand::UPDATE) {
    static struct option update_options[] = {
      {"type",		            required_argument, 0, 't'},
      {"no-confirm",                no_argument,       0, 'y'},
      {"skip-interactive",          no_argument,       0, 0},
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = update_options;
    specific_help = _(
      "'update' - Update all installed resolvables with newer versions, where applicable.\n"
      "\n"
      "  Command options:\n"
      "\n"
      "\t--type,-t <resolvable_type>\tType of resolvable (default: patch)\n"
      "\t--no-confirm,-y\t\t\tDo not require user confirmation\n"
      "\t--skip-interactive\t\tSkip interactive updates\n"
      "\t--auto-agree-with-licenses,-l\tAutomatically say 'yes' to third party license confirmation prompt.\n"
      "\t\t\t\t\tSee man zypper for more details.\n"
      );
  }
  else if (command == ZypperCommand::SEARCH) {
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
      {"sort-by-catalog", no_argument, 0, 0},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = search_options;
    specific_help = _(
      "search [options] [querystring...]\n"
      "\n"
      "'search' - Search for packages matching given search strings\n"
      "\n"
      "  Command options:\n"
      "    --match-all            Search for a match to all search strings (default)\n"
      "    --match-any            Search for a match to any of the search strings\n"
      "    --match-substrings     Matches for search strings may be partial words (default)\n"
      "    --match-words          Matches for search strings may only be whole words\n"
      "    --match-exact          Searches for an exact package name\n"
      "-d, --search-descriptions  Search also in package summaries and descriptions.\n"
      "-c, --case-sensitive       Perform case-sensitive search.\n"
      "-i, --installed-only       Show only packages that are already installed.\n"
      "-u, --uninstalled-only     Show only packages that are not currently installed.\n"
      "-t, --type                 Search only for packages of the specified type.\n"
      "    --sort-by-name         Sort packages by name (default).\n"
      "    --sort-by-catalog      Sort packages by catalog (source).\n"
      "\n"
      "* and ? wildcards can also be used within search strings.\n"
      );
  }
  else if (command == ZypperCommand::PATCH_CHECK) {
    static struct option patch_check_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = patch_check_options;
    specific_help = _(
      "patch-check\n"
      "\n"
      "Check for available patches\n"
      "\n"
      "This command has no additional options.\n"
      );
  }
  else if (command == ZypperCommand::SHOW_PATCHES) {
    static struct option patches_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = patches_options;
    specific_help = _(
      "patches\n"
      "\n"
      "List all available patches\n"
      "\n"
      "This command has no additional options.\n"
      );
  }
  else if (command == ZypperCommand::INFO) {
    static struct option info_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = info_options;
    specific_help = _(
      "info <name> ...\n"
      "\n"
      "'info' -- Show full information for packages\n"
      "\n"
      "This command has no additional options.\n"
      );
  }
  else if (command == ZypperCommand::RUG_PATCH_INFO) {
    static struct option patch_info_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = patch_info_options;
    specific_help = _(
      "patch-info <patchname> ...\n"
      "\n"
      "'patch-info' -- Show detailed information for patches\n"
      "\n"
      "This command has no additional options.\n"
      );
  }
  else if (command == ZypperCommand::MOO) {
    static struct option moo_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = moo_options;
    specific_help = _(
      "moo\n"
      "\n"
      "'moo' - Show an animal\n"
      "\n"
      "This command has no additional options.\n"
      );
  }
  else if (command == ZypperCommand::XML_LIST_UPDATES_PATCHES) {
    static struct option xml_updates_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = xml_updates_options;
    specific_help = _(
      "xml-updates\n"
      "\n"
      "'xml-updates' - Show updates and patches in xml format\n"
      "\n"
      "This command has no additional options.\n"
      );
	}

  parsed_opts copts = parse_options (argc, argv, specific_options);
  if (copts.count("_unknown"))
    return ZYPPER_EXIT_ERR_SYNTAX;

  // treat --help command option like global --help option from now on
  // i.e. when used together with command to print command specific help
  ghelp = ghelp || copts.count("help");

  vector<string> arguments;
  if (optind < argc) {
    cout_v << _("Non-option program arguments: ");
    while (optind < argc) {
      string argument = argv[optind++];
      cout_v << argument << ' ';
      arguments.push_back (argument);
    }
    cout_v << endl;
  }

  // === process options ===

  if (gopts.count("terse")) {
      cout_v << _("Ignoring --terse (provided only for rug compatibility)") << endl;
      WAR << "Ignoring --terse (provided only for rug compatibility)" << endl;
  }

  if (gopts.count("disable-repositories") ||
      gopts.count("disable-system-sources"))
  {
    MIL << "Repositories disabled, using target only." << endl;
    cout_n <<
        _("Repositories disabled, using database of installed packages only.")
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
	return ZYPPER_EXIT_ERR_INVALID_ARGS;
      gSettings.additional_sources.push_back(url); 
    }
  }

  // here come commands that need the lock
  try {
    if (command == ZypperCommand::LIST_REPOS)
      zypp_readonly_hack::IWantIt (); // #247001

    God = zypp::getZYpp();
  }
  catch (Exception & excpt_r) {
    ZYPP_CAUGHT (excpt_r);
    ERR  << "A ZYpp transaction is already in progress." << endl;
    cerr << _("A ZYpp transaction is already in progress."
        "This means, there is another application using libzypp library for"
        "package management running. All such applications must be closed before"
        "using this command.") << endl;
    return ZYPPER_EXIT_ERR_ZYPP;
  }

  ResObject::Kind kind;


  // === execute command ===

  // --------------------------( moo )----------------------------------------

  if (command == ZypperCommand::MOO)
  {
    if (ghelp) { cout << specific_help << endl; return !ghelp; }

    // TranslatorExplanation this is a hedgehog, paint another animal, if you want
    cout_n << _("   \\\\\\\\\\\n  \\\\\\\\\\\\\\__o\n__\\\\\\\\\\\\\\'/_") << endl;
    return ZYPPER_EXIT_OK;
  }

  // --------------------------( repo list )----------------------------------
  
  else if (command == ZypperCommand::LIST_REPOS)
  {
    if (ghelp) { cout << specific_help << endl; return !ghelp; }
    // if (ghelp) display_command_help()

    list_repos();
    return ZYPPER_EXIT_OK;
  }

  // --------------------------( addrepo )------------------------------------
  
  else if (command == ZypperCommand::ADD_REPO)
  {
    if (ghelp)
    {
      cout << specific_help;
      return ZYPPER_EXIT_OK;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for modifying system repositories.") << endl;
      return ZYPPER_EXIT_ERR_PRIVILEGES;
    }

    tribool enabled(indeterminate);
    tribool refresh(indeterminate);

    if (copts.count("disabled"))
      enabled = false;
    if (copts.count("no-refresh"))
      refresh = false;

    // add repository specified in .repo file
    if (copts.count("repo"))
    {
      return add_repo_from_file(copts["repo"].front(), enabled, refresh);
    }

    // force specific repository type. Validation is done in add_repo_by_url()
    string type = copts.count("type") ? copts["type"].front() : "";

    // display help message if insufficient info was given
    if (arguments.size() < 2)
    {
      cerr << _("Too few arguments. At least URL and alias are required.") << endl;
      ERR << "Too few arguments. At least URL and alias are required." << endl;
      cout_n << specific_help;
      return ZYPPER_EXIT_ERR_INVALID_ARGS;
    }

    Url url = make_url (arguments[0]);
    if (!url.isValid()) //! \todo error message
      return ZYPPER_EXIT_ERR_INVALID_ARGS;

    string alias;
    if (arguments.size() > 1)
      alias = arguments[1];
    //! \todo use timestamp as alias, if no alias was given
    if (alias.empty ())
      alias = url.asString();

    if (indeterminate(enabled)) enabled = true;
    if (indeterminate(refresh)) refresh = true;

    warn_if_zmd();

    // load gpg keys
    cond_init_target ();

    return add_repo_by_url(url, alias, type, enabled, refresh);
  }

  // --------------------------( delete repo )--------------------------------

  else if (command == ZypperCommand::REMOVE_REPO)
  {
    if (ghelp)
    {
      cout << specific_help;
      return ZYPPER_EXIT_OK;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for modifying system repositories.") << endl;
      return ZYPPER_EXIT_ERR_PRIVILEGES;
    }

    if (arguments.size() < 1)
    {
      cerr << _("Required argument missing.") << endl;
      ERR << "Required argument missing." << endl;
      cout_n << specific_help;
      return ZYPPER_EXIT_ERR_INVALID_ARGS;
    }

    warn_if_zmd ();

    bool found = remove_repo(arguments[0]);
    if (found)
      return ZYPPER_EXIT_OK;

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

    return ZYPPER_EXIT_OK;
  }

  // --------------------------( rename repo )--------------------------------

  else if (command == ZypperCommand::RENAME_REPO)
  {
    if (ghelp)
    {
      cout << specific_help;
      return ZYPPER_EXIT_OK;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for modifying system repositories.") << endl;
      return ZYPPER_EXIT_ERR_PRIVILEGES;
    }

    if (arguments.size() < 2)
    {
      cerr << _("Too few arguments. At least URL and alias are required.") << endl;
      ERR << "Too few arguments. At least URL and alias are required." << endl;
      cout_n << specific_help;
      return ZYPPER_EXIT_ERR_INVALID_ARGS;
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
      return ZYPPER_EXIT_ERR_ZYPP;
    }

    return ZYPPER_EXIT_OK;
  }

  // --------------------------( modify repo )--------------------------------

  else if (command == ZypperCommand::MODIFY_REPO)
  {
    if (ghelp)
    {
      cout << specific_help;
      return ZYPPER_EXIT_OK;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for modifying system repositories.") << endl;
      return ZYPPER_EXIT_ERR_PRIVILEGES;
    }

    if (arguments.size() < 1)
    {
      cerr << _("Alias is a required argument.") << endl;
      ERR << "Na alias argument given." << endl;
      cout_n << specific_help;
      return ZYPPER_EXIT_ERR_INVALID_ARGS;
    }

    modify_repo(arguments[0], copts);
  }

  // --------------------------( refresh )------------------------------------

  else if (command == ZypperCommand::REFRESH)
  {
    if (ghelp)
    {
      cout << specific_help;
      return ZYPPER_EXIT_OK;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for refreshing system repositories.") << endl;
      return ZYPPER_EXIT_ERR_PRIVILEGES;
    }

    refresh_repos();
  }

  // --------------------------( remove/install )-----------------------------

  else if (command == ZypperCommand::INSTALL ||
           command == ZypperCommand::REMOVE)
  {
    if (command == ZypperCommand::INSTALL) {
      if (ghelp || arguments.size() < 1) {
        cerr << specific_help;
        return !ghelp;
      }

      gData.packages_to_install = arguments;

      if (copts.count("auto-agree-with-licenses"))
        gSettings.license_auto_agree = true;

      if (copts.count("machine-readable"))
        gSettings.machine_readable = true;

    }

    if (command == ZypperCommand::REMOVE) {
      if (ghelp || arguments.size() < 1) {
        cerr << specific_help;
        return !ghelp;
      }

      gData.packages_to_uninstall = arguments;
    }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for installing or uninstalling packages.") << endl;
      return ZYPPER_EXIT_ERR_PRIVILEGES;
    }

    // read resolvable type
    string skind = copts.count("type")?  copts["type"].front() : "package";
    kind = string_to_kind (skind);
    if (kind == ResObject::Kind ()) {
      cerr << format(_("Unknown resolvable type: %s")) % skind << endl;
      return ZYPPER_EXIT_ERR_INVALID_ARGS;
    }

    init_repos();

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

    for ( vector<string>::const_iterator it = arguments.begin(); it != arguments.end(); ++it ) {
      if (command == ZypperCommand::INSTALL) {
        mark_for_install(kind, *it);
      }
      else {
        mark_for_uninstall(kind, *it);
      }
    }

    solve_and_commit (copts.count("no-confirm") || gSettings.non_interactive);
    return ZYPPER_EXIT_OK;
  }

  // --------------------------( search )-------------------------------------

  // TODO -c, --catalog option

  else if (command == ZypperCommand::SEARCH)
  {
    ZyppSearchOptions options;

    if (ghelp) {
      cout << specific_help;
      return !ghelp;
    }

    if (gSettings.disable_system_resolvables || copts.count("uninstalled-only"))
      options.setInstalledFilter(ZyppSearchOptions::UNINSTALLED_ONLY);

    if (copts.count("installed-only")) options.setInstalledFilter(ZyppSearchOptions::INSTALLED_ONLY);
    if (copts.count("match-any")) options.setMatchAny();
    if (copts.count("match-words")) options.setMatchWords();
    if (copts.count("match-exact")) options.setMatchExact();
    if (copts.count("search-descriptions")) options.setSearchDescriptions();
    if (copts.count("case-sensitive")) options.setCaseSensitive();

    if (copts.count("type")) {
      string skind = copts["type"].front();
      kind = string_to_kind (skind);
      if (kind == ResObject::Kind ()) {
        cerr << _("Unknown resolvable type ") << skind << endl;
        return ZYPPER_EXIT_ERR_INVALID_ARGS;
      }
      options.setKind(kind);
    }
    else if (gSettings.is_rug_compatible) {
      kind = ResTraits<Package>::kind;
      options.setKind(kind);
    }

    options.resolveConflicts();

    Table t;
    t.style(Ascii);

    ZyppSearch search(God,options,arguments);
    search.doSearch(FillTable(t, search.installedCache()));

    if (t.empty())
      cout_n << _("No resolvables found.") << endl;
    else {
      if (copts.count("sort-by-catalog")) t.sort(1);
      else t.sort(3); // sort by name
      cout << t;
    }

    return ZYPPER_EXIT_OK;
  }

  // --------------------------( patch check )--------------------------------

  // TODO: rug summary
  else if (command == ZypperCommand::PATCH_CHECK) {
    if (ghelp) {
      cout << specific_help;
      return !ghelp;
    }

    cond_init_target ();
    init_repos ();
    // TODO additional_sources
    // TODO warn_no_sources
    // TODO calc token?

    // now load resolvables:
    cond_load_resolvables();

    establish ();
    patch_check ();

    if (gData.security_patches_count > 0)
      return ZYPPER_EXIT_INF_SEC_UPDATE_NEEDED;
    if (gData.patches_count > 0)
      return ZYPPER_EXIT_INF_UPDATE_NEEDED;
    return ZYPPER_EXIT_OK;
  }

  // --------------------------( patches )------------------------------------

  else if (command == ZypperCommand::SHOW_PATCHES) {
    if (ghelp) {
      cout << specific_help;
      return !ghelp;
    }

    cond_init_target ();
    init_repos ();
    cond_load_resolvables();
    establish ();
    show_patches ();
    return ZYPPER_EXIT_OK;
  }

  // --------------------------( list updates )-------------------------------

  else if (command == ZypperCommand::LIST_UPDATES) {
    if (ghelp) {
      // FIXME catalog...
      cout << specific_help;
      return !ghelp;
    }

    string skind = copts.count("type")?  copts["type"].front() :
      gSettings.is_rug_compatible? "package" : "patch";
    kind = string_to_kind (skind);
    if (kind == ResObject::Kind ()) {
	cerr << format(_("Unknown resolvable type: %s")) % skind << endl;
	return ZYPPER_EXIT_ERR_INVALID_ARGS;
    }

    cond_init_target ();
    init_repos ();
    cond_load_resolvables();
    establish ();

    list_updates (kind);

    return ZYPPER_EXIT_OK;
  }


  // -----------------( xml list updates and patches )------------------------

  else if (command == ZypperCommand::XML_LIST_UPDATES_PATCHES) {

    if (ghelp) { cout << specific_help << endl; return !ghelp; }

    cond_init_target ();
    init_repos ();
    cond_load_resolvables();
    establish ();

    cout << "<?xml version='1.0'?>" << endl;
    cout << "<update-status version=\"0.4\">" << endl;
    cout << "<update-list>" << endl;
    xml_list_patches ();
    xml_list_updates ();
    cout << "</update-list>" << endl;
    cout << "</update-status>" << endl;

    return ZYPPER_EXIT_OK;
  }

  // -----------------------------( update )----------------------------------

  else if (command == ZypperCommand::UPDATE) {
    if (ghelp) { cout << specific_help; return !ghelp; }

    // check root user
    if (geteuid() != 0)
    {
      cerr << _("Root privileges are required for updating packages.") << endl;
      return ZYPPER_EXIT_ERR_PRIVILEGES;
    }

    if (copts.count("auto-agree-with-licenses"))
      gSettings.license_auto_agree = true;

    string skind = copts.count("type")?  copts["type"].front() :
      gSettings.is_rug_compatible? "package" : "patch";
    kind = string_to_kind (skind);
    if (kind == ResObject::Kind ()) {
	cerr << format(_("Unknown resolvable type: %s")) % skind << endl;
	return ZYPPER_EXIT_ERR_INVALID_ARGS;
    }

    cond_init_target ();
    init_repos ();
    cond_load_resolvables ();
    establish ();

    bool skip_interactive = copts.count("skip-interactive") || gSettings.non_interactive;
    mark_updates (kind, skip_interactive);

    // commit
    // returns ZYPPER_EXIT_OK, ZYPPER_EXIT_ERR_ZYPP,
    // ZYPPER_EXIT_INF_REBOOT_NEEDED, or ZYPPER_EXIT_INF_RESTART_NEEDED
    return solve_and_commit (copts.count("no-confirm") || gSettings.non_interactive);
  }

  // -----------------------------( info )------------------------------------

  else if (command == ZypperCommand::INFO ||
           command == ZypperCommand::RUG_PATCH_INFO) {
    if (ghelp || arguments.size() == 0) {
      cerr << specific_help;
      return !ghelp;
    }

    cond_init_target ();
    init_repos ();
    cond_load_resolvables ();
    establish ();

    printInfo(command,arguments);

    return ZYPPER_EXIT_OK;
  }

  // if the program reaches this line, something went wrong
  return ZYPPER_EXIT_ERR_BUG;
}

// ----------------------------------------------------------------------------

/// tell to report a bug, and how
// (multiline, with endls)
ostream& report_a_bug (ostream& stm) {
  return stm << _("Please file a bug report about this.") << endl
    // remember not to translate the URL
    // unless you translate the actual page :)
	     << _("See http://en.opensuse.org/Zypper#Troubleshooting for instructions.") << endl;
}

// ----------------------------------------------------------------------------

/// process one command from the OS shell or the zypper shell
// catch unexpected exceptions and tell the user to report a bug (#224216)
int safe_one_command(const ZypperCommand & command, int argc, char **argv)
{
  int ret = ZYPPER_EXIT_ERR_BUG;
  try {
    ret = one_command (command, argc, argv);
  }
  catch (const Exception & ex) {
    ZYPP_CAUGHT(ex);

		if (gSettings.machine_readable)
		{
	   	cerr << "ERROR:" << _("Unexpected exception.") << endl;
	    cerr << "ERROR:" << ex.asUserString() << endl;
		}
		else
		{
	   	cerr << _("Unexpected exception.") << endl;
	    cerr << ex.asUserString() << endl;
    	report_a_bug(cerr);
		}
  }
  return ret;
}

// ----------------------------------------------------------------------------

// Read a string. "\004" (^D) on EOF.
string readline_getline ()
{
  // A static variable for holding the line.
  static char *line_read = NULL;

  /* If the buffer has already been allocated,
     return the memory to the free pool. */
  if (line_read) {
    free (line_read);
    line_read = NULL;
  }

  /* Get a line from the user. */
  line_read = readline ("zypper> ");

  /* If the line has any text in it,
     save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);

  if (line_read)
    return line_read;
  else
    return "\004";
}

// ----------------------------------------------------------------------------

void command_shell ()
{
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
    int sh_argc = args.argc ();
    char **sh_argv = args.argv ();

    string command_str = sh_argv[0]? sh_argv[0]: "";

    if (command_str == "\004") // ^D
    {
      loop = false;
      cout << endl; // print newline after ^D
    }
    else
    {
      try
      {
        ZypperCommand command(command_str);
        if (command == ZypperCommand::SHELL_QUIT)
          loop = false;
        else if (command == ZypperCommand::SHELL)
          cout << _("You already are running zypper's shell.") << endl;
        else
          safe_one_command (command, sh_argc, sh_argv);
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

// ----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  struct Bye {
    ~Bye() {
      cerr_vv << "Exiting main()" << endl;
    }
  } say_goodbye __attribute__ ((__unused__));

	// set locale
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  // logging
  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile == NULL)
    logfile = ZYPP_CHECKPATCHES_LOG;
  zypp::base::LogControl::instance().logfile( logfile );

  // parse global options and the command
  ZypperCommand command = process_globals (argc, argv);
  switch(command.toEnum())
  {
  case ZypperCommand::SHELL_e:
    command_shell();
    return ZYPPER_EXIT_OK;

  case ZypperCommand::NONE_e:
    return ZYPPER_EXIT_ERR_SYNTAX;

  default:
    return safe_one_command(command, argc, argv);
  }

  cerr_v << "This line should never be reached." << endl;
  return ZYPPER_EXIT_ERR_BUG;
}

// Local Variables:
// c-basic-offset: 2
// End:
