#include <fstream>
#include <sstream>
#include <boost/format.hpp>
#include "zypper.h"
#include "zypper-misc.h"

#include <zypp/Patch.h>
#include <zypp/base/Algorithm.h>
#include <zypp/solver/detail/Helper.h>

#include <zypp/RepoManager.h>
#include <zypp/RepoInfo.h>
#include <zypp/repo/RepoException.h>

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

// return the default value on input failure
// TODO make this locale dependent?
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
  else {
    // store encountered target item (installed)
    installed_item = provider;
    if (!item) item = provider;
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
    cerr << kind << " '" << name << "' " << _("not found") << endl;
    return; //error?
  }

  if (installer.installed_item &&
      installer.installed_item.resolvable()->edition() == installer.item.resolvable()->edition() &&
      installer.installed_item.resolvable()->arch() == installer.item.resolvable()->arch()) {
    cout << _("skipping ") << kind.asString() << " '" << name << "' " << _("(already installed)") << endl;
  }
  else {
    // TODO don't use setToBeInstalled for this purpose but higher level solver API
    bool result = installer.item.status().setToBeInstalled( zypp::ResStatus::USER );
    if (!result) {
      cerr << format(_("Failed to add '%s' to the list of packages to be installed.")) % name << endl;
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
    cerr_vv << "Marking for deletion: " << provider << endl;
    bool result = provider.status().setToBeUninstalled( zypp::ResStatus::USER );
    if (!result) {
      cerr << _("Failed") << endl;
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
    stm << format (_(" Solution %s: ")) % n << (*ii)->description () << endl;
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

      if ( it->status().isToBeInstalled() )
        cerr << _("<install>   ");
      if ( it->status().isToBeUninstalled() )
        cerr << _("<uninstall> ");
      cerr << *res << endl;
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
/*
void cond_load_resolvables ()
{	
  // load repository resolvables
  load_sources();

  // load target resolvables
  if ( ! gSettings.disable_system_resolvables ) {
    load_target();
  }
}
*/
void load_target()
{
  cerr << _("Parsing RPM database...") << endl;
  ResStore tgt_resolvables(God->target()->resolvables());
  cerr_v << "   " <<  tgt_resolvables.size() << _(" resolvables.") << endl;
  God->addResolvables(tgt_resolvables, true /*installed*/);
}
/*
void load_sources()
{
  for ( std::list<Source_Ref>::iterator it = gData.sources.begin(); it !=  gData.sources.end(); ++it )
  {
    if (! it->enabled())
      continue;			// #217297

    cerr << _("Parsing metadata for ") << it->alias() << "..." << endl;
    ResStore src_resolvables(it->resolvables());
    cerr_v << "   " <<  src_resolvables.size() << _(" resolvables.") << endl;
    God->addResolvables(src_resolvables);
  }
}
*/

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
      if (!manager.isCached(repo))
      {
        cout_v << format(_("Repository '%s' not cached. Caching..."))
                         % repo.alias() << endl;
        manager.buildCache(repo);
      }

      repository = manager.createFromCache(repo);
      ResStore store = repository.resolvables();
      God->addResolvables(store);
  
      cout_vv << format(_("(%d resolvables found)")) % store.size() << endl;
    }
    catch ( const repo::RepoNotCachedException &e )
    {
      ZYPP_CAUGHT(e);
      cerr << format(_("Problem loading data from '%s'")) % repo.alias();
      cerr_v << ":" << e.msg();
      cerr << endl;
      cerr << format(_("Resolvables from '%s' not loaded because of error."))
                      % repo.alias() << endl;
    }
  }
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

  cout << gData.patches_count << _(" patches needed") << ". ( " << gData.security_patches_count << _(" security patches") << " )"  << std::endl;
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
  th << _("Catalog") << _("Name") << _("Version") << _("Category") << _("Status");
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

  th << _("Catalog") << _("Name") << _("Version") << _("Category") << _("Status");
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
  cout << tbl;
}

// collect items, select best edition.
class LookForArchUpdate : public zypp::resfilter::PoolItemFilterFunctor
{
public:
  PoolItem_Ref uninstalled;

  bool operator()( PoolItem_Ref provider )
    {
      if (!provider.status().isLocked()	// is not locked (taboo)
	  && (!uninstalled		// first match
	      // or a better edition than candidate
	      || uninstalled->edition().compare( provider->edition() ) < 0))
      {
	uninstalled = provider;	// store 
      }
      return true;		// keep going
    }
};

// Find best (according to edition) uninstalled item
// with same kind/name/arch as item.
// Similar to zypp::solver::detail::Helper::findUpdateItem
// but that allows changing the arch (#222140).
static
PoolItem_Ref
findArchUpdateItem (const ResPool & pool, PoolItem_Ref item)
{
  LookForArchUpdate info;

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
    PoolItem_Ref candidate = findArchUpdateItem (pool, *it);
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
    th << _("S") << _("Catalog"); // for translators: S stands for Status
    if (gSettings.is_rug_compatible) {
      th << "Bundle";
      ++cols;
    }
    th << _("Name") << _("Version") << _("Arch");
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

static
void mark_patch_updates (bool skip_interactive)
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

void mark_updates( const ResObject::Kind &kind, bool skip_interactive )
{
  bool k_is_patch = kind == ResTraits<Patch>::kind;

  if (k_is_patch) {
    mark_patch_updates (skip_interactive);
  }
  else {
    Candidates candidates;
    find_updates (kind, candidates);
    invokeOnEach (candidates.begin(), candidates.end(), mark_item_install);
  }
}


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
    cerr << _("Continue?") << " [y/n] " << (non_interactive ? "y\n" : "");
    if (non_interactive || readBoolAnswer()) {

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
            " with installation source or media. Try one of the following:\n"
            "\n"
            "- just retry previous command\n"
            "- refresh installation sources using 'zypper refresh'\n"
            "- use another installation media (if e.g. damaged)\n"
            "- use another installation source") << endl;
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

// TODO
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
      cout << it->resolvable()->name() << " " <<
        it->resolvable()->kind().asString() <<
        " " << _("license") << ": " <<
        it->resolvable()->licenseToConfirm() << endl;

      cout << _("In order to install this package, you must agree"
        " to terms of the above licencse. Continue?") << " [y/n] " <<
        (non_interactive ? "n\n" : "");

      if (non_interactive || !readBoolAnswer())
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
