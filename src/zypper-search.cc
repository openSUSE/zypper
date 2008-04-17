#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Algorithm.h"
#include "zypp/Patch.h"
#include "zypp/Pattern.h"
#include "zypp/Product.h"

#include "zypp/ResPoolProxy.h"

#include "zypper.h"
#include "zypper-main.h"
#include "zypper-tabulator.h"


#include "zypper-search.h"

using namespace zypp;
using namespace std;

extern ZYpp::Ptr God;


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


static string string_status (const ResStatus& rs)
{
  return rs.isInstalled () ? _("Installed"): _("Uninstalled");
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
    Patch::constPtr patch = asKind<Patch>(res);

    TableRow tr;
    tr << patch->repoInfo().name();
    tr << res->name () << res->edition ().asString();
    tr << patch->category();
    tr << string_status (it->status ());
    if (it->isBroken())
      tr <<  _("Broken");
    tbl << tr;
  }
  tbl.sort (1);                 // Name

  if (tbl.empty())
    zypper.out().info(_("No needed patches found."));
  else
    // display the result, even if --quiet specified
    cout << tbl;
}

void list_patterns(Zypper & zypper)
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

  ResPool::byKind_iterator
    it = God->pool().byKindBegin(ResKind::pattern),
    e  = God->pool().byKindEnd(ResKind::pattern);
  for (; it != e; ++it )
  {
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

void list_products(Zypper & zypper)
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
    tr << (it->isSatisfied() ? "i" : ""); 
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

// Local Variables:
// c-basic-offset: 2
// End:
