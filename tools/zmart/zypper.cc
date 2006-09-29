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
  {"req",	required_argument, 0, 'r'},
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
      cerr_v << "Exiting main()" << endl;
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
    ;

  if (gopts.count("version")) {
    cerr << "zypper 0.1" << endl;
    return 0;
  }

  if (gopts.count("verbose")) {
    gSettings.verbose += gopts["verbose"].size();
    cerr << "verbosity " << gSettings.verbose << endl;
  }

  // testing option
  if (gopts.count("opt")) {
    cerr << "Opt arg: ";
    std::copy (gopts["opt"].begin(), gopts["opt"].end(),
	       ostream_iterator<string> (cerr, ", "));
    cerr << endl;
  }

  // testing option
  if (gopts.count("req")) {
    cerr << "Req arg: ";
    std::copy (gopts["req"].begin(), gopts["req"].end(),
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
  cerr_v << "COMMAND: " << command << endl;

  // === command-specific options ===

  struct option no_options = {0, 0, 0, 0};
  struct option *specific_options = &no_options;
  string specific_help;

  string help_commands = "  Commands:\n"
      "\thelp\t\t\tHelp\n"
      "\tinstall, in\t\tInstall packages or resolvables\n"
      "\tremove, rm\t\tRemove packages or resolvables\n"
      "\tservice-list, sl\tList services aka installation sources\n"
      "\tservice-add, sa\t\tAdd a new service\n"
      "\tservice-delete, sd\tDelete a service\n"
      "\tservice-rename, sr\tRename a service\n"
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
      {0, 0, 0, 0}
    };
    specific_options = service_add_options;
    specific_help = "  Command options:\n"
      "\t--disabled\t\tAdd the service as disabled\n"
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
    cout << RANDOM_TOKEN;
    return 1;
  }
  
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
  
  if (command == "service-add" || command == "sa")
  {
    if (copts.count("disabled")) {
      cerr_v << "FAKE Disabled" << endl;
    }
    if (copts.count("no-refresh")) {
      cerr_v << "FAKE No Refresh" << endl;
    }

    if (help || arguments.size() < 1) {
      cerr << "service-add [options] URI [alias]\n"
	""
	   << specific_help
	;
      return !help;
    }
      
    Url url = make_url (arguments[0]);
    if (!url.isValid())
      return 1;
    string alias = url.asString();
    if (arguments.size() > 1)
      alias = arguments[1];

    // load gpg keys
    // FIXME only once
    cerr_v << "initializing target" << endl;
    God->initializeTarget("/");

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
      cerr << excpt_r.asUserString() << endl;
      return 1;
    }

    return 0;
  }

  if (command == "service-rename" || command == "sr")
  {
    if (help || arguments.size() < 2) {
      cerr << "service-rename [options] <URI|alias> <new-alias>\n"
	   << specific_help
	;
      return !help;
    }
    
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

    std::string previous_token;
  
    SourceManager_Ptr manager;
    manager = SourceManager::sourceManager();
  
    KeyRingCallbacks keyring_callbacks;
    DigestCallbacks digest_callbacks;
  
    if ( geteuid() != 0 )
    {
      cerr << "Sorry, you need root privileges to use system sources, disabling them..." << endl;
      gSettings.disable_system_sources = true;
      MIL << "system sources disabled" << endl;
    }

    if ( ! gSettings.disable_system_sources ) {
      cerr_v << "initializing sources" << endl;
      init_system_sources();
    }
  
    for ( std::list<Url>::const_iterator it = gSettings.additional_sources.begin(); it != gSettings.additional_sources.end(); ++it )
      {
	include_source_by_url( *it );
      }
  
    if ( gData.sources.empty() )
      {
	cerr << "Warning! No sources. Operating only over the installed resolvables. You will not be able to install stuff" << endl;
      } 
  
    // dont add rpms
    cerr_v << "initializing target" << endl;
    God->initializeTarget("/");
  
    cerr_v << "calculating token" << endl;
    std::string token = calculate_token();
  
    if ( token != gSettings.previous_token )
      {
	// something changed
	cerr_v << "loading sources" << endl;
	load_sources();
    
	if ( ! gSettings.disable_system_resolvables ) {
	  cerr_v << "loading target" << endl;
	  load_target();
	}

	for ( vector<string>::const_iterator it = gData.packages_to_install.begin(); it != gData.packages_to_install.end(); ++it ) {
	  mark_for_install(kind, *it);
	}
    
	for ( vector<string>::const_iterator it = gData.packages_to_uninstall.begin(); it != gData.packages_to_uninstall.end(); ++it ) {
	  mark_for_uninstall(kind, *it);
	}
    
	cerr_v << "resolving" << endl;
	resolve();
    
	cerr_v << "Problems:" << endl;
	Resolver_Ptr resolver = zypp::getZYpp()->resolver();
	ResolverProblemList rproblems = resolver->problems ();
	ResolverProblemList::iterator
	  b = rproblems.begin (),
	  e = rproblems.end (),
	  i;
	if (b == e) {
	  cerr_v << "(none)" << endl;
	}
	for (i = b; i != e; ++i) {
	    cerr_v << "PROB " << (*i)->description () << endl;
	    cerr_v << ":    " << (*i)->details () << endl;

	    ProblemSolutionList solutions = (*i)->solutions ();
	    ProblemSolutionList::iterator
	      bb = solutions.begin (),
	      ee = solutions.end (),
	      ii;
	    for (ii = bb; ii != ee; ++ii) {
	      cerr_v << " SOL  " << (*ii)->description () << endl;
	      cerr_v << " :    " << (*ii)->details () << endl;
	    }
	}

	cerr_v << "Summary:" << endl;
	show_summary();
      
	cerr << "Continue? [y/n] ";
	if (readBoolAnswer())
	  {
	    cerr_v << "committing" << endl;
	    ZYppCommitResult result = God->commit( ZYppCommitPolicy() );
	    cerr << result << std::endl; 
	  }
      }
  }
  return 0;
}
