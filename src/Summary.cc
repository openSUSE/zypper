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

#include "zypp/base/Logger.h"
#include "zypp/base/Measure.h"
#include "zypp/ResPool.h"
#include "zypp/Patch.h"
#include "zypp/Package.h"
#include "zypp/ui/Selectable.h"

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
  : _viewop(options), _show_repo_alias(false), _wrap_width(80), _force_no_color(false)
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
  _inst_size_change = ByteCount();

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
    toinstall[ResKind::srcpackage].insert(*it);
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
          support_needacc[res->kind()].insert(ResPair(nullres, res));
        else if (pkg->maybeUnsupported())
          unsupported[res->kind()].insert(ResPair(nullres, res));
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
            toupgrade[res->kind()].insert(rp);
            if (res->arch() != (*rmit)->arch())
              tochangearch[res->kind()].insert(rp);
            if (res->vendor() != (*rmit)->vendor())
              tochangevendor[res->kind()].insert(rp);
          }
          // reinstall
          else if (res->edition() == (*rmit)->edition())
          {
            if (res->arch() != (*rmit)->arch())
              tochangearch[res->kind()].insert(rp);
            else
              toreinstall[res->kind()].insert(rp);
            if (res->vendor() != (*rmit)->vendor())
              tochangevendor[res->kind()].insert(rp);
          }
          // downgrade
          else
          {
            todowngrade[res->kind()].insert(rp);
            if (res->arch() != (*rmit)->arch())
              tochangearch[res->kind()].insert(rp);
            if (res->vendor() != (*rmit)->vendor())
              tochangevendor[res->kind()].insert(rp);
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
        toinstall[res->kind()].insert(ResPair(NULL, res));
        _inst_size_change += res->installSize();
      }

      _todownload += res->downloadSize();
    }
  }

  m.elapsed();

  //bool toremove_by_solver = false;
  for (KindToResObjectSet::const_iterator it = to_be_removed.begin();
      it != to_be_removed.end(); ++it)
    for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
        resit != it->second.end(); ++resit)
    {
      /** \todo this does not work
      if (!toremove_by_solver)
      {
        PoolItem pi(*resit);
        if (pi.status() == ResStatus::SOLVER)
          toremove_by_solver = true;
      }*/
      toremove[it->first].insert(ResPair(nullres, *resit));
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

      candidates[*kit].insert(ResPair(nullres, candidate));
    }
  // compare available updates with the list of packages to be upgraded
  for_(it, toupgrade.begin(), toupgrade.end())
    set_difference(
        candidates[it->first].begin(), candidates[it->first].end(),
        it->second.begin(), it->second.end(),
        inserter(notupdated[it->first], notupdated[it->first].begin()),
        Summary::ResPairNameCompare());

  m.stop();
}

// --------------------------------------------------------------------------

unsigned Summary::packagesToRemove() const
{
  // total packages to remove (packages only - patches, patterns, and products
  // are virtual; srcpackages do not get removed by zypper)
  KindToResPairSet::const_iterator it = toremove.find(ResKind::package);
  if (it != toremove.end())
    return it->second.size();
  return 0;
}

// --------------------------------------------------------------------------

unsigned Summary::packagesToUpgrade() const
{
  // total packages to remove (packages only - patches, patterns, and products
  // are virtual; srcpackages do not get removed by zypper)
  KindToResPairSet::const_iterator it = toupgrade.find(ResKind::package);
  if (it != toupgrade.end())
    return it->second.size();
  return 0;
}

// --------------------------------------------------------------------------

unsigned Summary::packagesToDowngrade() const
{
  // total packages to remove (packages only - patches, patterns, and products
  // are virtual; srcpackages do not get removed by zypper)
  KindToResPairSet::const_iterator it = todowngrade.find(ResKind::package);
  if (it != todowngrade.end())
    return it->second.size();
  return 0;
}

// --------------------------------------------------------------------------

void Summary::writeResolvableList(ostream & out, const ResPairSet & resolvables)
{
  // find multi-version packages
  map<string, unsigned> dupes;
  // no need to do this if SHOW_VERSION is on
  if (!(_viewop & SHOW_VERSION))
  {
    for_(resit, resolvables.begin(), resolvables.end())
      dupes[resit->second->name()]++;
    // remove the single-versions from the map
    for (map<string, unsigned>::iterator it = dupes.begin(); it != dupes.end(); /**/)
    {
      if (it->second == 1)
        dupes.erase(it++); // postfix! Incrementing before erase
      else
        ++it;
    }
  }

  if ((_viewop & DETAILS) == 0)
  {
    ostringstream s;
    for (ResPairSet::const_iterator resit = resolvables.begin();
        resit != resolvables.end(); ++resit)
        // name
      s << (resit->second->kind() == ResKind::product ?
              resit->second->summary() :
              resit->second->name())
        // version (if multiple versions are present)
        << (dupes.find(resit->second->name()) != dupes.end() ?
             string("-") + resit->second->edition().asString() :
             string())
        << " ";
    mbs_write_wrapped(out, s.str(), 2, _wrap_width);
    out << endl;
    return;
  }

  Table t; t.lineStyle(none); t.wrap(0); t.margin(2);

  for (ResPairSet::const_iterator resit = resolvables.begin();
      resit != resolvables.end(); ++resit)
  {
    TableRow tr;

    // name
    tr << (resit->second->kind() == ResKind::product ?
        resit->second->summary() :
        resit->second->name()) +
      // version (if multiple versions are present)
      (dupes.find(resit->second->name()) != dupes.end() && !(_viewop & SHOW_VERSION) ?
        string("-") + resit->second->edition().asString() :
        string());

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
      tr << (_show_repo_alias ?
          resit->second->repoInfo().alias() :
          resit->second->repoInfo().name());
    }
    if (_viewop & SHOW_VENDOR)
    {
      if (resit->first && resit->first->vendor() != resit->second->vendor())
        tr << resit->first->vendor() + " -> " +
              resit->second->vendor();
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
  for_(it, toinstall.begin(), toinstall.end())
  {
    string label;
    if (it->first == ResKind::package)
      label = _PL(
        "The following NEW package is going to be installed:",
        "The following NEW packages are going to be installed:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following NEW patch is going to be installed:",
        "The following NEW patches are going to be installed:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following NEW pattern is going to be installed:",
        "The following NEW patterns are going to be installed:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following NEW product is going to be installed:",
        "The following NEW products are going to be installed:",
        it->second.size());
    else if (it->first == ResKind::srcpackage)
      label = _PL(
        "The following source package is going to be installed:",
        "The following source packages are going to be installed:",
        it->second.size());
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeRemoved(ostream & out)
{
  ViewOptions vop = _viewop;
  unsetViewOption(SHOW_REPO); // never show repo here, it's always @System
  for_(it, toremove.begin(), toremove.end())
  {
    string label;
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to be REMOVED:",
        "The following packages are going to be REMOVED:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to be REMOVED:",
        "The following patches are going to be REMOVED:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to be REMOVED:",
        "The following patterns are going to be REMOVED:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to be REMOVED:",
        "The following products are going to be REMOVED:",
        it->second.size());
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
  _viewop = vop;
}

// --------------------------------------------------------------------------

void Summary::writeUpgraded(ostream & out)
{
  for_(it, toupgrade.begin(), toupgrade.end())
  {
    string label;
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to be upgraded:",
        "The following packages are going to be upgraded:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to be upgraded:",
        "The following patches are going to be upgraded:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to be upgraded:",
        "The following patterns are going to be upgraded:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to be upgraded:",
        "The following products are going to be upgraded:",
        it->second.size());
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeDowngraded(ostream & out)
{
  for_(it, todowngrade.begin(), todowngrade.end())
  {
    string label;
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to be downgraded:",
        "The following packages are going to be downgraded:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to be downgraded:",
        "The following patches are going to be downgraded:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to be downgraded:",
        "The following patterns are going to be downgraded:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to be downgraded:",
        "The following products are going to be downgraded:",
        it->second.size());
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeReinstalled(ostream & out)
{
  for_(it, toreinstall.begin(), toreinstall.end())
  {
    string label;
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to be reinstalled:",
        "The following packages are going to be reinstalled:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to be reinstalled:",
        "The following patches are going to be reinstalled:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to be reinstalled:",
        "The following patterns are going to be reinstalled:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to be reinstalled:",
        "The following products are going to be reinstalled:",
        it->second.size());
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
    // in the toinstall set (the ones selected by the solver)
    for_(sit, q.begin(), q.end())
    {
      if (sit->isSystem()) // is it necessary to have the system solvable?
        continue;
      if (sit->name() == obj->name())
        continue; // ignore self-recommends (should not happen, though)

      XXX << "rec: " << *sit << endl;
      ResObject::constPtr recobj = makeResObject(*sit);
      ResPairSet::const_iterator match =
        toinstall[sit->kind()].find(ResPair(nullres, recobj));
      if (match != toinstall[sit->kind()].end())
      {
        if (recommended[sit->kind()].insert(*match).second)
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
        toinstall[sit->kind()].find(ResPair(nullres, reqobj));
      if (match != toinstall[sit->kind()].end())
      {
        if (required[sit->kind()].insert(*match).second)
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
  XXX << obj << endl;
  ResObject::constPtr nullres;
  Capabilities req = obj->dep(dep);
  for_(capit, req.begin(), req.end())
  {
    sat::WhatProvides q(*capit);
    for_(selit, q.selectableBegin(), q.selectableEnd())
    {
      ui::Selectable::Ptr s = *selit;
      if (s->name() == obj->name())
        continue; // ignore self-deps

      XXX << dep << ": " << *s << endl;
      if (s->status() == ui::S_NoInst)
      {
        result[s->kind()].insert(Summary::ResPair(nullres, s->candidateObj()));
        break;
      }
    }
  }
}

// --------------------------------------------------------------------------

void Summary::writeRecommended(ostream & out)
{
  // lazy-compute the installed recommended objects
  if (recommended.empty())
  {
    ResObject::constPtr obj;
    for_(kindit, toinstall.begin(), toinstall.end())
      for_(it, kindit->second.begin(), kindit->second.end())
        // collect recommends of all packages request by user
        if (it->second->poolItem().status().getTransactByValue() != ResStatus::SOLVER)
          collectInstalledRecommends(it->second);
  }

  // lazy-compute the not-to-be-installed recommended objects
  if (noinstrec.empty())
  {
    ResObject::constPtr obj;
    for_(kindit, toinstall.begin(), toinstall.end())
      for_(it, kindit->second.begin(), kindit->second.end())
        if (it->second->poolItem().status().getTransactByValue() != ResStatus::SOLVER)
          collectNotInstalledDeps(Dep::RECOMMENDS, it->second, noinstrec);
  }

  for_(it, recommended.begin(), recommended.end())
  {
    string label = "The following recommended packages were selected automatically:";
    if (it->first == ResKind::package)
      label = _PL(
        "The following recommended package was automatically selected:",
        "The following recommended packages were automatically selected:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following recommended patch was automatically selected:",
        "The following recommended patches were automatically selected:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following recommended pattern was automatically selected:",
        "The following recommended patterns were automatically selected:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following recommended product was automatically selected:",
        "The following recommended products were automatically selected:",
        it->second.size());
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }

  for_(it, noinstrec.begin(), noinstrec.end())
  {
    string label;
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is recommended, but will not be installed:",
        "The following packages are recommended, but will not be installed:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is recommended, but will not be installed:",
        "The following patches are recommended, but will not be installed:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is recommended, but will not be installed:",
        "The following patterns are recommended, but will not be installed:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is recommended, but will not be installed:",
        "The following products are recommended, but will not be installed:",
        it->second.size());
    out << endl << label << endl;

    writeResolvableList(out, it->second);
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
  if (noinstsug.empty())
  {
    ResObject::constPtr obj;
    for_(kindit, toinstall.begin(), toinstall.end())
      for_(it, kindit->second.begin(), kindit->second.end())
        // collect recommends of all packages request by user
        if (it->second->poolItem().status().getTransactByValue() != ResStatus::SOLVER)
          collectNotInstalledDeps(Dep::SUGGESTS, it->second, noinstsug);
  }

  for_(it, noinstsug.begin(), noinstsug.end())
  {
    string label;
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is suggested, but will not be installed:",
        "The following packages are suggested, but will not be installed:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is suggested, but will not be installed:",
        "The following patches are suggested, but will not be installed:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is suggested, but will not be installed:",
        "The following patterns are suggested, but will not be installed:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is suggested, but will not be installed:",
        "The following products are suggested, but will not be installed:",
        it->second.size());
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeChangedArch(ostream & out)
{
  ViewOptions vop = _viewop;
  setViewOption(SHOW_ARCH); // always show arch here
  for_(it, tochangearch.begin(), tochangearch.end())
  {
    string label;
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to change architecture:",
        "The following packages are going to change architecture:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to change architecture:",
        "The following patches are going to change architecture:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to change architecture:",
        "The following patterns are going to change architecture:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to change architecture:",
        "The following products are going to change architecture:",
        it->second.size());
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
  for_(it, tochangevendor.begin(), tochangevendor.end())
  {
    string label;
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is going to change vendor:",
        "The following packages are going to change vendor:",
        it->second.size());
    else if (it->first == ResKind::patch)
      label = _PL(
        "The following patch is going to change vendor:",
        "The following patches are going to change vendor:",
        it->second.size());
    else if (it->first == ResKind::pattern)
      label = _PL(
        "The following pattern is going to change vendor:",
        "The following patterns are going to change vendor:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product is going to change vendor:",
        "The following products are going to change vendor:",
        it->second.size());
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
  _viewop = vop;
}

// --------------------------------------------------------------------------

void Summary::writeUnsupported(ostream & out)
{
  for_(it, unsupported.begin(), unsupported.end())
  {
    string label;
    // we only look at vendor support in packages
    if (it->first == ResKind::package)
      label = _PL(
        "The following package is not supported by its vendor:",
        "The following packages are not supported by their vendor:",
        it->second.size());
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeNeedACC(ostream & out)
{
  for_(it, support_needacc.begin(), support_needacc.end())
  {
    string label;
    // we only look at vendor support in packages
    if (it->first == ResKind::package)
      label = _PL(
        "The following package needs additional customer contract to get support:",
        "The following packages need additional customer contract to get support:",
        it->second.size());
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

void Summary::writeNotUpdated(std::ostream & out)
{
  for_(it, notupdated.begin(), notupdated.end())
  {
    string label;
    // we only look at update candidates for packages and products
    if (it->first == ResKind::package)
      label = _PL(
        "The following package update will NOT be installed:",
        "The following package updates will NOT be installed:",
        it->second.size());
    else if (it->first == ResKind::product)
      label = _PL(
        "The following product update will NOT be installed:",
        "The following product updates will NOT be installed:",
        it->second.size());
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }
}

// --------------------------------------------------------------------------

void Summary::writeDownloadAndInstalledSizeSummary(ostream & out)
{
  if (!_inst_pkg_total && toremove.empty())
    return; // nothing to do, keep silent

  // download size info
  ostringstream s;
  if (_todownload > 0)
    s << format(_("Overall download size: %s.")) % _todownload << " ";

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

  i = toupgrade.find(ResKind::package);
  if (i != toupgrade.end())
  {
    count = i->second.size();
    fprint_color(s, str::form("%d ", count), COLOR_CONTEXT_HIGHLIGHT);
    // translators: this text will be preceded by a number e.g. "5 packages to ..."
    s << _PL("package to upgrade", "packages to upgrade", count);
    gotcha = true;
  }
  i = todowngrade.find(ResKind::package);
  if (i != todowngrade.end())
  {
    count = i->second.size();
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
  i = toinstall.find(ResKind::package);
  if (i != toinstall.end())
  {
    count = i->second.size();
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
  i = toreinstall.find(ResKind::package);
  if (i != toreinstall.end())
  {
    count = i->second.size();
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
  i = toremove.find(ResKind::package);
  if (i != toremove.end())
  {
    count = i->second.size();
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
  i = tochangevendor.find(ResKind::package);
  if (i != tochangevendor.end())
  {
    count = i->second.size();
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
  i = tochangearch.find(ResKind::package);
  if (i != tochangearch.end())
  {
    count = i->second.size();
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
      if (!res->summary().empty())
        out << " summary=\"" << xml_encode(res->summary()) << "\"";
      if (!res->description().empty())
        out << ">" << endl << xml_encode(res->description()) << "</solvable>" << endl;
      else
        out << "/>" << endl;
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

  if (!toupgrade.empty())
  {
    out << "<to-upgrade>" << endl;
    writeXmlResolvableList(out, toupgrade);
    out << "</to-upgrade>" << endl;
  }

  if (!todowngrade.empty())
  {
    out << "<to-downgrade>" << endl;
    writeXmlResolvableList(out, todowngrade);
    out << "</to-downgrade>" << endl;
  }

  if (!toinstall.empty())
  {
    out << "<to-install>" << endl;
    writeXmlResolvableList(out, toinstall);
    out << "</to-install>" << endl;
  }

  if (!toreinstall.empty())
  {
    out << "<to-reinstall>" << endl;
    writeXmlResolvableList(out, toreinstall);
    out << "</to-reinstall>" << endl;
  }

  if (!toremove.empty())
  {
    out << "<to-remove>" << endl;
    writeXmlResolvableList(out, toremove);
    out << "</to-remove>" << endl;
  }

  if (!tochangearch.empty())
  {
    out << "<to-change-arch>" << endl;
    writeXmlResolvableList(out, tochangearch);
    out << "</to-change-arch>" << endl;
  }

  if (!tochangevendor.empty())
  {
    out << "<to-change-vendor>" << endl;
    writeXmlResolvableList(out, tochangevendor);
    out << "</to-change-vendor>" << endl;
  }

  if (_viewop & SHOW_UNSUPPORTED && !unsupported.empty())
  {
    out << "<unsupported>" << endl;
    writeXmlResolvableList(out, unsupported);
    out << "</unsupported>" << endl;
  }

  out << "</install-summary>" << endl;
}
