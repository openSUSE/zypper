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

/*
RpmCallbacks rpm_callbacks;
SourceCallbacks source_callbacks;
MediaCallbacks media_callbacks;
*/
typedef map<string, list<string> > parsed_opts;

static struct option global_options[] = {
  {"help", no_argument, 0, 'h'},
  {"verbose", no_argument, 0, 'v'},
  {"version", no_argument, 0, 'V'},
  {"req", required_argument, 0, 'r'},
  {"opt", optional_argument, 0, 'o'},
  {0, 0, 0, 0}
};

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
  cerr << "OS " << optstring << endl;
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


int main(int argc, char **argv)
{
  parsed_opts gopts = parse_options (argc, argv, global_options);

  if (gopts.count("help"))
    cerr << "Global help" << endl;
  if (gopts.count("opt")) {
    cerr << "Opt arg: ";
    std::copy (gopts["opt"].begin(), gopts["opt"].end(),
	       ostream_iterator<string> (cerr, ", "));
    cerr << endl;
  }
  if (gopts.count("req")) {
    cerr << "Req arg: ";
    std::copy (gopts["req"].begin(), gopts["req"].end(),
	       ostream_iterator<string> (cerr, ", "));
    cerr << endl;
  }
  string command;
  if (optind < argc) {
    command = argv[optind++];
    cout << "COMMAND: " << command << endl;
  }

  struct option * specific_options = NULL;
  const char * specific_optstr = NULL;
  if (command == "service-list" || command == "sl") {
    static struct option service_list_options[] = {
      {"terse", no_argument, 0, 't'},
      {0, 0, 0, 0}
    };

    specific_options = service_list_options;
    specific_optstr = "+t";
  }
  else if (command == "service-add" || command == "sa") {
    static struct option service_add_options[] = {
      {"disabled", no_argument, 0, 'd'},
      {"no-refresh", no_argument, 0, 'n'},
      {0, 0, 0, 0}
    };

    specific_options = service_add_options;
    specific_optstr = "+dn";
  }
  else {
    cerr << "Unknown command" << endl;
    return 1;
  }

  int optc;
  while (1) {
    int option_index = 0;

    // +: do not permute, stop at the 1st nonoption, which is the command
    optc = getopt_long (argc, argv, specific_optstr,
                        specific_options, &option_index);
    if (optc == -1)
      break;			// options done

    switch (optc) {
    case 0:
      printf ("option %s", specific_options[option_index].name);
      if (optarg)
	printf (" with arg %s", optarg);
      printf ("\n");
      break;

    case 'd':
      cout << "Disabled" << endl;
      break;
    case 'n':
      cout << "Norefresh" << option_index << endl;
      break;
    case 't':
      cout << "Terse" << endl;
      break;

    case '?':
      cout << "Unknown option " << optopt << ':' << argv[optind-1] <<endl;
      break;

    default:
      printf ("?? getopt returned character code 0%o ??\n", optc);
    }
  }

  if (optind < argc) {
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
      printf ("%s ", argv[optind++]);
    printf ("\n");
  }


  /*
  // the 1st non-option is interpreted as --command
  po::positional_options_description pos_options;
  pos_options.add("command", 1);
  
  po::options_description general_options("General options");
  general_options.add_options()
      ("help,h", "produce a help message")
      ("version,v", "output the version number")
      ;

  po::options_description operation_options("Operations");
  operation_options.add_options()
      ("install,i", po::value< vector<string> >(), "install packages or resolvables")
      ("list-system-sources,l", "Show available system sources")
      ("add-source,a", po::value< string >(), "Add a new source from a URL")
      ;
  
  po::options_description source_options("Source options");
  source_options.add_options()
      ("disable-system-sources,D", "Don't read the system sources.")
      ("sources,S", po::value< vector<string> >(), "Read from additional sources")
      ;
  
  po::options_description target_options("Target options");
  target_options.add_options()
      ("disable-system-resolvables,T", "Don't read system installed resolvables.")
      ;

  // Declare an options description instance which will include
  // all the options
  po::options_description all_options("Allowed options");
  all_options.add(general_options).add(source_options).add(operation_options).add(target_options);

  // Declare an options description instance which will be shown
  // to the user
  po::options_description visible_options("Allowed options");
  visible_options.add(general_options).add(source_options).add(operation_options).add(target_options);

  po::variables_map vm;
  //po::store(po::parse_command_line(argc, argv, visible_options), vm);
  po::store(po::command_line_parser(argc, argv).options(visible_options).positional(pos_options).run(), vm);
  po::notify(vm);
  */

  /*  
  if (vm.count("list-system-sources"))
  {
    if ( geteuid() != 0 )
    {
      cout << "Sorry, you need root privileges to view system sources." << endl;
      exit(-1);
    }
    
    list_system_sources();
    return 1;
  }
  
  if (vm.count("help")) {
    cout << visible_options << "\n";
    return 1;
  }

  if (vm.count("version")) {
    cout << "zmart 0.1" << endl;
    return 1;
  }

  if (vm.count("disable-system-sources"))
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

  if (vm.count("add-source"))
  {
    string urlStr = vm["add-source"].as< string >();
    Url url;
    try {
      url = Url( urlStr );
    }
    catch ( const Exception & excpt_r )
    {
      ZYPP_CAUGHT( excpt_r );
      cerr << "URL is invalid." << endl;
      cerr << excpt_r.asUserString() << endl;
      exit( 1 );
    }
    
    try {
      add_source_by_url(url, "aliasfixme");
    }
    catch ( const Exception & excpt_r )
    {
      cerr << excpt_r.asUserString() << endl;
      exit(1);
    }
    
    exit(0);
  }
  
  if (vm.count("disable-system-resolvables"))
  {
    MIL << "system resolvables disabled" << endl;
    cout << "Ignoring installed resolvables..." << endl;
    gSettings.disable_system_resolvables = true;
  }
  
  if (vm.count("install"))
  {
    vector<string> to_install = vm["install"].as< vector<string> >();
    //MIL << "Additional installs are: " << to_install << "\n";
    for ( vector<string>::const_iterator it = to_install.begin(); it != to_install.end(); ++it )
    {
      gData.packages_to_install = to_install;
    }
  }
  
  if (vm.count("sources"))
  {
    vector<string> sources = vm["sources"].as< vector<string> >();
    //MIL << "Additional sources are: " << sources << "\n";
    for ( vector<string>::const_iterator it = sources.begin(); it != sources.end(); ++it )
    {
      try
      {
        gSettings.additional_sources.push_back(Url(*it)); 
      }
      catch ( const Exception &e )
      {
        cout << "wrong url " << *it << std::endl;
        exit(-1);
      }
    }
  }
  
  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile != NULL)
    zypp::base::LogControl::instance().logfile( logfile );
  else
    zypp::base::LogControl::instance().logfile( ZYPP_CHECKPATCHES_LOG );
  
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
    return -1;
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
  
  
  */  
  return 0;
}


