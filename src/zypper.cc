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
  {"help",	no_argument, 0, 'h'},
  {"verbose",	no_argument, 0, 'v'},
  {"version",	no_argument, 0, 'V'},
  {"terse",	no_argument, 0, 't'},
  {"table-style", required_argument, 0, 's'},
  {"rug-compatible", no_argument, 0, 'r'},
  {"opt",	optional_argument, 0, 'o'},
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
    cerr << "URL is invalid." << endl;
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
  "\tservice-list, sl\tList services aka installation sources\n"
  "\tservice-add, sa\t\tAdd a new service\n"
  "\tservice-delete, sd\tDelete a service\n"
  "\tservice-rename, sr\tRename a service\n"
  "\trefresh, ref\t\tRefresh all installation sources\n"
  "\tpatch-check, pchk\tCheck for patches\n"
  "\tpatches, pch\t\tList patches\n"
  "\tlist-updates, lu\tList updates\n"
  "\tupdate, up\t\tUpdate packages\n"
  "\tinfo, if\t\tShow full info for packages\n"
  "\tpatch-info\t\tShow full info for patches\n"
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

  string help_global_options = "  Options:\n"
    "\t--help, -h\t\tHelp\n"
    "\t--version, -V\t\tOutput the version number\n"
    "\t--verbose, -v\t\tIncrease verbosity\n"
    "\t--terse, -t\t\tTerse output for machine consumption\n"
    "\t--table-style, -s\tTable style (integer)\n"
    "\t--rug-compatible, -r\tTurn on rug compatibility\n"
    ;

  if (gopts.count("verbose")) {
    gSettings.verbose += gopts["verbose"].size();
    cerr << "verbosity " << gSettings.verbose << endl;
  }

  if (gopts.count("table-style")) {
    unsigned s;
    str::strtonum (gopts["table-style"].front(), s);
    if (s < _End)
      Table::defaultStyle = (TableStyle) s;
    else
      cerr << "Invalid table style " << s << endl;
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
      cerr << "Try -h for help" << endl;
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

  string help_global_source_options = "  Source options:\n"
      "\t--disable-system-sources, -D\t\tDon't read the system sources\n"
      "\t--source, -S\t\tRead additional source\n"
      ;

  string help_global_target_options = "  Target options:\n"
      "\t--disable-system-resolvables, -T\t\tDon't read system installed resolvables\n"
      ;

  if (command == "install" || command == "in") {
    static struct option install_options[] = {
      {"catalog",	   required_argument, 0, 'c'},
      {"type",	     required_argument, 0, 't'},
      {"no-confirm", no_argument,       0, 'y'},
      {"help",       no_argument,       0, 'h'}
    };
    specific_options = install_options;
    specific_help = "  Command options:\n"
      "\t--catalog,-c\t\tOnly from this catalog (FIXME)\n"
      "\t--type,-t\t\tType of resolvable (default: package)\n"
      "\t--no-confirm,-y\tDon't require user confirmation\n"
      ;
  }
  else if (command == "remove" || command == "rm") {
    static struct option remove_options[] = {
      {"type",       required_argument, 0, 't'},
      {"no-confirm", no_argument,       0, 'y'},
      {"help",       no_argument,       0, 'h'}
    };
    specific_options = remove_options;
    specific_help = "  Command options:\n"
      "\t--type,-t\t\tType of resolvable (default: package)\n"
      "\t--no-confirm,-y\tDon't require user confirmation\n"
      ;
  }
  else if (command == "service-add" || command == "sa") {
    static struct option service_add_options[] = {
      {"disabled", no_argument, 0, 'd'},
      {"no-refresh", no_argument, 0, 'n'},
      {"repo", required_argument, 0, 'r'},
      {"help", no_argument, 0, 'h'}
    };
    specific_options = service_add_options;
    specific_help = "service-add [options] URI [alias]\n"
      "\n"
      "Add a service (installation source) to the system."
      "\n"
      "  Command options:\n"
      "\t--repo,-r <FILE.repo>\tRead the URL and alias from a file\n"
      "\t\t\t\t(even remote)\n"
      ;
  }
  else if (command == "service-list" || command == "sl") {
    static struct option service_list_options[] = {
      {"help", no_argument, 0, 'h'}
    };
    specific_options = service_list_options;
    specific_help = "service-list\n"
      "\n"
      "List all defined system services (installation sources)."
      "\n"
      "This command has no options.\n"
      ;
  }
  else if (command == "service-delete" || command == "sd") {
    static struct option service_delete_options[] = {
      {"help", no_argument, 0, 'h'}
    };
    specific_options = service_delete_options;
    specific_help = "service-delete [options] <URI|alias>\n"
      "\n"
      "Remove service (installation source) from the system."
      "\n"
      "This command has no options.\n"
      ;
  }
  else if (command == "service-rename" || command == "sr") {
    static struct option service_rename_options[] = {
      {"help", no_argument, 0, 'h'}
    };
    specific_options = service_rename_options;
    specific_help = "service-rename [options] <URI|alias> <new-alias>\n"
      "\n"
      "Assign new alias to the service specified by URI or current alias."
      "\n"
      "This command has no options.\n"
      ;
  }
  else if (command == "refresh" || command == "ref") {
    static struct option refresh_options[] = {
      {"help", no_argument, 0, 'h'}
    };
    specific_options = refresh_options;
    specific_help = _("zypper refresh\n"
      "\n"
      "Refresh all installation sources found in system.\n")
      ;
  }
  else if (command == "list-updates" || command == "lu") {
    static struct option list_updates_options[] = {
      {"type",		required_argument, 0, 't'},
      {"help", no_argument, 0, 'h'}
    };
    specific_options = list_updates_options;
    specific_help = "list-updates [options]\n"
      "\n"
      "List all available updates\n"
      "\n"
      "  Command options:\n"
      "\t--type,-t\t\tType of resolvable (default: patch!)\n"
      ;
  }
  else if (command == "update" || command == "up") {
    static struct option update_options[] = {
      {"type",		   required_argument, 0, 't'},
      {"no-confirm", no_argument,       0, 'y'},
      {"help", no_argument, 0, 'h'}
    };
    specific_options = update_options;
    specific_help = "  Command options:\n"
      "\t--type,-t\t\tType of resolvable (default: patch!)\n"
      "\t--no-confirm,-y\t\tDon't require user confirmation\n"
      ;
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
      {"help", no_argument, 0, 'h'}
    };
    specific_options = search_options;
    specific_help =
      "zypper [global-options] search [options] [querystring...]\n"
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
      "-u, --uninstalled-only     Show only packages that are not curenly installed.\n"
      "-t, --type                 Search only for packages of specified type.\n"
      "    --sort-by-name         Sort packages by name (default).\n"
      "    --sort-by-catalog      Sort packages by catalog.\n" // ??? catalog/source?
      "\n"
      "* and ? wildcards can also be used within search strings.\n"
      ;
  }
  else if (command == "patch-check" || command == "pchk") {
    static struct option patch_check_options[] = {
      {"help", no_argument, 0, 'h'}
    };
    specific_options = patch_check_options;
    specific_help = "patch-check\n"
      "\n"
      "Check for available patches\n"
      "\n"
      "This command has no options.\n"
      ;
  }
  else if (command == "patches" || command == "pch") {
    static struct option patches_options[] = {
      {"help", no_argument, 0, 'h'}
    };
    specific_options = patches_options;
    specific_help = "patches\n"
      "\n"
      "List all available patches\n"
      "\n"
      "This command has no options.\n"
      ;
  }
  else if (command == "info" || command == "if") {
    static struct option info_options[] = {
      {"help", no_argument, 0, 'h'}
    };
    specific_options = info_options;
    specific_help =
      "zypper [global-options] info [name...]\n"
      "\n"
      "'info' - Show full information for packages\n"
      ;
  }
  else if (command == "patch-info") {
    static struct option patch_info_options[] = {
      {"help", no_argument, 0, 'h'}
    };
    specific_options = patch_info_options;
    specific_help =
      "zypper [global-options] patch-info [patchname...]\n"
      "\n"
      "'patch-info' - Show detailed information for patches\n"
      ;
  }
  else if (!command.empty()) { // empty command is treated earlier
    cerr << "Unknown command '" << command << "'." << endl << endl;
    cerr << help_commands;
    return 1;
  }

  parsed_opts copts = parse_options (argc, argv, specific_options);
  if (copts.count("_unknown"))
    return 1;

  // treat --help command option like global --help option from now on
  // i.e. when used together with command to print command specific help
  ghelp = ghelp || copts.count("help");

  vector<string> arguments;
  if (optind < argc) {
    cerr_v << "non-option ARGV-elements: ";
    while (optind < argc) {
      string argument = argv[optind++];
      cerr_v << argument << ' ';
      arguments.push_back (argument);
    }
    cerr_v << endl;
  }

  // === process options ===

  if (gopts.count("terse")) {
      cerr_v << "FAKE Terse" << endl;
  }

  if (gopts.count("disable-system-sources"))
  {
    MIL << "system sources disabled" << endl;
    gSettings.disable_system_sources = true;
  }
  else
  {
    MIL << "system sources enabled" << endl;
  }

  if (gopts.count("disable-system-resolvables"))
  {
    MIL << "system resolvables disabled" << endl;
    cerr << "Ignoring installed resolvables..." << endl;
    gSettings.disable_system_resolvables = true;
  }
  
  if (gopts.count("source")) {
    list<string> sources = gopts["source"];
    for (list<string>::const_iterator it = sources.begin(); it != sources.end(); ++it )
    {
      Url url = make_url (*it);
      if (!url.isValid())
	return 1;
      gSettings.additional_sources.push_back(url); 
    }
  }
  

  // here come commands that need the lock
  try {
    God = zypp::getZYpp();
  }
  catch (Exception & excpt_r) {
    ZYPP_CAUGHT (excpt_r);
    ERR  << "a ZYpp transaction is already in progress." << endl;
    cerr << "a ZYpp transaction is already in progress." << endl;
    return 1;
  }

  ResObject::Kind kind;


  // === execute command ===

  // --------------------------( moo )----------------------------------------

  if (command == "moo") {
    cout << "   \\\\\\\\\\\n  \\\\\\\\\\\\\\__o\n__\\\\\\\\\\\\\\'/_" << endl;
    return 0;
  }

  // --------------------------( service list )-------------------------------
  
  else if (command == "service-list" || command == "sl")
  {
    if (ghelp) {
      cerr << specific_help << endl;
      return !ghelp;
    }

    if ( geteuid() != 0 )
    {
      cerr << "Sorry, you need root privileges to view system sources." << endl;
      return 1;
    }
    
    list_system_sources();
    return 0;
  }

  // --------------------------( service add )--------------------------------
  
  else if (command == "service-add" || command == "sa")
  {
    if (copts.count("disabled")) {
      cerr_v << "FAKE Disabled" << endl;
    }
    if (copts.count("no-refresh")) {
      cerr_v << "FAKE No Refresh" << endl;
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
      return 1;
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
      add_source_by_url(url, alias);
    }
    catch ( const Exception & excpt_r )
    {
      cerr << excpt_r.asUserString() << endl;
      return 1;
    }

    return 0;
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
      return 1;
    }

    return 0;
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
      return 1;
    }

    return 0;
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
      cerr << "Unknown resolvable type " << skind << endl;
      return 1;
    }

    cond_init_system_sources ();

    for ( std::list<Url>::const_iterator it = gSettings.additional_sources.begin(); it != gSettings.additional_sources.end(); ++it )
      {
	include_source_by_url( *it );
      }
  
    if ( gData.sources.empty() )
      {
	cerr << "Warning! No sources. Operating only over the installed resolvables. You will not be able to install stuff" << endl;
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

      solve_and_commit (copts.count("no-confirm"));
    }
    return 0;
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
        cerr << "Unknown resolvable type " << skind << endl;
        return 1;
      }
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

    return 0;
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
      return 2;
    if (gData.patches_count > 0)
      return 1;
    return 0;
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
    return 0;
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
	cerr << "Unknown resolvable type " << skind << endl;
	return 1;
    }

    cond_init_target ();
    cond_init_system_sources ();
    cond_load_resolvables ();
    establish ();

    list_updates (kind);

    return 0;
  }

  // -----------------------------( update )----------------------------------

  else if (command == "update" || command == "up") {
    if (ghelp) {
      cerr << "update [options]\n"
	   << specific_help
	;
      return !ghelp;
    }

    string skind = copts.count("type")?  copts["type"].front() :
      gSettings.is_rug_compatible? "package" : "patch";
    kind = string_to_kind (skind);
    if (kind == ResObject::Kind ()) {
	cerr << "Unknown resolvable type " << skind << endl;
	return 1;
    }

    cond_init_target ();
    cond_init_system_sources ();
    cond_load_resolvables ();
    establish ();

    mark_updates (kind);
    solve_and_commit (copts.count("no-confirm"));
    return 0;
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

    return 0;
  }

  return 1; // if the program reaches this line, something went wrong
}

void command_shell ()
{
  bool loop = true;
  while (loop) {
    // read a line
    cerr << "zypper> " << flush;
    string line = zypp::str::getline (cin);
    cerr_vv << "Got: " << line << endl;
    // reset optind etc
    optind = 0;
    // split it up and create sh_argc, sh_argv
    Args args(line);
    int sh_argc = args.argc ();
    char **sh_argv = args.argv ();

    string command = sh_argv[0]? sh_argv[0]: "";
    // TODO check empty command

    if (command == "exit")
      loop = false;
    else
      one_command (command, sh_argc, sh_argv);
  }
}

int main(int argc, char **argv)
{
  struct Bye {
    ~Bye() {
      cerr_vv << "Exiting main()" << endl;
    }
  } say_goodbye __attribute__ ((__unused__));

  // logging
  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile == NULL)
    logfile = ZYPP_CHECKPATCHES_LOG;
  zypp::base::LogControl::instance().logfile( logfile );
  
  // parse global options and the command
  string command = process_globals (argc, argv);
  int ret = 0;
  if (command == "shell" || command == "sh")
    command_shell ();
  else
    ret = one_command (command, argc, argv);

  return ret;
}
// Local Variables:
// c-basic-offset: 2
// End:
