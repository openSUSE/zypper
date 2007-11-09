#include <fstream>
#include <sstream>
#include <ctype.h>
#include <boost/format.hpp>
#include <boost/logic/tribool_io.hpp>

#include <zypp/ZYppFactory.h>

#include <zypp/Edition.h>
#include <zypp/Patch.h>
#include <zypp/Package.h>
#include <zypp/SrcPackage.h>
#include <zypp/base/Algorithm.h>
#include <zypp/solver/detail/Helper.h>
#include <zypp/media/MediaException.h>
#include <zypp/FileChecker.h>

#include <zypp/RepoInfo.h>

#include <zypp/CapFactory.h>

#include <zypp/target/store/xml_escape_parser.hpp>

#include "zypper.h"
#include "zypper-utils.h"
#include "zypper-getopt.h"
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

/**
 * Stops iterations on first item and stores edition of the found item.
 *
 * If no item has been found, the \ref found variable is set to false.
 */
struct VersionGetter
{
  Edition edition;
  bool found;

  VersionGetter() : found(false) {}

  bool operator()(const zypp::PoolItem& item)
  {
    edition = item.resolvable()->edition();
    found = true;
    return false; // don't iterate further
  }
};

/**
 * Stops iterations on first item having revision newer than edition passed in
 * contructor. Stores the edition of the found item.
 *
 * If no item has been found, the \ref found variable is set to false.
 */
struct NewerVersionGetter
{
  const Edition & old_edition;
  Edition edition;
  bool found;

  NewerVersionGetter(const Edition & edition) : old_edition(edition), found(false) {}

  bool operator()(const zypp::PoolItem& item)
  {
    if (item.resolvable()->edition() > old_edition)
    {
      edition = item.resolvable()->edition();
      found = true;
      return false; // don't iterate further
    }

    return true;
  }
};

// on error print a message and return noCap
Capability safe_parse_cap (const ResObject::Kind &kind, const string & capstr) {
  Capability cap;
  try {
    // expect named caps as NAME[OP<EDITION>]
    // transform to NAME[ OP <EDITION>] (add spaces)
    string new_capstr = capstr;
    cout_vv << "capstr: " << capstr << endl;
    string::size_type op_pos = capstr.find_first_of("<>=");
    if (op_pos != string::npos)
    {
      new_capstr.insert(op_pos, " ");
      cout_vv << "new capstr: " << new_capstr << endl;
      op_pos = new_capstr.find_first_not_of("<>=", op_pos + 1);
      if (op_pos != string::npos && new_capstr.size() > op_pos)
      {
        new_capstr.insert(op_pos, " ");
        cout_vv << "new capstr: " << new_capstr << endl;
      }
    }
    // if we are about to install stuff and
    // if this is not a candidate for a versioned capability, take it like
    // a package name and check if it is already installed
    else if (command == ZypperCommand::INSTALL)
    {
      using namespace zypp::functor;
      using namespace zypp::resfilter;

      // get the installed version
      VersionGetter vg;
      invokeOnEach(
          God->pool().byNameBegin(capstr),
          God->pool().byNameEnd(capstr),
          chain(ByKind(kind),ByInstalled()),
          functorRef<bool,const zypp::PoolItem&> (vg));
      // installed found
      if (vg.found)
      {
        // check for newer version of that resolvable
        NewerVersionGetter nvg(vg.edition);
        invokeOnEach(
             God->pool().byNameBegin(capstr),
             God->pool().byNameEnd(capstr),
             chain(resfilter::ByKind(kind),not_c(ByInstalled())),
             functorRef<bool,const zypp::PoolItem&> (nvg));
        // newer version found
        if (nvg.found)
        {
          cout_vv << "installed resolvable named " << capstr
            << " found, changing capability to " << new_capstr << endl;
          new_capstr = capstr + " > " + vg.edition.asString();
        }
      }
    }

    cap = CapFactory().parse (kind, new_capstr);
  }
  catch (const Exception& e) {
    ZYPP_CAUGHT(e);
    cerr << format (_("Cannot parse capability '%s'.")) % capstr << endl;
  }
  return cap;
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
    cerr << format(_("%s '%s' not found")) % kind_to_string_localized(kind,1) % name << endl;
    WAR << format("%s '%s' not found") % kind % name << endl;

    return;
  }

  if (installer.installed_item &&
      installer.installed_item.resolvable()->edition() == installer.item.resolvable()->edition() &&
      installer.installed_item.resolvable()->arch() == installer.item.resolvable()->arch() &&
      ( ! copts.count("force") ) )
  {
    // if it is needed install anyway, even if it is installed
    if ( installer.item.status().isNeeded() )
    {
      installer.item.status().setTransact( true, zypp::ResStatus::USER );
    }

    cout_n << format(_("skipping %s '%s' (already installed)")) % kind_to_string_localized(kind,1) % name << endl;
  }
  else {

    // TODO don't use setToBeInstalled for this purpose but higher level solver API
    bool result = installer.item.status().setToBeInstalled( zypp::ResStatus::USER );
    if (!result)
    {
      // this is because the resolvable is installed and we are forcing.
      installer.item.status().setTransact( true, zypp::ResStatus::USER );
      //cerr << format(_("Failed to add '%s' to the list of packages to be installed.")) % name << endl;
      //ERR << "Could not set " << name << " as to-be-installed" << endl;
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
    // TranslatorExplanation e.g. "package 'pornview' not found"
    cerr << format(_("%s '%s' not found")) % kind_to_string_localized(kind,1) % name << endl;
    return; //error?
  }
}

void mark_by_name (bool install_not_delete,
		   const ResObject::Kind &kind,
		   const string &name )
{
  if (install_not_delete)
    mark_for_install(kind, name);
  else
    mark_for_uninstall(kind, name);
}

// don't try NAME-EDITION yet, could be confused by
// dbus-1-x11, java-1_4_2-gcj-compat, ...
/*
bool mark_by_name_edition (...)
  static const regex rx_name_edition("(.*?)-([0-9].*)");

  smatch m;
  if (! is_cap && regex_match (capstr, m, rx_name_edition)) {
    capstr = m.str(1) + " = " + m.str(2);
    is_cap = true;
  }

*/

void mark_by_capability (bool install_not_delete,
			 const ResObject::Kind &kind,
			 const string &capstr )
{
  Capability cap = safe_parse_cap (kind, capstr);

  if (cap != Capability::noCap) {
    cout_vv << "Capability: " << cap << endl;

    Resolver_Ptr resolver = zypp::getZYpp()->resolver();
    if (install_not_delete) {
      cerr_vv << "Adding requirement " << cap << endl;
      resolver->addRequire (cap);
    }
    else {
      cerr_vv << "Adding conflict " << cap << endl;
      resolver->addConflict (cap);
    }
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
tribool show_problem (const ResolverProblem & prob, ProblemSolutionList & todo)
{
  ostream& stm = cerr;
  string det;
  stm << _("Problem: ") << prob.description () << endl;
  det = prob.details ();
  if (!det.empty ())
    stm << "  " << det << endl;

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

  if (gSettings.non_interactive)
    return false;

  int reply;
  do {
    // without solutions, its useless to prompt
    if (solutions.empty())
       return false;
    // TranslatorExplanation: dependency problem solution input prompt
    cerr << _("Choose the number, (s)kip, (r)etry or (c)ancel> ") << flush;
    string reply_s = str::getline (cin, zypp::str::TRIM);

    if (! cin.good()) {
      cerr_v << "cin: " << cin.rdstate() << endl;
      return false;
    }
    if (isupper( reply_s[0] ))
      reply_s[0] = tolower( reply_s[0] );

    // translators: corresponds to (r)etry
    if (reply_s == _("r"))
      return true;
    // translators: corresponds to (c)ancel
    else if (reply_s == _("c"))
      return false;
    // translators: corresponds to (s)kip
    else if (reply_s == _("s"))
      return indeterminate; // continue with next problem

    str::strtonum (reply_s, reply);
  } while (reply <= 0 || reply >= n);

  cout_n << format (_("Applying solution %s")) % reply << endl;
  ProblemSolutionList::iterator reply_i = solutions.begin ();
  advance (reply_i, reply - 1);
  todo.push_back (*reply_i);

  tribool go_on = indeterminate; // continue with next problem
  return go_on;
}

// return true to retry solving, false to cancel transaction
bool show_problems ()
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

  // display the number of problems
  if (rproblems.size() > 1) {
    stm << format (_("%s Problems:")) % rproblems.size() << endl;
  }
  else if (rproblems.empty()) {
    // should not happen! If solve() failed at least one problem must be set!
    stm << _("Specified capability not found") << endl;
    return false;
  }

  // for many problems, list them shortly first
  if (rproblems.size() > 1)
  {
    for (i = b; i != e; ++i) {
      stm << _("Problem: ") << (*i)->description () << endl;
    }
  }
  // now list all problems with solution proposals
  for (i = b; i != e; ++i) {
    stm << endl;
    tribool stopnow = show_problem (*(*i), todo);
    if (! indeterminate (stopnow)) {
      retry = stopnow == true;
      break;
    }
  }

  if (retry)
  {
    cout_n << _("Resolving dependencies...") << endl;
    resolver->applySolutions (todo);
  }
  return retry;
}

typedef map<KindOf<Resolvable>,set<ResObject::constPtr> > KindToResObjectSet;

void show_summary_resolvable_list(const string & label,
                                  KindToResObjectSet::const_iterator it)
{
  cout << endl << label << endl;

  // get terminal width from COLUMNS env. var.
  unsigned cols = 0, cols_written = 0;
  const char *cols_s = getenv("COLUMNS");
  string cols_str("80");
  if (cols_s != NULL)
    cols_str = cols_s;
  str::strtonum (cols_str, cols);
  if (cols == 0)
    cols = 77;

#define INDENT "  "

  for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
      resit != it->second.end(); ++resit)
  {
    ResObject::constPtr res(*resit);

    if (gSettings.verbosity == VERBOSITY_NORMAL)
    {
      // watch the terminal widht
      if (cols_written == 0)
        cout << INDENT;
      else if (cols_written + res->name().size() + 1  > cols)
      {
        cout << endl;
        cols_written = 0;
      }

      cols_written += res->name().size();
    }
    else
      cout << INDENT;

    // resolvable name
    cout << res->name() << (gSettings.verbosity ? "" : " ");
    // plus edition and architecture for verbose output
    cout_v << "-" << res->edition() << "." << res->arch();
    // plus repo providing this package
    if (res->repository() != Repository::noRepository)
      cout_v << "  (" << res->repository().info().name() << ")";
    // new line after each package in the verbose mode
    cout_v << endl;
  }

  if (gSettings.verbosity == VERBOSITY_NORMAL)
    cout << endl;
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

  MIL << "Pool contains " << God->pool().size() << " items." << std::endl;
  DBG << "Install summary:" << endl;


  KindToResObjectSet to_be_installed;
  KindToResObjectSet to_be_removed;

  // collect resolvables to be installed/removed and set the return status
  for ( ResPool::const_iterator it = God->pool().begin(); it != God->pool().end(); ++it )
  {
    ResObject::constPtr res = it->resolvable();
    if ( it->status().isToBeInstalled() || it->status().isToBeUninstalled() )
    {
      if (retv == -1) retv = 0;

      if (it->resolvable()->kind() == ResTraits<Patch>::kind)
      {
        Patch::constPtr patch = asKind<Patch>(it->resolvable());

        // set return value to 'reboot needed'
        if (patch->reboot_needed())
          retv = ZYPPER_EXIT_INF_REBOOT_NEEDED;
        // set return value to 'restart needed' (restart of package manager)
        // however, 'reboot needed' takes precedence
        else if (retv != ZYPPER_EXIT_INF_REBOOT_NEEDED && patch->affects_pkg_manager())
          retv = ZYPPER_EXIT_INF_RESTART_NEEDED;
      }

      if ( it->status().isToBeInstalled()
	   && it->resolvable()->kind() != ResTraits<Atom>::kind )
      {
        DBG << "<install>   ";
        to_be_installed[it->resolvable()->kind()].insert(it->resolvable());
      }
      if ( it->status().isToBeUninstalled()
	   && it->resolvable()->kind() != ResTraits<Atom>::kind )
      {
        DBG << "<uninstall> ";
        to_be_removed[it->resolvable()->kind()].insert(it->resolvable());
      }
      DBG << *res << endl;
    }
  }

  if (retv == -1)
  {
    if (gSettings.machine_readable)
      cout << "<message type=\"warning\">" << _("Nothing to do.") << "</message>" << endl;
    else
      cout << _("Nothing to do.") << endl;

    return retv;
  }

  // no output for machines for now
  if (gSettings.machine_readable)
    return retv;

  KindToResObjectSet toinstall;
  KindToResObjectSet toupgrade;
  KindToResObjectSet todowngrade;
  KindToResObjectSet toremove;

  // iterate the to_be_installed to find installs/upgrades/downgrades + size info
  ByteCount download_size, new_installed_size;
  for (KindToResObjectSet::const_iterator it = to_be_installed.begin();
      it != to_be_installed.end(); ++it)
  {
    for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
        resit != it->second.end(); ++resit)
    {
      ResObject::constPtr res(*resit);

      // find in to_be_removed:
      bool upgrade_downgrade = false;
      for (set<ResObject::constPtr>::iterator rmit = to_be_removed[res->kind()].begin();
          rmit != to_be_removed[res->kind()].end(); ++rmit)
      {
        if (res->name() == (*rmit)->name())
        {
          if (res->edition() > (*rmit)->edition())
            toupgrade[res->kind()].insert(res);
          else
            todowngrade[res->kind()].insert(res);

          new_installed_size += res->size() - (*rmit)->size();

          to_be_removed[res->kind()].erase(*rmit);
          upgrade_downgrade = true;
          break;
        }
      }

      if (!upgrade_downgrade)
      {
        toinstall[res->kind()].insert(res);
        new_installed_size += res->size();
      }

      download_size += res->downloadSize();
    }
  }

  for (KindToResObjectSet::const_iterator it = to_be_removed.begin();
      it != to_be_removed.end(); ++it)
    for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
        resit != it->second.end(); ++resit)
    {
      toremove[it->first].insert(*resit);
      new_installed_size -= (*resit)->size();
    }

  // show summary
  for (KindToResObjectSet::const_iterator it = toupgrade.begin();
      it != toupgrade.end(); ++it)
  {
    string title = boost::str(format(_PL(
        // TranslatorExplanation %s is a "package", "patch", "pattern", etc
        "The following %s is going to be upgraded:",
        "The following %s are going to be upgraded:",
        it->second.size()
    )) % kind_to_string_localized(it->first, it->second.size()));

    show_summary_resolvable_list(title, it);
  }

  for (KindToResObjectSet::const_iterator it = todowngrade.begin();
      it != todowngrade.end(); ++it)
  {
    string title = boost::str(format(_PL(
        // TranslatorExplanation %s is a "package", "patch", "pattern", etc
        "The following %s is going to be downgraded:",
        // TranslatorExplanation %s is a "packages", "patches", "patterns", etc
        "The following %s are going to be downgraded:",
        it->second.size()
    )) % kind_to_string_localized(it->first, it->second.size()));

    show_summary_resolvable_list(title, it);
  }

  for (KindToResObjectSet::const_iterator it = toinstall.begin();
      it != toinstall.end(); ++it)
  {
    string title = boost::str(format(_PL(
        // TranslatorExplanation %s is a "package", "patch", "pattern", etc
        "The following NEW %s is going to be installed:",
        // TranslatorExplanation %s is a "packages", "patches", "patterns", etc
        "The following NEW %s are going to be installed:",
        it->second.size()
    )) % kind_to_string_localized(it->first, it->second.size()));

    show_summary_resolvable_list(title, it);
  }

  for (KindToResObjectSet::const_iterator it = toremove.begin();
      it != toremove.end(); ++it)
  {
    string title = boost::str(format(_PL(
        // TranslatorExplanation %s is a "package", "patch", "pattern", etc
        "The following %s is going to be REMOVED:",
        // TranslatorExplanation %s is a "packages", "patches", "patterns", etc
        "The following %s are going to be REMOVED:",
        it->second.size()
    )) % kind_to_string_localized(it->first, it->second.size()));

    show_summary_resolvable_list(title, it);
  }

  cout << endl;

  if (download_size > 0)
  {
    cout_n << format(_("Overall download size: %s.")) % download_size;
    cout_n << " ";
  }
  if (new_installed_size > 0)
    // TrasnlatorExplanation %s will be substituted by a byte count e.g. 212 K
    cout_n << format(_("After the operation, additional %s will be used."))
        % new_installed_size.asString(0,1,1);
  //! \todo uncomment the following for bug #309112
  /*
  else if (new_installed_size == 0)
    cout_n << _("No additional space will be used or freed after the operation.");*/
  else
  {
    // get the absolute size
    ByteCount abs;
    abs = (-new_installed_size);
    // TrasnlatorExplanation %s will be substituted by a byte count e.g. 212 K
    cout_n << format(_("After the operation, %s will be freed."))
        % abs.asString(0,1,1);
  }
  cout_n << endl;

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

void establish ()
{
  int locks = God->applyLocks();
  cout_v <<  format(_("%s items locked")) % locks << endl;
  cout_v << _("Establishing status of aggregates") << endl;
  God->resolver()->establishPool();
  dump_pool ();
}

bool resolve()
{
  establish ();
  cout_v << _("Resolving dependencies...") << endl;
  God->resolver()->setForceResolve(
      gSettings.is_rug_compatible ? true : copts.count("force-resolution") );
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
      _XDEBUG( count << ": " << *it );
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
    tr << patch->repository().info().name();
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

bool xml_list_patches ()
{
  // returns true if affects_pkg_manager patches are availble

  bool pkg_mgr_available = false;


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

      if (patch->affects_pkg_manager())
	pkg_mgr_available = true;
    }
  }


  unsigned int patchcount=0;

  it = pool.byKindBegin<Patch> ();

  for (; it != e; ++it )
  {
    ResObject::constPtr res = it->resolvable();

    patchcount++;

    if ( it->status().isNeeded())
    {
      Patch::constPtr patch = asKind<Patch>(res);
      if ((pkg_mgr_available && patch->affects_pkg_manager())  ||
	!pkg_mgr_available )
      {
        cout << " <update ";
        cout << "name=\"" << res->name () << "\" " ;
        cout << "edition=\""  << res->edition ().asString() << "\" ";
        cout << "category=\"" <<  patch->category() << "\" ";
        cout << "pkgmanager=\"" << ((patch->affects_pkg_manager()) ? "true" : "false") << "\" ";
        cout << "restart=\"" << ((patch->reboot_needed()) ? "true" : "false") << "\" ";
        cout << "interactive=\"" << ((patch->interactive()) ? "true" : "false") << "\" ";
        cout << "kind=\"" << "patch" << "\" ";
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

  if (patchcount == 0)
  {
    cout << "<appletinfo status=\"no-update-repositories\"/>" << endl;
  }

  return pkg_mgr_available;

}


// ----------------------------------------------------------------------------

void list_patch_updates(bool best_effort)
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
	tr << patch->repository().info().name();
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

// collect items, select best edition
//   this is used to find best available or installed.
// The name of the class is a bit misleading though ...

class LookForArchUpdate : public zypp::resfilter::PoolItemFilterFunctor
{
public:
  PoolItem_Ref best;

  bool operator()( PoolItem_Ref provider )
    {
      if (!provider.status().isLocked()	// is not locked (taboo)
	  && (!best 			// first match
	      // or a better edition than candidate
	      || best->edition().compare( provider->edition() ) < 0))
      {
	best = provider;	// store
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
findArchUpdateItem( const ResPool & pool, PoolItem_Ref item )
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

  _XDEBUG("findArchUpdateItem(" << item << ") => " << info.best);
  return info.best;
}

// ----------------------------------------------------------------------------

typedef set<PoolItem_Ref> Candidates;

static void
find_updates( const ResObject::Kind &kind, Candidates &candidates )
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
    PoolItem_Ref candidate = findArchUpdateItem( pool, *it );
    if (!candidate.resolvable())
      continue;

    cerr_vv << "item " << *it << endl;
    cerr_vv << "cand " << candidate << endl;
    candidates.insert (candidate);
  }
}

// ----------------------------------------------------------------------------

void list_updates( const ResObject::Kind &kind, bool best_effort )
{
  bool k_is_patch = kind == ResTraits<Patch>::kind;
  if (k_is_patch)
    list_patch_updates( best_effort );
  else {
    Table tbl;

    // show repo only if not best effort or --from-repo set
    // on best_effort, the solver will determine the repo if we don't limit it to a specific one
    bool hide_repo = best_effort || copts.count("repo");

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
    find_updates( kind, candidates );

    Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
    for (ci = cb; ci != ce; ++ci) {
//      ResStatus& candstat = ci->status();
//      candstat.setToBeInstalled (ResStatus::USER);
      ResObject::constPtr res = ci->resolvable();
      TableRow tr (cols);
      tr << "v";
      if (!hide_repo) {
	tr << res->repository().info().name();
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
// best-effort update


// find installed item matching passed one
//   use LookForArchUpdate as callback handler in order to cope with
//   multiple installed resolvables of the same name.
//   LookForArchUpdate will return the one with the highest edition.

PoolItem_Ref
findInstalledItem( PoolItem_Ref item )
{
  const zypp::ResPool& pool = God->pool();
  LookForArchUpdate info;

  invokeOnEach( pool.byNameBegin( item->name() ),
		pool.byNameEnd( item->name() ),
		// get installed, equal kind
		functor::chain (
		  resfilter::ByInstalled (),
		  resfilter::ByKind( item->kind() ) ),
		functor::functorRef<bool,PoolItem> (info) );

  _XDEBUG("findInstalledItem(" << item << ") => " << info.best);
  return info.best;
}


// require update of installed item
//   The PoolItem passed to require_item_update() is the installed resolvable
//   to which an update candidate is guaranteed to exist.
//
// may be useful as a functor
bool require_item_update (const PoolItem& pi) {
  Resolver_Ptr resolver = zypp::getZYpp()->resolver();

  PoolItem_Ref installed = findInstalledItem( pi );

  // require anything greater than the installed version
  try {
    Capability cap;
    cap = CapFactory().parse( installed->kind(), installed->name(), Rel::GT, installed->edition() );
    resolver->addRequire( cap );
  }
  catch (const Exception& e) {
    ZYPP_CAUGHT(e);
    cerr << "Cannot parse '" << installed->name() << " < " << installed->edition() << "'" << endl;
  }

  return true;
}

// ----------------------------------------------------------------------------

void xml_list_updates()
{
  Candidates candidates;
  find_updates (ResTraits<Package>::kind, candidates);

  Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
  for (ci = cb; ci != ce; ++ci) {
    ResObject::constPtr res = ci->resolvable();

    cout << " <update ";
    cout << "name=\"" << res->name () << "\" " ;
    cout << "edition=\""  << res->edition ().asString() << "\" ";
    cout << "kind=\"" << "package" << "\" ";
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
void mark_patch_updates( bool skip_interactive )
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
	    if (skip_interactive && (patch->interactive() || !patch->licenseToConfirm().empty())) {
	      // Skipping a patch because it is marked as interactive or has
	      // license to confirm and --skip-interactive is requested.
	      // TranslatorExplanation %s is the name of a patch
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

void mark_updates( const ResObject::Kind &kind, bool skip_interactive, bool best_effort )
{
  bool k_is_patch = kind == ResTraits<Patch>::kind;

  if (k_is_patch) {
    mark_patch_updates(skip_interactive);
  }
  else {
    Candidates candidates;
    find_updates (kind, candidates);
    if (best_effort)
      invokeOnEach (candidates.begin(), candidates.end(), require_item_update);
    else
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
int solve_and_commit () {
  while (true) {
    bool success = resolve();
    if (success)
      break;

    success = show_problems ();
    if (! success) {
      // TODO cancel transaction?
      return ZYPPER_EXIT_ERR_ZYPP; // #242736
    }
  }


  // returns -1, 0, ZYPPER_EXIT_INF_REBOOT_NEEDED, or ZYPPER_EXIT_INF_RESTART_NEEDED
  int retv = show_summary();
  bool was_installed = false;
  if (retv >= 0) { // there are resolvables to install/uninstall
    if (read_bool_answer(_("Continue?"), true)) {

      if (!confirm_licenses()) return ZYPPER_EXIT_OK;

      cerr_v << _("committing") << endl;

      try {
        //! \todo fix the media reporting correctly
        gData.show_media_progress_hack = true;

        // FIXME do resync if in shell mode, how do I know if in shell mode?
        ZYppCommitResult result = God->commit( ZYppCommitPolicy().syncPoolAfterCommit(false) );
        was_installed = true;

        gData.show_media_progress_hack = false;

        if (!result._errors.empty())
          retv = ZYPPER_EXIT_ERR_ZYPP;

        cerr_v << result << std::endl;
      }
      catch ( const media::MediaException & e ) {
        ZYPP_CAUGHT(e);
        report_problem(e,
            _("Problem downloading the package file from the repository:"),
            _("Please, see the above error message to for a hint."));
        return ZYPPER_EXIT_ERR_ZYPP;
      }
      catch ( const zypp::repo::RepoException & e ) {
        ZYPP_CAUGHT(e);
        report_problem(e,
            _("Problem downloading the package file from the repository:"),
            _("Please, see the above error message to for a hint."));
        return ZYPPER_EXIT_ERR_ZYPP;
      }
      catch ( const zypp::FileCheckException & e ) {
        ZYPP_CAUGHT(e);
        report_problem(e, _("The package integrity check failed. This may be a problem"
            " with the repository or media. Try one of the following:\n"
            "\n"
            "- just retry previous command\n"
            "- refresh the repositories using 'zypper refresh'\n"
            "- use another installation medium (if e.g. damaged)\n"
            "- use another repository"));
        return ZYPPER_EXIT_ERR_ZYPP;
      }
      catch ( const Exception & excpt_r ) {
        ZYPP_CAUGHT( excpt_r );
        ZYPP_RETHROW( excpt_r );
      }
    }
  }

  if (retv < 0)
    retv = ZYPPER_EXIT_OK;
  else if (was_installed)
  {
    if (retv == ZYPPER_EXIT_INF_REBOOT_NEEDED)
    {
      if (gSettings.machine_readable)
        cout << "<message type=\"warning\">" << _("One of installed patches requires reboot of"
            " your machine. Please, do it as soon as possible.") << "</message>" << endl;
      else
        cout << _("WARNING: One of installed patches requires a reboot of"
            " your machine. Please do it as soon as possible.") << endl;
    }
    else if (retv == ZYPPER_EXIT_INF_RESTART_NEEDED)
    {
      if (!gSettings.machine_readable)
        cout << _("WARNING: One of installed patches affects the package"
            " manager itself, thus it requires its restart before executing"
            " any further operations.") << endl;
    }
  }

  return retv;
}

// TODO confirm licenses
// - make this more user-friendly e.g. show only license name and
//  ask for [y/n/r] with 'r' for read the license text
//  (opened throu more or less, etc...)
// - after negative answer, call solve_and_commit() again
bool confirm_licenses()
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
	            % it->resolvable()->name()
	            % kind_to_string_localized(it->resolvable()->kind(),1)
	            << endl;

        MIL << format("Automatically agreeing with %s %s license.")
            % it->resolvable()->name() % it->resolvable()->kind().asString()
            << endl;

        continue;
      }

      cout << format(_("%s %s license:")) % it->resolvable()->name()
                % kind_to_string_localized(it->resolvable()->kind(), 1)
        << it->resolvable()->licenseToConfirm() << endl;

      string question = _("In order to install this package, you must agree"
        " to terms of the above license. Continue?");

      if (!read_bool_answer(question, gSettings.license_auto_agree))
      {
        confirmed = false;

        if (gSettings.non_interactive)
        {
          //! \todo do this with _PL()
          cout << endl <<
             _("Aborting installation due to the need for"
              " license(s) confirmation.") << " ";
          // TranslatorExplanation Don't translate the '--auto-agree-with-licenses',
          // it is a command line option
          cout << _("Please, restart the operation in interactive"
              " mode and confirm your agreement with required license(s),"
              " or use the --auto-agree-with-licenses option.")
            << endl;
          MIL << "License(s) NOT confirmed (non-interactive without auto confirmation)" << endl;
        }
        else
        {
          cout << endl;
            // TranslatorExplanation e.g. "... with flash package license."
          cout << format(
              _("Aborting installation due to user disagreement with %s %s license."))
                % it->resolvable()->name()
                % kind_to_string_localized(it->resolvable()->kind(), 1)
              << endl;
            MIL << "License(s) NOT confirmed (interactive)" << endl;
        }

        break;
      }
    }
  }

  return confirmed;
}


struct FindSrcPackage
{
  FindSrcPackage(SrcPackage::Ptr & srcpkg) : _srcpkg(srcpkg) {}

  bool operator() (ResObject::Ptr res)
  {
    SrcPackage::Ptr srcpkg = asKind<SrcPackage>(res);
    cout_vv << "Considering srcpakcage " << srcpkg->name() << "-" << srcpkg->edition() << ": ";
    if (_srcpkg)
    {
      if (_srcpkg->edition() < srcpkg->edition())
        cout_vv << "newer edition (" << srcpkg->edition() << " > " << _srcpkg->edition() << ")" << endl;
      else
        cout_vv << "is older than the current candidate";
    }
    else
      cout_vv << "first candindate";

    cout_vv << endl;

    _srcpkg.swap(srcpkg);

    return true;
  }

  SrcPackage::Ptr & _srcpkg;
};


int source_install(std::vector<std::string> & arguments)
{
  /*
   * Workflow:
   *
   * 1. load repo resolvables (to gData.repo_resolvables)
   * 2. interate all SrcPackage resolvables with specified name
   * 3. find the latest version or version satisfying specification.
   * 4. install the source package with ZYpp->installSrcPackage(SrcPackage::constPtr);
   */

  int ret = ZYPPER_EXIT_OK;

  for (vector<string>::const_iterator it = arguments.begin();
       it != arguments.end(); ++it)
  {
    SrcPackage::Ptr srcpkg;

    gData.repo_resolvables.forEach(
      functor::chain(
        resfilter::ByName(*it),
        resfilter::ByKind(ResTraits<SrcPackage>::kind)),
        FindSrcPackage(srcpkg));

    if (srcpkg)
    {
      cout << format(_("Installing source package %s-%s"))
          % srcpkg->name() % srcpkg->edition() << endl;
      MIL << "Going to install srcpackage: " << srcpkg << endl;

      try
      {
        God->installSrcPackage(srcpkg);

        cout << format(_("Source package %s-%s successfully installed."))
            % srcpkg->name() % srcpkg->edition() << endl;
      }
      catch (const Exception & ex)
      {
        ZYPP_CAUGHT(ex);
        cerr << format(_("Problem installing source package %s-%s:"))
            % srcpkg->name() % srcpkg->edition() << endl;
        cerr << ex.asUserString() << endl;

        ret = ZYPPER_EXIT_ERR_ZYPP;
      }
    }
    else
      cerr << format(_("Source package '%s' not found.")) % (*it) << endl;
  }

  return ret;
}

// Local Variables:
// c-basic-offset: 2
// End:
