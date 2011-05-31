#include <iostream> // for xml and table output
#include <sstream>
#include <boost/format.hpp>

#include "zypp/base/Logger.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Algorithm.h"
#include "zypp/PoolQuery.h"

#include "zypp/Patch.h"

#include "SolverRequester.h"
#include "Table.h"
#include "update.h"
#include "main.h"

using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;

typedef set<PoolItem> Candidates;

static void
find_updates( const ResKindSet & kinds, Candidates & candidates );

// ----------------------------------------------------------------------------
//
// Updates
//
// The following scenarios are handled distinctly:
// * -t patch (default), no arguments
// * -t package, no arguments
//   - uses Resolver::doUpdate()
// * -t {other}, no arguments
// * -t patch foo
// * -t package foo
//   - addRequires(>installed-version) if available
// * -t {other} foo
//   - addRequires(>installed-version) if available
//
// update summary must correspond to list-updates and patch-check
// ----------------------------------------------------------------------------

void patch_check ()
{
  Out & out = Zypper::instance()->out();
  RuntimeData & gData = Zypper::instance()->runtimeData();
  DBG << "patch check" << endl;
  gData.patches_count = gData.security_patches_count = 0;

  ResPool::byKind_iterator
    it = God->pool().byKindBegin(ResKind::patch),
    e = God->pool().byKindEnd(ResKind::patch);
  for (; it != e; ++it )
  {
    ResObject::constPtr res = it->resolvable();
    Patch::constPtr patch = asKind<Patch>(res);

    if (it->isRelevant() && !it->isSatisfied())
    {
      gData.patches_count++;
      if (patch->categoryEnum() == Patch::CAT_SECURITY)
        gData.security_patches_count++;
    }
  }

  ostringstream s;
  // translators: %d is the number of needed patches
  s << format(_PL("%d patch needed", "%d patches needed", gData.patches_count))
      % gData.patches_count
    << " ("
    // translators: %d is the number of security patches
    << format(_PL("%d security patch", "%d security patches", gData.security_patches_count))
      % gData.security_patches_count
    << ")";
  out.info(s.str(), Out::QUIET);
}

// ----------------------------------------------------------------------------
inline const char *const patchStatusAsString( const PoolItem & pi_r )
{
  switch ( pi_r.status().validate() )
  {
    case zypp::ResStatus::BROKEN:	return pi_r.isUnwanted() ? "unwanted"
								 : "needed";	break;
    case zypp::ResStatus::SATISFIED:	return "applied";	break;
    case zypp::ResStatus::NONRELEVANT:	return "not-needed";	break;

    case zypp::ResStatus::UNDETERMINED:	// fall through
    default:
      break;
  }
  return "undetermined";
}

// returns true if restartSuggested() patches are availble
static bool xml_list_patches (Zypper & zypper)
{
  const zypp::ResPool& pool = God->pool();

  unsigned int patchcount=0;
  bool pkg_mgr_available = false;
  Patch::constPtr patch;

  ResPool::byKind_iterator
    it = pool.byKindBegin(ResKind::patch),
    e  = pool.byKindEnd(ResKind::patch);

  // check whether there are packages affecting the update stack
  for (; it != e; ++it)
  {
    patch = asKind<Patch>(it->resolvable());
    if (it->isRelevant() && !it->isSatisfied() && patch->restartSuggested())
    {
      pkg_mgr_available = true;
      break;
    }
  }

  it = pool.byKindBegin(ResKind::patch);
  for (; it != e; ++it, ++patchcount)
  {
    if (zypper.cOpts().count("all") || it->isBroken())
    {
      ResObject::constPtr res = it->resolvable();
      Patch::constPtr patch = asKind<Patch>(res);

      // if updates stack patches are available, show only those
      if ((pkg_mgr_available && patch->restartSuggested()) || !pkg_mgr_available)
      {
        cout << " <update ";
        cout << "name=\"" << res->name () << "\" ";
        cout << "edition=\""  << res->edition ().asString() << "\" ";
	cout << "arch=\""  << res->arch().asString() << "\" ";
	cout << "status=\""  << patchStatusAsString( *it ) << "\" ";
        cout << "category=\"" <<  patch->category() << "\" ";
        cout << "pkgmanager=\"" << (patch->restartSuggested() ? "true" : "false") << "\" ";
        cout << "restart=\"" << (patch->rebootSuggested() ? "true" : "false") << "\" ";

        Patch::InteractiveFlags ignoreFlags = Patch::NoFlags;
        if (zypper.globalOpts().reboot_req_non_interactive)
          ignoreFlags |= Patch::Reboot;

        cout << "interactive=\"" << (patch->interactiveWhenIgnoring(ignoreFlags) ? "true" : "false") << "\" ";
        cout << "kind=\"patch\"";
        cout << ">" << endl;
        cout << "  <summary>" << xml_encode(patch->summary()) << "  </summary>" << endl;
        cout << "  <description>" << xml_encode(patch->description()) << "</description>" << endl;
        cout << "  <license>" << xml_encode(patch->licenseToConfirm()) << "</license>" << endl;

        if ( !patch->repoInfo().alias().empty() )
        {
          cout << "  <source url=\"" << xml_encode(patch->repoInfo().url().asString());
          cout << "\" alias=\"" << xml_encode(patch->repoInfo().alias()) << "\"/>" << endl;
        }

        cout << " </update>" << endl;
      }
    }
  }

  //! \todo change this from appletinfo to something general, define in xmlout.rnc
  if (patchcount == 0)
    cout << "<appletinfo status=\"no-update-repositories\"/>" << endl;

  return pkg_mgr_available;
}

// ----------------------------------------------------------------------------

static void xml_list_updates(const ResKindSet & kinds)
{
  Candidates candidates;
  find_updates (kinds, candidates);

  Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
  for (ci = cb; ci != ce; ++ci) {
    ResObject::constPtr res = ci->resolvable();

    cout << " <update ";
    cout << "name=\"" << res->name () << "\" " ;
    cout << "edition=\""  << res->edition ().asString() << "\" ";
    cout << "arch=\""  << res->arch().asString() << "\" ";
    cout << "kind=\"" << res->kind() << "\" ";
    cout << ">" << endl;
    cout << "  <summary>" << xml_encode(res->summary()) << "  </summary>" << endl;
    cout << "  <description>" << xml_encode(res->description()) << "</description>" << endl;
    cout << "  <license>" << xml_encode(res->licenseToConfirm()) << "</license>" << endl;

    if ( !res->repoInfo().alias().empty() )
    {
        cout << "  <source url=\"" << xml_encode(res->repoInfo().url().asString());
        cout << "\" alias=\"" << xml_encode(res->repoInfo().alias()) << "\"/>" << endl;
    }

    cout << " </update>" << endl;
  }
}

// ----------------------------------------------------------------------------

static bool list_patch_updates(Zypper & zypper)
{
  bool all = zypper.cOpts().count("all");
  // if --date is specified
  Date date_limit;
  {
    parsed_opts::const_iterator it;
    it = zypper.cOpts().find("date");
    if (it != zypper.cOpts().end())
    {
      for_(i, it->second.begin(), it->second.end())
      {
        // ISO 8601 format
          date_limit = Date(*i, "%F");
          break;
      }
    }
  }

  // if --category is specified
  string category;
  {
    parsed_opts::const_iterator it;
    it = zypper.cOpts().find("category");
    if (it != zypper.cOpts().end())
    {
      for_(i, it->second.begin(), it->second.end())
      {
        category = *i;
        break;
      }
    }
    else {
      it = zypper.cOpts().find("g");
      if (it != zypper.cOpts().end())
      {
        for_(i, it->second.begin(), it->second.end())
        {
          category = *i;
          break;
        }
      }
    }
  }

  Table tbl;
  Table pm_tbl; // only those that affect packagemanager (restartSuggested()), they have priority
  TableHeader th;
  unsigned cols;

  th << (zypper.globalOpts().is_rug_compatible ? _("Catalog") : _("Repository"))
     << _("Name") << _("Version") << _("Category") << _("Status");
  cols = 5;
  tbl << th;
  pm_tbl << th;

  const zypp::ResPool& pool = God->pool();
  ResPool::byKind_iterator
    it = pool.byKindBegin(ResKind::patch),
    e  = pool.byKindEnd(ResKind::patch);
  for (; it != e; ++it )
  {
    ResObject::constPtr res = it->resolvable();

    // show only needed and wanted/unlocked (bnc #420606) patches unless --all
    if (all || (it->isBroken() && !it->isUnwanted()))
    {
      Patch::constPtr patch = asKind<Patch>(res);
      if (date_limit != Date() && patch->timestamp() > date_limit ) {
          DBG << patch->ident() << " skipped. (too new and date limit specified)" << endl;
          continue;
      }

      if (!category.empty() && category != patch->category())
      {
        DBG << "candidate patch " << patch << " is not in the specified category" << endl;
        continue;
      }

      // table
      {
        TableRow tr (cols);
        tr << (zypper.config().show_alias ? patch->repoInfo().alias() : patch->repoInfo().name());
        tr << res->name () << res->edition ().asString();
        tr << patch->category();
        tr << (it->isBroken() ? _("needed") : _("not needed"));

        if (!all && patch->restartSuggested ())
          pm_tbl << tr;
        else
          tbl << tr;
      }
    }
  }

  // those that affect the package manager go first
  // (TODO: user option for this?)
  bool affectpm = false;
  if (!pm_tbl.empty())
  {
    affectpm = true;
    if (!tbl.empty())
    {
      zypper.out().info(_("The following software management updates will be installed first:"));
      zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL);
    }
    pm_tbl.sort(1); // Name
    cout << pm_tbl;
  }

  tbl.sort(1); // Name
  if (tbl.empty() && !affectpm)
    zypper.out().info(_("No updates found."));
  else if (!tbl.empty())
  {
    if (affectpm)
    {
      zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL);
      zypper.out().info(_("The following updates are also available:"));
    }
    zypper.out().info("", Out::QUIET, Out::TYPE_NORMAL);
    cout << tbl;
  }

  return affectpm;
}

// ----------------------------------------------------------------------------

/**
 * Find all available updates of given kind.
 */
static void
find_updates( const ResKind & kind, Candidates & candidates )
{
  const zypp::ResPool& pool = God->pool();
  DBG << "Looking for update candidates of kind " << kind << endl;

  // package updates
  if (kind == ResKind::package && !Zypper::instance()->cOpts().count("all"))
  {
    God->resolver()->doUpdate();
    ResPool::const_iterator
      it = God->pool().begin(),
      e  = God->pool().end();
    for (; it != e; ++it)
      // show every package picked by doUpdate for installation
      // except the ones which are not currently installed (bnc #483910)
      if (it->status().isToBeInstalled())
      {
        ui::Selectable::constPtr s =
            ui::Selectable::get((*it)->kind(), (*it)->name());
        if (s->hasInstalledObj())
          candidates.insert(*it);
      }
    return;
  }


  // get --all available updates, no matter if they are installable or break
  // some current policy
  for_(it, pool.proxy().byKindBegin(kind), pool.proxy().byKindEnd(kind))
  {
    if (!(*it)->hasInstalledObj())
      continue;

    PoolItem candidate = (*it)->highestAvailableVersionObj(); // bnc #557557
    if (!candidate)
      continue;
    if (compareByNVRA((*it)->installedObj().resolvable(), candidate.resolvable()) >= 0)
      continue;

    DBG << "selectable: " << **it << endl;
    DBG << "candidate: " << candidate << endl;
    candidates.insert (candidate);
  }
}

// ----------------------------------------------------------------------------

/**
 * Find all available updates of given kinds.
 */
void
find_updates( const ResKindSet & kinds, Candidates & candidates )
{
  for (ResKindSet::const_iterator kit = kinds.begin(); kit != kinds.end(); ++kit)
    find_updates(*kit, candidates);

  if (kinds.empty())
    WAR << "called with empty kinds set" << endl;
}

// ----------------------------------------------------------------------------

string i18n_kind_updates(const ResKind & kind)
{
  if (kind == ResKind::package)
    return _("Package updates");
  else if (kind == ResKind::patch)
    return _("Patches");
  else if (kind == ResKind::pattern)
    return _("Pattern updates");
  else if (kind == ResKind::product)
    return _("Product updates");

  return boost::str(format("%s updates") % kind);
}

// ----------------------------------------------------------------------------

// FIXME rewrite this function so that first the list of updates is collected and later correctly presented (bnc #523573)

void list_updates(Zypper & zypper, const ResKindSet & kinds, bool best_effort)
{
  if (zypper.out().type() == Out::TYPE_XML)
  {
    cout << "<update-status version=\"0.6\">" << endl;
    cout << "<update-list>" << endl;
  }

  // whether some of the listed patches affects package management itself
  // false indicates that we are not checking for patches at all
  // (no 'zypper lp' or 'zypper lu -t patch ...'), or there are no patches
  // affecting the package management stack
  bool affects_pkgmgr = false;

  unsigned kind_size = kinds.size();
  ResKindSet localkinds = kinds;
  ResKindSet::iterator it;

  // patch updates first
  it = localkinds.find(ResKind::patch);
  if(it != localkinds.end())
  {
    if (zypper.out().type() == Out::TYPE_XML)
      affects_pkgmgr = xml_list_patches(zypper);
    else
    {
      if (kinds.size() > 1)
      {
        zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL);
        zypper.out().info(i18n_kind_updates(*it), Out::QUIET, Out::TYPE_NORMAL);
      }
      affects_pkgmgr = list_patch_updates(zypper);
    }
    localkinds.erase(it);
  }

  // list other kinds (only if there are no _patches_ affecting the package manager)

  // XML output here
  if (zypper.out().type() == Out::TYPE_XML)
  {
    if (!affects_pkgmgr)
      xml_list_updates(localkinds);
    cout << "</update-list>" << endl;
    cout << "</update-status>" << endl;
    return;
  }

  if (affects_pkgmgr)
    return;

  // normal output here
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
    if (!hide_repo)
      th << (zypper.globalOpts().is_rug_compatible ? _("Catalog") : _("Repository"));

    if (zypper.globalOpts().is_rug_compatible)
      th << _("Bundle");

    name_col = th.cols();
    th << _("Name");
    // best_effort does not know version or arch yet
    if (!best_effort)
    {
      if (*it == ResKind::package)
        th << _("Current Version");
      th << _("Available Version") << _("Arch");
    }

    tbl << th;

    unsigned int cols = th.cols();

    ResPoolProxy uipool( ResPool::instance().proxy() );

    Candidates candidates;
    find_updates( *it, candidates );

    Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
    for (ci = cb; ci != ce; ++ci) {
      ResObject::constPtr res = ci->resolvable();
      TableRow tr (cols);
      tr << "v";
      if (!hide_repo) {
        tr << (zypper.config().show_alias ?  res->repoInfo().alias() : res->repoInfo().name());
      }
      if (zypper.globalOpts().is_rug_compatible)
        tr << "";               // Bundle
      tr << res->name ();

      // strictly speaking, we could show version and arch even in best_effort
      //  iff there is only one candidate. But we don't know the number of candidates here.
      if (!best_effort)
      {
        // for packages show also the current installed version (bnc #466599)
        if (*it == ResKind::package)
        {
          ui::Selectable::Ptr sel( uipool.lookup( *ci ) );
          if ( sel->hasInstalledObj() )
            tr << sel->installedObj()->edition().asString();
        }
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

// ----------------------------------------------------------------------------

void list_patches_by_issue(Zypper & zypper)
{
  // lp --issues               - list all issues which need to be fixed
  // lp --issues=foo           - look for foo in issue # or description
  // lp --bz, --bugzilla       - list all bugzilla issues
  // lp --cve                  - list all CVE issues
  // lp --bz=foo --cve=foo     - look for foo in bugzillas or CVEs
  // lp --all                  - list all, not only needed patches

  // --bz, --cve can't be used together with --issue; this case is ruled out
  // in the initial arguments validation in Zypper.cc

  typedef set<pair<string, string> > Issues;
  bool only_needed = !zypper.cOpts().count("all");
  bool specific = false; // whether specific issue numbers were given

  Table t;
  TableHeader th;
  th << _("Issue") << _("No.") << _("Patch") << _("Category") << _("Status");
  t << th;

#define FILL_ISSUES(TYPE, ID) \
  it = zypper.cOpts().find(TYPE); \
  if (it != zypper.cOpts().end()) \
    for_(i, it->second.begin(), it->second.end()) \
    { \
      issues.insert(pair<string, string>(ID, *i)); \
      if (!i->empty()) \
        specific = true; \
    } \

  // make a set of unique issues
  Issues issues;
  parsed_opts::const_iterator it;
  FILL_ISSUES("bugzilla", "bugzilla");
  FILL_ISSUES("bz", "bugzilla");
  FILL_ISSUES("cve", "cve");
  FILL_ISSUES("issues", "issues");

  // remove those without arguments if there are any with arguments
  // (will show only the specified ones)
  if (specific)
    for (Issues::const_iterator i = issues.begin(); i != issues.end(); )
    {
      if (i->second.empty())
      {
        zypper.out().warning(str::form(_(
            "Ignoring %s without argument because similar option with"
            " an argument has been specified."),
            ("--" + i->first).c_str()));
        issues.erase(i++);
      }
      else
        ++i;
    }

  // construct a PoolQuery for each argument separately and add the results
  // to the Table.
  string issuesstr;
  for_(issue, issues.begin(), issues.end())
  {
    DBG << "querying: " << issue->first << " = " << issue->second << endl;
    PoolQuery q;
    q.setMatchSubstring();
    q.setCaseSensitive(false);
    q.addKind(ResKind::patch); // is this unnecessary? only patches should have updateReference* attributes

    // specific bugzilla or CVE
    if (specific)
    {
      if (issue->first == "bugzilla" || issue->first == "cve")
        q.addAttribute(sat::SolvAttr::updateReferenceId, issue->second);
    }
    // all bugzillas or CVEs
    else
    {
      if (issue->first == "bugzilla")
        q.addAttribute(sat::SolvAttr::updateReferenceType, "bugzilla");
      else if (issue->first == "cve")
        q.addAttribute(sat::SolvAttr::updateReferenceType, "cve");
    }
    // look for substring in description and reference ID (issue number)
    if (issue->first == "issues")
    {
      // whether argument was given or not, it does not matter; without
      // argument, all issues will be found
      q.addAttribute(sat::SolvAttr::updateReferenceId, issue->second);
      issuesstr = issue->second;
    }

    for_(it, q.begin(), q.end())
    {
      PoolItem pi(*it);
      if (only_needed && (!pi.isBroken() || pi.isUnwanted()))
        continue;

      Patch::constPtr patch = asKind<Patch>(pi.resolvable());
      DBG << "got: " << patch << endl;

      // Print details about each match in that solvable:
      for_( d, it.matchesBegin(), it.matchesEnd() )
      {
        string itype =
          d->subFind(sat::SolvAttr::updateReferenceType).asString();
        if (issue->first != "issues" && itype != issue->first)
          continue;

        TableRow tr;
        tr << itype;
        tr << d->subFind(sat::SolvAttr::updateReferenceId).asString();
        tr << (patch->name() + "-" + patch->edition().asString());
        tr << patch->category();
        tr << (pi.isBroken() ? _("needed") : _("not needed"));
        t << tr;
      }
    }
  }

  // look for matches in patch descriptions
  Table t1;
  TableHeader th1;
  th1 << _("Name") << _("Version") << _("Category") << _("Summary");
  t1 << th1;
  if (!issuesstr.empty())
  {
    PoolQuery q;
    q.setMatchSubstring();
    q.setCaseSensitive(false);
    q.addKind(ResKind::patch);
    q.addAttribute(sat::SolvAttr::summary, issuesstr);
    q.addAttribute(sat::SolvAttr::description, issuesstr);

    for_(it, q.begin(), q.end())
    {
      PoolItem pi(*it);
      if (only_needed && (!pi.isBroken() || pi.isUnwanted()))
        continue;
      Patch::constPtr patch = asKind<Patch>(pi.resolvable());

      TableRow tr;
      tr << patch->name() << patch->edition().asString() << patch->category();
      //! \todo could show a highlighted match with a portion of surrounding
      //! text. Needs case-insensitive find.
      tr << patch->summary();
      t1 << tr;
    }
  }

  if (!zypper.globalOpts().no_abbrev)
    t1.allowAbbrev(3);
  t.sort(3);
  t1.sort(0);

  if (t.empty() && t1.empty())
    zypper.out().info(_("No matching issues found."));
  else
  {
    if (!t.empty())
    {
      if (!issuesstr.empty())
      {
        cout << endl;
        zypper.out().info(_(
            "The following matches in issue numbers have been found:"));
      }

      cout << endl << t;
    }

    if (!t1.empty())
    {
      if (!t.empty())
        cout << endl;
      zypper.out().info(_(
          "Matches in patch descriptions of the following patches have been"
          " found:"));
      cout << endl << t1;
    }
  }
}

// ----------------------------------------------------------------------------

void mark_updates_by_issue(Zypper & zypper)
{
  typedef set<pair<string, string> > Issues;
  Issues issues;
  parsed_opts::const_iterator it = zypper.cOpts().find("bugzilla");
  if (it != zypper.cOpts().end())
    for_(i, it->second.begin(), it->second.end())
      issues.insert(pair<string, string>("b", *i));
  it = zypper.cOpts().find("bz");
  if (it != zypper.cOpts().end())
    for_(i, it->second.begin(), it->second.end())
      issues.insert(pair<string, string>("b", *i));
  it = zypper.cOpts().find("cve");
  if (it != zypper.cOpts().end())
    for_(i, it->second.begin(), it->second.end())
      issues.insert(pair<string, string>("c", *i));

  SolverRequester::Options sropts;
  sropts.skip_interactive = zypper.cOpts().count("skip-interactive");
  sropts.force = zypper.cOpts().count("force");

  for_(issue, issues.begin(), issues.end())
  {
    PoolQuery q;
    q.setMatchExact();
    q.setCaseSensitive(false);
    q.addKind(ResKind::patch); // is this unnecessary?
    if (issue->first == "b")
      q.addAttribute(sat::SolvAttr::updateReferenceId, issue->second);
    else if (issue->first == "c")
      q.addAttribute(sat::SolvAttr::updateReferenceId, issue->second);

    SolverRequester sr(sropts);

    bool found = false;
    for_(sit, q.begin(), q.end()) // can't use poolItem iterator, since that
    {                             // does not have matches iterator used below
      PoolItem pi(*sit);
      if (!pi.isBroken()) // not needed
        continue;

      DBG << "got: " << *sit << endl;

      for_( d, sit.matchesBegin(), sit.matchesEnd() )
      {
        if (issue->first == "b" &&
            d->subFind(sat::SolvAttr::updateReferenceType).asString() == "bugzilla")
        {
          if (sr.installPatch(pi))
            found = true;
          else
            DBG << str::form("fix for bugzilla issue number %s was not marked.",
                issue->second.c_str());
        }
        else if (issue->first == "c" &&
            d->subFind(sat::SolvAttr::updateReferenceType).asString() == "cve")
        {
          if (sr.installPatch(pi))
            found = true;
          else
            DBG << str::form("fix for CVE issue number %s was not marked.",
                issue->second.c_str());
        }
      }
    }
    sr.printFeedback(zypper.out());
    if (!found)
    {
      if (issue->first == "b")
        zypper.out().info(str::form(_(
            "Fix for bugzilla issue number %s was not found or is not needed."),
            issue->second.c_str()));
      else if (issue->first == "c")
        zypper.out().info(str::form(_(
            "Fix for CVE issue number %s was not found or is not needed."),
            issue->second.c_str()));
      zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    }
  } // next issue from --bz --cve
}
