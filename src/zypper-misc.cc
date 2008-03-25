#include <fstream>
#include <sstream>
#include <ctype.h>
#include <boost/format.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp>

#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"

#include "zypp/Edition.h"
#include "zypp/Patch.h"
#include "zypp/Package.h"
#include "zypp/SrcPackage.h"
#include "zypp/base/Algorithm.h"
#include "zypp/solver/detail/Helper.h"
#include "zypp/media/MediaException.h"
#include "zypp/FileChecker.h"

#include "zypp/RepoInfo.h"

#include "zypp/Capabilities.h"

#include "zypper.h"
#include "zypper-main.h"
#include "zypper-utils.h"
#include "zypper-getopt.h"
#include "zypper-misc.h"
#include "zypper-prompt.h"
#include "output/prompt.h"

using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;
extern RuntimeData gData;


/**
 * Loops through resolvables, checking if there is license to confirm. When
 * run interactively, it displays a dialog, otherwise it answers automatically
 * according to --auto-agree-with-licenses present or not present.
 * 
 * \returns true if all licenses have been confirmed, false otherwise.  
 */
static bool confirm_licenses(Zypper & zypper);


// converts a user-supplied kind to a zypp kind object
// returns an empty one if not recognized
ResObject::Kind string_to_kind (const string &skind)
{
  ResObject::Kind empty;
  string lskind = str::toLower (skind);
  if (lskind == "package")
    return ResTraits<Package>::kind;
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
  if (lskind == "atom")
    return ResTraits<Atom>::kind;
//   if (lskind == "system")
//     return ResTraits<SystemResObject>::kind;
  if (lskind == "srcpackage")
    return ResTraits<SrcPackage>::kind;
  // not recognized
  return empty;
}

// copied from yast2-pkg-bindings:PkgModuleFunctions::DoProvideNameKind
bool ProvideProcess::operator()( const PoolItem& provider )
{
  DBG << "Considering " << provider << endl;
  // 1. compatible arch
  // 2. best arch
  // 3. best edition

  // check the version if it's specified
  if (!version.empty() && version != provider->edition().asString()) {
    DBG << format ("Skipping version %s (requested: %s)")
      % provider->edition().asString() % version << endl;
    return true;
  }

  if (!provider.status().isInstalled()) {
    // deselect the item if it's already selected,
    // only one item should be selected
    if (provider.status().isToBeInstalled()) {
      DBG << "  Deselecting" << endl;
      provider.status().resetTransact(whoWantsIt);
    }

    // regarding items which are installable only
    if (!provider->arch().compatibleWith( _architecture )) {
      DBG << format ("provider %s has incompatible arch '%s'")
        % provider->name() % provider->arch().asString() << endl;
    }
    else if (!item) {
      DBG << "  First match" << endl;
      item = provider;
    }
    else if (item->arch().compare( provider->arch() ) < 0) {
      DBG << "  Better arch" << endl;
      item = provider;
    }
    else if (item->edition().compare( provider->edition() ) < 0) {
      DBG << "  Better edition" << endl;
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
static Capability safe_parse_cap (Zypper & zypper,
                           const ResObject::Kind &kind, const string & capstr)
{
  Capability cap;
  try {
    // expect named caps as NAME[OP<EDITION>]
    // transform to NAME[ OP <EDITION>] (add spaces)
    string new_capstr = capstr;
    DBG << "capstr: " << capstr << endl;
    string::size_type op_pos = capstr.find_first_of("<>=");
    if (op_pos != string::npos)
    {
      new_capstr.insert(op_pos, " ");
      DBG << "new capstr: " << new_capstr << endl;
      op_pos = new_capstr.find_first_not_of("<>=", op_pos + 1);
      if (op_pos != string::npos && new_capstr.size() > op_pos)
      {
        new_capstr.insert(op_pos, " ");
        DBG << "new capstr: " << new_capstr << endl;
      }
    }
    // if we are about to install stuff and
    // if this is not a candidate for a versioned capability, take it like
    // a package name and check if it is already installed
    else if (zypper.command() == ZypperCommand::INSTALL)
    {
      using namespace zypp::functor;
      using namespace zypp::resfilter;

      // get the installed version
      VersionGetter vg;
      invokeOnEach(
          God->pool().byIdentBegin(kind, capstr),
          God->pool().byIdentEnd(kind,capstr),
          ByInstalled(),
          functorRef<bool,const zypp::PoolItem&> (vg));
      // installed found
      if (vg.found)
      {
        // check for newer version of that resolvable
        NewerVersionGetter nvg(vg.edition);
        invokeOnEach(
             God->pool().byIdentBegin(kind, capstr),
             God->pool().byIdentEnd(kind,capstr),
             not_c(ByInstalled()),
             functorRef<bool,const zypp::PoolItem&> (nvg));
        // newer version found
        if (nvg.found)
        {
          DBG << "installed resolvable named " << capstr
            << " found, changing capability to " << new_capstr << endl;
          new_capstr = capstr + " > " + vg.edition.asString();
        }
      }
    }
    cap = Capability( new_capstr.c_str(), kind );
  }
  catch (const Exception& e) {
    //! \todo check this handling (should we fail or set a special exit code?)
    ZYPP_CAUGHT(e);
    zypper.out().error(boost::str(
      format(_("Cannot parse capability '%s'.")) % capstr));
  }
  return cap;
}

// this does only resolvables with this _name_.
// we could also act on _provides_
// TODO edition, arch
static void mark_for_install(Zypper & zypper,
                      const ResObject::Kind &kind,
		      const std::string &name)
{
  // name and kind match:
  ProvideProcess installer (ZConfig::instance().systemArchitecture(), "" /*version*/);
  DBG << "Iterating over [" << kind << "]" << name << endl;
  invokeOnEach(
      God->pool().byIdentBegin(kind, name),
      God->pool().byIdentEnd(kind, name),
      zypp::functor::functorRef<bool,const zypp::PoolItem&> (installer));

  DBG << "... done" << endl;
  if (!installer.item) {
    // TranslatorExplanation e.g. "package 'pornview' not found"
    zypper.out().warning(boost::str(
      format(_("%s '%s' not found")) % kind_to_string_localized(kind,1) % name));
    WAR << format("%s '%s' not found") % kind % name << endl;
    zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
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

    zypper.out().info(boost::str(format(
      _("skipping %s '%s' (the newest version already installed)"))
      % kind_to_string_localized(kind,1) % name));
  }
  else {

    //! \todo don't use setToBeInstalled for this purpose but higher level solver API
    bool result = installer.item.status().setToBeInstalled( zypp::ResStatus::USER );
    if (!result)
    {
      // this is because the resolvable is installed and we are forcing.
      installer.item.status().setTransact( true, zypp::ResStatus::USER );
      if (!copts.count("force"))
      {
        zypper.out().error(boost::str(
          format(_("Failed to add '%s' to the list of packages to be installed."))
          % name));
        ERR << "Could not set " << name << " as to-be-installed" << endl;
      }
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
    DBG << "Marking for deletion: " << provider << endl;
    bool result = provider.status().setToBeUninstalled( zypp::ResStatus::USER );
    if (!result) {
      Zypper::instance()->out().error(boost::str(format(
          _("Failed to add '%s' to the list of packages to be removed."))
          % provider.resolvable()->name()));
      ERR << "Could not set " << provider.resolvable()->name()
          << " as to-be-uninstalled" << endl;
    }
    return true;		// get all of them
  }
};

// mark all matches
static void mark_for_uninstall(Zypper & zypper,
                        const ResObject::Kind &kind,
			const std::string &name)
{
  const ResPool &pool = God->pool();
  // name and kind match:

  DeleteProcess deleter;
  DBG << "Iterating over " << name << endl;
  invokeOnEach( pool.byIdentBegin( kind, name ),
		pool.byIdentEnd( kind, name ),
		resfilter::ByInstalled(),
		zypp::functor::functorRef<bool,const zypp::PoolItem&> (deleter)
		);
  DBG << "... done" << endl;
  if (!deleter.found) {
    // TranslatorExplanation e.g. "package 'pornview' not found"
    zypper.out().error(boost::str(
      format(_("%s '%s' not found")) % kind_to_string_localized(kind,1) % name));
    zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    return;
  }
}

void mark_by_name (Zypper & zypper,
                   bool install_not_remove,
		   const ResObject::Kind &kind,
		   const string &name )
{
  if (install_not_remove)
    mark_for_install(zypper, kind, name);
  else
    mark_for_uninstall(zypper, kind, name);
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

void mark_by_capability (Zypper & zypper,
                         bool install_not_remove,
			 const ResObject::Kind &kind,
			 const string &capstr )
{
  Capability cap = safe_parse_cap (zypper, kind, capstr);

  if (!cap.empty()) {
    DBG << "Capability: " << cap << endl;

    Resolver_Ptr resolver = zypp::getZYpp()->resolver();
    if (install_not_remove) {
      DBG << "Adding requirement " << cap << endl;
      resolver->addRequire (cap);
    }
    else {
      DBG << "Adding conflict " << cap << endl;
      resolver->addConflict (cap);
    }
  }
}

// ----------------------------------------------------------------------------

void remove_selections(Zypper & zypper)
{
  MIL << "Removing user selections from the solver pool" << endl;

  DBG << "Removing user setToBeInstalled()/Removed()" << endl;

  // iterate pool, searching for ResStatus::isByUser()
  // TODO optimize: remember user selections and iterate by name
  // TODO optimize: it seems this is actually needed only if the selection was
  //      not committed (user has chosen not to continue)
  const ResPool & pool = God->pool();

  for (ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it)
    if (it->status().isByUser())
    {
      DBG << "Removing user setToBeInstalled()/Removed()" << endl;
      it->status().resetTransact(zypp::ResStatus::USER);
    }

  DBG << "Removing user addRequire() addConflict()" << endl;

  Resolver_Ptr solver = God->resolver();
  // FIXME port this
//   CapSet capSet = solver->getConflict();
//   for (CapSet::const_iterator it = capSet.begin(); it != capSet.end(); ++it)
//   {
//     DBG << "removing conflict: " << (*it) << endl;
//     solver->removeConflict(*it);
//   }
//   capSet = solver->getRequire();
//   for (CapSet::const_iterator it = capSet.begin(); it != capSet.end(); ++it)
//   {
//     DBG << "removing require: " << (*it) << endl;
//     solver->removeRequire(*it);
//   }

  MIL << "DONE" << endl;
}

// ----------------------------------------------------------------------------

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
static tribool show_problem (Zypper & zypper,
                      const ResolverProblem & prob,
                      ProblemSolutionList & todo)
{
  ostringstream stm;
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
      stm << indent(det, 2) << endl;
  }

  if (zypper.globalOpts().non_interactive)
    return false;

  unsigned int problem_count = God->resolver()->problems().size();

  int reply;
  do {
    // without solutions, its useless to prompt
    if (solutions.empty())
    {
      zypper.out().error(stm.str());
      return false;
    }

    if (!zypper.globalOpts().machine_readable)
    {
      if (problem_count > 1)
        stm << _PL(
          "Choose the above solution using '1' or skip, retry or cancel",
          "Choose from above solutions by number or skip, retry or cancel",
          solutions.size());
      else
        // translators: translate 'c' to whatever you translated the 'c' in
        // "#/c" and "#/s/r/c" strings
        stm << _PL(
          "Choose the above solution using '1' or cancel using 'c'",
          "Choose from above solutions by number or cancel",
          solutions.size());
    }

    PromptOptions popts;
    if (problem_count > 1)
    {
      // translators: answers for dependency problem solution input prompt:
      // "Choose from above solutions by number or skip, retry or cancel"
      // Translate the letters to whatever is suitable for your language.
      // The anserws must be separated by slash characters '/' and must
      // correspond to number/skip/retry/cancel in that order.
      // The answers should be lower case letters.
      popts.setOptions(_("#/s/r/c"), 3);
    }
    else
    {
      // translators: answers for dependency problem solution input prompt:
      // "Choose from above solutions by number or cancel"
      // Translate the letter 'c' to whatever is suitable for your language
      // and the same as you translated it in the "#/s/r/c" string
      // See the "#/s/r/c" comment for other details
      popts.setOptions(_("#/c"), 1);
    }

    zypper.out().prompt(PROMPT_DEP_RESOLVE, stm.str(), popts);
    //string reply = get_prompt_reply(promptstr, popts); \TODO

    string reply_s = str::getline (cin, zypp::str::TRIM);

    if (! cin.good()) {
      zypper.out().error("cin: " + cin.rdstate());
      return false;
    }
    if (isupper( reply_s[0] ))
      reply_s[0] = tolower( reply_s[0] );

    // translators: corresponds to (r)etry
    if (problem_count > 1 && reply_s == _("r"))
      return true;
    // translators: corresponds to (c)ancel
    else if (reply_s == _("c") || reply_s.empty())
      return false;
    // translators: corresponds to (s)kip
    else if (problem_count > 1 && reply_s == _("s"))
      return indeterminate; // continue with next problem

    str::strtonum (reply_s, reply);
  } while (reply <= 0 || reply >= n);

  zypper.out().info(boost::str(format (_("Applying solution %s")) % reply), Out::HIGH);
  ProblemSolutionList::iterator reply_i = solutions.begin ();
  advance (reply_i, reply - 1);
  todo.push_back (*reply_i);

  tribool go_on = indeterminate; // continue with next problem
  return go_on;
}

// return true to retry solving, false to cancel transaction
static bool show_problems(Zypper & zypper)
{
  bool retry = true;
  Resolver_Ptr resolver = zypp::getZYpp()->resolver();
  ResolverProblemList rproblems = resolver->problems ();
  ResolverProblemList::iterator
    b = rproblems.begin (),
    e = rproblems.end (),
    i;
  ProblemSolutionList todo;

  // display the number of problems
  if (rproblems.size() > 1)
    zypper.out().info(boost::str(format(
      _PL("%d Problem:", "%d Problems:", rproblems.size())) % rproblems.size()));
  else if (rproblems.empty())
  {
    // should not happen! If solve() failed at least one problem must be set!
    zypper.out().error(_("Specified capability not found"));
    zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    return false;
  }

  // for many problems, list them shortly first
  //! \todo handle resolver problems caused by --capability mode arguments specially to give proper output (bnc #337007)
  if (rproblems.size() > 1)
  {
    for (i = b; i != e; ++i)
      zypper.out().info(boost::str(
        format(_("Problem: %s")) % (*i)->description()));
  }
  // now list all problems with solution proposals
  for (i = b; i != e; ++i)
  {
    zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL); // visual separator
    tribool stopnow = show_problem(zypper, *(*i), todo);
    if (! indeterminate (stopnow)) {
      retry = stopnow == true;
      break;
    }
  }

  if (retry)
  {
    zypper.out().info(_("Resolving dependencies..."));
    resolver->applySolutions (todo);
  }
  return retry;
}

typedef map<Resolvable::Kind,set<ResObject::constPtr> > KindToResObjectSet;

static void show_summary_resolvable_list(const string & label,
                                         KindToResObjectSet::const_iterator it,
                                         Out & out)
{
  ostringstream s;
  s << endl << label << endl;

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
//! \todo make function to wrap & indent the text 
  for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
      resit != it->second.end(); ++resit)
  {
    ResObject::constPtr res(*resit);

    if (out.verbosity() == Out::NORMAL)
    {
      // watch the terminal widht
      if (cols_written == 0)
        s << INDENT;
      else if (cols_written + res->name().size() + 1  > cols)
      {
        s << endl;
        cols_written = 0;
      }

      cols_written += res->name().size();
    }
    else
      s << INDENT;

    // resolvable name
    s << res->name() << (out.verbosity() > Out::NORMAL ? "" : " ");
    // plus edition and architecture for verbose output
    //cout_v << "-" << res->edition() << "." << res->arch();
    // plus repo providing this package
    //if (!res->repoInfo().alias().empty())
      //cout_v << "  (" << res->repoInfo().name() << ")";
    // new line after each package in the verbose mode
    //cout_v << endl;
  }

  if (out.verbosity() == Out::NORMAL)
    s << endl;

  out.info(s.str(), Out::QUIET); //! \todo special output needed for this
}

typedef enum
{
  TO_UPGRADE,
  TO_DOWNGRADE,
  TO_INSTALL,
  TO_REINSTALL,
  TO_REMOVE
} SummaryType;

static void xml_print_to_transact_tag(SummaryType stype, bool end = false)
{
  switch (stype)
  {
  case TO_UPGRADE:
    cout << "<" << (end ? "/" : "") << "to-upgrade>" << endl;
    break;
  case TO_DOWNGRADE:
    cout << "<" << (end ? "/" : "") << "to-downgrade>" << endl;
    break;
  case TO_INSTALL:
    cout << "<" << (end ? "/" : "") << "to-install>" << endl;
    break;
  case TO_REINSTALL:
    cout << "<" << (end ? "/" : "") << "to-reinstall>" << endl;
    break;
  case TO_REMOVE:
    cout << "<" << (end ? "/" : "") << "to-remove>" << endl;
    break;
  }
}

static void show_summary_of_type(Zypper & zypper,
                                 SummaryType stype,
                                 const KindToResObjectSet & summset)
{
  // xml install summary
  if (zypper.out().type() == Out::TYPE_XML)
  {
    bool empty = true;
    for (KindToResObjectSet::const_iterator it = summset.begin();
        it != summset.end(); ++it)
      if (!it->second.empty()) { empty = false; break; }
    if (empty)
      return;

    xml_print_to_transact_tag(stype);

    for (KindToResObjectSet::const_iterator it = summset.begin();
        it != summset.end(); ++it)
      for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
          resit != it->second.end(); ++resit)
      {
        ResObject::constPtr res(*resit);

        cout << "<solvable";
        cout << " type=\"" << it->first << "\"";
        cout << " name=\"" << res->name() << "\"";
        cout << " edition=\"" << res->edition() << "\"";
        //! \todo cout << " edition-old=\"" << << "\""; 
        cout << " arch=\"" << res->arch() << "\"";
        if (!res->summary().empty())
          cout << " summary=\"" << xml_encode(res->summary()) << "\"";
        if (!res->description().empty())
          cout << ">" << endl << xml_encode(res->description()) << "</solvable>" << endl;
        else
          cout << "/>" << endl;
      }

    xml_print_to_transact_tag(stype, true);

    return;
  }

  // normal install summary
  for (KindToResObjectSet::const_iterator it = summset.begin();
      it != summset.end(); ++it)
  {
    string title;
    switch (stype)
    {
    case TO_UPGRADE:
      title = boost::str(format(_PL(
          // TranslatorExplanation %s is a "package", "patch", "pattern", etc
          "The following %s is going to be upgraded:",
          "The following %s are going to be upgraded:",
          it->second.size()
      )) % kind_to_string_localized(it->first, it->second.size()));
      break;
    case TO_DOWNGRADE:
      title = boost::str(format(_PL(
          // TranslatorExplanation %s is a "package", "patch", "pattern", etc
          "The following %s is going to be downgraded:",
          // TranslatorExplanation %s is a "packages", "patches", "patterns", etc
          "The following %s are going to be downgraded:",
          it->second.size()
      )) % kind_to_string_localized(it->first, it->second.size()));
      break;
    case TO_INSTALL:
      title = boost::str(format(_PL(
          // TranslatorExplanation %s is a "package", "patch", "pattern", etc
          "The following NEW %s is going to be installed:",
          // TranslatorExplanation %s is a "packages", "patches", "patterns", etc
          "The following NEW %s are going to be installed:",
          it->second.size()
      )) % kind_to_string_localized(it->first, it->second.size()));
      break;
    case TO_REINSTALL:
      title = boost::str(format(_PL(
          // TranslatorExplanation %s is a "package", "patch", "pattern", etc
          "The following %s is going to be re-installed:",
          // TranslatorExplanation %s is a "packages", "patches", "patterns", etc
          "The following %s are going to be re-installed:",
          it->second.size()
      )) % kind_to_string_localized(it->first, it->second.size()));
      break;
    case TO_REMOVE:
      title = boost::str(format(_PL(
          // TranslatorExplanation %s is a "package", "patch", "pattern", etc
          "The following %s is going to be REMOVED:",
          // TranslatorExplanation %s is a "packages", "patches", "patterns", etc
          "The following %s are going to be REMOVED:",
          it->second.size()
      )) % kind_to_string_localized(it->first, it->second.size()));
      break;
    }

    show_summary_resolvable_list(title, it, zypper.out());
  }
}

/**
 * @return (-1) - nothing to do,
 *  0 - there is at least 1 resolvable to be installed/uninstalled,
 *  ZYPPER_EXIT_INF_REBOOT_NEEDED - if one of patches to be installed needs machine reboot,
 *  ZYPPER_EXIT_INF_RESTART_NEEDED - if one of patches to be installed needs package manager restart
 */
int summary(Zypper & zypper)
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

  if (retv == -1 && zypper.runtimeData().srcpkgs_to_install.empty())
  {
    zypper.out().info(_("Nothing to do."));
    return retv;
  }

  KindToResObjectSet toinstall;
  KindToResObjectSet toupgrade;
  KindToResObjectSet todowngrade;
  KindToResObjectSet toreinstall;
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
          else if (res->edition() == (*rmit)->edition())
            toreinstall[res->kind()].insert(res);
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

  for (list<SrcPackage::constPtr>::const_iterator it = zypper.runtimeData().srcpkgs_to_install.begin();
      it != zypper.runtimeData().srcpkgs_to_install.end(); ++it)
    toinstall[ResTraits<SrcPackage>::kind].insert(*it);

  // "</install-summary>"
  if (zypper.out().type() == Out::TYPE_XML)
  {
    cout << "<install-summary";
    cout << " download-size=\"" << ((ByteCount::SizeType) download_size) << "\"";
    cout << " space-usage-diff=\"" << ((ByteCount::SizeType) new_installed_size) << "\"";
    cout << ">" << endl;
  }

  // show summary
  show_summary_of_type(zypper, TO_UPGRADE, toupgrade);
  show_summary_of_type(zypper, TO_DOWNGRADE, todowngrade);
  show_summary_of_type(zypper, TO_INSTALL, toinstall);
  show_summary_of_type(zypper, TO_REINSTALL, toreinstall);
  show_summary_of_type(zypper, TO_REMOVE, toremove);

  // "</install-summary>"
  if (zypper.out().type() == Out::TYPE_XML)
    cout << "</install-summary>" << endl;

  zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL); // visual separator

  // count and download size info
  ostringstream s;
  if (download_size > 0)
  {
    s << format(_("Overall download size: %s.")) % download_size;
    s << " ";
  }
  if (new_installed_size > 0)
    // TrasnlatorExplanation %s will be substituted by a byte count e.g. 212 K
    s << format(_("After the operation, additional %s will be used."))
        % new_installed_size.asString(0,1,1);
  else if (new_installed_size == 0)
    s << _("No additional space will be used or freed after the operation.");
  else
  {
    // get the absolute size
    ByteCount abs;
    abs = (-new_installed_size);
    // TrasnlatorExplanation %s will be substituted by a byte count e.g. 212 K
    s << format(_("After the operation, %s will be freed."))
        % abs.asString(0,1,1);
  }
  zypper.out().info(s.str());

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

static void dump_pool ()
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

bool resolve(Zypper & zypper)
{
  int locks = God->applyLocks();
  zypper.out().info(
    boost::str(format(_PL("%s item locked", "%s items locked", locks)) % locks),
    Out::HIGH);

  dump_pool();

  // --force-resolution command line parameter value
  tribool force_resolution = indeterminate;
  vector<string>::size_type count = copts.count("force-resolution");
  if (count)
  {
    string value = copts["force-resolution"].front();
    if (value == "on" || value == "true" || value == "1" || value == "yes")
      force_resolution = true;
    else if (value == "off" || value == "false" || value == "0" || value == "no")
      force_resolution = false;
    else
    {
      zypper.out().error(
        boost::str(format(_("Invalid value '%s' of the %s parameter"))
          % value % "force-resolution"),
        boost::str(format(_("Valid values are '%s' and '%s'")) % "on" % "off"));
    }

    if (count > 1)
      zypper.out().warning(boost::str(format(
        _("Considering only the first value of the %s parameter, ignoring the rest"))
          % "force-resolution"));
  }

  // if --force-resolution was not specified on the command line, force
  // the resolution by default, don't force it only in non-interactive mode
  // and not rug_compatible mode
  if (indeterminate(force_resolution))
  {
    if (zypper.globalOpts().non_interactive &&
        !zypper.globalOpts().is_rug_compatible)
      force_resolution = false;
    else
      force_resolution = true;
  }

  DBG << "force resolution: " << force_resolution << endl;
  ostringstream s;
  s << _("Force resolution:") << " " << (force_resolution ? _("Yes") : _("No"));
  zypper.out().info(s.str(), Out::HIGH);
  God->resolver()->setForceResolve( force_resolution );

  zypper.out().info(_("Resolving dependencies..."), Out::HIGH);
  DBG << "Calling the solver..." << endl;
  return God->resolver()->resolvePool();
}

void patch_check ()
{
  Out & out = Zypper::instance()->out();

  DBG << "patch check" << endl;
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

  ostringstream s;
  // translators: %d is the number of needed patches
  s << format(_PL("%d patch needed", "%d patches needed", gData.patches_count))
      % gData.patches_count
    << " ("
    // translators: %d is the number of needed patches
    << format(_PL("%d security patch", "%d security patches", gData.security_patches_count))
      % gData.security_patches_count
    << ")";
  out.info(s.str(), Out::QUIET);
}

static string string_status (const ResStatus& rs)
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

// patches
void show_patches(Zypper & zypper)
{
  MIL << "Pool contains " << God->pool().size() << " items. Checking whether available patches are needed." << std::endl;

  Table tbl;
  TableHeader th;
  th << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "))
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
    tr << patch->repoInfo().name();
    tr << res->name () << res->edition ().asString();
    tr << patch->category();
    tr << string_status (it->status ());
    tbl << tr;
  }
  tbl.sort (1);			// Name

  if (tbl.empty())
    zypper.out().info(_("No needed patches found."));
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
        cout << "name=\"" << res->name () << "\" ";
        cout << "edition=\""  << res->edition ().asString() << "\" ";
        cout << "category=\"" <<  patch->category() << "\" ";
        cout << "pkgmanager=\"" << (patch->affects_pkg_manager() ? "true" : "false") << "\" ";
        cout << "restart=\"" << (patch->reboot_needed() ? "true" : "false") << "\" ";
        cout << "interactive=\"" << (patch->interactive() ? "true" : "false") << "\" ";
        cout << "kind=\"patch\"";
        cout << ">" << endl;
        cout << "  <summary>" << xml_encode(patch->summary()) << "  </summary>" << endl;
        cout << "  <description>" << xml_encode(patch->description()) << "</description>" << endl;
        cout << "  <license>" << xml_encode(patch->licenseToConfirm()) << "</license>" << endl;

        if ( !patch->repoInfo().alias().empty() )
        {
          cout << "  <source url=\"" << xml_encode(patch->repoInfo().baseUrlsBegin()->asString());
          cout << "\" alias=\"" << xml_encode(patch->repoInfo().alias()) << "\"/>" << endl;
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

static void list_patch_updates(Zypper & zypper, bool best_effort)
{
  Table tbl;
  Table pm_tbl;	// only those that affect packagemanager: they have priority
  TableHeader th;
  unsigned cols;

  th << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "))
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
    if ( it->status().isNeeded() )
    {
      Patch::constPtr patch = asKind<Patch>(res);

      {
	TableRow tr (cols);
	tr << patch->repoInfo().name();
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
  if (!pm_tbl.empty ())
  {
    if (!tbl.empty ())
      zypper.out().warning(
        _("These are only the updates affecting the updater itself.\n"
          "Other updates are available too.\n"));
    tbl = pm_tbl;
  }

  tbl.sort (1); 		// Name

  if (tbl.empty())
    zypper.out().info(_("No updates found."));
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
  PoolItem best;

  bool operator()( PoolItem provider )
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
PoolItem
findArchUpdateItem( const ResPool & pool, PoolItem item )
{
  LookForArchUpdate info;

  invokeOnEach( pool.byIdentBegin( item->kind(), item->name() ),
                pool.byIdentEnd( item->kind(), item->name() ),
                // get uninstalled, equal kind and arch, better edition
                functor::chain (
                  functor::chain (
                    resfilter::ByUninstalled (),
                    resfilter::byArch<CompareByEQ<Arch> >( item->arch() ) ),
                  resfilter::byEdition<CompareByGT<Edition> >( item->edition() )),
                functor::functorRef<bool,PoolItem> (info) );

  _XDEBUG("findArchUpdateItem(" << item << ") => " << info.best);
  return info.best;
}

// ----------------------------------------------------------------------------

typedef set<PoolItem> Candidates;

/**
 * Find all available updates of given kind.
 */
static void
find_updates( const ResKind & kind, Candidates & candidates )
{
  const zypp::ResPool& pool = God->pool();
  ResPool::byKind_iterator
    it = pool.byKindBegin (kind),
    e  = pool.byKindEnd (kind);
  DBG << "Finding update candidates of kind " << kind << endl;
  for (; it != e; ++it)
  {
    if (it->status().isUninstalled())
      continue;
    // (actually similar to ProvideProcess?)
    PoolItem candidate = findArchUpdateItem( pool, *it );
    if (!candidate.resolvable())
      continue;

    DBG << "item " << *it << endl;
    DBG << "cand " << candidate << endl;
    candidates.insert (candidate);
  }
}

/**
 * Find all available updates of given kinds.
 */
static void
find_updates( const ResKindSet & kinds, Candidates & candidates )
{
  for (ResKindSet::const_iterator kit = kinds.begin(); kit != kinds.end(); ++kit)
    find_updates(*kit, candidates);

  if (kinds.empty())
    WAR << "called with empty kinds set" << endl;
}

string i18n_kind_updates(const ResKind & kind)
{
  if (kind == ResTraits<Package>::kind)
    return _("Package updates");
  else if (kind == ResTraits<Patch>::kind)
    return _("Patches");
  else if (kind == ResTraits<Pattern>::kind)
    return _("Pattern updates");
  else if (kind == ResTraits<Product>::kind)
    return _("Product updates");

  return boost::str(format("%s updates") % kind);
}

// ----------------------------------------------------------------------------

void list_updates(Zypper & zypper, const ResKindSet & kinds, bool best_effort)
{
  if (zypper.out().type() == Out::TYPE_XML)
  {
    cout << "<update-status version=\"0.6\">" << endl;
    cout << "<update-list>" << endl;
  }
  bool not_affects_pkgmgr = false;

  unsigned kind_size = kinds.size();
  ResKindSet localkinds = kinds;
  ResKindSet::iterator it;
  it = localkinds.find(ResTraits<Patch>::kind);
  if(it != localkinds.end())
  {
    if (zypper.out().type() == Out::TYPE_XML)
      not_affects_pkgmgr = !xml_list_patches();
    else
    {
      zypper.out().info(i18n_kind_updates(*it), Out::QUIET, Out::TYPE_NORMAL);
      zypper.out().info("", Out::QUIET, Out::TYPE_NORMAL); // visual separator
      list_patch_updates(zypper, best_effort );
    }
    localkinds.erase(it);
  }

  if (zypper.out().type() == Out::TYPE_XML)
  {
    if (not_affects_pkgmgr)
      xml_list_updates(localkinds);
    cout << "</update-list>" << endl;
    cout << "</update-status>" << endl;
    return;
  }

  for (it = localkinds.begin(); it != localkinds.end(); ++it)
  {
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
      th << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "));
    }
    if (zypper.globalOpts().is_rug_compatible) {
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
    find_updates( *it, candidates );

    Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
    for (ci = cb; ci != ce; ++ci) {
//      ResStatus& candstat = ci->status();
//      candstat.setToBeInstalled (ResStatus::USER);
      ResObject::constPtr res = ci->resolvable();
      TableRow tr (cols);
      tr << "v";
      if (!hide_repo) {
	tr << res->repoInfo().name();
      }
      if (zypper.globalOpts().is_rug_compatible)
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

    if (kind_size > 1)
    {
      zypper.out().info("", Out::QUIET, Out::TYPE_NORMAL); // visual separator
      zypper.out().info(i18n_kind_updates(*it), Out::QUIET, Out::TYPE_NORMAL);
      zypper.out().info("", Out::QUIET, Out::TYPE_NORMAL); // visual separator
    }

    if (tbl.empty())
      zypper.out().info(_("No updates found."));
    else
      cout << tbl;
  }
}

// may be useful as a functor
static bool mark_item_install (const PoolItem& pi) {
  bool result = pi.status().setToBeInstalled( zypp::ResStatus::USER );
  if (!result) {
    DBG << "Marking " << pi << "for installation failed" << endl;
  }
  return result;
}


// ----------------------------------------------------------------------------
// best-effort update


// find installed item matching passed one
//   use LookForArchUpdate as callback handler in order to cope with
//   multiple installed resolvables of the same name.
//   LookForArchUpdate will return the one with the highest edition.

static PoolItem
findInstalledItem( PoolItem item )
{
  const zypp::ResPool& pool = God->pool();
  LookForArchUpdate info;

  invokeOnEach( pool.byIdentBegin( item->kind(), item->name() ),
                pool.byIdentEnd( item->kind(), item->name() ),
                resfilter::ByInstalled (),
                functor::functorRef<bool,PoolItem> (info) );

  _XDEBUG("findInstalledItem(" << item << ") => " << info.best);
  return info.best;
}


// require update of installed item
//   The PoolItem passed to require_item_update() is the installed resolvable
//   to which an update candidate is guaranteed to exist.
//
// may be useful as a functor
static bool require_item_update (const PoolItem& pi) {
  Resolver_Ptr resolver = zypp::getZYpp()->resolver();

  PoolItem installed = findInstalledItem( pi );

  // require anything greater than the installed version
  try {
    Capability  cap( installed->name(), Rel::GT, installed->edition(), installed->kind() );
    resolver->addRequire( cap );
  }
  catch (const Exception& e) {
    ZYPP_CAUGHT(e);
    Zypper::instance()->out().error(boost::str(format(
      _("Cannot parse '%s < %s'")) % installed->name() % installed->edition()));
  }

  return true;
}

// ----------------------------------------------------------------------------

void xml_list_updates(const ResKindSet & kinds)
{
  Candidates candidates;
  find_updates (kinds, candidates);

  Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
  for (ci = cb; ci != ce; ++ci) {
    ResObject::constPtr res = ci->resolvable();

    cout << " <update ";
    cout << "name=\"" << res->name () << "\" " ;
    cout << "edition=\""  << res->edition ().asString() << "\" ";
    cout << "kind=\"" << res->kind() << "\" ";
    cout << ">" << endl;
    cout << "  <summary>" << xml_encode(res->summary()) << "  </summary>" << endl;
    cout << "  <description>" << xml_encode(res->description()) << "</description>" << endl;
    cout << "  <license>" << xml_encode(res->licenseToConfirm()) << "</license>" << endl;

    if ( !res->repoInfo().alias().empty() )
    {
    	cout << "  <source url=\"" << xml_encode(res->repoInfo().baseUrlsBegin()->asString());
    	cout << "\" alias=\"" << xml_encode(res->repoInfo().alias()) << "\"/>" << endl;
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
	      Zypper::instance()->out().warning(boost::str(format(
	          _("%s is interactive, skipped."))
	          % res));
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

//! \todo mechanism for updating the update stack before the rest.
void mark_updates(const ResKindSet & kinds, bool skip_interactive, bool best_effort )
{
//  unsigned kind_size = kinds.size();
  ResKindSet localkinds = kinds;
  ResKindSet::iterator it;
  it = localkinds.find(ResTraits<Patch>::kind);
  if(it != localkinds.end()) // patches wanted
    mark_patch_updates(skip_interactive);

  Candidates candidates;
  find_updates (localkinds, candidates);
  if (best_effort)
    invokeOnEach (candidates.begin(), candidates.end(), require_item_update);
  else
    invokeOnEach (candidates.begin(), candidates.end(), mark_item_install);
}

// ----------------------------------------------------------------------------

/**
 * @return ZYPPER_EXIT_OK - successful commit,
 *  ZYPPER_EXIT_ERR_ZYPP - if ZYppCommitResult contains resolvables with errors,
 *  ZYPPER_EXIT_INF_REBOOT_NEEDED - if one of patches to be installed needs machine reboot,
 *  ZYPPER_EXIT_INF_RESTART_NEEDED - if one of patches to be installed needs package manager restart
 */
void solve_and_commit (Zypper & zypper)
{
  MIL << "solving..." << endl;

  while (true) {
    bool success = resolve(zypper);
    if (success)
      break;

    success = show_problems(zypper);
    if (! success) {
      // TODO cancel transaction?
      zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP); // #242736
      return;
    }
  }


  // returns -1, 0, ZYPPER_EXIT_INF_REBOOT_NEEDED, or ZYPPER_EXIT_INF_RESTART_NEEDED
  int retv = summary(zypper);
  bool was_installed = false;
  if (retv >= 0 || !zypper.runtimeData().srcpkgs_to_install.empty())
  {
    // there are resolvables to install/uninstall
    if (read_bool_answer(PROMPT_YN_INST_REMOVE_CONTINUE, _("Continue?"), true))
    {
      if (!confirm_licenses(zypper)) return;
      if (retv >= 0)
      {
        try
        {
          gData.show_media_progress_hack = true;
  
          ostringstream s;
          s << _("committing"); MIL << "committing...";
  
          ZYppCommitResult result;
          if (copts.count("dry-run"))
          {
            s << " " << _("(dry run)") << endl; MIL << "(dry run)";
            zypper.out().info(s.str(), Out::HIGH);
  
            result = God->commit(ZYppCommitPolicy().dryRun(true));
          }
          else
          {
            zypper.out().info(s.str(), Out::HIGH);
  
            result = God->commit(
              ZYppCommitPolicy().syncPoolAfterCommit(zypper.runningShell()));
  
            was_installed = true;
          }
          
  
          MIL << endl << "DONE" << endl;
  
          gData.show_media_progress_hack = false;
          
          if (!result._errors.empty())
            retv = ZYPPER_EXIT_ERR_ZYPP;
  
          s.clear(); s << result;
          zypper.out().info(s.str(), Out::HIGH);
        }
        catch ( const media::MediaException & e ) {
          ZYPP_CAUGHT(e);
          zypper.out().error(e,
              _("Problem downloading the package file from the repository:"),
              _("Please see the above error message for a hint."));
          zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
          return;
        }
        catch ( const zypp::repo::RepoException & e ) {
          ZYPP_CAUGHT(e);
          zypper.out().error(e,
              _("Problem downloading the package file from the repository:"),
              _("Please see the above error message for a hint."));
          zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
          return;
        }
        catch ( const zypp::FileCheckException & e ) {
          ZYPP_CAUGHT(e);
          zypper.out().error(e,
              _("The package integrity check failed. This may be a problem"
              " with the repository or media. Try one of the following:\n"
              "\n"
              "- just retry previous command\n"
              "- refresh the repositories using 'zypper refresh'\n"
              "- use another installation medium (if e.g. damaged)\n"
              "- use another repository"));
          zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
          return;
        }
        catch ( const Exception & e ) {
          ZYPP_CAUGHT(e);
          zypper.out().error(e,
              _("Problem occured during or after installation or removal of packages:"),
              _("Please see the above error message for a hint."));
          zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
        }
      }
      // install any pending source packages
      if (!zypper.runtimeData().srcpkgs_to_install.empty())
        install_src_pkgs(zypper);
    }
  }

  if (retv < 0)
    retv = ZYPPER_EXIT_OK;
  else if (was_installed)
  {
    if (retv == ZYPPER_EXIT_INF_REBOOT_NEEDED)
      zypper.out().warning(
        _("One of installed patches requires reboot of"
          " your machine. Reboot as soon as possible."));
    else if (retv == ZYPPER_EXIT_INF_RESTART_NEEDED)
      zypper.out().warning(
        _("One of installed patches affects the package"
          " manager itself, thus it requires its restart before executing"
          " any further operations."),
        Out::NORMAL, Out::TYPE_NORMAL);
  }

  if (zypper.exitCode() == ZYPPER_EXIT_OK) // don't overwrite previously set exit code
    zypper.setExitCode(retv);
}

// TODO confirm licenses
// - make this more user-friendly e.g. show only license name and
//  ask for [y/n/r] with 'r' for read the license text
//  (opened throu more or less, etc...)
// - after negative answer, call solve_and_commit() again
static bool confirm_licenses(Zypper & zypper)
{
  bool confirmed = true;

  for (ResPool::const_iterator it = God->pool().begin(); it != God->pool().end(); ++it)
  {
    if (it->status().isToBeInstalled() &&
        !it->resolvable()->licenseToConfirm().empty())
    {
      if (zypper.cmdOpts().license_auto_agree)
      {
      	zypper.out().info(boost::str(
            // TranslatorExplanation The first %s is name of the resolvable, the second is its kind (e.g. 'zypper package')
      	    format(_("Automatically agreeing with %s %s license."))
            % it->resolvable()->name()
            % kind_to_string_localized(it->resolvable()->kind(),1)));

        MIL << format("Automatically agreeing with %s %s license.")
            % it->resolvable()->name() % it->resolvable()->kind().asString()
            << endl;

        continue;
      }

      // license text
      ostringstream s;
      s << format(_("%s %s license:")) % it->resolvable()->name()
          % kind_to_string_localized(it->resolvable()->kind(), 1)
        << it->resolvable()->licenseToConfirm();
      zypper.out().info(s.str(), Out::QUIET);

      // lincense prompt
      string question = _("In order to install this package, you must agree"
        " to terms of the above license. Continue?");
      if (!read_bool_answer(PROMPT_YN_LICENSE_AGREE, question, zypper.cmdOpts().license_auto_agree))
      {
        confirmed = false;

        if (zypper.globalOpts().non_interactive)
        {
          zypper.out().info(
            _("Aborting installation due to the need for license confirmation."),
            Out::QUIET);
          zypper.out().info(boost::str(format(
            // translators: %sanslate the '--auto-agree-with-licenses',
            // it is a command line option
            _("Please restart the operation in interactive"
              " mode and confirm your agreement with required licenses,"
              " or use the %s option.")) % "--auto-agree-with-licenses"),
            Out::QUIET);

          MIL << "License(s) NOT confirmed (non-interactive without auto confirmation)" << endl;
        }
        else
        {
          zypper.out().info(boost::str(format(
              // translators: e.g. "... with flash package license."
              _("Aborting installation due to user disagreement with %s %s license."))
                % it->resolvable()->name()
                % kind_to_string_localized(it->resolvable()->kind(), 1)),
              Out::QUIET);
            MIL << "License(s) NOT confirmed (interactive)" << endl;
        }

        break;
      }
    }
  }

  return confirmed;
}

static SrcPackage::constPtr source_find( const string & arg )
{
   /*
   * Workflow:
   *
   * 1. interate all SrcPackage resolvables with specified name
   * 2. find the latest version or version satisfying specification.
   */
    SrcPackage::constPtr srcpkg;

    ResPool pool(God->pool());
    DBG << "looking source for : " << arg << endl;
    for_( srcit, pool.byIdentBegin<SrcPackage>(arg), 
              pool.byIdentEnd<SrcPackage>(arg) )
    {
      DBG << *srcit << endl;
      if ( ! srcit->status().isInstalled() ) // this will be true for all of the srcpackages, won't it?
      {
        SrcPackage::constPtr _srcpkg = asKind<SrcPackage>(srcit->resolvable());

        DBG << "Considering srcpakcage " << _srcpkg->name() << "-" << _srcpkg->edition() << ": ";
        if (srcpkg)
        {
          if (_srcpkg->edition() > srcpkg->edition())
          {
            DBG << "newer edition (" << srcpkg->edition() << " > " << _srcpkg->edition() << ")";
            _srcpkg.swap(srcpkg);
          }
          else
            DBG << "is older than the current candidate";
        }
        else
        {
          DBG << "first candindate";
          _srcpkg.swap(srcpkg);
        }
        DBG << endl;
      }
    }

    return srcpkg;
}

void build_deps_install(Zypper & zypper)
{
  /*
   * Workflow:
   *
   * 1. find the latest version or version satisfying specification.
   * 2. install the source package with ZYpp->installSrcPackage(SrcPackage::constPtr);
   */

  for (vector<string>::const_iterator it = zypper.arguments().begin();
       it != zypper.arguments().end(); ++it)
  {
    SrcPackage::constPtr srcpkg = source_find(*it);

    if (srcpkg)
    {
      DBG << format("Injecting build requieres for source package %s-%s")
          % srcpkg->name() % srcpkg->edition() << endl;

      // add all src requires to pool DEPRECATED: srcpakcages will be in
      // the pool (together with their build-deps) like normal packages
      // so only require the srcpackage
      /*
      for_( itc, srcpkg->dep(Dep::REQUIRES).begin(), srcpkg->dep(Dep::REQUIRES).end() )
      {
        God->resolver()->addRequire(*itc);
        DBG << "added req: " << *itc << endl;
      }*/
      God->resolver()->addRequire(Capability(srcpkg->name(), Rel::EQ, srcpkg->edition(), ResTraits<SrcPackage>::kind));
      //installer.item.status().setToBeInstalled( zypp::ResStatus::USER );
    }
    else
    {
      zypper.out().error(boost::str(format(
          _("Source package '%s' not found.")) % (*it)));
      zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    }
  }
}


void find_src_pkgs(Zypper & zypper)
{
  /*
   * Workflow:
   *
   * 1. find the latest version or version satisfying specification.
   * 2. install the source package with ZYpp->installSrcPackage(SrcPackage::constPtr);
   */

  for (vector<string>::const_iterator it = zypper.arguments().begin();
       it != zypper.arguments().end(); ++it)
  {
    SrcPackage::constPtr srcpkg = source_find(*it);

    if (srcpkg)
      zypper.runtimeData().srcpkgs_to_install.push_back(srcpkg);
    else
      zypper.out().error(boost::str(format(
          _("Source package '%s' not found.")) % (*it)));
  }
}

void install_src_pkgs(Zypper & zypper)
{
  for_(it, zypper.runtimeData().srcpkgs_to_install.begin(), zypper.runtimeData().srcpkgs_to_install.end())
  {
    SrcPackage::constPtr srcpkg = *it;
    zypper.out().info(boost::str(format(
        _("Installing source package %s-%s"))
        % srcpkg->name() % srcpkg->edition()));
    MIL << "Going to install srcpackage: " << srcpkg << endl;
  
    try
    {
      God->installSrcPackage(srcpkg);
  
      zypper.out().info(boost::str(format(
          _("Source package %s-%s successfully installed."))
          % srcpkg->name() % srcpkg->edition()));
    }
    catch (const Exception & ex)
    {
      ZYPP_CAUGHT(ex);
      zypper.out().error(ex,
        boost::str(format(_("Problem installing source package %s-%s:"))
          % srcpkg->name() % srcpkg->edition()));
  
      zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    }
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
