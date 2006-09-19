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

ZYpp::Ptr God;
RuntimeData gData;
Settings gSettings;
int verbose = 0;

/*
RpmCallbacks rpm_callbacks;
SourceCallbacks source_callbacks;
MediaCallbacks media_callbacks;
*/
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
  const char *optstring = longopts2optstring (longopts).c_str ();
  short2long_t short2long = make_short2long (longopts);

  while (1) {
    int option_index = 0;
    optc = getopt_long (argc, argv, optstring,
			longopts, &option_index);
    if (optc == -1)
      break;			// options done

    switch (optc) {
    case '?':
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
      break;
    }
  }
  return result;
}

static struct option global_options[] = {
  {"help", no_argument, 0, 'h'},
  {"verbose", no_argument, 0, 'v'},
  {"version", no_argument, 0, 'V'},
  {"req", required_argument, 0, 'r'},
  {"opt", optional_argument, 0, 'o'},
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
  parsed_opts gopts = parse_options (argc, argv, global_options);

  if (gopts.count("help")) {
    cerr << "  Options:\n"
	"\t--help, -h\t\tHelp\n"
	"\t--version, -V\t\tOutput the version number\n"
	"\t--verbose, -v\t\tIncrease verbosity\n"
	"\t--terse, -t\t\tTerse output for machine consumption\n"
	;
    
  }

  if (gopts.count("version")) {
    cerr << "zypper 0.1" << endl;
  }

  if (gopts.count("verbose")) {
    verbose += gopts.count("verbose");
    cerr << "verbosity " << verbose << endl;
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
  if (command.empty())
    command = "help";
  cout << "COMMAND: " << command << endl;

  struct option no_options = {0, 0, 0, 0};
  struct option *specific_options = &no_options;

  string help_commands = "  Commands:\n"
      "\tinstall, in\t\tInstall packages or resolvables\n"
      "\tservice-list, sl\t\tList services aka installation sources\n"
      "\tservice-add, sa\t\tAdd a new service\n"
      ;

  string help_global_source_options = "  Source options:\n"
      "\t--disable-system-sources, -D\t\tDon't read the system sources\n"
      "\t--source, -S\t\tRead additional source\n"
      ;

  string help_global_target_options = "  Target options:\n"
      "\t--disable-system-resolvables, -T\t\tDon't read system installed resolvables\n"
      ;

  if (command == "install" || command == "in") {
  }
  else if (command == "service-add" || command == "sa") {
    static struct option service_add_options[] = {
      {"disabled", no_argument, 0, 'd'},
      {"no-refresh", no_argument, 0, 'n'},
      {0, 0, 0, 0}
    };

    specific_options = service_add_options;
  }
  else if (command == "service-list" || command == "sl") {
    static struct option service_list_options[] = {
      {"terse", no_argument, 0, 't'},
      {0, 0, 0, 0}
    };

    specific_options = service_list_options;
  }
  else {
    // no options. or make this an exhaustive thing?
    //    cerr << "Unknown command" << endl;
    //    return 1;
  }

  parsed_opts copts = parse_options (argc, argv, specific_options);

  list<string> arguments;
  if (optind < argc) {
    cerr << "non-option ARGV-elements: ";
    while (optind < argc) {
      string argument = argv[optind++];
      cerr << argument << ' ';
      arguments.push_back (argument);
    }
    cerr << endl;
  }

  if (copts.count("disabled")) {
      cout << "FAKE Disabled" << endl;
  }

  if (copts.count("no-refresh")) {
      cout << "FAKE No Refresh" << endl;
  }

  if (copts.count("terse")) {
      cout << "FAKE Terse" << endl;
  }


  if (command == "service-list" || command == "sl")
  {
    if ( geteuid() != 0 )
    {
      cout << "Sorry, you need root privileges to view system sources." << endl;
      return 1;
    }
    
    list_system_sources();
    return 0;
  }
  
  if (command == "help") {
    cout << "Help command" << endl;
    return 0;
  }

  if (gopts.count("disable-system-sources"))
  {
    MIL << "system sources disabled" << endl;
    gSettings.disable_system_sources = true;
  }
  else
  {
    if ( geteuid() != 0 )
    {
      cout << "Sorry, you need root privileges to use system sources, disabling them..." << endl;
      gSettings.disable_system_sources = true;
      MIL << "system sources disabled" << endl;
    }
    else
    {
      MIL << "system sources enabled" << endl;
    }
  }

  if (command == "service-add" || command == "sa")
  {
    Url url = make_url (arguments.front());
    if (!url.isValid())
      return 1;
    
    try {
      add_source_by_url(url, "aliasfixme");
    }
    catch ( const Exception & excpt_r )
    {
      cerr << excpt_r.asUserString() << endl;
      return 1;
    }
    return 0;
  }
  
  if (gopts.count("disable-system-resolvables"))
  {
    MIL << "system resolvables disabled" << endl;
    cout << "Ignoring installed resolvables..." << endl;
    gSettings.disable_system_resolvables = true;
  }
  
  if (command == "install" || command == "in")
  {
    gData.packages_to_install = vector<string>(arguments.begin(), arguments.end());
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
  
  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile == NULL)
    logfile = ZYPP_CHECKPATCHES_LOG;
  zypp::base::LogControl::instance().logfile( logfile );
  
  std::string previous_token;
  
  God = NULL;
  try
  {
    God = zypp::getZYpp();
  }
  catch (Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
    ERR  << "a ZYpp transaction is already in progress." << endl;
    cerr << "a ZYpp transaction is already in progress." << endl;
    cout << RANDOM_TOKEN;
    return 1;
  }
  
  SourceManager_Ptr manager;
  manager = SourceManager::sourceManager();
  
  KeyRingCallbacks keyring_callbacks;
  DigestCallbacks digest_callbacks;
  
  if ( ! gSettings.disable_system_sources )
    init_system_sources();
  
  for ( std::list<Url>::const_iterator it = gSettings.additional_sources.begin(); it != gSettings.additional_sources.end(); ++it )
  {
    include_source_by_url( *it );
  }
  
  cout << endl;
  
  if ( gData.sources.empty() )
  {
    cout << "Warning! No sources. Operating only over the installed resolvables. You will not be able to install stuff" << endl;
  } 
  
  // dont add rpms
  God->initializeTarget("/");
  
  std::string token = calculate_token();
  
  if ( token != gSettings.previous_token )
  {
    // something changed
    load_sources();
    
    if ( ! gSettings.disable_system_resolvables )
      load_target();
    
    for ( vector<string>::const_iterator it = gData.packages_to_install.begin(); it != gData.packages_to_install.end(); ++it )
    {
      mark_package_for_install(*it);
    }
    
    resolve();
    
    show_summary();
      
    std::cout << "Continue? [y/n] ";
    if (readBoolAnswer())
    {
      ZYppCommitResult result = God->commit( ZYppCommitPolicy() );
      std::cout << result << std::endl; 
    }
  }
  
  

  return 0;
}


