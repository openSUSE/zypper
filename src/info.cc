/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

#include <boost/format.hpp>

#include "zypp/ZYpp.h"
#include "zypp/base/Algorithm.h"
#include "zypp/Package.h"
#include "zypp/Patch.h"
#include "zypp/Pattern.h"
#include "zypp/Product.h"
#include "zypp/PoolQuery.h"

#include "Zypper.h"
#include "main.h"
//#include "misc.h"
#include "Table.h"
#include "utils/richtext.h"
#include "utils/misc.h" // for kind_to_string_localized and string_patch_status
#include "search.h"
#include "update.h"

#include "info.h"

using namespace std;
using namespace zypp;
using boost::format;

extern ZYpp::Ptr God;


void printNVA(const ResObject::constPtr & res)
{
  cout << _("Name: ") << res->name() << endl;
  cout << _("Version: ") << res->edition().asString() << endl;
  cout << _("Arch: ") << res->arch().asString() << endl;
  cout << _("Vendor: ") << res->vendor() << endl;
}

void printSummaryDesc(const ResObject::constPtr & res)
{
  cout << _("Summary: ") << res->summary() << endl;
  cout << _("Description: ") << endl;
  const string& s = res->description();
  if (s.find("DT:Rich")!=s.npos){
    string ns = processRichText(s);
    cout << ns << endl;
  }
  else
  {
    cout << s << endl;
  }
}

/**
 *
 */
void printInfo(Zypper & zypper, const ResKind & kind)
{
  ResPool pool = God->pool();

  cout << endl;

  for(vector<string>::const_iterator nameit = zypper.arguments().begin();
      nameit != zypper.arguments().end(); ++nameit )
  {
    PoolQuery q;
    q.addKind(kind);
    q.addAttribute(sat::SolvAttr::name, *nameit);
    q.setMatchExact();

    if (q.empty())
    {
      // TranslatorExplanation E.g. "package 'zypper' not found."
      //! \todo use a separate string for each kind so that it is translatable.
      cout << "\n" << format(_("%s '%s' not found."))
          % kind_to_string_localized(kind, 1) % *nameit
          << endl;
    }
    else
    {
      ui::Selectable::constPtr s = *q.selectableBegin();
      // print info
      // TranslatorExplanation E.g. "Information for package zypper:"

      if (zypper.out().type() != Out::TYPE_XML)
      {
        cout << endl << format(_("Information for %s %s:"))
            % kind_to_string_localized(kind, 1) % *nameit;

        cout << endl << endl;
      }

      if (kind == ResKind::package)
        printPkgInfo(zypper, *s);
      else if (kind == ResKind::patch)
        printPatchInfo(zypper, *s);
      else if (kind == ResKind::pattern)
        printPatternInfo(zypper, *s);
      else if (kind == ResKind::product)
        printProductInfo(zypper, *s);
      else
        // TranslatorExplanation %s = resolvable type (package, patch, pattern, etc - untranslated).
        zypper.out().info(
          boost::str(format(_("Info for type '%s' not implemented.")) % kind));
    }
  }

  if (false)
  {
    string s00 = _("None");
    string s0 = _("Requires");
    string s1 = _("Provides");
    string s2 = _("Conflicts");
    string s3 = _("Obsoletes");
    // translators: package requirements table header
    string s4 = _("Requirement");
    // translators: package requirements table header
    string s5 = _("Provided By");
    // translators: package conflicts table header
    string s6 = _("Conflict");
  }
}

static void printRequires(const PoolItem & pi)
{
  cout << _("Requires:") << endl;
  std::list<Capability> capList = std::list<Capability>(pi->prerequires().begin(), pi->prerequires().end());
  capList.assign(pi->requires().begin(), pi->requires().end());
  for (std::list<Capability>::const_iterator it = capList.begin(); it != capList.end(); ++it)
  {
    cout << *it << endl;
  }
}

static void printRecommends(const PoolItem & pi)
{
  cout << _("Recommends:") << endl;
  Capabilities capSet = pi->recommends();
  for (Capabilities::const_iterator it = capSet.begin(); it != capSet.end(); ++it)
  {
    cout << *it << endl;
  }
}

/**
 * Print package information.
 * <p>
 * Generates output like this:
<pre>
Catalog: system
Name: gvim
Version: 6.4.6-19
Arch: x86_64
Installed: Yes
Status: up-to-date
Installed Size: 2881221
Summary: A GUI for Vi
Description: Start: /usr/X11R6/bin/gvim

Copy and modify /usr/share/vim/current/gvimrc to ~/.gvimrc if needed.
</pre>
 *
 */
void printPkgInfo(Zypper & zypper, const ui::Selectable & s)
{
  PoolItem installed = s.installedObj();
  PoolItem theone = s.updateCandidateObj();
  if (!theone)
    theone = installed;
  Package::constPtr pkg = asKind<Package>(theone.resolvable());

  cout << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "))
       << (zypper.config().show_alias ?
           theone.resolvable()->repository().info().alias() :
           theone.resolvable()->repository().info().name()) << endl;

  printNVA(theone.resolvable());

  // if running on SUSE Linux Enterprise, report unsupported packages
  Product::constPtr platform = God->target()->baseProduct();
  if (platform && platform->name().find("SUSE_SLE") != string::npos)
    cout << _("Support Level: ") << asUserString(pkg->vendorSupport()) << endl;

  cout << _("Installed: ") << (installed ? _("Yes") : _("No")) << endl;

  //! \todo fix this - arch?
  cout << _("Status: ");
  if (installed &&
      installed.resolvable()->edition() >= theone.resolvable()->edition())
  {
    cout << _("up-to-date") << endl;
  }
  else if (installed)
  {
    cout << str::form(_("out-of-date (version %s installed)"),
        installed.resolvable()->edition().asString().c_str())
      << endl;
  }
  else
    cout << _("not installed") << endl;

  cout << _("Installed Size: ") << theone.resolvable()->installSize() << endl;

  printSummaryDesc(theone.resolvable());

  bool requires = zypper.cOpts().count("requires");
  bool recommends = zypper.cOpts().count("recommends");

  if (requires)
    printRequires(theone);

  if (recommends)
  {
    if (requires)
      cout << endl; // visual separator

    printRecommends(theone);
  }
}

/**
 * Print patch information.
 * <p>
 * Generates output like this:
 * <pre>
Name: xv
Version: 1448-0
Arch: noarch
Status: Satisfied
Category: recommended
Created On: 5/31/2006 2:34:37 AM
Reboot Required: No
Package Manager Restart Required: No
Interactive: No
Summary: XV can not grab in KDE
Description: XV can not grab in KDE
Provides:
patch: xv = 1448-0

Requires:
atom: xv = 3.10a-1091.2
</pre>
 *
 */
void printPatchInfo(Zypper & zypper, const ui::Selectable & s )
{
  const PoolItem & pool_item = s.theObj();
  printNVA(pool_item.resolvable());

  cout << _("Status: ") << string_patch_status(pool_item) << endl;

  Patch::constPtr patch = asKind<Patch>(pool_item.resolvable());
  cout << _("Category: ") << patch->category() << endl;
  cout << _("Created On: ") << patch->timestamp().asString() << endl;
  cout << _("Reboot Required: ") << (patch->rebootSuggested() ? _("Yes") : _("No")) << endl;

  if (!zypper.globalOpts().is_rug_compatible)
    cout << _("Package Manager Restart Required") << ": ";
  else
    cout << _("Restart Required: ");
  cout << (patch->restartSuggested() ? _("Yes") : _("No")) << endl;

  Patch::InteractiveFlags ignoreFlags = Patch::NoFlags;
  if (zypper.globalOpts().reboot_req_non_interactive)
    ignoreFlags |= Patch::Reboot;

  cout << _("Interactive: ") << (patch->interactiveWhenIgnoring(ignoreFlags) ? _("Yes") : _("No")) << endl;

  printSummaryDesc(pool_item.resolvable());

  cout << _("Provides:") << endl;
  Capabilities capSet = pool_item.resolvable()->dep(zypp::Dep::PROVIDES);
  // WhatProvides can be used here. The result can be represented as a table of
  // a "Capability" (it->asString()) | "Provided By" (WhatProvides(c))
  for (Capabilities::const_iterator it = capSet.begin(); it != capSet.end(); ++it)
    cout << *it << endl;

  cout << endl << _("Conflicts:") << endl;
  capSet = pool_item.resolvable()->dep(zypp::Dep::CONFLICTS);
  for (Capabilities::const_iterator it = capSet.begin(); it != capSet.end(); ++it)
    cout << *it << endl;

  if (zypper.cOpts().count("requires"))
  {
    cout << endl; // visual separator
    printRequires(pool_item);
  }

  if (zypper.cOpts().count("recommends"))
  {
    cout << endl; // visual separator
    printRecommends(pool_item);
  }
}

static string string_weak_status(const ResStatus & rs)
{
  if (rs.isRecommended())
    return _("Recommended");
  if (rs.isSuggested())
    return _("Suggested");
  return "";
}

/**
 * Print pattern information.
 * <p>
 * Generates output like this:
<pre>
Information for pattern sw_management:

Catalog: factory
Name: sw_management
Version: 11.0-2
Arch: x86_64
Installed: Yes
Summary: Software Management
Description:
This pattern provides a graphical application and a command line tool for keeping your system up to date.
</pre>
 *
 */
void printPatternInfo(Zypper & zypper, const ui::Selectable & s)
{
  const PoolItem & pool_item = s.theObj();

  cout << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "))
       << (zypper.config().show_alias ?
           pool_item.resolvable()->repository().info().alias() :
           pool_item.resolvable()->repository().info().name()) << endl;

  printNVA(pool_item.resolvable());

  cout << _("Installed: ") << (pool_item.isSatisfied() ? _("Yes") : _("No")) << endl;

  printSummaryDesc(pool_item.resolvable());

  if (zypper.globalOpts().is_rug_compatible)
    return;

  // show contents
  Table t;
  TableHeader th;
  th << _("S") << _("Name") << _("Type") << _("Dependency");
  t << th;

  //God->resolver()->solve();

  Pattern::constPtr pattern = asKind<Pattern>(pool_item.resolvable());
  Pattern::Contents contents = pattern->contents();
  for_(sit, contents.selectableBegin(), contents.selectableEnd())
  {
    const ui::Selectable & s = **sit;
    TableRow tr;

    tr << (s.installedEmpty() ? "" : "i");
    tr << s.name() << s.kind().asString() << string_weak_status(s.theObj().status());

    t << tr;
  }

  cout << _("Contents") << ":";
  if (t.empty())
    cout << " " << _("(empty)") << endl;
  else
    cout << endl << endl << t;
}

/**
 * Print product information.
 * <p>
 * Generates output like this:
<pre>
Information for product openSUSE-factory:

Repository: factory
Name: openSUSE-factory
Version: 11.0
Arch: x86_64
Category: base
Installed: No
Summary: openSUSE FACTORY 11.0
Description:
</pre>
 *
 */
void printProductInfo(Zypper & zypper, const ui::Selectable & s)
{
  const PoolItem & pool_item = s.theObj(); // should be the only one

  if (zypper.out().type() == Out::TYPE_XML)
  {
    Product::constPtr pp = asKind<Product>(pool_item.resolvable());
    cout
      << asXML(*pp, pool_item.status().isInstalled())
      << endl;
  }
  else
  {
    cout << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "))
         << (zypper.config().show_alias ?
             pool_item.resolvable()->repository().info().alias() :
             pool_item.resolvable()->repository().info().name()) << endl;

    printNVA(pool_item.resolvable());

    Product::constPtr product = asKind<Product>(pool_item.resolvable());
    cout << _("Is Base")   << ": "
      << (product->isTargetDistribution()  ? _("Yes") : _("No")) << endl;
    cout << _("Flavor")     << ": "
      << product->flavor() << endl;
    cout << _("Installed")  << ": "
      << (pool_item.status().isInstalled() ? _("Yes") : _("No")) << endl;
    cout << _("Short Name")
      << ": " << product->shortName() << endl;
    printSummaryDesc(pool_item.resolvable());
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
