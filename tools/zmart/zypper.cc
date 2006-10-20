// zypper - a command line interface for libzypp
// http://en.opensuse.org/User:Mvidner

// (initially based on dmacvicar's zmart)

#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <list>
#include <map>
#include <iterator>

#include <unistd.h>
#include <getopt.h>

#include "zmart.h"
#include "zmart-sources.h"
#include "zmart-misc.h"

#include "zmart-rpm-callbacks.h"
#include "zmart-keyring-callbacks.h"
#include "zmart-source-callbacks.h"
#include "zmart-media-callbacks.h"
#include "zypper-tabulator.h"
#include "zypper-search.h"

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

typedef map<string, list<string> > parsed_opts;

string longopts2optstring (const struct option *longopts) {
  // + - do not permute, stop at the 1st nonoption, which is the command
  // : - return : to indicate missing arg, not ?
  string optstring = "+:";
  for (; longopts && longopts->name; ++longopts) {
    if (!longopts->flag && longopts->val) {
      optstring += (char) longopts->val;
      if (longopts->has_arg == required_argument)
	optstring += ':';
      else if (longopts->has_arg == optional_argument)
	optstring += "::";
    }
  }
  //cerr << optstring << endl;
  return optstring;
}

typedef map<int,const char *> short2long_t;

short2long_t make_short2long (const struct option *longopts) {
  short2long_t result;
  for (; longopts && longopts->name; ++longopts) {
    if (!longopts->flag && longopts->val) {
      result[longopts->val] = longopts->name;
    }
  }
  return result;
}

// longopts.flag must be NULL
parsed_opts parse_options (int argc, char **argv,
			   const struct option *longopts) {
  parsed_opts result;
  opterr = 0; 			// we report errors on our own
  int optc;
  string optstring = longopts2optstring (longopts);
  short2long_t short2long = make_short2long (longopts);

  while (1) {
    int option_index = 0;
    optc = getopt_long (argc, argv, optstring.c_str (),
			longopts, &option_index);
    if (optc == -1)
      break;			// options done

    switch (optc) {
    case '?':
      result["_unknown"].push_back("");
      cerr << "Unknown option " << argv[optind - 1] << endl;
      break;
    case ':':
      cerr << "Missing argument for " << argv[optind - 1] << endl;
      break;
    default:
      const char *mapidx = optc? short2long[optc] : longopts[option_index].name;

      // creates if not there
      list<string>& value = result[mapidx];
      if (optarg)
	value.push_back (optarg);
      else
	value.push_back ("");
      break;
    }
  }
  return result;
}

static struct option global_options[] = {
  {"help",	no_argument, 0, 'h'},
  {"verbose",	no_argument, 0, 'v'},
  {"version",	no_argument, 0, 'V'},
  {"terse",	no_argument, 0, 't'},
  {"table-style", required_argument, 0, 's'},
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

int main(int argc, char **argv)
{
  struct Bye {
    ~Bye() {
      cerr_vv << "Exiting main()" << endl;
    }
  } say_goodbye __attribute__ ((__unused__));

  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile == NULL)
    logfile = ZYPP_CHECKPATCHES_LOG;
  zypp::base::LogControl::instance().logfile( logfile );
  
  bool help = false;
  parsed_opts gopts = parse_options (argc, argv, global_options);
  if (gopts.count("_unknown"))
    return 1;

  // Help is parsed by setting the help flag for a command, which may be empty
  // $0 -h,--help
  // $0 command -h,--help
  // The help command is eaten and transformed to the help option
  // $0 help
  // $0 help command
  if (gopts.count("help"))
    help = true;

  string help_global_options = "  Options:\n"
    "\t--help, -h\t\tHelp\n"
    "\t--version, -V\t\tOutput the version number\n"
    "\t--verbose, -v\t\tIncrease verbosity\n"
    "\t--terse, -t\t\tTerse output for machine consumption\n"
    "\t--table-style, -s\tTable style (integer)\n"
    ;

  if (gopts.count("version")) {
    cerr << "zypper 0.1" << endl;
    return 0;
  }

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
    help = true;
    if (optind < argc) {
      command = argv[optind++];
    }
    else {
      command = "";
    }
  }

  if (command.empty()) {
    cerr_vv << "No command" <<endl;
    //command = "help";
  }
  cerr_vv << "COMMAND: " << command << endl;

  // === command-specific options ===

  struct option no_options = {0, 0, 0, 0};
  struct option *specific_options = &no_options;
  string specific_help;

  string help_commands = "  Commands:\n"
      "\thelp\t\t\tHelp\n"
      "\tinstall, in\t\tInstall packages or resolvables\n"
      "\tremove, rm\t\tRemove packages or resolvables\n"
      "\tsearch, se\t\tSearch for packages matching a pattern\n"
      "\tservice-list, sl\tList services aka installation sources\n"
      "\tservice-add, sa\t\tAdd a new service\n"
      "\tservice-delete, sd\tDelete a service\n"
      "\tservice-rename, sr\tRename a service\n"
      "\tpatch-check, pchk\tCheck for patches\n"
      "\tpatches, pch\t\tList patches\n"
      "\tlist-updates, lu\tList updates\n"
      "\tupdate, up\tUpdate packages\n"
      ;

  string help_global_source_options = "  Source options:\n"
      "\t--disable-system-sources, -D\t\tDon't read the system sources\n"
      "\t--source, -S\t\tRead additional source\n"
      ;

  string help_global_target_options = "  Target options:\n"
      "\t--disable-system-resolvables, -T\t\tDon't read system installed resolvables\n"
      ;

  if (command == "install" || command == "in") {
    static struct option install_options[] = {
      {"catalog",	required_argument, 0, 'c'},
      {"type",		required_argument, 0, 't'},
      {0, 0, 0, 0}
    };
    specific_options = install_options;
    specific_help = "  Command options:\n"
      "\t--catalog,-c\t\tOnly from this catalog (FIXME)\n"
      "\t--type,-t\t\tType of resolvable (default: package)\n"
      ;
  }
  else if (command == "remove" || command == "rm") {
    static struct option remove_options[] = {
      {"type",		required_argument, 0, 't'},
      {0, 0, 0, 0}
    };
    specific_options = remove_options;
    specific_help = "  Command options:\n"
      "\t--type,-t\t\tType of resolvable (default: package)\n"
      ;
  }
  else if (command == "service-add" || command == "sa") {
    static struct option service_add_options[] = {
      {"disabled", no_argument, 0, 'd'},
      {"no-refresh", no_argument, 0, 'n'},
      {"repo", required_argument, 0, 'r'},
      {0, 0, 0, 0}
    };
    specific_options = service_add_options;
    specific_help = "  Command options:\n"
      "\t--repo,-r <FILE.repo>\tRead the URL and alias from a file\n"
      "\t\t\t\t(even remote)\n"
      ;
  }
  else if (command == "service-list" || command == "sl") {
    static struct option service_list_options[] = {
      {0, 0, 0, 0}
    };
    specific_options = service_list_options;
    specific_help = "  Command options:\n"
      "\n"
      ;
  }
  else if (command == "list-updates" || command == "lu") {
    static struct option remove_options[] = {
      {"type",		required_argument, 0, 't'},
      {0, 0, 0, 0}
    };
    specific_options = remove_options;
    specific_help = "  Command options:\n"
      "\t--type,-t\t\tType of resolvable (default: patch)\n"
      ;
  }
  else if (command == "update" || command == "up") {
    static struct option remove_options[] = {
      {"type",		required_argument, 0, 't'},
      {0, 0, 0, 0}
    };
    specific_options = remove_options;
    specific_help = "  Command options:\n"
      "\t--type,-t\t\tType of resolvable (default: patch)\n"
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
      {"help", no_argument, 0, 0},
    };
    specific_options = search_options;
    specific_help =
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
      "-u, --uninstalled-only     Show only packages that are not curenly installed.\n"
      "-t, --type                 Search only for packages of specified type.\n"
      "    --sort-by-name         Sort packages by name (default).\n"
      "    --sort-by-catalog      Sort packages by catalog.\n" // ??? catalog/source?
      "\n"
      "* and ? wildcards can also be used within search strings.\n"
      ;
  }
  else {
    cerr_vv << "No options declared for command " << command << endl;
    // no options. or make this an exhaustive thing?
    //    cerr << "Unknown command" << endl;
    //    return 1;
  }

  parsed_opts copts = parse_options (argc, argv, specific_options);
  if (copts.count("_unknown"))
    return 1;

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
  
  // === execute command ===

  if (command.empty()) {
    if (help) {
      cerr << help_global_options << help_commands;
    }
    else {
      cerr << "Try -h for help" << endl;
    }
    return 0;
  }

  if (command == "moo") {
    cout << "   \\\\\\\\\\\n  \\\\\\\\\\\\\\__o\n__\\\\\\\\\\\\\\'/_" << endl;
    return 0;
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

  // --------------------------( service list )-------------------------------
  
  if (command == "service-list" || command == "sl")
  {
    if ( geteuid() != 0 )
    {
      cerr << "Sorry, you need root privileges to view system sources." << endl;
      return 1;
    }
    
    list_system_sources();
    return 0;
  }

  // --------------------------( service add )--------------------------------
  
  if (command == "service-add" || command == "sa")
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

    if (help || (arguments.size() < 1 && repoalias.empty ())) {
      cerr << "service-add [options] URI [alias]\n"
	""
	   << specific_help
	;
      return !help;
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

  if (command == "service-delete" || command == "sd")
  {
    if (help || arguments.size() < 1) {
      cerr << "service-delete [options] <URI|alias>\n"
	   << specific_help
	;
      return !help;
    }

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

  if (command == "service-rename" || command == "sr")
  {
    if (help || arguments.size() < 2) {
      cerr << "service-rename [options] <URI|alias> <new-alias>\n"
	   << specific_help
	;
      return !help;
    }
    
    cond_init_target ();
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
  
  ResObject::Kind kind;

  // --------------------------( remove/install )-----------------------------
  
  if (command == "install" || command == "in") {
    if (help || arguments.size() < 1) {
      cerr << "install [options] name...\n"
	   << specific_help
	;
      return !help;
    }
      
    gData.packages_to_install = arguments;
  }
  if (command == "remove" || command == "rm") {
    if (help || arguments.size() < 1) {
      cerr << "remove [options] name...\n"
	   << specific_help
	;
      return !help;
    }

    gData.packages_to_uninstall = arguments;
  }

  if (!gData.packages_to_install.empty() || !gData.packages_to_uninstall.empty()) {
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
    if (true) {
	cond_load_resolvables ();

	for ( vector<string>::const_iterator it = gData.packages_to_install.begin(); it != gData.packages_to_install.end(); ++it ) {
	  mark_for_install(kind, *it);
	}
    
	for ( vector<string>::const_iterator it = gData.packages_to_uninstall.begin(); it != gData.packages_to_uninstall.end(); ++it ) {
	  mark_for_uninstall(kind, *it);
	}

	solve_and_commit ();
    }
    return 0;
  }

  // --------------------------( search )-------------------------------------

  // FIXME --uninstalled-only does not really exclude installed resolvables
  // FIXME search for all resolvables displays installed packages twice
  // FIXME source (catalog) information missing for installed packages
  // TODO print rug's v status  

  if (command == "search" || command == "se") {
    ZyppSearchOptions options;

    if (help || copts.count("help")) {
      cerr << specific_help;
      return !help;
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

    ZyppSearch search(options,arguments);
    search.doSearch(FillTable(t));

    if (t.empty())
      cout << "No packages found." << endl;
    else {
      if (copts.count("sort-by-catalog")) t.sort(1);
      else t.sort(3); // sort by name
      cout << t;
    }
  }

  // --------------------------( patch check )--------------------------------

  // TODO: rug summary
  if (command == "patch-check" || command == "pchk") {
    if (help) {
      cerr << "patch-check\n"
	   << specific_help
	;
      return !help;
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

  if (command == "patches" || command == "pch") {
    if (help) {
      cerr << "patches\n"
	   << specific_help
	;
      return !help;
    }

    cond_init_target ();
    cond_init_system_sources ();
    cond_load_resolvables ();
    establish ();
    show_pool ();
    return 0;
  }

  // --------------------------( list updates )-------------------------------

  if (command == "list-updates" || command == "lu") {
    if (help) {
      // FIXME catalog...
      cerr << "list-updates [options]\n"
	   << specific_help
	;
      return !help;
    }

    string skind = copts.count("type")?  copts["type"].front() : "patch";
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

  if (command == "update" || command == "up") {
    if (help) {
      cerr << "update [options]\n"
	   << specific_help
	;
      return !help;
    }

    string skind = copts.count("type")?  copts["type"].front() : "patch";
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
    solve_and_commit ();
    return 0;
  }

  return 0;
}
// Local Variables:
// c-basic-offset: 2
// End:
