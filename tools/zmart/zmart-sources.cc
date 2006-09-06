
#include "zmart.h"
#include "zmart-sources.h"

#include <zypp/target/store/PersistentStorage.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;
extern RuntimeData gData;
extern Settings gSettings;

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

void include_source_by_url( const Url &url )
{
  try
  {
    //cout << "Creating source from " << url << endl;
    Source_Ref src;
    src = SourceFactory().createFrom(url, "/", url.asString(), "");
    //cout << "Source created.. " << endl << src << endl;
    gData.sources.push_back(src);
  }
  catch( const Exception & excpt_r )
  {
    cerr << "Can't access repository" << endl;
    ZYPP_CAUGHT( excpt_r );
    exit(-1);
  }
  
}

void list_system_sources()
{
  zypp::storage::PersistentStorage store;
  
  std::list<zypp::source::SourceInfo> sources;
  
  try
  {
    store.init( "/" );
    sources = store.storedSources();
  }
  catch ( const Exception &e )
  {
    cout << "Error reading system sources: " << e.msg() << std::endl;
    exit(-1); 
  }
  
  for( std::list<zypp::source::SourceInfo>::const_iterator it = sources.begin() ;
       it != sources.end() ; ++it )
  {
    SourceInfo source = *it;
    cout << ( source.enabled() ? "[x]" : "[ ]" );
    cout << ( source.autorefresh() ? "* " : "  " );
    if ( source.alias() != source.url().asString() )
      cout << source.alias() << " (" << source.url() << ")" << endl;
    else
      cout << source.url() << endl;
  }
    
  /*   
  Sub'd? | Name                                                   | Service
  -------+--------------------------------------------------------+-------------------------------------------------------
  Yes    | non-oss                                                | non-oss
  Yes    | 20060629-170551                                        | 20060629-170551
  Yes    | 20060824-092918                                        | 20060824-092918
  Yes    | mozilla                                                | mozilla
  Yes    | 20060612-143432                                        | 20060612-143432
  | SUSE-Linux-10.1-Updates                                | SUSE-Linux-10.1-Updates
  Yes    | SUSE-Linux-10.1-DVD9-x86-x86_64-10.1-0-20060505-104407 | SUSE-Linux-10.1-DVD9-x86-x86_64-10.1-0-20060505-104407
  */
}

static
std::string timestamp ()
{
  time_t t = time(NULL);
  struct tm * tmp = localtime(&t);

  if (tmp == NULL) {
    return "";
  }

  char outstr[50];
  if (strftime(outstr, sizeof(outstr), "%Y%m%d-%H%M%S", tmp) == 0) {
    return "";
  }
  return outstr;
}

void add_source_by_url( const zypp::Url &url, std::string alias  )
{
  SourceManager_Ptr manager = SourceManager::sourceManager();
  manager->restore ("/", true /*use_cache*/);

  list<SourceManager::SourceId> sourceIds;

  Pathname path;
  Pathname cache;
  bool is_base = false;
  if (alias.empty ())
    alias = timestamp();
   
  // more products?
  // try
  Source_Ref source = SourceFactory().createFrom( url, path, alias, cache, is_base );
  SourceManager::SourceId sourceId = manager->addSource( source );

  //if (enableSource)
    source.enable();
  //else
  //  source.disable();
  
  //source.setAutorefresh (autoRefresh);

    sourceIds.push_back( sourceId );
      cout << "Added Installation Sources:" << endl;
  
    list<SourceManager::SourceId>::const_iterator it;
    for( it = sourceIds.begin(); it != sourceIds.end(); ++it ) {
      Source_Ref source = manager->findSource(*it);
      cout << ( source.enabled() ? "[x]" : "[ ]" );
      cout << ( source.autorefresh() ? "* " : "  " );
      cout << source.alias() << " (" << source.url() << ")" << endl;
    }

    manager->store( "/", true /*metadata_cache*/ );
}
