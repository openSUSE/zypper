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
  , _show_alias(Zypper::instance()->config().show_alias)
{
  Zypper & zypper = *Zypper::instance();
  if (zypper.cOpts().find("repo") != zypper.cOpts().end())
  {
    list<RepoInfo> & repos = zypper.runtimeData().repos;
    for_(it, repos.begin(), repos.end())
      _repos.insert(it->alias());
  }

  TableHeader header;

  //
  // *** CAUTION: It's a mess, but adding/changing colums here requires
  //              adapting OutXML::searchResult !
  //
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

    // hide patterns with user visible flag not set (bnc #538152)
    if (pi->kind() == zypp::ResKind::pattern)
    {
      Pattern::constPtr ptrn = asKind<Pattern>(pi.resolvable());
      if (ptrn && !ptrn->userVisible())
        continue;
    }

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
          if (identical(*instit, pi) &&
               (_repos.empty() ||
                _repos.find(pi.resolvable()->repoInfo().alias())!=_repos.end()))
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
        << (_show_alias ?
            pi->repository().info().alias() : pi->repository().info().name())
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
        << (_show_alias ?
            pi->repository().info().alias() : pi->repository().info().name());
    }

    *_table << row;
  }

  // --uninstalled
  if (_inst_notinst == false)
    return true;
  // --repo => we only want the repo resolvables, not @System (bnc #467106)
  if (!_repos.empty())
    return true;

  // now list the system packages
  for_(it, s->installedBegin(), s->installedEnd())
  {
    // show installed objects only if there is no counterpart in repos
    bool has_counterpart = false;
    for_(ait, s->availableBegin(), s->availableEnd())
      if (identical(*it, *ait))
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

    *_table << row;
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
  Zypper & zypper = *Zypper::instance();
  if (zypper.cOpts().find("repo") != zypper.cOpts().end())
  {
    list<RepoInfo> & repos = zypper.runtimeData().repos;
    for_(it, repos.begin(), repos.end())
      _repos.insert(it->alias());
  }

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
  // hide patterns with user visible flag not set (bnc #538152)
  if (s->kind() == zypp::ResKind::pattern)
  {
    Pattern::constPtr ptrn = s->candidateAsKind<Pattern>();
    if (ptrn && !ptrn->userVisible())
      return true;
  }

  TableRow row;

  // whether to show the solvable as 'installed'
  bool installed = false;

  if (zypp::traits::isPseudoInstalled(s->kind()))
    installed = s->theObj().isSatisfied();
  // check for installed counterpart in one of specified repos (bnc #467106)
  else if (!_repos.empty())
  {
    for_(ait, s->availableBegin(), s->availableEnd())
      for_(iit, s->installedBegin(), s->installedEnd())
        if (identical(*ait, *iit) &&
            _repos.find(ait->resolvable()->repoInfo().alias()) != _repos.end())
        {
          installed = true;
          break;
        }
  }
  // if no --repo is specified, we don't care where does the installed package
  // come from
  else
    installed = !s->installedEmpty();


  if (s->kind() != zypp::ResKind::srcpackage)
  {
    if (installed)
    {
      // not-installed only
      if (inst_notinst == false)
        return true;
      row << "i";
    }
    // this happens if the solvable has installed objects, but no counterpart
    // of them in specified repos
    else if (s->hasInstalledObj())
    {
      // not-installed only
      if (inst_notinst == true)
        return true;
      row << "v";
    }
    else
    {
      // installed only
      if (inst_notinst == true)
        return true;
      row << "";
    }
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
  , _show_alias(Zypper::instance()->config().show_alias)
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
    << (_show_alias ?
        pi->repository().info().alias() : pi->repository().info().name())
    << pi->name()
    << pi->edition().asString()
    << patch->category()
    << string_patch_status(pi);

  *_table << row;

  return true;
}


/*
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
*/

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
    // hide patterns with user visible flag not set (bnc #538152)
    if (!pattern->userVisible())
      continue;

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
        row << (identical(installed, pi) ? "i" : "v");
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

// common product_table_row data
static void add_product_table_row( Zypper & zypper, TableRow & tr,  const Product::constPtr & product )
{
  // repository
  if (!zypper.globalOpts().is_rug_compatible)
    tr << product->repoInfo().name();
  // internal (unix) name
  tr << product->name ();
  // full name (bnc #589333)
  tr << product->summary();
  // version
  tr << product->edition().asString();
  if (zypper.globalOpts().is_rug_compatible)
    // rug 'Category'
    tr << (product->isTargetDistribution() ? "base" : "");
  else
  {
    // architecture
    tr << product->arch().asString();
    // is base
    tr << (product->isTargetDistribution() ? _("Yes") : _("No"));
  }
}

static void list_product_table(Zypper & zypper)
{
  MIL << "Going to list products." << std::endl;

  Table tbl;
  TableHeader th;

  // translators: S for installed Status
  th << _("S");
  if (!zypper.globalOpts().is_rug_compatible)
    th << _("Repository");
  // translators: used in products. Internal Name is the unix name of the
  // product whereas simply Name is the official full name of the product.
  th << _("Internal Name");
  th << _("Name");
  th << _("Version");
  if (zypper.globalOpts().is_rug_compatible)
     // translators: product category (base/addon), the rug term
     th << _("Category");
  else
  {
    th << _("Arch");
    th << _("Is Base");
  }
  tbl << th;

  bool installed_only = zypper.cOpts().count("installed-only");
  bool notinst_only = zypper.cOpts().count("uninstalled-only");

  ResPoolProxy::const_iterator
      it = God->pool().proxy().byKindBegin(ResKind::product),
      e  = God->pool().proxy().byKindEnd(ResKind::product);
  for (; it != e; ++it )
  {
    ui::Selectable::constPtr s = *it;

    // get the first installed object
    PoolItem installed;
    if (!s->installedEmpty())
      installed = s->installedObj();

    bool missedInstalled = installed; // if no available hits, we need to print it

    // show available objects
    for_(it, s->availableBegin(), s->availableEnd())
    {
      Product::constPtr product = asKind<Product>(it->resolvable());
      TableRow tr;
      zypp::PoolItem pi = *it;

      if (installed)
      {
        if (identical(installed, pi))
        {
          if (notinst_only)
            continue;
          tr << "i";
          // this is needed, other isTargetDistribution would not return
          // true for the installed base product
          product = asKind<Product>(installed);
          missedInstalled = false;
        }
        else
        {
          if (installed_only)
            continue;
          tr << "v";
        }
      }
      else
      {
        if (installed_only)
          continue;
        tr << "";
      }
      add_product_table_row( zypper, tr, product );
      tbl << tr;
    }

    if ( missedInstalled ) // no available hit, we need to print it
    {
      // show installed product in ablence of an available one:
      if (notinst_only)
        continue;

      TableRow tr;
      tr << "i";
      add_product_table_row( zypper, tr, installed->asKind<Product>() );
      tbl << tr;
    }
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
  Capability cap(Capability::guessPackageSpec(str));
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
