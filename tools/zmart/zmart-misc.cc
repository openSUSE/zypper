#include <sstream>

#include "zmart.h"
#include "zmart-misc.h"

#include <zypp/Patch.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;
extern RuntimeData gData;
extern Settings gSettings;

// read callback answer
//   can either be '0\n' -> false
//   or '1\n' -> true
// reads characters from stdin until newline. Defaults to 'false'
bool readBoolAnswer()
{
  char c = 0;
  int  count = 0;
  while ( (c != 'y') && (c != 'Y') && (c != 'N') && (c != 'n') )
    cin >> c ;
      
  if ( ( c == 'y' ) || ( c == 'Y' ) ) 
    return true;
  else
    return false;
}

void mark_package_for_install( const std::string &name )
{
  // as documented in ResPool::setAdditionalFoo
  CapSet capset;
  capset.insert (CapFactory().parse( ResTraits<Package>::kind, name));

  // The user is setting this capablility
  ResPool::AdditionalCapSet aCapSet;
  aCapSet[ResStatus::USER] = capset;
  God->pool().setAdditionalRequire( aCapSet );
}

void mark_package_for_uninstall( const std::string &name )
{
  CapSet capset;
  capset.insert (CapFactory().parse( ResTraits<Package>::kind, name));

  // The user is setting this capablility
  ResPool::AdditionalCapSet aCapSet;
  aCapSet[ResStatus::USER] = capset;
  God->pool().setAdditionalConflict( aCapSet );
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
      cout << *res << std::endl;
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
    ResStore src_resolvables(it->resolvables());
    cout << "   " <<  src_resolvables.size() << " resolvables." << endl;
    God->addResolvables(src_resolvables);
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



