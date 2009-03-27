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
#include "zypp/ResPool.h"
#include "zypp/Patch.h"
#include "zypp/Package.h"

#include "main.h"
#include "utils/text.h"
#include "utils/misc.h"

#include "Summary.h"

using namespace std;
using namespace zypp;
using boost::format;

// --------------------------------------------------------------------------

bool Summary::ResPairNameCompare::operator()(
    const ResPair & p1, const ResPair & p2) const
{
  return ::strcoll(p1.second->name().c_str(), p2.second->name().c_str()) < 0;
}

// --------------------------------------------------------------------------

Summary::Summary(const zypp::ResPool & pool, const ViewOptions options)
  : _viewop(options), _wrap_width(80)
{
  readPool(pool);
}

// --------------------------------------------------------------------------

struct ResNameCompare
{
  bool operator()(
      zypp::ResObject::constPtr r1, zypp::ResObject::constPtr r2) const
  {
    return ::strcoll(r1->name().c_str(), r2->name().c_str()) < 0;
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

      // FIXME asKind not working?
      Package::constPtr pkg = asKind<Package>(res);
      if (pkg)
      {
        // FIXME refactor with libzypp Package::vendorSupportAvailable()
        if (pkg->maybeUnsupported())
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
            toreinstall[res->kind()].insert(rp);
            if (res->arch() != (*rmit)->arch())
              tochangearch[res->kind()].insert(rp);
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

void Summary::writeResolvableList(ostream & out, const ResPairSet & resolvables)
{
  if (_viewop == DEFAULT)
  {
    ostringstream s;
    for (ResPairSet::const_iterator resit = resolvables.begin();
        resit != resolvables.end(); ++resit)
      s << resit->second->name() << " ";
    wrap_text(out, s.str(), 2, _wrap_width);
    out << endl;
  }
}

// --------------------------------------------------------------------------

// plus edition and architecture for verbose output
/*
if (out.verbosity() > Out::NORMAL)
{
  s << "-" << res->edition() << "." << res->arch();

  const string & reponame =  res->repoInfo().name();
  if (!res->vendor().empty() || !reponame.empty())
  {
    s << "  (";
    // plus repo providing this package
    if (!reponame.empty())
      s << reponame;
    // plus package vendor
    if (!res->vendor().empty())
      s << (reponame.empty() ? "" : ", ") << res->vendor();
    s << ")";
  }
  // new line after each package in the verbose mode
  s << endl;
}
*/

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

void Summary::writeRecommended(ostream & out)
{/*
  for_(it, recommended.begin(), recommended.end())
  {
    string label;
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }*/
}

// --------------------------------------------------------------------------

void Summary::writeSuggested(ostream & out)
{/*
  for_(it, suggested.begin(), suggested.end())
  {
    string label;
    out << endl << label << endl;

    writeResolvableList(out, it->second);
  }*/
}

// --------------------------------------------------------------------------

void Summary::writeChangedArch(ostream & out)
{
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
}

// --------------------------------------------------------------------------

void Summary::writeChangedVendor(ostream & out)
{
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
}

// --------------------------------------------------------------------------

void Summary::writeUnsupported(ostream & out)
{
  for_(it, unsupported.begin(), unsupported.end())
  {
    string label;
    // we only look vendor support in packages
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

void Summary::writeDownloadAndInstalledSizeSummary(ostream & out)
{
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

  wrap_text(out, s.str(), 0, _wrap_width);
  out << endl;
}

// --------------------------------------------------------------------------

void Summary::dumpTo(ostream & out)
{
  _wrap_width = get_screen_width();

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
    writeUnsupported(out);
  out << endl;
  //! \todo write package counts
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
        out << " arch-old=\"" << rold->edition() << "\"";
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
