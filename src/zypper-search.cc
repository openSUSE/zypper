#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Algorithm.h"
#include "zypp/Patch.h"
#include "zypp/Pattern.h"

#include "zypper.h"
#include "zypper-main.h"
#include "zypper-tabulator.h"


#include "zypper-search.h"

using namespace zypp;
using namespace std;

extern ZYpp::Ptr God;

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

// Local Variables:
// c-basic-offset: 2
// End:
