#include <fstream>
#include <sstream>
#include <boost/format.hpp>

#include <zypp/Patch.h>
#include <zypp/base/Algorithm.h>
#include <zypp/solver/detail/Helper.h>

#include <zypp/RepoManager.h>
#include <zypp/RepoInfo.h>
#include <zypp/repo/RepoException.h>
#include <zypp/target/store/xml_escape_parser.hpp>

#include "zypper.h"
#include "zypper-misc.h"
#include "zypper-callbacks.h"

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;
extern RuntimeData gData;
extern Settings gSettings;


void cond_init_target () {
  static bool done = false;
  //! \todo do this so that it works in zypper shell
  if (!done) {
    cout_v << _("Initializing Target") << endl;
    God->initializeTarget(gSettings.root_dir);
    done = true;
  }
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
  else {
    // store encountered target item (installed)
    installed_item = provider;
    if (!item) item = provider;
  }

  return true;
}

static std::string xml_escape( const std::string &text )
{
	iobind::parser::xml_escape_parser parser;
	return parser.escape(text);
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
  cout_vv << "Iterating over [" << kind << "]" << name << endl;
  invokeOnEach( pool.byNameBegin( name ),
		pool.byNameEnd( name ),
		resfilter::ByKind( kind ),
		zypp::functor::functorRef<bool,const zypp::PoolItem&> (installer)
		);
  cout_vv << "... done" << endl;
  if (!installer.item) {
    // TranslatorExplanation e.g. "package 'pornview' not found"
    cerr << format(_("%s '%s' not found")) % kind % name << endl;
    WAR << format("%s '%s' not found") % kind % name << endl;

    return;
  }

  if (installer.installed_item &&
      installer.installed_item.resolvable()->edition() == installer.item.resolvable()->edition() &&
      installer.installed_item.resolvable()->arch() == installer.item.resolvable()->arch()) {
    cout_n << format(_("skipping %s '%s' (already installed)")) % kind.asString() % name << endl;
  }
  else {
    // TODO don't use setToBeInstalled for this purpose but higher level solver API
    bool result = installer.item.status().setToBeInstalled( zypp::ResStatus::USER );
    if (!result) {
      cerr << format(_("Failed to add '%s' to the list of packages to be installed.")) % name << endl;
      ERR << "Could not set " << name << " as to-be-installed" << endl;
    }
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
    cout_vv << "Marking for deletion: " << provider << endl;
    bool result = provider.status().setToBeUninstalled( zypp::ResStatus::USER );
    if (!result) {
      cerr << format(
          _("Failed to add '%s' to the list of packages to be removed."))
          % provider.resolvable()->name() << endl;
      ERR << "Could not set " << provider.resolvable()->name()
          << " as to-be-uninstalled" << endl;
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
    cerr << _("Not found") << endl;
    return; //error?
  }
}

// debugging
static
ostream& operator << (ostream & stm, ios::iostate state)
{
  return stm << (state & ifstream::eofbit ? "Eof ": "")
	     << (state & ifstream::badbit ? "Bad ": "")
	     << (state & ifstream::failbit ? "Fail ": "")
	     << (state == 0 ? "Good ": "");
}

//! @return true to retry solving now, false to cancel, indeterminate to continue
tribool show_problem (bool non_interactive, const ResolverProblem & prob, ProblemSolutionList & todo)
{
  ostream& stm = cerr;
  string det;
  stm << _("Problem: ") << prob.description () << endl;
  det = prob.details ();
  if (!det.empty ())
    stm << " " << det << endl;

  int n;
  ProblemSolutionList solutions = prob.solutions ();
  ProblemSolutionList::iterator
    bb = solutions.begin (),
    ee = solutions.end (),
    ii;
  for (n = 1, ii = bb; ii != ee; ++n, ++ii) {
    // TranslatorExplanation %d is the solution number
    stm << format (_(" Solution %d: ")) % n << (*ii)->description () << endl;
    det = (*ii)->details ();
    if (!det.empty ())
      stm << "  " << det << endl;
  }

  if (non_interactive)
    return false;

  int reply;
  do {
    // input prompt
    cerr << _("number, (r)etry or (c)ancel> ") << flush;
    string reply_s = str::getline (cin, zypp::str::TRIM);

    if (! cin.good()) {
      cerr_v << "cin: " << cin.rdstate() << endl;
      return false;
    }
    // translators: corresponds to (r)etry
    if (reply_s == _("r"))
      return true;
    // translators: corresponds to (c)ancel
    else if (reply_s == _("c"))
      return false;

    str::strtonum (reply_s, reply);
  } while (reply <= 0 || reply >= n);

  cerr << format (_("Applying solution %s")) % reply << endl;
  ProblemSolutionList::iterator reply_i = solutions.begin ();
  advance (reply_i, reply - 1);
  todo.push_back (*reply_i);

  tribool go_on = indeterminate; // continue with next problem
  return go_on;
}

// return true to retry solving, false to cancel transaction
bool show_problems (bool non_interactive)
{
  bool retry = true;
  ostream& stm = cerr;
  Resolver_Ptr resolver = zypp::getZYpp()->resolver();
  ResolverProblemList rproblems = resolver->problems ();
  ResolverProblemList::iterator
    b = rproblems.begin (),
    e = rproblems.end (),
    i;
  ProblemSolutionList todo;
  bool no_problem = b == e;
  if (!no_problem) {
    stm << format (_("%s Problems:")) % rproblems.size() << endl;
  }
  for (i = b; i != e; ++i) {
    stm << _("Problem: ") << (*i)->description () << endl;
  }
  for (i = b; i != e; ++i) {
    stm << endl;
    tribool stopnow = show_problem (non_interactive, *(*i), todo);
    if (! indeterminate (stopnow)) {
      retry = stopnow == true;
      break;
    }
  }

  if (retry)
    resolver->applySolutions (todo);
  return retry;
}

/**
 * @return (-1) - nothing to do,
 *  0 - there is at least 1 resolvable to be installed/uninstalled,
 *  ZYPPER_EXIT_INF_REBOOT_NEEDED - if one of patches to be installed needs machine reboot,
 *  ZYPPER_EXIT_INF_RESTART_NEEDED - if one of patches to be installed needs package manager restart
 */
int show_summary()
{
  int retv = -1; // nothing to do;

	if (!gSettings.machine_readable)
	  cerr << _("Summary:") << endl;

  MIL << "Pool contains " << God->pool().size() << " items." << std::endl;
  for ( ResPool::const_iterator it = God->pool().begin(); it != God->pool().end(); ++it )
  {
    ResObject::constPtr res = it->resolvable();
    if ( it->status().isToBeInstalled() || it->status().isToBeUninstalled() )
    {
      if (retv == -1) retv = 0;

      if (it->resolvable()->kind() == ResTraits<Patch>::kind) {
        Patch::constPtr patch = asKind<Patch>(it->resolvable());
        
        // set return value to 'reboot needed'
        if (patch->reboot_needed())
          retv = ZYPPER_EXIT_INF_REBOOT_NEEDED;
        // set return value to 'restart needed' (restart of package manager)
        // however, 'reboot needed' takes precedence
        else if (retv != ZYPPER_EXIT_INF_REBOOT_NEEDED && patch->affects_pkg_manager())
          retv = ZYPPER_EXIT_INF_RESTART_NEEDED;
      }

			if (!gSettings.machine_readable)
			{
	      if ( it->status().isToBeInstalled() )
	        cerr << _("<install>   ");
	      if ( it->status().isToBeUninstalled() )
	        cerr << _("<uninstall> ");
	      cerr << *res << endl;
			}
    }
  }
  if (retv == -1)
    cerr << _("Nothing to do.") << endl;

  return retv;
}
/*
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
*/

void cond_load_resolvables()
{	
  load_repo_resolvables();

  if ( ! gSettings.disable_system_resolvables ) {
    load_target_resolvables();
  }
}

/** read repository resolvables */
void load_repo_resolvables()
{
  RepoManager manager;

  for (std::list<RepoInfo>::iterator it = gData.repos.begin();
       it !=  gData.repos.end(); ++it)
  {
    RepoInfo repo(*it);

    if (! it->enabled())
      continue;     // #217297

    Repository repository;

    try 
    {
      // if there is no metadata locally
      if ( manager.metadataStatus(repo).empty() )
      {
        cout_v << format(_("Retrieving repository '%s' information..."))
                         % repo.alias() << endl;
        manager.refreshMetadata(repo);
      }

      if (!manager.isCached(repo))
      {
        cout_v << format(_("Repository '%s' not cached. Caching..."))
                         % repo.alias() << endl;
        manager.buildCache(repo);
      }
      // TranslatorExplanation speaking of a repository
      cout_n << format(_("Reading repository %s...")) % repo.alias() << flush;
      repository = manager.createFromCache(repo);

      ResStore store = repository.resolvables();
      cout_v << " " << format(_("(%d resolvables found)")) % store.size();

      God->addResolvables(store);
      cout_n << endl;
    }
    catch (const Exception & e)
    {
      ZYPP_CAUGHT(e);
      cerr << format(_("Problem loading data from '%s'")) % repo.alias();
      cerr_v << ":" << endl << e.msg();
      cerr << endl;
      cerr << format(_("Resolvables from '%s' not loaded because of error."))
                      % repo.alias() << endl;
    }
  }
}

void load_target_resolvables()
{
  cout_n << _("Reading RPM database...");
  MIL << "Going to read RPM database" << endl;

  ResStore tgt_resolvables(God->target()->resolvables());

  cout_v << "   " <<  format(_("(%s resolvables)")) % tgt_resolvables.size();
  cout_n << endl;
  DBG << tgt_resolvables.size() << " resolvables read";

  God->addResolvables(tgt_resolvables, true /*installed*/);
}

void establish ()
{
  cerr_v << _("Establishing status of aggregates") << endl;
  God->resolver()->establishPool();
  dump_pool ();
}

bool resolve()
{
  establish ();
  cerr_v << _("Resolving dependencies...") << endl;
  return God->resolver()->resolvePool();
}

void patch_check ()
{
  cout_vv << "patch check" << endl;
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

  cout << format(_("%d patches needed (%d security patches)"))
                 % gData.patches_count % gData.security_patches_count
       << std::endl;
}

string string_status (const ResStatus& rs)
{
  bool i = rs.isInstalled ();
  if (rs.isUndetermined ())
    return i? _("Installed"): _("Uninstalled");
  else if (rs.isEstablishedUneeded ())
    return i? _("No Longer Applicable"): _("Not Applicable");
  else if (rs.isEstablishedSatisfied ())
    return i? _("Applied"): _("Not Needed");
  else if (rs.isEstablishedIncomplete ())
    return i? _("Broken"): _("Needed");
  // if ResStatus interface changes
  return _("error");
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
  th << (gSettings.is_rug_compatible ? _("Catalog: ") : _("Repository: "))
     << _("Name") << _("Version") << _("Category") << _("Status");
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
    tr << patch->repository().info().alias ();
    tr << res->name () << res->edition ().asString();
    tr << patch->category();
    tr << string_status (it->status ());
    tbl << tr;
  }
  tbl.sort (1);			// Name

  if (tbl.empty())
    cout_n << _("No needed patches found.") << endl;
  else
    // display the result, even if --quiet specified
    cout << tbl;
}

// ----------------------------------------------------------------------------

void xml_list_patches ()
{
  const zypp::ResPool& pool = God->pool();
  ResPool::byKind_iterator
    it = pool.byKindBegin<Patch> (),
    e  = pool.byKindEnd<Patch> ();

  for (; it != e; ++it )
  {
    ResObject::constPtr res = it->resolvable();
    if ( it->status().isNeeded() )
    {
      Patch::constPtr patch = asKind<Patch>(res);

      cout << " <update ";
      cout << "name=\"" << res->name () << "\" " ;
      cout << "edition=\""  << res->edition ().asString() << "\" ";
      cout << "category=\"" <<  patch->category() << "\" ";
      cout << "pkgmanager=\"" << ((patch->affects_pkg_manager()) ? "true" : "false") << "\" ";
      cout << "restart=\"" << ((patch->reboot_needed()) ? "true" : "false") << "\" ";
      cout << "interactive=\"" << ((patch->interactive()) ? "true" : "false") << "\" ";
      cout << "resolvabletype=\"" << "patch" << "\" ";
      cout << ">" << endl;
      cout << "  <summary>" << xml_escape(patch->summary()) << "  </summary>" << endl;
      cout << "  <description>" << xml_escape(patch->description()) << "</description>" << endl;
      cout << "  <license>" << xml_escape(patch->licenseToConfirm()) << "</license>" << endl;


      if ( patch->repository() != Repository::noRepository )
      {
        cout << "  <source url=\"" << *(patch->repository().info().baseUrlsBegin());
        cout << "\" alias=\"" << patch->repository().info().alias() << "\"/>" << endl;
      }

      cout << " </update>" << endl;
    }
  }
}


// ----------------------------------------------------------------------------

void list_patch_updates( const string &repo_alias, bool best_effort )
{
  Table tbl;
  Table pm_tbl;	// only those that affect packagemanager: they have priority
  TableHeader th;
  unsigned cols;

  th << (gSettings.is_rug_compatible ? _("Catalog: ") : _("Repository: "))
     << _("Name") << _("Version") << _("Category") << _("Status");
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
	tr << patch->repository().info().alias ();
	tr << res->name () << res->edition ().asString();
	tr << patch->category();
	tr << string_status (it->status ());

	if (patch->affects_pkg_manager ())
	  pm_tbl << tr;
	else
	  tbl << tr;
      }
    }
  }

  // those that affect the package manager go first
  // (TODO: user option for this?)
  if (!pm_tbl.empty ()) {
    if (!tbl.empty ()) {
      cerr << _("WARNING: These are only the updates affecting the updater itself.\n"
		"There are others available too.\n") << flush;
		
    }
    tbl = pm_tbl;
  }

  tbl.sort (1); 		// Name

  if (tbl.empty())
    cout_n << _("No updates found.") << endl;
  else
    cout << tbl;
}

// ----------------------------------------------------------------------------

// collect items, select best edition.
class LookForArchUpdate : public zypp::resfilter::PoolItemFilterFunctor
{
public:
  PoolItem_Ref uninstalled;
  string _repo_alias;

  LookForArchUpdate( const string &repo_alias = "" )
  {
    _repo_alias = repo_alias;
  }

  bool operator()( PoolItem_Ref provider )
    {
      if (!provider.status().isLocked()	// is not locked (taboo)
	  && (!uninstalled		// first match
	      // or a better edition than candidate
	      || uninstalled->edition().compare( provider->edition() ) < 0)
	  && (_repo_alias.empty()
	      || provider->repository().info().alias() == _repo_alias) )
      {
	uninstalled = provider;	// store 
      }
      return true;		// keep going
    }
};

// ----------------------------------------------------------------------------

// Find best (according to edition) uninstalled item
// with same kind/name/arch as item.
// Similar to zypp::solver::detail::Helper::findUpdateItem
// but that allows changing the arch (#222140).
static
PoolItem_Ref
findArchUpdateItem( const ResPool & pool, PoolItem_Ref item, const string &repo_alias )
{
  LookForArchUpdate info( repo_alias );

  invokeOnEach( pool.byNameBegin( item->name() ),
		pool.byNameEnd( item->name() ),
		// get uninstalled, equal kind and arch, better edition
		functor::chain (
		  functor::chain (
		    functor::chain (
		      resfilter::ByUninstalled (),
		      resfilter::ByKind( item->kind() ) ),
		    resfilter::byArch<CompareByEQ<Arch> >( item->arch() ) ),
		  resfilter::byEdition<CompareByGT<Edition> >( item->edition() )),
		functor::functorRef<bool,PoolItem> (info) );

  _XDEBUG("findArchUpdateItem(" << item << ") => " << info.uninstalled);
  return info.uninstalled;
}

// ----------------------------------------------------------------------------

typedef set<PoolItem_Ref> Candidates;

static void
find_updates( const ResObject::Kind &kind, const string &repo_alias, Candidates &candidates )
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
    PoolItem_Ref candidate = findArchUpdateItem( pool, *it, repo_alias );
    if (!candidate.resolvable())
      continue;

    cerr_vv << "item " << *it << endl;
    cerr_vv << "cand " << candidate << endl;
    candidates.insert (candidate);
  }
}

// ----------------------------------------------------------------------------

void list_updates( const ResObject::Kind &kind, const string &repo_alias, bool best_effort )
{
  bool k_is_patch = kind == ResTraits<Patch>::kind;
  if (k_is_patch)
    list_patch_updates( repo_alias, best_effort );
  else {
    Table tbl;

    // show repo only if not best effort or --from-repo set
    //   on best_effort, the solver will determine the repo if we don't limit it to a specific one
    bool hide_repo = best_effort && repo_alias.empty();

    // header
    TableHeader th;
    unsigned int name_col;
    // TranslatorExplanation S stands for Status
    th << _("S");
    if (!hide_repo) {
      th << (gSettings.is_rug_compatible ? _("Catalog: ") : _("Repository: ")); 
    }
    if (gSettings.is_rug_compatible) {
      th << _("Bundle");
    }
    name_col = th.cols();
    th << _("Name");
    if (!best_effort) {		// best_effort does not know version or arch yet
      th << _("Version") << _("Arch");
    }
    tbl << th;

    unsigned int cols = th.cols();

    Candidates candidates;
    find_updates( kind, repo_alias, candidates );	// best_effort could be passed here ...

    Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
    for (ci = cb; ci != ce; ++ci) {
//      ResStatus& candstat = ci->status();
//      candstat.setToBeInstalled (ResStatus::USER);
      ResObject::constPtr res = ci->resolvable();
      TableRow tr (cols);
      tr << "v";
      if (!hide_repo) {
	tr << res->repository().info().alias();
      }
      if (gSettings.is_rug_compatible)
	tr << "";		// Bundle
      tr << res->name ();

      // strictly speaking, we could show version and arch even in best_effort
      //  iff there is only one candidate. But we don't know the number of candidates here.
      if (!best_effort) {
	 tr << res->edition ().asString ()
	    << res->arch ().asString ();
      }
      tbl << tr;
    }
    tbl.sort( name_col );

    if (tbl.empty())
      cout_n << _("No updates found.") << endl;
    else
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

// ----------------------------------------------------------------------------

void xml_list_updates()
{
  Candidates candidates;
  string repo_alias;
  find_updates (ResTraits<Package>::kind, repo_alias, candidates);

  Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
  for (ci = cb; ci != ce; ++ci) {
    ResObject::constPtr res = ci->resolvable();

    cout << " <update ";
    cout << "name=\"" << res->name () << "\" " ;
    cout << "edition=\""  << res->edition ().asString() << "\" ";
    cout << "resolvabletype=\"" << "package" << "\" ";
    cout << ">" << endl;
    cout << "  <summary>" << xml_escape(res->summary()) << "  </summary>" << endl;
    cout << "  <description>" << xml_escape(res->description()) << "</description>" << endl;
    cout << "  <license>" << xml_escape(res->licenseToConfirm()) << "</license>" << endl;
    
    if ( res->repository() != Repository::noRepository )
    {
    	cout << "  <source url=\"" << *(res->repository().info().baseUrlsBegin());
    	cout << "\" alias=\"" << res->repository().info().alias() << "\"/>" << endl;
    }

    cout << " </update>" << endl;
  }
}


// ----------------------------------------------------------------------------

static
void mark_patch_updates( const std::string &repo_alias, bool skip_interactive )
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
	    // #221476
	    if (skip_interactive && patch->interactive()) {
	      // Skipping a patch because it is interactive and
	      // --skip-interactive is requested. %s is a name of a
	      // patch
	      cerr << format (_("WARNING: %s is interactive, skipped."))
		% res << endl;
	    }
	    else {
	      nothing_found = false;
	      mark_item_install (*it);
	    }
	  }
	}
      }
    }
  }
}

// ----------------------------------------------------------------------------

void mark_updates( const ResObject::Kind &kind, const std::string &repo_alias, bool skip_interactive, bool best_effort )
{
  bool k_is_patch = kind == ResTraits<Patch>::kind;

  if (k_is_patch) {
    mark_patch_updates( repo_alias, skip_interactive );
  }
  else {
    Candidates candidates;
    find_updates (kind, repo_alias, candidates);	// best_effort could be passed here ...
    invokeOnEach (candidates.begin(), candidates.end(), mark_item_install);
  }
}

// ----------------------------------------------------------------------------

/**
 * @return ZYPPER_EXIT_OK - successful commit,
 *  ZYPPER_EXIT_ERR_ZYPP - if ZYppCommitResult contains resolvables with errors,
 *  ZYPPER_EXIT_INF_REBOOT_NEEDED - if one of patches to be installed needs machine reboot,
 *  ZYPPER_EXIT_INF_RESTART_NEEDED - if one of patches to be installed needs package manager restart
 */
int solve_and_commit (bool non_interactive) {
  while (true) {
    bool success = resolve();
    if (success)
      break;

    success = show_problems (non_interactive);
    if (! success) {
      // TODO cancel transaction?
      return ZYPPER_EXIT_ERR_ZYPP; // #242736
    }
  }


  // returns -1, 0, ZYPPER_EXIT_INF_REBOOT_NEEDED, or ZYPPER_EXIT_INF_RESTART_NEEDED
  int retv = show_summary();

  if (retv >= 0) { // there are resolvables to install/uninstall
    if (read_bool_answer(_("Continue?"), non_interactive)) {

      if (!confirm_licenses(non_interactive)) return ZYPPER_EXIT_OK;

      cerr_v << _("committing") << endl;
      
      try {
        // FIXME do resync if in shell mode, how
        // do I know if in shell mode?
        ZYppCommitResult result = God->commit( ZYppCommitPolicy().syncPoolAfterCommit(false) );

        if (!result._errors.empty())
          retv = ZYPPER_EXIT_ERR_ZYPP;

        cerr_v << result << std::endl;
      }
      catch ( const Exception & excpt_r ) {
        ZYPP_CAUGHT( excpt_r );
        
        // special handling for failed integrity exception
        if (excpt_r.msg().find("fails integrity check") != string::npos) {
          cerr << endl
            << _("Package integrity check failed. This may be a problem"
            " with repository or media. Try one of the following:\n"
            "\n"
            "- just retry previous command\n"
            "- refresh repositories using 'zypper refresh'\n"
            "- use another installation media (if e.g. damaged)\n"
            "- use another repository") << endl;
          return ZYPPER_EXIT_ERR_ZYPP;
        }
        else
          ZYPP_RETHROW( excpt_r );
      }
    }
  }

  if (retv < 0)
    retv = ZYPPER_EXIT_OK;
  else if (retv == ZYPPER_EXIT_INF_REBOOT_NEEDED)
    cout << _("WARNING: One of installed patches requires reboot of"
      " your machine. Please, do it as soon as possible.") << endl;
  else if (retv == ZYPPER_EXIT_INF_RESTART_NEEDED)
    cout << _("WARNING: One of installed patches affects the package"
      " manager itself, thus it requires restart before executing"
      " next operations.") << endl;

  return retv;
}

// TODO confirm licenses
// - make this more user-friendly e.g. show only license name and
//  ask for [y/n/r] with 'r' for read the license text
//  (opened throu more or less, etc...)
// - after negative answer, call solve_and_commit() again 
bool confirm_licenses(bool non_interactive)
{
  bool confirmed = true;

  for (ResPool::const_iterator it = God->pool().begin(); it != God->pool().end(); ++it)
  {
    if (it->status().isToBeInstalled() &&
        !it->resolvable()->licenseToConfirm().empty())
    {
      if (gSettings.license_auto_agree)
      {
        // TranslatorExplanation The first %s is name of the resolvable, the second is its kind (e.g. 'zypper package')
			  if (!gSettings.machine_readable)
        	cout << format(_("Automatically agreeing with %s %s license."))
	            % it->resolvable()->name() % it->resolvable()->kind().asString()
	            << endl;

        MIL << format("Automatically agreeing with %s %s license.")
            % it->resolvable()->name() % it->resolvable()->kind().asString()
            << endl;

        continue;
      }

      cout << it->resolvable()->name() << " " <<
        it->resolvable()->kind().asString() <<
        " " << _("license") << ": " <<
        it->resolvable()->licenseToConfirm() << endl;

      string question = _("In order to install this package, you must agree"
        " to terms of the above licencse. Continue?");

      if (non_interactive || !read_bool_answer(question, false))
      {
        confirmed = false;
        
        if (non_interactive)
        {
          cout << endl <<
             _("Aborting installation due to the need of"
            " license(s) confirmation.") <<
            " " << _("Please, restart the operation in interactive"
            " mode and confirm agreement with required license(s).")
            << endl;
        }
        else
        {
          cout << endl <<
            _("Aborting installation due to user disagreement"
            " with ") << it->resolvable()->name() << " " <<
            it->resolvable()->kind().asString() <<
            " " << _("license") << "." << endl;
        }

        break;
      }
    }
  }

  return confirmed;
}

// Local Variables:
// c-basic-offset: 2
// End:
