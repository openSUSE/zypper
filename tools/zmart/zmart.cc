#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

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

#include "zmart-rpm-callbacks.h"
#include "zmart-keyring-callbacks.h"
#include "zmart-source-callbacks.h"

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::zmart"

#define RANDOM_TOKEN "sad987432JJDJD948394DDDxxx22"

using namespace zypp::detail;

using namespace std;
using namespace zypp;

#define ZYPP_CHECKPATCHES_LOG "/var/log/zypp-zmart.log"

//using namespace DbXml;

ZYpp::Ptr God;

enum OutputType
{
  OutputTypeSimple,
  OutputTypeNice,
  OutputTypeXML
};

enum SourcesFrom
{
  SourcesFromSystem,
  SourcesFromParam
};

struct Settings
{
  Settings()
  : sources_from(SourcesFromSystem),
  output_type(OutputTypeNice),
  previous_token(RANDOM_TOKEN),
  previous_code(-1)
  {}
  
  SourcesFrom sources_from;
  OutputType output_type;
  Url source;
  std::string previous_token;
  int previous_code;
  std::string command;
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
};

RuntimeData gData;
Settings gSettings;

RpmCallbacks rpm_callbacks;
SourceCallbacks source_callbacks;

void report( const char *msg )
{
  if ( gSettings.output_type == OutputTypeNice )
    cout << msg << endl;
}

void output_simple( const ResStore &store )
{
}

void output_nice( const ResStore &store )
{ 
}

void init_sources()
{
  SourceManager_Ptr manager;
  manager = SourceManager::sourceManager();
  
  if ( gSettings.sources_from == SourcesFromSystem )
  {
    try
    {
      report("Restoring system sources...");
      manager->restore("/");
    }
    catch (Exception & excpt_r)
    {
      ZYPP_CAUGHT (excpt_r);
      ERR << "Couldn't restore sources" << endl;
      report("Fail to restore sources");
      exit(-1);
    }
    
    for ( SourceManager::Source_const_iterator it = manager->Source_begin(); it !=  manager->Source_end(); ++it )
    {
      Source_Ref src = manager->findSource(it->alias());
      gData.sources.push_back(src);
    }
  }
  else
  {
    Source_Ref src;
    try
    {
      report("Creating source from Url...");
      src = SourceFactory().createFrom(gSettings.source, "/", gSettings.source.asString(), "");
    }
    catch( const Exception & excpt_r )
    {
      cerr << "Can't access repository" << endl;
      ZYPP_CAUGHT( excpt_r );
      exit(-1);
    }
    gData.sources.push_back(src);
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
    
    if ( gSettings.sources_from == SourcesFromSystem )
    {
      if ( gSettings.output_type == OutputTypeNice )
        cout << "Refreshing source " <<  src.alias() << endl;
      src.refresh();
    }
    
    token_stream << "[" << src.alias() << "| " << src.url() << src.timestamp() << "]";
    MIL << "Source: " << src.alias() << " from " << src.timestamp() << std::endl;  
  }
  
  token_stream << "[" << "target" << "| " << God->target()->timestamp() << "]";
  
  //static std::string digest(const std::string& name, std::istream& is
  token = Digest::digest("sha1", token_stream);
  
  if ( gSettings.output_type == OutputTypeSimple )
    cout << token;
  
  MIL << "new token [" << token << "]" << " previous: [" << gSettings.previous_token << "] previous code: " << gSettings.previous_code << std::endl;
  
  return token;
}

void load_target()
{
  report("Adding system resolvables to pool...");
  God->addResolvables( God->target()->resolvables(), true);
}

void load_sources()
{
  for ( std::list<Source_Ref>::iterator it = gData.sources.begin(); it !=  gData.sources.end(); ++it )
  {
    Source_Ref src = *it;
    // skip non YUM sources for now
    if ( it->type() == "YUM" )
    {
      cout << "Adding " << it->alias() << " resolvables to the pool..." << endl;
      God->addResolvables(it->resolvables());
    }
  }
}

void resolve()
{
  report("Resolving dependencies ...");
  God->resolver()->establishPool();
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
  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile != NULL)
    zypp::base::LogControl::instance().logfile( logfile );
  else
    zypp::base::LogControl::instance().logfile( ZYPP_CHECKPATCHES_LOG );
  
  std::string previous_token;
  
  int argpos = 1;
  
  if ( argc == 1 )
    usage( argc, argv );
  
  while ( argpos < argc )
  {
    if (strcmp( argv[argpos], "--source") == 0)
    {
      ++argpos;
      if (strcmp( argv[argpos], "system") == 0)
      {
        gSettings.sources_from = SourcesFromSystem;
        // do something
      }
      else
      {
        gSettings.sources_from = SourcesFromParam;
        try
        {
          gSettings.source = Url(argv[argpos]); 
        }
        catch ( const Exception &e )
        {
          cerr << "wrong url " << argv[argpos] << std::endl;
          exit(-1);
        }
      }
      ++argpos;
    }
    else if (strcmp( argv[argpos], "--output") == 0)
    {
      ++argpos;
      if (strcmp( argv[argpos], "nice") == 0)
      {
        gSettings.output_type = OutputTypeNice;
        // do something
      }
      else if (strcmp( argv[argpos], "simple") == 0)
      {
        gSettings.output_type = OutputTypeSimple;
      }
      ++argpos;
    }
    else if (strcmp( argv[argpos], "--token") == 0)
    {
      ++argpos;
      gSettings.previous_token = argv[argpos];
      ++argpos;
    }
    else if (strcmp( argv[argpos], "--last-code") == 0)
    {
      ++argpos;
      gSettings.previous_code = str::strtonum<int>(argv[argpos]) ;
      ++argpos;
    }
    else if ( strcmp( argv[argpos], "patch") == 0 )
    {
      gSettings.command = "patch" ;
      ++argpos;
    }
    else
    {
      cerr << "Unrecognized option " << argv[argpos] << std::endl;
      usage(argc, argv);
      exit(-1);
    }
  }
  
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
  
  init_sources();
  
  // dont add rpms
  God->initializeTarget("/");
  
  std::string token = calculate_token();
  
  if ( token != gSettings.previous_token )
  {
    // something changed
    load_sources();
    load_target();
    
    resolve();
    
    show_pool();
  
    if ( gData.security_patches_count > 0 )
      return 2;
  
    if ( gData.patches_count > 0 )
      return 1;
  }
  
  return 0;
}


