#include <iostream>

#include <boost/format.hpp>

#include "zypp/ZYpp.h"
#include "zypp/base/Algorithm.h"
#include "zypp/Patch.h"
#include "zypp/Pattern.h"
#include "zypp/Product.h"
#include "zypp/PoolQuery.h"

#include "zypper.h"
#include "zypper-main.h"
#include "zypper-misc.h"
#include "zypper-tabulator.h"
#include "zypper-info.h"

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
  cout << res->description() << endl;
}

/**
 * 
 */
void printInfo(const Zypper & zypper, const ResKind & kind)
{
  ResPool pool = God->pool();

  cout << endl;

  for(vector<string>::const_iterator nameit = zypper.arguments().begin();
      nameit != zypper.arguments().end(); ++nameit )
  {

    // find the resolvable among installed
    /*
    PoolItem installed;
    for_( it, pool.byIdentBegin( kind, *nameit ),
        pool.byIdentEnd( kind, *nameit ) )
    {
      if (it->status().isInstalled()) { installed = *it; break; }
    }
*/
    // find installation candidate
//    ProvideProcess installer (ZConfig::instance().systemArchitecture(), "" /*version*/);
    PoolQuery q;
    q.addKind(kind);
    q.addAttribute(sat::SolvAttr::name, *nameit);
    /*
    invokeOnEach( pool.byIdentBegin( kind, *nameit ), 
        pool.byIdentEnd( kind, *nameit ),
        zypp::functor::functorRef<bool,const zypp::PoolItem&> (installer) );
*/
    
    //if (!installer.item) {
    if (q.empty())
    {
      // TranslatorExplanation E.g. "package 'zypper' not found."
      cout << "\n" << format(_("%s '%s' not found."))
          % kind_to_string_localized(kind, 1) % *nameit
          << endl;
    }
    else
    {
      ui::Selectable::constPtr s = *q.selectableBegin();
      // print info
      // TranslatorExplanation E.g. "Information for package zypper:"
      cout << endl << format(_("Information for %s %s:"))
          % kind_to_string_localized(kind, 1) % *nameit;

      cout << endl << endl;

      if (kind == ResTraits<Package>::kind)
        printPkgInfo(zypper, *s);
      else if (kind == ResTraits<Patch>::kind)
        printPatchInfo(zypper, *s);
      else if (kind == ResTraits<Pattern>::kind)
        printPatternInfo(zypper, *s);
      else if (kind == ResTraits<Product>::kind)
        printProductInfo(zypper, *s);
      else
        // TranslatorExplanation %s = resolvable type (package, patch, pattern, etc - untranslated).
        cout << format(_("Info for type '%s' not implemented.")) % kind << endl;
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
void printPkgInfo(const Zypper & zypper, const ui::Selectable & s)
{
  PoolItem theone = s.theObj();
  PoolItem installed = s.installedObj();
  cout << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "))
       << theone.resolvable()->repository().info().name() << endl;

  printNVA(installed ? installed.resolvable() : theone.resolvable());

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

  cout << _("Installed Size: ") << theone.resolvable()->installsize() << endl;

  printSummaryDesc(theone.resolvable());
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
void printPatchInfo(const Zypper & zypper, const ui::Selectable & s )
{
  const PoolItem & pool_item = s.theObj();
  const PoolItem & ins_pool_item = s.installedObj();
  printNVA(pool_item.resolvable());

  cout << _("Status: "); // TODO debug
  bool i = bool(ins_pool_item);
  if (pool_item.isBroken ())
    cout << (i ? _("broken"): _("satisfied"));
  cout << endl;

  Patch::constPtr patch = asKind<Patch>(pool_item.resolvable());
  cout << _("Category: ") << patch->category() << endl;
  cout << _("Created On: ") << patch->timestamp().asString() << endl;
  cout << _("Reboot Required: ") << (patch->reboot_needed() ? _("Yes") : _("No")) << endl;

  if (!zypper.globalOpts().is_rug_compatible)
    cout << _("Package Manager Restart Required") << ": ";
  else
    cout << _("Restart Required: ");
  cout << (patch->affects_pkg_manager() ? _("Yes") : _("No")) << endl;

  cout << _("Interactive: ") << (patch->interactive() ? _("Yes") : _("No")) << endl;

  printSummaryDesc(pool_item.resolvable());

  cout << _("Provides:") << endl;
  Capabilities capSet = pool_item.resolvable()->dep(zypp::Dep::PROVIDES);
  for (Capabilities::const_iterator it = capSet.begin(); it != capSet.end(); ++it) {
    // FIXME cout << it->refers().asString() << ": " << it->asString() << endl;
    cout << *it << endl;
  }

  cout << endl << _("Requires:") << endl;
  capSet = pool_item.resolvable()->dep(zypp::Dep::REQUIRES);
  for (Capabilities::const_iterator it = capSet.begin(); it != capSet.end(); ++it) {
    // FIXME cout << it->refers().asString() << ": " << it->asString() << endl;
    cout << *it << endl;
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
void printPatternInfo(const Zypper & zypper, const ui::Selectable & s)
{
  const PoolItem & pool_item = s.theObj();
  const PoolItem & ins_pool_item = s.installedObj();

  cout << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "))
       << pool_item.resolvable()->repository().info().name() << endl;

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
void printProductInfo(const Zypper & zypper, const ui::Selectable & s)
{
  const PoolItem & pool_item = s.theObj();
  const PoolItem & ins_pool_item = s.installedObj();

  cout << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "))
       << pool_item.resolvable()->repository().info().name() << endl;

  printNVA(pool_item.resolvable());

  Product::constPtr product = asKind<Product>(pool_item.resolvable());
  cout << _("Category: ") << product->type() << endl;
  cout << _("Installed: ") << (!ins_pool_item ? "No" : "Yes") << endl;
  cout << _("Short Name: ") << product->shortName() << endl;
  cout << _("Long Name: ") << product->longName() << endl;

  printSummaryDesc(pool_item.resolvable());
}

// Local Variables:
// c-basic-offset: 2
// End:
