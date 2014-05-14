/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <string.h>
#include <iostream>
#include <sstream>
#include <boost/format.hpp>

#include <zypp/ZYppFactory.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/Measure.h>
#include <zypp/ResPool.h>
#include <zypp/Patch.h>
#include <zypp/Package.h>
#include <zypp/ui/Selectable.h>

#include "main.h"
#include "utils/text.h"
#include "utils/colors.h"
#include "utils/misc.h"
#include "Table.h"
#include "Zypper.h"

#include "Summary.h"

using namespace std;
using namespace zypp;
using boost::format;

// --------------------------------------------------------------------------

bool Summary::ResPairNameCompare::operator()(
    const ResPair & p1, const ResPair & p2) const
{
  int ret = ::strcoll(p1.second->name().c_str(), p2.second->name().c_str());
  if (ret == 0)
    return p1.second->edition() < p2.second->edition();
  return ret < 0;
}

// --------------------------------------------------------------------------

Summary::Summary(const zypp::ResPool & pool, const ViewOptions options)
  : _viewop(options)
  , _wrap_width(80)
  , _force_no_color(false)
  , _download_only(false)
{
  readPool(pool);
}

// --------------------------------------------------------------------------

struct ResNameCompare
{
  bool operator()(
      zypp::ResObject::constPtr r1, zypp::ResObject::constPtr r2) const
  {
    int ret = ::strcoll(r1->name().c_str(), r2->name().c_str());
    if (ret == 0)
      return r1->edition() < r2->edition();
    return ret < 0;
  }
};

typedef std::map<
  zypp::Resolvable::Kind,
  std::set<zypp::ResObject::constPtr, ResNameCompare> > KindToResObjectSet;

// --------------------------------------------------------------------------

void Summary::readPool(const zypp::ResPool & pool)
{
  // reset stats
  _need_reboot = false;
  _need_restart = false;
  _inst_pkg_total = 0;

  _todownload = ByteCount();
  _incache = ByteCount();
  _inst_size_change = ByteCount();

  // find multi-version packages, which actually have mult. versions installed
  for_( ident, sat::Pool::instance().multiversionBegin(), sat::Pool::instance().multiversionEnd() )
  {
    ui::Selectable::Ptr s = pool.proxy().lookup(*ident);
    bool got_multi = s && (
        s->installedSize() > 1 ||
        (s->installedSize() == 1 && s->toInstall()) );
    if (got_multi)
      multi_installed.insert( s->name() );
  }
  // collect resolvables to be installed/removed

  KindToResObjectSet to_be_installed;
  KindToResObjectSet to_be_removed;

  MIL << "Pool contains " << pool.size() << " items." << std::endl;
  DBG << "Install summary:" << endl;

  debug::Measure m;

  for (ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it)
  {
    if (it->status().isToBeInstalled() || it->status().isToBeUninstalled())
    {
      if (it->resolvable()->kind() == ResKind::patch)
      {
        Patch::constPtr patch = asKind<Patch>(it->resolvable());

        // set the 'need reboot' flag
        if (patch->rebootSuggested())
          _need_reboot = true;
        else if (patch->restartSuggested())
          _need_restart = true;
      }

      if (it->status().isToBeInstalled())
      {
        DBG << "<install>   ";
        to_be_installed[it->resolvable()->kind()].insert(it->resolvable());
      }
      if (it->status().isToBeUninstalled())
      {
        DBG << "<uninstall> ";
        to_be_removed[it->resolvable()->kind()].insert(it->resolvable());
      }
      DBG << *it << endl;
    }
  }

  // total packages to download & install
  // (packages & srcpackages only - patches, patterns, and products are virtual)
  _inst_pkg_total =
    to_be_installed[ResKind::package].size() +
    to_be_installed[ResKind::srcpackage].size();

  m.elapsed();

/* This will work again after commit refactoring: all the srcpackages will be in the pool

  for (list<SrcPackage::constPtr>::const_iterator it = zypper.runtimeData().srcpkgs_to_install.begin();
      it != zypper.runtimeData().srcpkgs_to_install.end(); ++it)
    _toinstall[ResKind::srcpackage].insert(*it);
*/

  // iterate the to_be_installed to find installs/upgrades/downgrades + size info

  ResObject::constPtr nullres;

  for (KindToResObjectSet::const_iterator it = to_be_installed.begin();
      it != to_be_installed.end(); ++it)
  {
    for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
        resit != it->second.end(); ++resit)
    {
      ResObject::constPtr res(*resit);

      Package::constPtr pkg = asKind<Package>(res);
      if (pkg)
      {
        if (pkg->vendorSupport() & VendorSupportACC)
          _support_needacc[res->kind()].insert(ResPair(nullres, res));
        else if (pkg->maybeUnsupported())
          _unsupported[res->kind()].insert(ResPair(nullres, res));
      }

      // find in to_be_removed:
      bool upgrade_downgrade = false;
      for (set<ResObject::constPtr>::iterator rmit = to_be_removed[res->kind()].begin();
          rmit != to_be_removed[res->kind()].end(); ++rmit)
      {
        if (res->name() == (*rmit)->name())
        {
          ResPair rp(*rmit, res);

          // upgrade
          if (res->edition() > (*rmit)->edition())
          {
            // don't put multiversion packages to '_toupgrade', they will
            // always be reported as newly installed (and removed)
            if (multi_installed.find(res->name()) != multi_installed.end())
              continue;

            _toupgrade[res->kind()].insert(rp);
            if (res->arch() != (*rmit)->arch())
              _tochangearch[res->kind()].insert(rp);
            if (!VendorAttr::instance().equivalent(res->vendor(), (*rmit)->vendor()))
              _tochangevendor[res->kind()].insert(rp);
          }
          // reinstall
          else if (res->edition() == (*rmit)->edition())
          {
            if (res->arch() != (*rmit)->arch())
              _tochangearch[res->kind()].insert(rp);
            else
              _toreinstall[res->kind()].insert(rp);
            if (!VendorAttr::instance().equivalent(res->vendor(), (*rmit)->vendor()))
              _tochangevendor[res->kind()].insert(rp);
          }
          // downgrade
          else
          {
            // don't put multiversion packages to '_todowngrade', they will
            // always be reported as newly installed (and removed)
            if (multi_installed.find(res->name()) != multi_installed.end())
              continue;

            _todowngrade[res->kind()].insert(rp);
            if (res->arch() != (*rmit)->arch())
              _tochangearch[res->kind()].insert(rp);
            if (!VendorAttr::instance().equivalent(res->vendor(), (*rmit)->vendor()))
              _tochangevendor[res->kind()].insert(rp);
          }

          _inst_size_change += res->installSize() - (*rmit)->installSize();

          // this turned out to be an upgrade/downgrade
          to_be_removed[res->kind()].erase(*rmit);
          upgrade_downgrade = true;
          break;
        }
      }

      if (!upgrade_downgrade)
      {
        _toinstall[res->kind()].insert(ResPair(NULL, res));
        _inst_size_change += res->installSize();
      }

      if ( pkg && pkg->isCached() )
	_incache += res->downloadSize();
      else
	_todownload += res->downloadSize();
    }
  }

  m.elapsed();

  // collect the rest (not upgraded/downgraded) of to_be_removed as '_toremove'
  // and decrease installed size change accordingly

  //bool _toremove_by_solver = false;
  for (KindToResObjectSet::const_iterator it = to_be_removed.begin();
      it != to_be_removed.end(); ++it)
    for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
        resit != it->second.end(); ++resit)
    {
      /** \todo this does not work
      if (!_toremove_by_solver)
      {
        PoolItem pi(*resit);
        if (pi.status() == ResStatus::SOLVER)
          _toremove_by_solver = true;
      }*/
      _toremove[it->first].insert(ResPair(nullres, *resit));
      _inst_size_change -= (*resit)->installSize();
    }

  m.elapsed();

  // *** notupdated ***

  // get all available updates, no matter if they are installable or break
  // some current policy
  KindToResPairSet candidates;
  ResKindSet kinds;
  kinds.insert(ResKind::package);
  kinds.insert(ResKind::product);
  for_(kit, kinds.begin(), kinds.end())
  {
    for_(it, pool.proxy().byKindBegin(*kit), pool.proxy().byKindEnd(*kit))
    {
      if (!(*it)->hasInstalledObj())
        continue;

      ResObject::constPtr candidate =
        (*it)->highestAvailableVersionObj().resolvable();

      if (!candidate)
        continue;
      if (compareByNVRA((*it)->installedObj().resolvable(), candidate) >= 0)
        continue;
      // ignore higher versions with different arch (except noarch) bnc #646410
      if ((*it)->installedObj()->arch() != candidate->arch()
          && (*it)->installedObj()->arch() != Arch_noarch
          && candidate->arch() != Arch_noarch)
        continue;
      // mutliversion packages do not end up in _toupgrade, so we need to remove
      // them from candidates if the candidate actually installs (bnc #629197)
      if (multi_installed.find(candidate->name()) != multi_installed.end()
          && candidate->poolItem().status().isToBeInstalled())
        continue;

      candidates[*kit].insert(ResPair(nullres, candidate));
    }
    MIL << *kit << " update candidates: " << candidates[*kit].size() << endl;
    MIL << "to be actually updated: " << _toupgrade[*kit].size() << endl;
  }

  // compare available updates with the list of packages to be upgraded
  //
  // note: operator[] (kindToResPairSet[kind]) actually creates ResPairSet when
  //       used. This avoids bnc #594282 which occured when there was
  //       for_(it, _toupgrade.begin(), _toupgrade.end()) loop used here and there
  //       were no upgrades for that kind.
  for_(kit, kinds.begin(), kinds.end())
    set_difference(
        candidates[*kit].begin(), candidates[*kit].end(),
        _toupgrade [*kit].begin(), _toupgrade [*kit].end(),
        inserter(_notupdated[*kit], _notupdated[*kit].begin()),
        Summary::ResPairNameCompare());

  // remove kinds with empty sets after the set_difference
  for (KindToResPairSet::iterator it = _notupdated.begin(); it != _notupdated.end();)
  {
    if (it->second.empty())
      _notupdated.erase(it++);
    else
      ++it;
  }
  for (KindToResPairSet::iterator it = _toupgrade.begin(); it != _toupgrade.end();)
  {
    if (it->second.empty())
      _toupgrade.erase(it++);
    else
      ++it;
  }

  m.stop();
}

// --------------------------------------------------------------------------

unsigned Summary::packagesToRemove() const
{
  // total packages to remove (packages only - patches, patterns, and products
  // are virtual; srcpackages do not get removed by zypper)
  KindToResPairSet::const_iterator it = _toremove.find(ResKind::package);
  if (it != _toremove.end())
    return it->second.size();
  return 0;
}

// --------------------------------------------------------------------------

unsigned Summary::packagesToUpgrade() const
{
  // total packages to remove (packages only - patches, patterns, and products
  // are virtual; srcpackages do not get removed by zypper)
  KindToResPairSet::const_iterator it = _toupgrade.find(ResKind::package);
  if (it != _toupgrade.end())
    return it->second.size();
  return 0;
}

// --------------------------------------------------------------------------

unsigned Summary::packagesToDowngrade() const
{
  // total packages to remove (packages only - patches, patterns, and products
  // are virtual; srcpackages do not get removed by zypper)
  KindToResPairSet::const_iterator it = _todowngrade.find(ResKind::package);
  if (it != _todowngrade.end())
    return it->second.size();
  return 0;
}

// --------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////
namespace
{
  inline std::string ResPair2Name( const Summary::ResPairSet::value_type & resp_r )
  {
    if ( resp_r.second->kind() == ResKind::product )
      // If two products are involved, show the old ones summary.
      // (The following product is going to be upgraded/downgraded:)
      return resp_r.first ? resp_r.first->summary() : resp_r.second->summary();

    return resp_r.second->name();
  }
} // namespace
///////////////////////////////////////////////////////////////////
void Summary::writeResolvableList(ostream & out, const ResPairSet & resolvables)
{

  if ((_viewop & DETAILS) == 0)
  {
    ostringstream s;
    for (ResPairSet::const_iterator resit = resolvables.begin();
        resit != resolvables.end(); ++resit)
    {
      // name
      s << ResPair2Name( *resit );

      // version (if multiple versions are present)
      if (!(_viewop & SHOW_VERSION) && multi_installed.find(resit->second->name()) != multi_installed.end())
      {
        if (resit->first && resit->first->edition() != resit->second->edition())
          s << "-" << resit->first->edition().asString()
            << "->" << resit->second->edition().asString();
        else
          s << "-" << resit->second->edition().asString();
      }

      s << " ";
    }
    mbs_write_wrapped(out, s.str(), 2, _wrap_width);
    out << endl;
    return;
  }

  Table t; t.lineStyle(none); t.wrap(0); t.margin(2);

  for (ResPairSet::const_iterator resit = resolvables.begin();
      resit != resolvables.end(); ++resit)
  {
    TableRow tr;

    string name = ResPair2Name( *resit );

    // version (if multiple versions are present)
    if (!(_viewop & SHOW_VERSION) && multi_installed.find(resit->second->name()) != multi_installed.end())
    {
      if (resit->first && resit->first->edition() != resit->second->edition())
        name += string("-") + resit->first->edition().asString()
             + "->" + resit->second->edition().asString();
      else
        name += string("-") + resit->second->edition().asString();
    }

    tr << name;

    if (_viewop & SHOW_VERSION)
    {
      if (resit->first && resit->first->edition() != resit->second->edition())
        tr << resit->first->edition().asString() + " -> " +
              resit->second->edition().asString();
      else
        tr << resit->second->edition().asString();
    }
    if (_viewop & SHOW_ARCH)
    {
      if (resit->first && resit->first->arch() != resit->second->arch())
        tr << resit->first->arch().asString() + " -> " +
              resit->second->arch().asString();
      else
        tr << resit->second->arch().asString();
    }
    if (_viewop & SHOW_REPO)
    {
      // we do not know about repository changes, only show the repo from
      // which the package will be installed
      tr << resit->second->repoInfo().asUserString();
    }
    if (_viewop & SHOW_VENDOR)
    {
      if (resit->first && ! VendorAttr::instance().equivalent(resit->first->vendor(), resit->second->vendor()))
        tr << resit->first->vendor() + " -> " + resit->second->vendor();
      else
        tr << resit->second->vendor();
    }
    t << tr;
  }

  out << t << endl;
}

// --------------------------------------------------------------------------

void Summary::writeNewlyInstalled(ostream & out)
{
  for_(it, _toinstall.begin(), _toinstall.end())
  {
    string label("%d");
    if (it->first == ResKind::package)
      label = _PL(
        "The following NEW package is going to be installed:",
        "The following %d NEW packages are going to be installed:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following NEW patch is going to be installed:",
        "The following %d NEW patches are going to be installed:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following NEW pattern is going to be installed:",
        "The following %d NEW patterns are going to be installed:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following NEW product is going to be installed:",
        "The following %d NEW products are going to be installed:",
        it->second.size());
    else if (it->first == ResKind::srcpackage)
      label = _PL(
        "The following source package is going to be installed:",
        "The following %d source packages are going to be installed:",
        it->second.size());
    else if (it->first == ResKind::application)
      label = _PL(
        "The following application is going to be installed:",
        "The following %d applications are going to be installed:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeRemoved(ostream & out)
{
  ViewOptions vop = _viewop;
  unsetViewOption(SHOW_REPO); // never show repo here, it's always @System
  for_(it, _toremove.begin(), _toremove.end())
  {
    string label("%d");
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to be REMOVED:",
        "The following %d packages are going to be REMOVED:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to be REMOVED:",
        "The following %d patches are going to be REMOVED:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to be REMOVED:",
        "The following %d patterns are going to be REMOVED:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to be REMOVED:",
        "The following %d products are going to be REMOVED:",
        it->second.size());
    else if (it->first == ResKind::application)
      label = _PL(
        "The following application is going to be REMOVED:",
        "The following %d applications are going to be REMOVED:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
  _viewop = vop;
}

// --------------------------------------------------------------------------

void Summary::writeUpgraded(ostream & out)
{
  for_(it, _toupgrade.begin(), _toupgrade.end())
  {
    string label("%d");
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to be upgraded:",
        "The following %d packages are going to be upgraded:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to be upgraded:",
        "The following %d patches are going to be upgraded:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to be upgraded:",
        "The following %d patterns are going to be upgraded:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to be upgraded:",
        "The following %d products are going to be upgraded:",
        it->second.size());
    else if (it->first == ResKind::application)
      label = _PL(
        "The following application is going to be upgraded:",
        "The following %d applications are going to be upgraded:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeDowngraded(ostream & out)
{
  for_(it, _todowngrade.begin(), _todowngrade.end())
  {
    string label("%d");
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to be downgraded:",
        "The following %d packages are going to be downgraded:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to be downgraded:",
        "The following %d patches are going to be downgraded:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to be downgraded:",
        "The following %d patterns are going to be downgraded:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to be downgraded:",
        "The following %d products are going to be downgraded:",
        it->second.size());
    else if (it->first == ResKind::application)
      label = _PL(
        "The following application is going to be downgraded:",
        "The following %d applications are going to be downgraded:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeReinstalled(ostream & out)
{
  for_(it, _toreinstall.begin(), _toreinstall.end())
  {
    string label("%d");
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to be reinstalled:",
        "The following %d packages are going to be reinstalled:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to be reinstalled:",
        "The following %d patches are going to be reinstalled:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to be reinstalled:",
        "The following %d patterns are going to be reinstalled:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to be reinstalled:",
        "The following %d products are going to be reinstalled:",
        it->second.size());
    else if (it->first == ResKind::application)
      label = _PL(
        "The following application is going to be reinstalled:",
        "The following %d applications are going to be reinstalled:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::collectInstalledRecommends(const ResObject::constPtr & obj)
{
  XXX << obj << endl;
  ResObject::constPtr nullres;

  Capabilities rec = obj->recommends();
  for_(capit, rec.begin(), rec.end())
  {
    sat::WhatProvides q(*capit);
    // not using selectables here: matching found resolvables against those
    // in the _toinstall set (the ones selected by the solver)
    for_(sit, q.begin(), q.end())
    {
      if (sit->isSystem()) // is it necessary to have the system solvable?
        continue;
      if (sit->name() == obj->name())
        continue; // ignore self-recommends (should not happen, though)

      XXX << "rec: " << *sit << endl;
      ResObject::constPtr recobj = makeResObject(*sit);
      ResPairSet::const_iterator match =
        _toinstall[sit->kind()].find(ResPair(nullres, recobj));
      if (match != _toinstall[sit->kind()].end())
      {
        if (_recommended[sit->kind()].insert(*match).second)
          collectInstalledRecommends(recobj);
        break;
      }
    }
  }

  Capabilities req = obj->requires();
  for_(capit, req.begin(), req.end())
  {
    sat::WhatProvides q(*capit);
    for_(sit, q.begin(), q.end())
    {
      if (sit->isSystem()) // is it necessary to have the system solvable?
        continue;
      if (sit->name() == obj->name())
        continue; // ignore self-requires
      XXX << "req: " << *sit << endl;
      ResObject::constPtr reqobj = makeResObject(*sit);
      ResPairSet::const_iterator match =
        _toinstall[sit->kind()].find(ResPair(nullres, reqobj));
      if (match != _toinstall[sit->kind()].end())
      {
        if (_required[sit->kind()].insert(*match).second)
          collectInstalledRecommends(reqobj);
        break;
      }
    }
  }
}

// --------------------------------------------------------------------------

static void collectNotInstalledDeps(
    const Dep & dep,
    const ResObject::constPtr & obj,
    Summary::KindToResPairSet & result)
{
  static std::vector<ui::Selectable::Ptr> tmp;	// reuse capacity
  //DBG << obj << endl;
  Capabilities req = obj->dep(dep);
  for_( capit, req.begin(), req.end() )
  {
    tmp.clear();
    sat::WhatProvides q(*capit);
    for_( it, q.selectableBegin(), q.selectableEnd() )
    {
      if ( (*it)->name() == obj->name() )
        continue;		// ignore self-deps

      if ( (*it)->offSystem() )
      {
	if ( (*it)->toDelete() )
	  continue;		// ignore explicitly deleted
	tmp.push_back( (*it) );	// remember uninstalled
      }
      else
      {
	// at least one of the recommendations is/gets installed: discard all
	tmp.clear();
	break;
      }
    }
    if ( ! tmp.empty() )
    {
      // collect remembered ones
      for_( it, tmp.begin(), tmp.end() )
      {
	//DBG << dep << " :" << (*it)->onSystem() << ": " << dump(*(*it)) << endl;
	result[(*it)->kind()].insert(Summary::ResPair(nullptr, (*it)->candidateObj()));
      }
    }
  }
}

// --------------------------------------------------------------------------

void Summary::writeRecommended(ostream & out)
{
  // lazy-compute the installed recommended objects
  if (_recommended.empty())
  {
    ResObject::constPtr obj;
    for_(kindit, _toinstall.begin(), _toinstall.end())
      for_(it, kindit->second.begin(), kindit->second.end())
        // collect recommends of all packages request by user
        if (it->second->poolItem().status().getTransactByValue() != ResStatus::SOLVER)
          collectInstalledRecommends(it->second);
  }

  // lazy-compute the not-to-be-installed recommended objects
  if (_noinstrec.empty())
  {
    ResObject::constPtr obj;
    for_(kindit, _toinstall.begin(), _toinstall.end())
      for_(it, kindit->second.begin(), kindit->second.end())
        if (it->second->poolItem().status().getTransactByValue() != ResStatus::SOLVER)
          collectNotInstalledDeps(Dep::RECOMMENDS, it->second, _noinstrec);
  }

  for_(it, _recommended.begin(), _recommended.end())
  {
    string label( "%d" );
    if (it->first == ResKind::package)
      label = _PL(
        "The following recommended package was automatically selected:",
        "The following %d recommended packages were automatically selected:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following recommended patch was automatically selected:",
        "The following %d recommended patches were automatically selected:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following recommended pattern was automatically selected:",
        "The following %d recommended patterns were automatically selected:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following recommended product was automatically selected:",
        "The following %d recommended products were automatically selected:",
        it->second.size());
    else if (it->first == ResKind::srcpackage)
      label = _PL(
        "The following recommended source package was automatically selected:",
        "The following %d recommended source packages were automatically selected:",
        it->second.size());
    else if (it->first == ResKind::application)
      label = _PL(
        "The following recommended application was automatically selected:",
        "The following %d recommended applications were automatically selected:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }

  for_(it, _noinstrec.begin(), _noinstrec.end())
  {
    std::string label( "%d" );;
    // For packages, check the reason for not being installed. One reason can be that
    // the solver is told to install only required packages. If not, a package might be
    // unwanted because the user has removed it manually (added to /var/lib/zypp/SoftLocks)
    // or it will not be installed due to conflicts/dependency issues.
    if (it->first == ResKind::package)
    {
      Resolver_Ptr resolver = zypp::getZYpp()->resolver();

      ResPairSet softLocked;
      ResPairSet conflicts;
      ResPairSet notRequired;
      for_( pair_it, it->second.begin(), it->second.end() )
      {
        if ( resolver->onlyRequires() ) // only required packages will be installed
        {
          notRequired.insert(*pair_it);
        }
        else    // recommended packages should be installed - but...
        {
          if ( pair_it->second->poolItem().status().isSoftLocked() )
          {
            softLocked.insert(*pair_it);
          }
          else
          {
            conflicts.insert(*pair_it);
          }
        }
      }

      if ( resolver->onlyRequires() )
      {
	label = _PL( "The following package is recommended, but will not be installed (only required packages will be installed):",
		     "The following %d packages are recommended, but will not be installed (only required packages will be installed):",
		     it->second.size() );
	label = str::form( label.c_str(), it->second.size() );
	out << endl << label << endl;
	writeResolvableList(out, notRequired);
      }
      else
      {
        if ( !softLocked.empty() )
        {
	  label = _PL( "The following package is recommended, but will not be installed because it's unwanted (was manually removed before):",
		       "The following %d packages are recommended, but will not be installed because they are unwanted (were manually removed before):",
		       it->second.size() );
	label = str::form( label.c_str(), it->second.size() );
	out << endl << label << endl;
	  writeResolvableList(out, softLocked);
        }
        if ( !conflicts.empty() )
        {
	  label = _PL( "The following package is recommended, but will not be installed due to conflicts or dependency issues:",
		       "The following %d packages are recommended, but will not be installed due to conflicts or dependency issues:",
		       it->second.size() );
	  label = str::form( label.c_str(), it->second.size() );
          out << endl << label << endl;
          writeResolvableList(out, conflicts);
        }
      }
    }
    else
    {
      if (it->first == ResKind::patch)
        label = _PL( "The following patch is recommended, but will not be installed:",
		     "The following %d patches are recommended, but will not be installed:",
		     it->second.size() );
      else if (it->first == ResKind::pattern)
        label = _PL( "The following pattern is recommended, but will not be installed:",
		     "The following %d patterns are recommended, but will not be installed:",
		     it->second.size() );
      else if (it->first == ResKind::product)
	label = _PL( "The following product is recommended, but will not be installed:",
		     "The following %d products are recommended, but will not be installed:",
		     it->second.size() );
      else if (it->first == ResKind::application)
	label = _PL( "The following application is recommended, but will not be installed:",
		     "The following %d applications are recommended, but will not be installed:",
		     it->second.size() );
      label = str::form( label.c_str(), it->second.size() );
      out << endl << label << endl;
      writeResolvableList(out, it->second);
    }
  }

/*
  for_(it, required.begin(), required.end())
  {
    string label = "These are required:";
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
*/
}

// --------------------------------------------------------------------------

void Summary::writeSuggested(ostream & out)
{
  if (_noinstsug.empty())
  {
    ResObject::constPtr obj;
    for_(kindit, _toinstall.begin(), _toinstall.end())
      for_(it, kindit->second.begin(), kindit->second.end())
        // collect recommends of all packages request by user
        if (it->second->poolItem().status().getTransactByValue() != ResStatus::SOLVER)
          collectNotInstalledDeps(Dep::SUGGESTS, it->second, _noinstsug);
  }

  for_(it, _noinstsug.begin(), _noinstsug.end())
  {
    string label("%d");
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is suggested, but will not be installed:",
        "The following %d packages are suggested, but will not be installed:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is suggested, but will not be installed:",
        "The following %d patches are suggested, but will not be installed:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is suggested, but will not be installed:",
        "The following %d patterns are suggested, but will not be installed:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is suggested, but will not be installed:",
        "The following %d products are suggested, but will not be installed:",
        it->second.size());
    else if (it->first == ResKind::application)
      label = _PL(
        "The following application is suggested, but will not be installed:",
        "The following %d applications are suggested, but will not be installed:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeChangedArch(ostream & out)
{
  ViewOptions vop = _viewop;
  setViewOption(SHOW_ARCH); // always show arch here
  for_(it, _tochangearch.begin(), _tochangearch.end())
  {
    string label("%d");
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to change architecture:",
        "The following %d packages are going to change architecture:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to change architecture:",
        "The following %d patches are going to change architecture:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to change architecture:",
        "The following %d patterns are going to change architecture:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to change architecture:",
        "The following %d products are going to change architecture:",
        it->second.size());
    else if (it->first == ResKind::application)
      label = _PL(
        "The following application is going to change architecture:",
        "The following %d applications are going to change architecture:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
  _viewop = vop;
}

// --------------------------------------------------------------------------

void Summary::writeChangedVendor(ostream & out)
{
  ViewOptions vop = _viewop;
  setViewOption(SHOW_VENDOR); // always show vendor here
  for_(it, _tochangevendor.begin(), _tochangevendor.end())
  {
    string label("%d");
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to change vendor:",
        "The following %d packages are going to change vendor:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to change vendor:",
        "The following %d patches are going to change vendor:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to change vendor:",
        "The following %d patterns are going to change vendor:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to change vendor:",
        "The following %d products are going to change vendor:",
        it->second.size());
    else if (it->first == ResKind::application)
      label = _PL(
        "The following application is going to change vendor:",
        "The following %d applications are going to change vendor:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
  _viewop = vop;
}

// --------------------------------------------------------------------------

void Summary::writeUnsupported(ostream & out)
{
  for_(it, _unsupported.begin(), _unsupported.end())
  {
    string label("%d");
    // we only look at vendor support in packages
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is not supported by its vendor:",
        "The following %d packages are not supported by their vendor:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeNeedACC(ostream & out)
{
  for_(it, _support_needacc.begin(), _support_needacc.end())
  {
    string label("%d");
    // we only look at vendor support in packages
    if (it->first == ResKind::package)
      label = _PL(
        "The following package needs additional customer contract to get support:",
        "The following %d packages need additional customer contract to get support:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

void Summary::writeNotUpdated(std::ostream & out)
{
  for_(it, _notupdated.begin(), _notupdated.end())
  {
    string label("%d");
    // we only look at update candidates for packages and products
    if (it->first == ResKind::package)
      label = _PL(
        "The following package update will NOT be installed:",
        "The following %d package updates will NOT be installed:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product update will NOT be installed:",
        "The following %d product updates will NOT be installed:",
        it->second.size());
    else if (it->first == ResKind::application)
      label = _PL(
        "The following application update will NOT be installed:",
        "The following %d application updates will NOT be installed:",
        it->second.size());
    label = str::form( label.c_str(), it->second.size() );
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeDownloadAndInstalledSizeSummary(ostream & out)
{
  if (!_inst_pkg_total && _toremove.empty())
    return; // nothing to do, keep silent

  // download size info
  ostringstream s;
  if (_todownload || _incache )
    s << format(_("Overall download size: %1%. Already cached: %2% ")) % _todownload % _incache << " ";

  if (_download_only)
    s << _("Download only.");
  else
  {
    // installed size change info
    if (_inst_size_change > 0)
      // TrasnlatorExplanation %s will be substituted by a byte count e.g. 212 K
      s << format(_("After the operation, additional %s will be used."))
          % _inst_size_change.asString(0,1,1);
    else if (_inst_size_change == 0)
      s << _("No additional space will be used or freed after the operation.");
    else
    {
      // get the absolute size
      ByteCount abs;
      abs = (-_inst_size_change);
      // TrasnlatorExplanation %s will be substituted by a byte count e.g. 212 K
      s << format(_("After the operation, %s will be freed.")) % abs.asString(0,1,1);
    }
  }

  mbs_write_wrapped(out, s.str(), 0, _wrap_width);
  out << endl;
}

void Summary::writePackageCounts(ostream & out)
{
  if (!packagesToGetAndInstall() && !packagesToRemove())
    return;

  ostringstream s;
  bool gotcha = false;
  unsigned count;
  KindToResPairSet::const_iterator i;

  i = _toupgrade.find(ResKind::package);
  if (i != _toupgrade.end() && (count = i->second.size()) )
  {
    fprint_color(s, str::form("%d ", count), COLOR_CONTEXT_HIGHLIGHT);
    // translators: this text will be preceded by a number e.g. "5 packages to ..."
    s << _PL("package to upgrade", "packages to upgrade", count);
    gotcha = true;
  }
  i = _todowngrade.find(ResKind::package);
  if (i != _todowngrade.end() && (count = i->second.size()) )
  {
    if (gotcha)
      s << ", ";
    fprint_color(s, str::form("%d ", count), COLOR_CONTEXT_HIGHLIGHT);
    if (gotcha)
      // translators: this text will be preceded by a number e.g. "5 to ..."
      s << _PL("to downgrade", "to downgrade", count);
    else
      // translators: this text will be preceded by a number e.g. "5 packages to ..."
      s << _PL("package to downgrade", "packages to downgrade", count);
    gotcha = true;
  }
  i = _toinstall.find(ResKind::package);
  if (i != _toinstall.end() && (count = i->second.size()) )
  {
    if (gotcha)
      s << ", ";
    fprint_color(s, str::form("%d ", count), COLOR_CONTEXT_HIGHLIGHT);
    if (gotcha)
      // translators: this text will be preceded by a number e.g. "5 new"
      s << _PL("new", "new", count);
    else
      // translators: this text will be preceded by a number e.g. "5 new to install"
      s << _PL("new package to install", "new packages to install", count);
    gotcha = true;
  }
  i = _toreinstall.find(ResKind::package);
  if (i != _toreinstall.end() && (count = i->second.size()) )
  {
    if (gotcha)
      s << ", ";
    fprint_color(s, str::form("%d ", count), COLOR_CONTEXT_HIGHLIGHT);
    if (gotcha)
      // translators: this text will be preceded by a number e.g. "5 to ..."
      s << _PL("to reinstall", "to reinstall", count);
    else
      // translators: this text will be preceded by a number e.g. "5 packages to ..."
      s << _PL("package to reinstall", "packages to reinstall", count);
    gotcha = true;
  }
  i = _toremove.find(ResKind::package);
  if (i != _toremove.end() && (count = i->second.size()) )
  {
    if (gotcha)
      s << ", ";
    fprint_color(s, str::form("%d ", count), COLOR_CONTEXT_NEGATIVE);
    if (gotcha)
      // translators: this text will be preceded by a number e.g. "5 to ..."
      s << _PL("to remove", "to remove", count);
    else
      // translators: this text will be preceded by a number e.g. "5 packages to ..."
      s << _PL("package to remove", "packages to remove", count);
    gotcha = true;
  }
  i = _tochangevendor.find(ResKind::package);
  if (i != _tochangevendor.end() && (count = i->second.size()) )
  {
    if (gotcha)
      s << ", ";
    fprint_color(s, str::form("%d ", count), COLOR_CONTEXT_NEGATIVE);
    if (gotcha)
      // translators: this text will be preceded by a number e.g. "5 to ..."
      s << _PL("to change vendor", " to change vendor", count);
    else
      // translators: this text will be preceded by a number e.g. "5 packages ..."
      s << _PL("package will change vendor", "packages will change vendor", count);
    gotcha = true;
  }
  i = _tochangearch.find(ResKind::package);
  if (i != _tochangearch.end() && (count = i->second.size()) )
  {
    if (gotcha)
      s << ", ";
    fprint_color(s, str::form("%d ", count), COLOR_CONTEXT_HIGHLIGHT);
    if (gotcha)
      // translators: this text will be preceded by a number e.g. "5 to ..."
      s << _PL("to change arch", "to change arch", count);
    else
      // translators: this text will be preceded by a number e.g. "5 packages ..."
      s << _PL("package will change arch", "packages will change arch", count);
    gotcha = true;
  }
  i = _toinstall.find(ResKind::srcpackage);
  if (i != _toinstall.end() && (count = i->second.size()) )
  {
    if (gotcha)
      s << ", ";
    fprint_color(s, str::form("%d ", count), COLOR_CONTEXT_HIGHLIGHT);
    if (gotcha)
      // translators: this text will be preceded by a number e.g. "5 new"
      s << _PL("source package", "source packages", count);
    else
      // translators: this text will be preceded by a number e.g. "5 new to install"
      s << _PL("source package to install", "source packages to install", count);
    gotcha = true;
  }
  s << "." <<  endl;
  mbs_write_wrapped(out, s.str(), 0, _wrap_width);
}

// --------------------------------------------------------------------------

void Summary::dumpTo(ostream & out)
{
  struct SetColor
  {
    SetColor(bool force) : docolors(Zypper::instance()->config().do_colors)
    { if (force) Zypper::instance()->config().do_colors = false; }
    ~SetColor()
    { Zypper::instance()->config().do_colors = docolors; }
    bool docolors;
  };
  SetColor setcolor(_force_no_color);

  _wrap_width = get_screen_width();

  if (_viewop & SHOW_NOT_UPDATED)
    writeNotUpdated(out);
  writeNewlyInstalled(out);
  writeRemoved(out);
  writeUpgraded(out);
  writeDowngraded(out);
  writeReinstalled(out);
  if (_viewop & SHOW_RECOMMENDED)
    writeRecommended(out);
  if (_viewop & SHOW_SUGGESTED)
    writeSuggested(out);
  writeChangedArch(out);
  writeChangedVendor(out);
  if (_viewop & SHOW_UNSUPPORTED)
  {
    writeNeedACC(out);
    writeUnsupported(out);
  }
  out << endl;
  writePackageCounts(out);
  writeDownloadAndInstalledSizeSummary(out);
}

// --------------------------------------------------------------------------

void Summary::writeXmlResolvableList(ostream & out, const KindToResPairSet & resolvables)
{
  for_(it, resolvables.begin(), resolvables.end())
  {
    for_(pairit, it->second.begin(), it->second.end())
    {
      ResObject::constPtr res(pairit->second);
      ResObject::constPtr rold(pairit->first);

      out << "<solvable";
      out << " type=\"" << res->kind() << "\"";
      out << " name=\"" << res->name() << "\"";
      out << " edition=\"" << res->edition() << "\"";
      out << " arch=\"" << res->arch() << "\"";
      if (rold)
      {
        out << " edition-old=\"" << rold->edition() << "\"";
        out << " arch-old=\"" << rold->arch() << "\"";
      }
      {
	const std::string & text( res->summary() );
	if ( !text.empty() )
	  out << " summary=\"" << xml::escape(text) << "\"";
      }
      {
	const std::string & text( res->description() );
	if ( !text.empty())
	  out << ">\n" << "<description>" << xml::escape( text ) << "</description>" << "</solvable>" << endl;
	else
	  out << "/>" << endl;
      }
    }
  }
}

// --------------------------------------------------------------------------

void Summary::dumpAsXmlTo(ostream & out)
{
  out << "<install-summary";
  out << " download-size=\"" << ((ByteCount::SizeType) _todownload) << "\"";
  out << " space-usage-diff=\"" << ((ByteCount::SizeType) _inst_size_change) << "\"";
  out << ">" << endl;

  if (!_toupgrade.empty())
  {
    out << "<to-upgrade>" << endl;
    writeXmlResolvableList(out, _toupgrade);
    out << "</to-upgrade>" << endl;
  }

  if (!_todowngrade.empty())
  {
    out << "<to-downgrade>" << endl;
    writeXmlResolvableList(out, _todowngrade);
    out << "</to-downgrade>" << endl;
  }

  if (!_toinstall.empty())
  {
    out << "<to-install>" << endl;
    writeXmlResolvableList(out, _toinstall);
    out << "</to-install>" << endl;
  }

  if (!_toreinstall.empty())
  {
    out << "<to-reinstall>" << endl;
    writeXmlResolvableList(out, _toreinstall);
    out << "</to-reinstall>" << endl;
  }

  if (!_toremove.empty())
  {
    out << "<to-remove>" << endl;
    writeXmlResolvableList(out, _toremove);
    out << "</to-remove>" << endl;
  }

  if (!_tochangearch.empty())
  {
    out << "<to-change-arch>" << endl;
    writeXmlResolvableList(out, _tochangearch);
    out << "</to-change-arch>" << endl;
  }

  if (!_tochangevendor.empty())
  {
    out << "<to-change-vendor>" << endl;
    writeXmlResolvableList(out, _tochangevendor);
    out << "</to-change-vendor>" << endl;
  }

  if (_viewop & SHOW_UNSUPPORTED && !_unsupported.empty())
  {
    out << "<_unsupported>" << endl;
    writeXmlResolvableList(out, _unsupported);
    out << "</_unsupported>" << endl;
  }

  out << "</install-summary>" << endl;
}
