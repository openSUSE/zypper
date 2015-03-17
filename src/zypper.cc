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

#include <zypp/zypp_detail/ZYppReadOnlyHack.h>

#include "zmart.h"
#include "zmart-sources.h"
#include "zmart-misc.h"

#include "zmart-rpm-callbacks.h"
#include "zmart-keyring-callbacks.h"
#include "zmart-source-callbacks.h"
#include "zmart-media-callbacks.h"
#include "zypper-tabulator.h"
#include "zypper-search.h"
#include "zypper-info.h"
#include "zypper-getopt.h"

using namespace std;
using namespace boost;
using namespace zypp;
using namespace zypp::detail;

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
  {"version",         no_argument,       0, 'V'},
  {"terse",           no_argument,       0, 't'},
  {"table-style",     required_argument, 0, 's'},
  {"rug-compatible",  no_argument,       0, 'r'},
  {"non-interactive", no_argument,       0, 'n'},
  {"no-gpg-checks",   no_argument,       0, 0},
  {"root",            required_argument, 0, 'R'},
  {"opt",             optional_argument, 0, 'o'},
  {0, 0, 0, 0}
};

//! a constructor wrapper catching exceptions
// returns an empty one on error
Url make_url (const string & url_s) {
  Url u;
  try {
    u = Url (url_s);
  }
  catch ( const Exception & excpt_r ) {
    ZYPP_CAUGHT( excpt_r );
    cerr << _("URL is invalid.") << endl;
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
  "\tservice-list, sl\tList services, also called installation sources\n"
  "\tservice-add, sa\t\tAdd a new service\n"
  "\tservice-delete, sd\tDelete a service\n"
  "\tservice-rename, sr\tRename a service\n"
  "\trefresh, ref\t\tRefresh all installation sources\n"
  "\tpatch-check, pchk\tCheck for patches\n"
  "\tpatches, pch\t\tList patches\n"
  "\tlist-updates, lu\tList updates\n"
  "\tupdate, up\t\tUpdate packages\n"
  "\tinfo, if\t\tShow full information for packages\n"
  "\tpatch-info\t\tShow full information for patches\n"
  "");

// global options
parsed_opts gopts;
bool ghelp = false;

// parses global options, returns the command
string process_globals(int argc, char **argv)
{
  // global options
  gopts = parse_options (argc, argv, global_options);
  if (gopts.count("_unknown"))
    return "_unknown";

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
    "\t--help, -h\t\tHelp\n"
    "\t--version, -V\t\tOutput the version number\n"
    "\t--verbose, -v\t\tIncrease verbosity\n"
    "\t--terse, -t\t\tTerse output for machine consumption\n"
    "\t--table-style, -s\tTable style (integer)\n"
    "\t--rug-compatible, -r\tTurn on rug compatibility\n"
    "\t--non-interactive, -n\tDo not ask anything, use default answers automatically.\n"
    "\t--no-gpg-checks\t\tIgnore GPG check failures and continue.\n"
    "\t--root, -R <dir>\tOperate on a different root directory\n");

  if (gopts.count("verbose")) {
    gSettings.verbose += gopts["verbose"].size();
    cerr << _("Verbosity ") << gSettings.verbose << endl;
    DBG << "Verbosity " << gSettings.verbose << endl;
  }

  if (gopts.count("non-interactive")) {
    gSettings.non_interactive = true;
    cout << _("Entering non-interactive mode.") << endl;
    MIL << "Entering non-interactive mode" << endl;
  }

  if (gopts.count("no-gpg-checks")) {
    gSettings.no_gpg_checks = true;
    cout << _("Entering 'no-gpg-checks' mode.") << endl;
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
    cerr << "Opt arg: ";
    std::copy (gopts["opt"].begin(), gopts["opt"].end(),
	       ostream_iterator<string> (cerr, ", "));
    cerr << endl;
  }

  string command;

  if (optind < argc) {
    command = argv[optind++];
  }
  if (command == "help") {
    ghelp = true;
    if (optind < argc) {
      command = argv[optind++];
    }
    else {
      command = "";
    }
  }

  if (command.empty()) {
    if (ghelp) {
      cerr << help_global_options << help_commands;
    }
    else if (gopts.count("version")) {
      cerr << PACKAGE_STRING;
#ifdef LIBZYPP_1xx
      cerr << " (libzypp-1.x.x)";
#endif
      cerr << endl;
    }
    else {
      cerr << _("Try -h for help") << endl;
    }
  }

  cerr_vv << "COMMAND: " << command << endl;
  return command;
}

/// process one command from the OS shell or the zypper shell
int one_command(const string& command, int argc, char **argv)
{
  // === command-specific options ===

  struct option no_options = {0, 0, 0, 0};
  struct option *specific_options = &no_options;
  string specific_help;

  string help_global_source_options = _(
      "  Source options:\n"
      "\t--disable-system-sources, -D\t\tDo not read the system sources\n"
      "\t--source, -S\t\tRead additional source\n"
      );

  string help_global_target_options = _("  Target options:\n"
      "\t--disable-system-resolvables, -T\t\tDo not read system installed resolvables\n"
      );

  if (command == "install" || command == "in") {
    static struct option install_options[] = {
      {"catalog",	   required_argument, 0, 'c'},
      {"type",	     required_argument, 0, 't'},
      {"no-confirm", no_argument,       0, 'y'},
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      // rug compatibility, we have --auto-agree-with-licenses
      {"agree-to-third-party-licenses",  no_argument,  0, 0},
      {"help",       no_argument,       0, 'h'},
      {"download-only",		no_argument,	0, 'd'},
      {0, 0, 0, 0}
    };
    specific_options = install_options;
    specific_help = _(
      "  Command options:\n"
      "\t--catalog,-c\t\t\tOnly from this catalog (under development)\n"
      "\t--type,-t\t\tType of resolvable (default: package)\n"
      "\t--no-confirm,-y\t\t\tDo not require user confirmation to proceed with installation\n"
      "\t--auto-agree-with-licenses,-l\tAutomatically say 'yes' to third party license confirmation prompt.\n"
      "\t\t\t\t\tSee man zypper for more details.\n"
      );
    // TranslatorExplanation: Command option help; leave leading tabs (\t) in place
    specific_help += _("\t--download-only,-d\t\tOnly download the packages, do not install.\n");
  }
  else if (command == "remove" || command == "rm") {
    static struct option remove_options[] = {
      {"type",       required_argument, 0, 't'},
      {"no-confirm", no_argument,       0, 'y'},
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      // rug compatibility, we have --auto-agree-with-licenses
      {"agree-to-third-party-licenses",  no_argument,  0, 0},
      {"help",       no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = remove_options;
    specific_help = _(
      "  Command options:\n"
      "\t--type,-t\t\tType of resolvable (default: package)\n"
      "\t--no-confirm,-y\tDo not require user confirmation\n"
      "\t--auto-agree-with-licenses,-l\tAutomatically say 'yes' to third party license confirmation prompt.\n"
      "\t\t\t\t\tSee man zypper for more details.\n"
      );
  }
  else if (command == "service-add" || command == "sa") {
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
      "service-add [options] URI [alias]\n"
      "\n"
      "Add a service (installation source) to the system."
      "\n"
      "  Command options:\n"
      "\t--repo,-r <FILE.repo>\tRead the URL and alias from a file\n"
      "\t\t\t\t(even remote)\n"
      "\t--type,-t <TYPE>\tType of repository (YaST, YUM, or Plaindir)\n"
      "\t--disabled,-d\t\tAdd the service as disabled\n"
      "\t--no-refresh,-n\t\tDo not automatically refresh the metadata\n"
      );
  }
  else if (command == "service-list" || command == "sl") {
    static struct option service_list_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_list_options;
    specific_help = _(
      "service-list\n"
      "\n"
      "List all defined system services (installation sources)."
      "\n"
      "This command has no options.\n"
      );
  }
  else if (command == "service-delete" || command == "sd") {
    static struct option service_delete_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_delete_options;
    specific_help = _(
      "service-delete [options] <URI|alias>\n"
      "\n"
      "Remove service (installation source) from the system."
      "\n"
      "This command has no options.\n"
      );
  }
  else if (command == "service-rename" || command == "sr") {
    static struct option service_rename_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = service_rename_options;
    specific_help = _(
      "service-rename [options] <URI|alias> <new-alias>\n"
      "\n"
      "Assign new alias to the service specified by URI or current alias."
      "\n"
      "This command has no options.\n"
      );
  }
  else if (command == "refresh" || command == "ref") {
    static struct option refresh_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = refresh_options;
    specific_help = _(
      "zypper refresh\n"
      "\n"
      "Refresh all installation sources found in the system.\n"
      );
  }
  else if (command == "list-updates" || command == "lu") {
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
      "\t--type,-t\t\tType of resolvable (default: patch)\n"
      );
  }
  else if (command == "update" || command == "up") {
    static struct option update_options[] = {
      {"type",		   required_argument, 0, 't'},
      {"no-confirm", no_argument,       0, 'y'},
      {"skip-interactive", no_argument, 0, 0},
      {"auto-agree-with-licenses",  no_argument,       0, 'l'},
      // rug compatibility, we have --auto-agree-with-licenses
      {"agree-to-third-party-licenses",  no_argument,  0, 0},
      {"help", no_argument, 0, 'h'},
      {"download-only",		no_argument,	0, 'd'},
      {0, 0, 0, 0}
    };
    specific_options = update_options;
    specific_help = _(
      "  Command options:\n"
      "\t--type,-t\t\tType of resolvable (default: patch)\n"
      "\t--no-confirm,-y\t\tDo not require user confirmation\n"
      "\t--skip-interactive\t\tSkip interactive updates\n"
      "\t--auto-agree-with-licenses,-l\tAutomatically say 'yes' to third party license confirmation prompt.\n"
      "\t\t\t\t\tSee man zypper for more details.\n"
      );
    // TranslatorExplanation: Command option help; leave leading tabs (\t) in place
    specific_help += _("\t--download-only,-d\t\tOnly download the packages, do not install.\n");
  }
  else if (command == "search" || command == "se") {
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
      "zypper [global-options] search [options] [querystring...]\n"
      "\n"
      "'search' - Search for packages matching given search strings\n"
      "\n"
      "  Command options:\n"
      "    --match-all            Search for a match to all search strings (default)\n"
      "    --match-any            Search for a match to any of the search strings\n"
      "    --match-substrings     Search for a match to partial words (default)\n"
      "    --match-words          Search for a match to whole words only\n"
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
  else if (command == "patch-check" || command == "pchk") {
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
      "This command has no options.\n"
      );
  }
  else if (command == "patches" || command == "pch") {
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
      "This command has no options.\n"
      );
  }
  else if (command == "info" || command == "if") {
    static struct option info_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = info_options;
    specific_help = _(
      "zypper [global-options] info [name...]\n"
      "\n"
      "'info' -- Show full information for packages\n"
      );
  }
  else if (command == "patch-info") {
    static struct option patch_info_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = patch_info_options;
    specific_help = _(
      "zypper [global-options] patch-info [patchname...]\n"
      "\n"
      "'patch-info' -- Show detailed information for patches\n"
      );
  }
  else if (command == "moo") {
    static struct option moo_options[] = {
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
    specific_options = moo_options;
    specific_help = _(
      "zypper [global-options] moo\n"
      "\n"
      "'moo' - Show an animal\n"
      );
  }
  else if (!command.empty()) { // empty command is treated earlier
    if (command != "help")	// #235709
      cerr << _("Unknown command") << " '" << command << "'." << endl << endl;
    cerr << help_commands;
    return ZYPPER_EXIT_ERR_SYNTAX;
  }

  parsed_opts copts = parse_options (argc, argv, specific_options);
  if (copts.count("_unknown"))
    return ZYPPER_EXIT_ERR_SYNTAX;

  // treat --help command option like global --help option from now on
  // i.e. when used together with command to print command specific help
  ghelp = ghelp || copts.count("help");

  vector<string> arguments;
  if (optind < argc) {
    cerr_v << _("Non-Option Program Arguments: ");
    while (optind < argc) {
      string argument = argv[optind++];
      cerr_v << argument << ' ';
      arguments.push_back (argument);
    }
    cerr_v << endl;
  }

  // === process options ===

  if (gopts.count("terse")) {
      cerr_v << _("Ignoring --terse (provided only for rug compatibility)") << endl;
  }

  if (gopts.count("disable-system-sources"))
  {
    MIL << "System sources disabled" << endl;
    gSettings.disable_system_sources = true;
  }
  else
  {
    MIL << "System sources enabled" << endl;
  }

  if (gopts.count("disable-system-resolvables"))
  {
    MIL << "System resolvables disabled" << endl;
    cerr << _("Ignoring installed resolvables...") << endl;
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
    const char *roh = getenv("ZYPP_READONLY_HACK");
    if (roh != NULL && roh[0] == '1')
      zypp_readonly_hack::IWantIt ();
    else if (command == "service-list" || command == "sl")
      zypp_readonly_hack::IWantIt (); // #247001

    God = zypp::getZYpp();
  }
  catch (Exception & excpt_r) {
    ZYPP_CAUGHT (excpt_r);
    ERR  << "A ZYpp transaction is already in progress." << endl;
    cerr << _("A ZYpp transaction is already in progress.") << endl;
    return ZYPPER_EXIT_ERR_ZYPP;
  }

  ResObject::Kind kind;


  // === execute command ===

  // --------------------------( moo )----------------------------------------

  if (command == "moo") {
    cout << "   \\\\\\\\\\\n  \\\\\\\\\\\\\\__o\n__\\\\\\\\\\\\\\'/_" << endl;
    return ZYPPER_EXIT_OK;
  }

  // --------------------------( service list )-------------------------------
  
  else if (command == "service-list" || command == "sl")
  {
    if (ghelp) {
      cerr << specific_help << endl;
      return !ghelp;
    }

    list_system_sources();
    return ZYPPER_EXIT_OK;
  }

  // --------------------------( service add )--------------------------------
  
  else if (command == "service-add" || command == "sa")
  {
    // TODO: repect values in .repo, have these as overrides
    bool enabled = ! copts.count("disabled");
    bool refresh = ! copts.count("no-refresh");

    string type = copts.count("type")?  copts["type"].front() : "";
    if (type != "" && type != "YaST" && type != "YUM" && type != "Plaindir") {
      cerr << _("Warning: Unknown metadata type ") << type << endl;
    }

    string repoalias, repourl;
    if (copts.count("repo")) {
      string filename = copts["repo"].front();
      // it may be an URL; cache the file while MediaWrapper exists
      MediaWrapper download (filename);
      filename = download.localPath ();
      cerr_vv << "Got: " << filename << endl;
      parse_repo_file (filename, repourl, repoalias);
    }

    if (ghelp || (arguments.size() < 1 && repoalias.empty ())) {
      cerr << specific_help;
      return !ghelp;
    }

    if (repourl.empty())
      repourl = arguments[0];

    Url url = make_url (repourl);
    if (!url.isValid())
      return ZYPPER_EXIT_ERR_INVALID_ARGS;
    string alias = repoalias;
    if (alias.empty ())
      alias = url.asString();
    if (arguments.size() > 1)
      alias = arguments[1];

    warn_if_zmd ();

    // load gpg keys
    cond_init_target ();

    try {
      // also stores it
      add_source_by_url(url, alias, type, enabled, refresh);
    }
    catch ( const Exception & excpt_r )
    {
      cerr << excpt_r.asUserString() << endl;
      return ZYPPER_EXIT_ERR_ZYPP;
    }

    return ZYPPER_EXIT_OK;
  }

  // --------------------------( service delete )-----------------------------

  else if (command == "service-delete" || command == "sd")
  {
    if (ghelp || arguments.size() < 1) {
      cerr << specific_help;
      return !ghelp;
    }

    warn_if_zmd ();
    try {
      // also stores it
      remove_source(arguments[0]);
    }
    catch ( const Exception & excpt_r )
    {
      ZYPP_CAUGHT (excpt_r);
      cerr << excpt_r.asUserString() << endl;
      return ZYPPER_EXIT_ERR_ZYPP;
    }

    return ZYPPER_EXIT_OK;
  }

  // --------------------------( service rename )-----------------------------

  else if (command == "service-rename" || command == "sr")
  {
    if (ghelp || arguments.size() < 2) {
      cerr << specific_help;
      return !ghelp;
    }

    cond_init_target ();
    warn_if_zmd ();
    try {
      // also stores it
      rename_source (arguments[0], arguments[1]);
    }
    catch ( const Exception & excpt_r )
    {
      cerr << excpt_r.asUserString() << endl;
      return ZYPPER_EXIT_ERR_ZYPP;
    }

    return ZYPPER_EXIT_OK;
  }
  
  // --------------------------( refresh )------------------------------------

  else if (command == "refresh" || command == "ref") {
    if (ghelp) {
      cerr << specific_help;
      return !ghelp;
    }
    
    refresh_sources();
  }

  // --------------------------( remove/install )-----------------------------

  else if (command == "install" || command == "in" ||
      command == "remove" || command == "rm") {

    if (command == "install" || command == "in") {
      if (ghelp || arguments.size() < 1) {
        cerr << "install [options] name...\n"
        << specific_help
        ;
        return !ghelp;
      }

      gData.packages_to_install = arguments;
    }

    if (copts.count("download-only"))
      gSettings.downloadOnly = true;

    if (copts.count("auto-agree-with-licenses")
	|| copts.count("agree-to-third-party-licenses"))
      gSettings.license_auto_agree = true;

    if (command == "remove" || command == "rm") {
      if (ghelp || arguments.size() < 1) {
        cerr << "remove [options] name...\n"
        << specific_help
        ;
        return !ghelp;
      }

      gData.packages_to_uninstall = arguments;
    }

    string skind = copts.count("type")?  copts["type"].front() : "package";
    kind = string_to_kind (skind);
    if (kind == ResObject::Kind ()) {
      cerr << _("Unknown resolvable type ") << skind << endl;
      return ZYPPER_EXIT_ERR_INVALID_ARGS;
    }

    cond_init_system_sources ();

    for ( std::list<Url>::const_iterator it = gSettings.additional_sources.begin(); it != gSettings.additional_sources.end(); ++it )
      {
	include_source_by_url( *it );
      }
  
    if ( gData.sources.empty() )
      {
	cerr << _("Warning: No sources. Operating only with the installed resolvables. Nothing can be installed.") << endl;
      } 

    cond_init_target ();
    cond_load_resolvables ();

    for ( vector<string>::const_iterator it = arguments.begin(); it != arguments.end(); ++it ) {
      if (command == "install" || command == "in") {
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

  else if (command == "search" || command == "se") {
    ZyppSearchOptions options;

    if (ghelp) {
      cerr << specific_help;
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
      cout << "No packages found." << endl;
    else {
      if (copts.count("sort-by-catalog")) t.sort(1);
      else t.sort(3); // sort by name
      cout << t;
    }

    return ZYPPER_EXIT_OK;
  }

  // --------------------------( patch check )--------------------------------

  // TODO: rug summary
  else if (command == "patch-check" || command == "pchk") {
    if (ghelp) {
      cerr << specific_help;
      return !ghelp;
    }

    cond_init_target ();
    cond_init_system_sources ();
    // TODO additional_sources
    // TODO warn_no_sources
    // TODO calc token?

    // now load resolvables:
    cond_load_resolvables ();

    establish ();
    patch_check ();

    if (gData.security_patches_count > 0)
      return ZYPPER_EXIT_INF_SEC_UPDATE_NEEDED;
    if (gData.patches_count > 0)
      return ZYPPER_EXIT_INF_UPDATE_NEEDED;
    return ZYPPER_EXIT_OK;
  }

  // --------------------------( patches )------------------------------------

  else if (command == "patches" || command == "pch") {
    if (ghelp) {
      cerr << specific_help;
      return !ghelp;
    }

    cond_init_target ();
    cond_init_system_sources ();
    cond_load_resolvables ();
    establish ();
    show_patches ();
    return ZYPPER_EXIT_OK;
  }

  // --------------------------( list updates )-------------------------------

  else if (command == "list-updates" || command == "lu") {
    if (ghelp) {
      // FIXME catalog...
      cerr << specific_help;
      return !ghelp;
    }

    string skind = copts.count("type")?  copts["type"].front() :
      gSettings.is_rug_compatible? "package" : "patch";
    kind = string_to_kind (skind);
    if (kind == ResObject::Kind ()) {
	cerr << _("Unknown resolvable type ") << skind << endl;
	return ZYPPER_EXIT_ERR_INVALID_ARGS;
    }

    cond_init_target ();
    cond_init_system_sources ();
    cond_load_resolvables ();
    establish ();

    list_updates (kind);

    return ZYPPER_EXIT_OK;
  }

  // -----------------------------( update )----------------------------------

  else if (command == "update" || command == "up") {
    if (ghelp) {
      cerr << "update [options]\n"
	   << specific_help
	;
      return !ghelp;
    }

    if (copts.count("download-only"))
      gSettings.downloadOnly = true;

    if (copts.count("auto-agree-with-licenses") ||
        copts.count("agree-to-third-party-licenses"))
      gSettings.license_auto_agree = true;

    string skind = copts.count("type")?  copts["type"].front() :
      gSettings.is_rug_compatible? "package" : "patch";
    kind = string_to_kind (skind);
    if (kind == ResObject::Kind ()) {
	cerr << _("Unknown resolvable type ") << skind << endl;
	return ZYPPER_EXIT_ERR_INVALID_ARGS;
    }

    cond_init_target ();
    cond_init_system_sources ();
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

  else if (command == "info" || command == "if" || command == "patch-info") {
    if (ghelp || arguments.size() == 0) {
      cerr << specific_help;
      return !ghelp;
    }

    cond_init_target ();
    cond_init_system_sources ();
    cond_load_resolvables ();
    establish ();

    printInfo(command,arguments);

    return ZYPPER_EXIT_OK;
  }

  // if the program reaches this line, something went wrong
  return ZYPPER_EXIT_ERR_BUG;
}

/// tell to report a bug, and how
// (multiline, with endls)
ostream& report_a_bug (ostream& stm) {
  return stm << _("Please file a bug report about this.") << endl
    // remember not to translate the URL
    // unless you translate the actual page :)
	     << _("See http://en.opensuse.org/Zypper#Troubleshooting for instructions.") << endl;
}

/// process one command from the OS shell or the zypper shell
// catch unexpected exceptions and tell the user to report a bug (#224216)
int safe_one_command(const string& command, int argc, char **argv)
{
  int ret = ZYPPER_EXIT_ERR_BUG;
  try {
    ret = one_command (command, argc, argv);
  }
  catch (const Exception & ex) {
    ZYPP_CAUGHT(ex);
    cerr << _("Unexpected exception.") << endl;
    cerr << ex.asUserString() << endl;
    report_a_bug(cerr);
  }
  return ret;
}

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

    string command = sh_argv[0]? sh_argv[0]: "";

    if (command == "\004") {
      loop = false;
      cout << endl;
    }
    else if (command == "exit" || command == "quit")
      loop = false;
    else
      safe_one_command (command, sh_argc, sh_argv);
  }

  if (!histfile.empty ())
    write_history (histfile.c_str ());
}

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
  string command = process_globals (argc, argv);

  // except for help, quit if not root (#344515)
  if (!ghelp && geteuid() != 0 )
  {
    cerr << _("Root privileges are required for running zypper.") << endl;
    return ZYPPER_EXIT_ERR_PRIVILEGES;
  }

  int ret = 0;
  if (command == "shell" || command == "sh")
    command_shell ();
  else
    ret = safe_one_command (command, argc, argv);

  return ret;
}
// Local Variables:
// c-basic-offset: 2
// End:
