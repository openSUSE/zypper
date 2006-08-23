#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <unistd.h>

#include <boost/program_options.hpp>

#include <zypp/base/LogControl.h>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/Locale.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/zypp_detail/ZYppReadOnlyHack.h>
#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>
#include <zypp/ResStore.h>
#include <zypp/base/String.h>
#include <zypp/Digest.h>
#include <zypp/CapFactory.h>

#include "zmart-rpm-callbacks.h"
#include "zmart-keyring-callbacks.h"
#include "zmart-source-callbacks.h"

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::zmart"

#define RANDOM_TOKEN "sad987432JJDJD948394DDDxxx22"

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace boost;

namespace po = boost::program_options;

#define ZYPP_CHECKPATCHES_LOG "/var/log/zypp-zmart.log"

//using namespace DbXml;

ZYpp::Ptr God;

struct Settings
{
  Settings()
  : previous_token(RANDOM_TOKEN),
  previous_code(-1),
  disable_system_sources(false)
  {}

  std::list<Url> additional_sources;
  std::string previous_token;
  int previous_code;
  std::string command;
  bool disable_system_sources;
};

struct RuntimeData
{
  RuntimeData()
  : patches_count(0),
    security_patches_count(0)
  {}
    
  std::list<Source_Ref> sources;
  int patches_count;
  int security_patches_count;
  vector<string> packages_to_install; 
};

RuntimeData gData;
Settings gSettings;

RpmCallbacks rpm_callbacks;
SourceCallbacks source_callbacks;

void output_simple( const ResStore &store )
{
}

void output_nice( const ResStore &store )
{ 
}

void init_system_sources()
{
  SourceManager_Ptr manager;
  manager = SourceManager::sourceManager();
  try
  {
    cout << "Restoring system sources..." << endl;
    manager->restore("/");
  }
  catch (Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
    ERR << "Couldn't restore sources" << endl;
    cout << "Fail to restore sources" << endl;
    exit(-1);
  }
    
  for ( SourceManager::Source_const_iterator it = manager->Source_begin(); it !=  manager->Source_end(); ++it )
  {
    Source_Ref src = manager->findSource(it->alias());
    gData.sources.push_back(src);
  }
}

void add_source_by_url( const Url &url )
{
  Source_Ref src;
  try
  {
    cout << "Creating source from " << url << endl;
    src = SourceFactory().createFrom(url, "/", url.asString(), "");
  }
  catch( const Exception & excpt_r )
  {
    cerr << "Can't access repository" << endl;
    ZYPP_CAUGHT( excpt_r );
    exit(-1);
  }
  gData.sources.push_back(src);
}

void mark_package_for_install( const std::string &name )
{
  CapSet capset;
  capset.insert (CapFactory().parse( ResTraits<Package>::kind, name));
  //capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo1 > 1.0"));
  // The user is setting this capablility
  ResPool::AdditionalCapSet aCapSet;
  aCapSet[ResStatus::USER] = capset;
  God->pool().setAdditionalRequire( aCapSet );
}

void show_summary()
{
  MIL << "Pool contains " << God->pool().size() << " items." << std::endl;
  for ( ResPool::const_iterator it = God->pool().begin(); it != God->pool().end(); ++it )
  {
    Resolvable::constPtr res = it->resolvable();
    if ( it->status().isToBeInstalled() || it->status().isToBeUninstalled() )
    {
      if ( it->status().isToBeInstalled() )
        cout << "<install>   ";
      if ( it->status().isToBeUninstalled() )
        cout << "<uninstall> ";
      cout << res->name() << " " << res->edition() << "]" << std::endl;
    }
  } 
}

std::string calculate_token()
{
  SourceManager_Ptr manager;
  manager = SourceManager::sourceManager();
  
  std::string token;
  stringstream token_stream;
  for ( std::list<Source_Ref>::iterator it = gData.sources.begin(); it !=  gData.sources.end(); ++it )
  {
    Source_Ref src = *it;
    
//     if ( gSettings.disable_system_sources == SourcesFromSystem )
//     {
//       if ( gSettings.output_type == OutputTypeNice )
//         cout << "Refreshing source " <<  src.alias() << endl;
//       src.refresh();
//     }
    
    token_stream << "[" << src.alias() << "| " << src.url() << src.timestamp() << "]";
    MIL << "Source: " << src.alias() << " from " << src.timestamp() << std::endl;  
  }
  
  token_stream << "[" << "target" << "| " << God->target()->timestamp() << "]";
  
  //static std::string digest(const std::string& name, std::istream& is
  token = Digest::digest("sha1", token_stream);
  
  //if ( gSettings.output_type == OutputTypeSimple )
  //  cout << token;
  
  MIL << "new token [" << token << "]" << " previous: [" << gSettings.previous_token << "] previous code: " << gSettings.previous_code << std::endl;
  
  return token;
}

void load_target()
{
  cout << "Adding system resolvables to pool..." << endl;
  God->addResolvables( God->target()->resolvables(), true);
}

void load_sources()
{
  for ( std::list<Source_Ref>::iterator it = gData.sources.begin(); it !=  gData.sources.end(); ++it )
  {
    Source_Ref src = *it;
    // skip non YUM sources for now
    //if ( it->type() == "YUM" )
    //{
      cout << "Adding " << it->alias() << " resolvables to the pool..." << endl;
      God->addResolvables(it->resolvables());
    //}
  }
}

void resolve()
{
  cout << "Resolving dependencies ..." << endl;
  God->resolver()->establishPool();
  God->resolver()->resolvePool();
}

void show_pool()
{
  MIL << "Pool contains " << God->pool().size() << " items. Checking whether available patches are needed." << std::endl;
  for ( ResPool::byKind_iterator it = God->pool().byKindBegin<Patch>(); it != God->pool().byKindEnd<Patch>(); ++it )
  {
    Resolvable::constPtr res = it->resolvable();
    Patch::constPtr patch = asKind<Patch>(res);
    //cout << patch->name() << " " << patch->edition() << " " << "[" << patch->category() << "]" << ( it->status().isNeeded() ? " [needed]" : " [unneeded]" )<< std::endl;
    if ( it->status().isNeeded() )
    {
      gData.patches_count++;
      if (patch->category() == "security")
        gData.security_patches_count++;
        
      cerr << patch->name() << " " << patch->edition() << " " << "[" << patch->category() << "]" << std::endl;
    }
  } 
  cout << gData.patches_count << " patches needed. ( " << gData.security_patches_count << " security patches )"  << std::endl;
}

void usage(int argc, char **argv)
{
  cerr << "usage: " << argv[0] << " [<previous token>] [previous result]" << endl;
  exit(-1);
}


int main(int argc, char **argv)
{ 
  po::options_description general_options("General options");
  general_options.add_options()
      ("help", "produce a help message")
      ("version", "output the version number")
      ;

  po::options_description source_options("Source options");
  source_options.add_options()
      ("disable-system-sources", "Don't read the system sources.")
      ("sources", po::value< vector<string> >(), "Read from additional sources")
      ;

  po::options_description operation_options("Operations");
  operation_options.add_options()
      ("install", po::value< vector<string> >(), "install packages or resolvables")
      ;

  // Declare an options description instance which will include
  // all the options
  po::options_description all_options("Allowed options");
  all_options.add(general_options).add(source_options).add(operation_options);

  // Declare an options description instance which will be shown
  // to the user
  po::options_description visible_options("Allowed options");
  visible_options.add(general_options).add(source_options).add(operation_options);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, visible_options), vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << visible_options << "\n";
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
    vector<string> sources = vm["input-file"].as< vector<string> >();
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
    add_source_by_url( *it );
  }
  
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
    load_target();
    
    for ( vector<string>::const_iterator it = gData.packages_to_install.begin(); it != gData.packages_to_install.end(); ++it )
    {
      mark_package_for_install(*it);
    }
    
    resolve();
    
    show_summary();
  
    if ( gData.security_patches_count > 0 )
      return 2;
  
    if ( gData.patches_count > 0 )
      return 1;
  }
  
  return 0;
}


