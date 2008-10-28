#include <iostream>

#include "zypp/ZYpp.h" // for zypp::ResPool::instance()

#include "zypp/base/Logger.h"
#include "zypp/base/Algorithm.h"
#include "zypp/Patch.h"
#include "zypp/Pattern.h"
#include "zypp/Product.h"
#include "zypp/sat/Solvable.h"

#include "zypp/PoolItem.h"
#include "zypp/ResPoolProxy.h"

#include "main.h"
#include "utils/misc.h" // for kind_to_string_localized and string_patch_status

#include "search.h"

using namespace zypp;
using namespace std;

extern ZYpp::Ptr God;


FillSearchTableSolvable::FillSearchTableSolvable(
    Table & table, zypp::TriBool inst_notinst)
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  , _inst_notinst(inst_notinst)
{
  TableHeader header;

  if (_gopts.is_rug_compatible)
  {
    header
      // translators: S for 'installed Status'
      << _("S")
      // translators: catalog (rug's word for repository) (header)
      << _("Catalog")
      // translators: Bundle is a term used in rug. See rug for how to translate it.
      << _("Bundle")
      // translators: name (general header)
      << _("Name")
      // translators: package version (header)
      << _("Version")
      // translators: package architecture (header)
      << _("Arch");
  }
  else
  {
    header
      // translators: S for 'installed Status'
      << _("S")
      // translators: name (general header)
      << _("Name")
      // translators: type (general header)
      << _("Type")
      // translators: package version (header)
      << _("Version")
      // translators: package architecture (header)
      << _("Arch")
      // translators: package's repository (header)
      << _("Repository");
  }

  *_table << header;
}

bool FillSearchTableSolvable::operator()(const zypp::ui::Selectable::constPtr & s) const
{
  // show available objects
  for_(it, s->availableBegin(), s->availableEnd())
  {
    TableRow row;
    zypp::PoolItem pi = *it;

    // installed status

    // patters
    if (zypp::traits::isPseudoInstalled(s->kind()))
    {
      // patches/patterns are installed if satisfied
      if (pi->kind() != zypp::ResKind::srcpackage && pi.isSatisfied())
      {
        // show only not installed
        if (_inst_notinst == false)
          continue;
        row << "i";
      }
      else
      {
        // show only installed
        if (_inst_notinst == true)
          continue;
        row << "";
      }
    }
    else // packages/products
    {
      if (s->installedEmpty())
      {
        // show only installed
        if (_inst_notinst == true)
          continue;
        row << "";
      }
      else
      {
        static bool installed;
        installed = false;
        for_(instit, s->installedBegin(), s->installedEnd())
        {
          if (equalNVRA(*instit->resolvable(), *pi.resolvable()))
          {
            installed = true;
            break;
          }
        }

        if (installed)
        {
          // show only not installed
          if (_inst_notinst == false)
            continue;
          row << "i";
        }
        else
        {
          // show only installed
          if (_inst_notinst == true)
            continue;
          row << "v";
        }
      }
    }

    if (_gopts.is_rug_compatible)
    {
      row
        << pi->repository().info().name()
        << ""
        << pi->name()
        << pi->edition().asString()
        << pi->arch().asString();
    }
    else
    {
      row
        << pi->name()
        << kind_to_string_localized(pi->kind(), 1)
        << pi->edition().asString()
        << pi->arch().asString()
        << pi->repository().info().name();
    }

    *_table << row;
  }

  if (_inst_notinst == false)
    return true;

  // now list the system packages
  for_(it, s->installedBegin(), s->installedEnd())
  {
    // show installed objects only if there is no counterpart in repos
    bool has_counterpart = false;
    for_(ait, s->availableBegin(), s->availableEnd())
      if (equalNVRA(*it->resolvable(), *ait->resolvable()))
      {
        has_counterpart = true;
        break;
      }
    if (has_counterpart)
      continue;

    TableRow row;
    zypp::PoolItem pi = *it;
    row << "i";
    if (_gopts.is_rug_compatible)
    {
      row
        << _("System Packages")
        // TODO what about rug's Bundle?
        << ""
        << pi->name()
        << pi->edition().asString()
        << pi->arch().asString();
    }
    else
    {
      row
        << pi->name()
        << kind_to_string_localized(pi->kind(), 1)
        << pi->edition().asString()
        << pi->arch().asString()
        << (string("(") + _("System Packages") + ")");
    }

    *_table << row;pi->repository().info().name();
  }

  //! \todo maintain an internal plaindir repo named "Local Packages"
  // for plain rpms (as rug does). ?

  return true;
}


FillSearchTableSelectable::FillSearchTableSelectable(
    Table & table, zypp::TriBool installed_only)
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  , inst_notinst(installed_only)
{
  TableHeader header;
  // translators: S for installed Status
  header << _("S");
  header << _("Name");
  // translators: package summary (header)
  header << _("Summary");
  header << _("Type");
  *_table << header;
}

bool FillSearchTableSelectable::operator()(const zypp::ui::Selectable::constPtr & s) const
{
  TableRow row;

  bool installed;
  if (zypp::traits::isPseudoInstalled(s->kind()))
    installed = s->theObj().isSatisfied();
  else
    installed = !s->installedEmpty();

  if (s->kind() != zypp::ResKind::srcpackage && installed)
  {
    // not-installed only
    if (inst_notinst == false)
      return true;
    row << "i";
  }
  else
  {
    // installed only
    if (inst_notinst == true)
      return true;
    row << "";
  }
  row << s->name();
  row << s->theObj()->summary();
  row << kind_to_string_localized(s->kind(), 1);
  *_table << row;
  return true;
}


FillPatchesTable::FillPatchesTable( Table & table, zypp::TriBool inst_notinst )
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  , _inst_notinst(inst_notinst)
{
  TableHeader header;

  header
    // translators: catalog (rug's word for repository) (header)
    << _("Catalog")
    << _("Name")
    << _("Version")
    // translators: patch category (recommended, security)
    << _("Category")
    // translators: patch status (installed, uninstalled, needed)
    << _("Status");

  *_table << header;
}

bool FillPatchesTable::operator()(const zypp::PoolItem & pi) const
{
  // only not installed
  if (pi.isSatisfied() && _inst_notinst == false)
    return true;
  // only installed
  else if (!pi.isSatisfied() && _inst_notinst == true)
    return true;

  TableRow row;

  zypp::Patch::constPtr patch = zypp::asKind<zypp::Patch>(pi.resolvable());

  row
    << pi->repository().info().name()
    << pi->name()
    << pi->edition().asString()
    << patch->category()
    << string_patch_status(pi);

  *_table << row;

  return true;
}


string selectable_search_repo_str(const ui::Selectable & s)
{
  string repostr;

  // show available objects
  for_(it, s.availableBegin(), s.availableEnd())
  {
    if (repostr.empty())
      repostr = (*it)->repository().info().name();
    else if (repostr != (*it)->repository().info().name())
    {
      repostr = _("(multiple)");
      return repostr;
    }
  }

  return repostr;
}


static string string_weak_status(const ResStatus & rs)
{
  if (rs.isRecommended())
    return _("Recommended");
  if (rs.isSuggested())
    return _("Suggested");
  return "";
}


void list_patches(Zypper & zypper)
{
  MIL
    << "Pool contains " << God->pool().size()
    << " items. Checking whether available patches are needed." << std::endl;

  Table tbl;

  FillPatchesTable callback(tbl);
  invokeOnEach(
    God->pool().byKindBegin(ResKind::patch),
    God->pool().byKindEnd(ResKind::patch),
    callback);

  tbl.sort (1);                 // Name

  if (tbl.empty())
    zypper.out().info(_("No needed patches found."));
  else
    // display the result, even if --quiet specified
    cout << tbl;
}

static void list_patterns_xml(Zypper & zypper)
{
  cout << "<pattern-list>" << endl;

  bool installed_only = zypper.cOpts().count("installed-only");
  bool notinst_only = zypper.cOpts().count("uninstalled-only");

  ResPool::byKind_iterator
    it = God->pool().byKindBegin(ResKind::pattern),
    e  = God->pool().byKindEnd(ResKind::pattern);
  for (; it != e; ++it )
  {
    if (it->isSatisfied() && notinst_only)
      continue;
    else if (!it->isSatisfied() && installed_only)
      continue;

    Pattern::constPtr pattern = asKind<Pattern>(it->resolvable());
    cout << asXML(*pattern, it->isSatisfied()) << endl;
  }

  cout << "</pattern-list>" << endl;
}

static void list_pattern_table(Zypper & zypper)
{
  MIL << "Going to list patterns." << std::endl;

  Table tbl;
  TableHeader th;

  // translators: S for installed Status
  th << _("S");
  th << _("Name") << _("Version");
  if (!zypper.globalOpts().is_rug_compatible)
    th << _("Repository") << _("Dependency");
  tbl << th;

  bool installed_only = zypper.cOpts().count("installed-only");
  bool notinst_only = zypper.cOpts().count("uninstalled-only");

  ResPool::byKind_iterator
    it = God->pool().byKindBegin(ResKind::pattern),
    e  = God->pool().byKindEnd(ResKind::pattern);
  for (; it != e; ++it )
  {
    if (it->isSatisfied() && notinst_only)
      continue;
    else if (!it->isSatisfied() && installed_only)
      continue;

    Pattern::constPtr pattern = asKind<Pattern>(it->resolvable());

    TableRow tr;
    tr << (it->isSatisfied() ? "i" : "");
    tr << pattern->name () << pattern->edition().asString();
    if (!zypper.globalOpts().is_rug_compatible)
    {
      tr << pattern->repoInfo().name();
      tr << string_weak_status(it->status ());
    }
    tbl << tr;
  }
  tbl.sort(1); // Name

  if (tbl.empty())
    zypper.out().info(_("No patterns found."));
  else
    // display the result, even if --quiet specified
    cout << tbl;
}

void list_patterns(Zypper & zypper)
{
  if (zypper.out().type() == Out::TYPE_XML)
    list_patterns_xml(zypper);
  else
    list_pattern_table(zypper);
}

void list_packages(Zypper & zypper)
{
  MIL << "Going to list packages." << std::endl;

  Table tbl;
  TableHeader th;

  // translators: S for installed Status
  th << _("S");
  if (zypper.globalOpts().is_rug_compatible)
    // translators: Bundle is a term used in rug. See rug for how to translate it.
    th << _("Bundle");
  else
    th << _("Repository");
  th << _("Name") << _("Version") << _("Arch");
  tbl << th;

  bool installed_only = zypper.cOpts().count("installed-only");
  bool notinst_only = zypper.cOpts().count("uninstalled-only");

  ResPoolProxy::const_iterator
    it = God->pool().proxy().byKindBegin(ResKind::package),
    e  = God->pool().proxy().byKindEnd(ResKind::package);
  for (; it != e; ++it )
  {
    ui::Selectable::constPtr s = *it;

    // get the first installed object
    PoolItem installed;
    if (!s->installedEmpty())
      installed = s->installedObj();

    // show available objects
    for_(it, s->availableBegin(), s->availableEnd())
    {
      TableRow row;

      zypp::PoolItem pi = *it;
      if (installed)
      {
        if (notinst_only)
          continue;
        row << (equalNVRA(*installed.resolvable(), *pi.resolvable()) ? "i" : "v");
      }
      else
      {
        if (installed_only)
          continue;
        row << "";
      }
      row << (zypper.globalOpts().is_rug_compatible ? "" : pi->repository().info().name())
          << pi->name()
          << pi->edition().asString()
          << pi->arch().asString();

      tbl << row;
    }
  }
  if (zypper.cOpts().count("sort-by-repo") || zypper.cOpts().count("sort-by-catalog"))
    tbl.sort(1); // Repo
  else
    tbl.sort(2); // Name

  if (tbl.empty())
    zypper.out().info(_("No packages found."));
  else
    // display the result, even if --quiet specified
    cout << tbl;
}

static void list_products_xml(Zypper & zypper)
{
  bool installed_only = zypper.cOpts().count("installed-only");
  bool notinst_only = zypper.cOpts().count("uninstalled-only");

  cout << "<product-list>" << endl;
  ResPool::byKind_iterator
    it = God->pool().byKindBegin(ResKind::product),
    e  = God->pool().byKindEnd(ResKind::product);
  for (; it != e; ++it )
  {
    if (it->status().isInstalled() && notinst_only)
      continue;
    else if (!it->status().isInstalled() && installed_only)
      continue;

    Product::constPtr product = asKind<Product>(it->resolvable());
    cout << asXML(*product, it->status().isInstalled()) << endl;
  }
  cout << "</product-list>" << endl;
}

static void list_product_table(Zypper & zypper)
{
  MIL << "Going to list packages." << std::endl;

  Table tbl;
  TableHeader th;

  // translators: S for installed Status
  th << _("S");
  th << _("Name");
  th << _("Version");
  if (zypper.globalOpts().is_rug_compatible)
     // translators: product category (the rug term)
     th << _("Category");
  else
    // translators: product type (addon/base) (rug calls it Category)
    th << _("Type");
  tbl << th;

  bool installed_only = zypper.cOpts().count("installed-only");
  bool notinst_only = zypper.cOpts().count("uninstalled-only");

  ResPool::byKind_iterator
    it = God->pool().byKindBegin(ResKind::product),
    e  = God->pool().byKindEnd(ResKind::product);
  for (; it != e; ++it )
  {
    Product::constPtr product = asKind<Product>(it->resolvable());

    TableRow tr;
    if (it->status().isInstalled())
    {
      if (notinst_only)
        continue;
      tr << "i";
    }
    else
    {
      if (installed_only)
        continue;
      tr << "";
    }
    tr << product->name () << product->edition().asString();
    tr << product->type();
    tbl << tr;
  }
  tbl.sort(1); // Name

  if (tbl.empty())
    zypper.out().info(_("No products found."));
  else
    // display the result, even if --quiet specified
    cout << tbl;
}

void list_products(Zypper & zypper)
{
  if (zypper.out().type() == Out::TYPE_XML)
    list_products_xml(zypper);
  else
    list_product_table(zypper);
}


void list_what_provides(Zypper & zypper, const string & str)
{
  Capability cap = safe_parse_cap (zypper, /*kind,*/ str);
  sat::WhatProvides q(cap);

  // is there a provider for the requested capability?
  if (q.empty())
  {
    zypper.out().info(str::form(_("No providers of '%s' found."), str.c_str()));
    return;
  }

  Table t;
  invokeOnEach(q.selectableBegin(), q.selectableEnd(), FillSearchTableSolvable(t) );
  cout << t;
}

// Local Variables:
// c-basic-offset: 2
// End:
