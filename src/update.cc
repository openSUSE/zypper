#include <iostream> // for xml and table output
#include <sstream>
#include <boost/format.hpp>

#include "zypp/base/Logger.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Algorithm.h"
#include "zypp/PoolQuery.h"

#include "zypp/Patch.h"

#include "Table.h"
#include "update.h"

using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;


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
      if (patch->category() == "security")
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

// returns true if restartSuggested() patches are availble
bool xml_list_patches ()
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
    if (it->isRelevant() && !it->isSatisfied())
    {
      ResObject::constPtr res = it->resolvable();
      Patch::constPtr patch = asKind<Patch>(res);

      // if updates stack patches are available, show only those
      if ((pkg_mgr_available && patch->restartSuggested()) || !pkg_mgr_available)
      {
        cout << " <update ";
        cout << "name=\"" << res->name () << "\" ";
        cout << "edition=\""  << res->edition ().asString() << "\" ";
        cout << "category=\"" <<  patch->category() << "\" ";
        cout << "pkgmanager=\"" << (patch->restartSuggested() ? "true" : "false") << "\" ";
        cout << "restart=\"" << (patch->rebootSuggested() ? "true" : "false") << "\" ";
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
    cout << "<appletinfo status=\"no-update-repositories\"/>" << endl;

  return pkg_mgr_available;
}

// ----------------------------------------------------------------------------

static void list_patch_updates(Zypper & zypper)
{
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

    if ( it->isRelevant() && ! it->isSatisfied() )
    {
      Patch::constPtr patch = asKind<Patch>(res);

      {
        TableRow tr (cols);
        tr << patch->repoInfo().name();
        tr << res->name () << res->edition ().asString();
        tr << patch->category();
        tr <<  _("Needed");        

        if (patch->restartSuggested ())
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

  tbl.sort (1);                 // Name

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
      if (!provider.status().isLocked() // is not locked (taboo)
          && (!best                     // first match
              // or a better edition than candidate
              || best->edition().compare( provider->edition() ) < 0))
      {
        best = provider;        // store
      }
      return true;              // keep going
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
  DBG << "Looking for update candidates of kind " << kind << endl;

  // package updates
  if (kind == ResKind::package && !Zypper::instance()->cOpts().count("all"))
  {
    God->resolver()->doUpdate();
    ResPool::const_iterator
      it = God->pool().begin(),
      e  = God->pool().end();
    for (; it != e; ++it)
      if (it->status().isToBeInstalled())
        candidates.insert(*it);
    return;
  }

  ResPool::byKind_iterator
    it = pool.byKindBegin (kind),
    e  = pool.byKindEnd (kind);
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

// ----------------------------------------------------------------------------

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

  // patch updates
  it = localkinds.find(ResKind::patch);
  if(it != localkinds.end())
  {
    if (zypper.out().type() == Out::TYPE_XML)
      not_affects_pkgmgr = !xml_list_patches();
    else
    {
      zypper.out().info(i18n_kind_updates(*it), Out::QUIET, Out::TYPE_NORMAL);
      zypper.out().info("", Out::QUIET, Out::TYPE_NORMAL); // visual separator
      list_patch_updates(zypper);
    }
    localkinds.erase(it);
  }

  // other kinds
  //! \todo list package updates according to Resolver::doUpdate()
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
      th << (zypper.globalOpts().is_rug_compatible ? _("Catalog") : _("Repository"));
    }
    if (zypper.globalOpts().is_rug_compatible) {
      th << _("Bundle");
    }
    name_col = th.cols();
    th << _("Name");
    if (!best_effort) {         // best_effort does not know version or arch yet
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
        tr << "";               // Bundle
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

// ----------------------------------------------------------------------------

// may be useful as a functor
static bool
mark_item_install (const PoolItem & pi)
{
  bool result = pi.status().setToBeInstalled( zypp::ResStatus::USER );
  if (!result)
    ERR << "Marking " << pi << "for installation failed" << endl;
  return result;
}

// ----------------------------------------------------------------------------
// best-effort update
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

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

static bool
mark_patch_update(const PoolItem & pi, bool skip_interactive, bool ignore_affects_pm)
{
  Patch::constPtr patch = asKind<Patch>(pi.resolvable());
  if (pi.isRelevant() && !pi.isSatisfied())
  {
    DBG << "patch " << patch->name() << " " << ignore_affects_pm << ", "
      << patch->restartSuggested() << endl;
    if (ignore_affects_pm || patch->restartSuggested())
    {
      // #221476
      if (skip_interactive
          && (patch->interactive() || !patch->licenseToConfirm().empty()))
      {
        // Skipping a patch because it is marked as interactive or has
        // license to confirm and --skip-interactive is requested.
        Zypper::instance()->out().warning(str::form(
          // translators: %s is the name of a patch
          _("'%s' is interactive, skipping."), patch->name().c_str()));
        return false;
      }
      else
      {
        mark_item_install(pi);
        return true;
      }
    }
  }

  return false;
}

// ----------------------------------------------------------------------------

static void
mark_patch_updates( Zypper & zypper, bool skip_interactive )
{
  DBG << "going to mark patches to install" << endl;

  // search twice: if there are none with restartSuggested(), retry on all
  bool any_marked = false;
  for(unsigned ignore_affects_pm = 0;
      !any_marked && ignore_affects_pm < 2; ++ignore_affects_pm)
  {
    if (zypper.arguments().empty() || zypper.globalOpts().is_rug_compatible)
    {
      DBG << "marking all needed patches" << endl;

      for_(it, God->pool().proxy().byKindBegin(ResKind::patch), 
               God->pool().proxy().byKindEnd  (ResKind::patch))
      {
        if (mark_patch_update((*it)->candidateObj(), skip_interactive, ignore_affects_pm))
          any_marked = true;
      }
    }
    else if (!zypper.arguments().empty())
    {
      for_(it, zypper.arguments().begin(), zypper.arguments().end())
      {
        // look for patches matching specified pattern
        PoolQuery q;
        q.addKind(ResKind::patch);
        q.addAttribute(sat::SolvAttr::name, *it);
        //! \todo should we look for patches requiring packages with matching name instead? 
        //q.addAttribute(sat::SolvAttr::require, *it);
        q.setMatchGlob();

        if (q.empty())
        {
          if (ignore_affects_pm) // avoid displaying this twice
            continue;
          if (it->find_first_of("?*") != string::npos) // wildcards used
            zypper.out().info(str::form(
                _("No patches matching '%s' found."), it->c_str()));
          else
            zypper.out().info(str::form(
                _("Patch '%s' not found."), it->c_str()));
          zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
        }
        else
        {
          for_(pit, q.selectableBegin(), q.selectableEnd())
          {
            any_marked = mark_patch_update((*pit)->candidateObj(), skip_interactive, ignore_affects_pm);
          }
        }
      }
    }
  }
}

// ----------------------------------------------------------------------------

void mark_updates(Zypper & zypper, const ResKindSet & kinds, bool skip_interactive, bool best_effort )
{
  MIL << endl;

  ResKindSet localkinds = kinds;

  if (zypper.arguments().empty() || zypper.globalOpts().is_rug_compatible)
  {
    for_(kindit, localkinds.begin(), localkinds.end())
    {
      if (*kindit == ResKind::package)
      {
        MIL << "Computing package update..." << endl;
        // this will do a complete pacakge update as far as possible
        God->resolver()->doUpdate();
        // no need to call Resolver::resolvePool() afterwards
        zypper.runtimeData().solve_before_commit = false;
      }
      else if (*kindit == ResKind::patch)
      {
        mark_patch_updates(zypper, skip_interactive);
      }
      else
      {
        Candidates candidates;
        find_updates (localkinds, candidates);
        if (best_effort)
          invokeOnEach (candidates.begin(), candidates.end(), require_item_update);
        else
          invokeOnEach (candidates.begin(), candidates.end(), mark_item_install);
      } 
    }
  }
  // treat arguments as package names (+allow wildcards)
  else if (!zypper.arguments().empty())
  {
    for_(kindit, localkinds.begin(), localkinds.end())
    {
      Resolver_Ptr solver = God->resolver();
      for_(it, zypper.arguments().begin(), zypper.arguments().end())
      {
        PoolQuery q;
        q.addKind(*kindit);
        q.addAttribute(sat::SolvAttr::name, *it);
        q.setMatchGlob();
        q.setInstalledOnly();
  
        if (q.empty())
        {
          if (it->find_first_of("?*") != string::npos) // wildcards used
            zypper.out().info(str::form(
                _("No packages matching '%s' are installed."), it->c_str()));
          else
            zypper.out().info(str::form(
                _("Package '%s' is not installed."), it->c_str()));
          zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
        }
        else
          for_(solvit, q.selectableBegin(), q.selectableEnd())
          {
            ui::Selectable::Ptr s = *solvit;
            PoolItem theone = s->theObj();
            if (equalNVRA(*s->installedObj().resolvable(), *theone.resolvable()))
            {
              DBG << "the One (" << theone << ") is installed, skipping." << endl;
              zypper.out().info(str::form(
                  _("No update candidate for '%s'."), s->name().c_str()));
            }
            else
            {
              //s->setCandidate(theone); ?
              //s->setStatus(ui::S_Update); ?
              Capability c(s->name(), Rel::GT, s->installedObj()->edition(), s->kind());
              solver->addRequire(c);
              DBG << *s << " update: adding requirement " << c << endl;
            }
          }
      }
    }
  }
}
