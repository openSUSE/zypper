#include <sstream>
#include <boost/format.hpp>
#include "zmart.h"
#include "zmart-misc.h"

#include <zypp/Patch.h>
#include <zypp/base/Algorithm.h>

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
  //int  count = 0;
  while ( (c != 'y') && (c != 'Y') && (c != 'N') && (c != 'n') )
    cin >> c ;
      
  if ( ( c == 'y' ) || ( c == 'Y' ) ) 
    return true;
  else
    return false;
}

// converts a user-supplied kind to a zypp kind object
// returns an empty one if not recognized
ResObject::Kind string_to_kind (const string &skind)
{
  ResObject::Kind empty;
  string lskind = str::toLower (skind);
  if (lskind == "package")
    return ResTraits<Package>::kind;
  if (lskind == "selection")
    return ResTraits<Selection>::kind;
  if (lskind == "pattern")
    return ResTraits<Pattern>::kind;
  if (lskind == "product")
    return ResTraits<Product>::kind;
  if (lskind == "patch")
    return ResTraits<Patch>::kind;
  if (lskind == "script")
    return ResTraits<Script>::kind;
  if (lskind == "message")
    return ResTraits<Message>::kind;
  if (lskind == "language")
    return ResTraits<Language>::kind;
  if (lskind == "atom")
    return ResTraits<Atom>::kind;
  if (lskind == "system")
    return ResTraits<SystemResObject>::kind;
  if (lskind == "srcpackage")
    return ResTraits<SrcPackage>::kind;
  // not recognized
  return empty;
}

// copied from yast2-pkg-bindings:PkgModuleFunctions::DoProvideNameKind
struct ProvideProcess
{
  PoolItem_Ref item;
  ResStatus::TransactByValue whoWantsIt;
  string version;
  Arch _architecture;

  ProvideProcess( Arch arch, const string &vers)
    : whoWantsIt(zypp::ResStatus::USER)
    , version(vers)
    , _architecture( arch )
    { }

  bool operator()( const PoolItem& provider )
  {
    cerr_vv << "Considering " << provider << endl;
    // 1. compatible arch
    // 2. best arch
    // 3. best edition
	
    // check the version if it's specified
    if (!version.empty() && version != provider->edition().asString()) {
      cerr_vv << format ("Skipping version %s (requested: %s)")
	% provider->edition().asString() % version << endl;
      return true;
    }

    if (!provider.status().isInstalled()) {
      // deselect the item if it's already selected,
      // only one item should be selected
      if (provider.status().isToBeInstalled()) {
	cerr_vv << "  Deselecting" << endl;
	provider.status().resetTransact(whoWantsIt);
      }

      // regarding items which are installable only
      if (!provider->arch().compatibleWith( _architecture )) {
	cerr_vv << format ("provider %s has incompatible arch '%s'")
	  % provider->name() % provider->arch().asString() << endl;
      }
      else if (!item) {
	cerr_vv << "  First match" << endl;
	item = provider;
      }
      else if (item->arch().compare( provider->arch() ) < 0) {
	cerr_vv << "  Better arch" << endl;
	item = provider;
      }
      else if (item->edition().compare( provider->edition() ) < 0) {
	cerr_vv << "  Better edition" << endl;
	item = provider;
      }
    }

    return true;
  }
};


// this does only resolvables with this _name_.
// we could also act on _provides_
// TODO edition, arch
void mark_for_install( const ResObject::Kind &kind,
		       const std::string &name )
{
  const ResPool &pool = God->pool();
  // name and kind match:

  ProvideProcess installer (God->architecture(), "" /*version*/);
  cerr_vv << "Iterating over " << name << endl;
  invokeOnEach( pool.byNameBegin( name ),
		pool.byNameEnd( name ),
		resfilter::ByKind( kind ),
		zypp::functor::functorRef<bool,const zypp::PoolItem&> (installer)
		);
  cerr_vv << "... done" << endl;
  if (!installer.item) {
    cerr << "Not found" << endl;
    return; //error?
  }
  
  bool result = installer.item.status().setToBeInstalled( zypp::ResStatus::USER );
  if (!result) {
    cerr << "Failed" << endl;
  }
}

struct DeleteProcess
{
  bool found;
  DeleteProcess ()
    : found(false)
  { }

  bool operator() ( const PoolItem& provider )
  {
    found = true;
    cerr_vv << "Marking for deletion: " << provider << endl;
    bool result = provider.status().setToBeUninstalled( zypp::ResStatus::USER );
    if (!result) {
      cerr << "Failed" << endl;
    }
    return true;		// get all of them
  }
};

// mark all matches
void mark_for_uninstall( const ResObject::Kind &kind,
			 const std::string &name )
{
  const ResPool &pool = God->pool();
  // name and kind match:

  DeleteProcess deleter;
  cerr_vv << "Iterating over " << name << endl;
  invokeOnEach( pool.byNameBegin( name ),
		pool.byNameEnd( name ),
		functor::chain (resfilter::ByInstalled(),
				resfilter::ByKind( kind )),
		zypp::functor::functorRef<bool,const zypp::PoolItem&> (deleter)
		);
  cerr_vv << "... done" << endl;
  if (!deleter.found) {
    cerr << "Not found" << endl;
    return; //error?
  }
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



