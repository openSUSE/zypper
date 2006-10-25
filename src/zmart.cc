#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <unistd.h>

#include <boost/program_options.hpp>

#include "zmart.h"
#include "zmart-sources.h"
#include "zmart-misc.h"

#include "zmart-rpm-callbacks.h"
#include "zmart-keyring-callbacks.h"
#include "zmart-source-callbacks.h"
#include "zmart-media-callbacks.h"

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace boost;

namespace po = boost::program_options;

ZYpp::Ptr God;
RuntimeData gData;
Settings gSettings;

ostream no_stream(NULL);

RpmCallbacks rpm_callbacks;
SourceCallbacks source_callbacks;
MediaCallbacks media_callbacks;

int main(int argc, char **argv)
{
  po::positional_options_description pos_options;
  pos_options.add("command", -1);
  
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
      mark_for_install(zypp::ResTraits<zypp::Package>::kind, *it);
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


