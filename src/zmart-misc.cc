#include <fstream>
#include <sstream>
#include <boost/format.hpp>
#include "zmart.h"
#include "zmart-misc.h"

#include <zypp/Patch.h>
#include <zypp/base/Algorithm.h>
#include <zypp/solver/detail/Helper.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;
extern RuntimeData gData;
extern Settings gSettings;


void cond_init_target () {
  static bool done = false;
  if (!done) {
#ifdef LIBZYPP_1xx
    cerr_v << "Initializing Target (old way)" << endl;
    God->initTarget("/", true);
#else
    cerr_v << "Initializing Target" << endl;
    God->initializeTarget("/");
#endif
    done = true;
  }
}

// return the default value on input failure
bool read_bool_with_default (bool defval) {
  istream & stm = cin;

  string c = "";
  while (stm.good () && c != "y" && c != "Y" && c != "N" && c != "n")
    c = zypp::str::getline (stm, zypp::str::TRIM);
      
  if (c == "y" || c == "Y")
    return true;
  else if (c == "n" || c == "N")
    return false;
  else
    return defval;
}

// Read an answer (ynYN)
// Defaults to 'false'
bool readBoolAnswer()
{
  return read_bool_with_default (false);
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
bool ProvideProcess::operator()( const PoolItem& provider )
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


// this does only resolvables with this _name_.
// we could also act on _provides_
// TODO edition, arch
void mark_for_install( const ResObject::Kind &kind,
		       const std::string &name )
{
  const ResPool &pool = God->pool();
  // name and kind match:

  ProvideProcess installer (God->architecture(), "" /*version*/);
  cerr_vv << "Iterating over [" << kind << "]" << name << endl;
  invokeOnEach( pool.byNameBegin( name ),
		pool.byNameEnd( name ),
		resfilter::ByKind( kind ),
		zypp::functor::functorRef<bool,const zypp::PoolItem&> (installer)
		);
  cerr_vv << "... done" << endl;
  if (!installer.item) {
    cerr << kind << " '" << name << "' not found" << endl;
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

void show_problems () {
  ostream& stm = cerr;
  Resolver_Ptr resolver = zypp::getZYpp()->resolver();
  ResolverProblemList rproblems = resolver->problems ();
  ResolverProblemList::iterator
    b = rproblems.begin (),
    e = rproblems.end (),
    i;
  if (b != e) {
    cerr << "Problems:" << endl;
  }
  for (i = b; i != e; ++i) {
    stm << "PROB " << (*i)->description () << endl;
    stm << ":    " << (*i)->details () << endl;

    ProblemSolutionList solutions = (*i)->solutions ();
    ProblemSolutionList::iterator
      bb = solutions.begin (),
      ee = solutions.end (),
      ii;
    for (ii = bb; ii != ee; ++ii) {
      stm << " SOL  " << (*ii)->description () << endl;
      stm << " :    " << (*ii)->details () << endl;
    }
  }
}

bool show_summary()
{
  cerr << "Summary:" << endl;
  bool nothing = true;

  MIL << "Pool contains " << God->pool().size() << " items." << std::endl;
  for ( ResPool::const_iterator it = God->pool().begin(); it != God->pool().end(); ++it )
  {
    ResObject::constPtr res = it->resolvable();
    if ( it->status().isToBeInstalled() || it->status().isToBeUninstalled() )
    {
      nothing = false;
      if ( it->status().isToBeInstalled() )
        cerr << "<install>   ";
      if ( it->status().isToBeUninstalled() )
        cerr << "<uninstall> ";
      cerr << *res << endl;
    }
  }
  if (nothing)
    cerr << "Nothing to do." << endl;

  return !nothing;
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

#ifdef LIBZYPP_1xx
  token_stream << "[" << "target" << "| " << Date::now() << "]"; // too bad
#else
  token_stream << "[" << "target" << "| " << God->target()->timestamp() << "]";
#endif
  
  //static std::string digest(const std::string& name, std::istream& is
  token = Digest::digest("sha1", token_stream);
  
  //if ( gSettings.output_type == OutputTypeSimple )
  //  cout << token;
  
  MIL << "new token [" << token << "]" << " previous: [" << gSettings.previous_token << "] previous code: " << gSettings.previous_code << std::endl;
  
  return token;
}

void cond_load_resolvables ()
{	
  // something changed
  load_sources();
    
  if ( ! gSettings.disable_system_resolvables ) {
    load_target();
  }
}

void load_target()
{
  cerr << "Parsing RPM database..." << endl;
  ResStore tgt_resolvables(God->target()->resolvables());
  cerr_v << "   " <<  tgt_resolvables.size() << " resolvables." << endl;
  God->addResolvables(tgt_resolvables, true /*installed*/);
}

void load_sources()
{
  for ( std::list<Source_Ref>::iterator it = gData.sources.begin(); it !=  gData.sources.end(); ++it )
  {
    if (! it->enabled())
      continue;			// #217297

    cerr << "Parsing metadata for " << it->alias() << "..." << endl;
    ResStore src_resolvables(it->resolvables());
    cerr_v << "   " <<  src_resolvables.size() << " resolvables." << endl;
    God->addResolvables(src_resolvables);
  }
}

void establish ()
{
  cerr_v << "Establishing status of aggregates" << endl;
  God->resolver()->establishPool();
  dump_pool ();
}

void resolve()
{
  establish ();
  cerr_v << "Resolving dependencies ..." << endl;
  God->resolver()->resolvePool();
}

//! are there applicable patches?
void patch_check ()
{
  cerr_vv << "patch check" << endl;
  gData.patches_count = gData.security_patches_count = 0;

  ResPool::byKind_iterator
    it = God->pool().byKindBegin<Patch>(),
    e = God->pool().byKindEnd<Patch>();
  for (; it != e; ++it )
  {
    ResObject::constPtr res = it->resolvable();
    Patch::constPtr patch = asKind<Patch>(res);

    if ( it->status().isNeeded() )
    {
      gData.patches_count++;
      if (patch->category() == "security")
        gData.security_patches_count++;
    }
  }

  cout << gData.patches_count << " patches needed. ( " << gData.security_patches_count << " security patches )"  << std::endl;
}

string string_status (const ResStatus& rs)
{
  bool i = rs.isInstalled ();
  if (rs.isUndetermined ())
    return i? "Installed": "Uninstalled";
  else if (rs.isEstablishedUneeded ())
    return i? "No Longer Applicable": "Not Applicable";
  else if (rs.isEstablishedSatisfied ())
    return i? "Applied": "Not Needed";
  else if (rs.isEstablishedIncomplete ())
    return i? "Broken": "Needed";
  // if ResStatus interface changes
  return "error";
}

void dump_pool ()
{
  int count = 1;
  static bool full_pool_shown = false;

  _XDEBUG( "---------------------------------------" );
  for (ResPool::const_iterator it =   God->pool().begin(); it != God->pool().end(); ++it, ++count) {
    
    if (!full_pool_shown                                    // show item if not shown all before
	|| it->status().transacts()                         // or transacts
	|| !it->status().isUndetermined())                  // or established status
    {
      _DEBUG( count << ": " << *it );
    }
  }
  _XDEBUG( "---------------------------------------" );
  full_pool_shown = true;
}

// patches
void show_patches()
{
  MIL << "Pool contains " << God->pool().size() << " items. Checking whether available patches are needed." << std::endl;

  Table tbl;
  TableHeader th;
  th << "Catalog" << "Name" << "Version" << "Category" << "Status";
  tbl << th;

  ResPool::byKind_iterator
    it = God->pool().byKindBegin<Patch>(),
    e  = God->pool().byKindEnd<Patch>();
  for (; it != e; ++it )
  {
    ResObject::constPtr res = it->resolvable();
    if ( it->status().isUndetermined() ) {
#warning is this a library bug?
      // these are duplicates of those that are determined
      continue;
    }
    Patch::constPtr patch = asKind<Patch>(res);

    TableRow tr;
    tr << patch->source ().alias ();
    tr << res->name () << res->edition ().asString();
    tr << patch->category();
    tr << string_status (it->status ());
    tbl << tr;
  }
  tbl.sort (1);			// Name
  cout << tbl;
}

void list_patch_updates ()
{
  Table tbl;
  Table pm_tbl;	// only those that affect packagemanager: they have priority
  TableHeader th;
  unsigned cols;

  th << "Catalog" << "Name" << "Version" << "Category" << "Status";
  cols = 5;
  tbl << th;
  pm_tbl << th;

  const zypp::ResPool& pool = God->pool();
  ResPool::byKind_iterator
    it = pool.byKindBegin<Patch> (),
    e  = pool.byKindEnd<Patch> ();
  for (; it != e; ++it )
  {
    ResObject::constPtr res = it->resolvable();
    if ( it->status().isNeeded() ) {
      Patch::constPtr patch = asKind<Patch>(res);

      if (true) {
	TableRow tr (cols);
	tr << patch->source ().alias ();
	tr << res->name () << res->edition ().asString();
	tr << patch->category();
	tr << string_status (it->status ());

	tbl << tr;
	if (patch->affects_pkg_manager ())
	  pm_tbl << tr;
      }
    }
  }

  // those that affect the package manager go first
  // (TODO: user option for this?)
  if (!pm_tbl.empty ())
    tbl = pm_tbl;

  tbl.sort (1); 		// Name
  cout << tbl;
}

typedef set<PoolItem_Ref> Candidates;

void find_updates( const ResObject::Kind &kind, Candidates &candidates )
{
  const zypp::ResPool& pool = God->pool();
  ResPool::byKind_iterator
    it = pool.byKindBegin (kind),
    e  = pool.byKindEnd (kind);
  cerr_vv << "Finding update candidates" << endl;
  for (; it != e; ++it)
  {
    if (it->status().isUninstalled())
      continue;
    // (actually similar to ProvideProcess?)
    PoolItem_Ref candidate = solver::detail::Helper::findUpdateItem (pool, *it);
    if (!candidate.resolvable())
      continue;

    cerr_vv << "item " << *it << endl;
    cerr_vv << "cand " << candidate << endl;
    candidates.insert (candidate);
  }
}

void list_updates( const ResObject::Kind &kind )
{
  bool k_is_patch = kind == ResTraits<Patch>::kind;
  if (k_is_patch)
    list_patch_updates ();
  else {
    Table tbl;
    TableHeader th;
    unsigned cols = 5;
    th << "S" << "Catalog";
    if (gSettings.is_rug_compatible) {
      th << "Bundle";
      ++cols;
    }
    th << "Name" << "Version" << "Arch";
    tbl << th;

    Candidates candidates;
    find_updates (kind, candidates);

    Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
    for (ci = cb; ci != ce; ++ci) {
//      ResStatus& candstat = ci->status();
//      candstat.setToBeInstalled (ResStatus::USER);
      ResObject::constPtr res = ci->resolvable();
      TableRow tr (cols);
      tr << "v" << res->source ().alias ();
      if (gSettings.is_rug_compatible)
	tr << "";		// Bundle
      tr << res->name ()
	 << res->edition ().asString ()
	 << res->arch ().asString ();
      tbl << tr;
    }
    tbl.sort (gSettings.is_rug_compatible? 3: 2); // Name
    cout << tbl;
  }
}

// may be useful as a functor
bool mark_item_install (const PoolItem& pi) {
  bool result = pi.status().setToBeInstalled( zypp::ResStatus::USER );
  if (!result) {
    cerr_vv << "Marking " << pi << "for installation failed" << endl;
  }
  return result;
}

void mark_patch_updates ()
{
  if (true) {
    // search twice: if there are none with affects_pkg_manager, retry on all
    bool nothing_found = true;
    for (int attempt = 0;
	 nothing_found && attempt < 2; ++attempt) {
      ResPool::byKind_iterator
	it = God->pool().byKindBegin<Patch> (),
	e  = God->pool().byKindEnd<Patch> ();
      for (; it != e; ++it )
      {
	ResObject::constPtr res = it->resolvable();

	if ( it->status().isNeeded() ) {
	  Patch::constPtr patch = asKind<Patch>(res);
	  if (attempt == 1 || patch->affects_pkg_manager ()) {
	    nothing_found = false;
	    mark_item_install (*it);
	  }
	}
      }
    }
  }
}

void mark_updates( const ResObject::Kind &kind )
{
  bool k_is_patch = kind == ResTraits<Patch>::kind;

  if (k_is_patch) {
    mark_patch_updates ();
  }
  else {
    Candidates candidates;
    find_updates (kind, candidates);
    invokeOnEach (candidates.begin(), candidates.end(), mark_item_install);
  }
}

void solve_and_commit (bool non_interactive) {
  resolve();
    
  show_problems ();

  if (show_summary()) {
      
    cerr << "Continue? [y/n] " << (non_interactive ? "y\n" : "");
    if (non_interactive || readBoolAnswer()) {
      cerr_v << "committing" << endl;
      ZYppCommitResult result = God->commit( ZYppCommitPolicy() );
      cerr_v << result << std::endl; 
    }
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
