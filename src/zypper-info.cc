#include "zypper-info.h"

#include <iostream>

#include <boost/format.hpp>

#include "zypp/ZYpp.h"
#include "zypp/base/Algorithm.h"
#include "zypp/Patch.h"

#include "zypper.h"
#include "zypper-main.h"
#include "zypper-utils.h"
#include "zypper-misc.h"

using namespace std;
using namespace zypp;
using boost::format;

extern ZYpp::Ptr God;
extern GlobalOptions gSettings;

/**
 * 
 */
void printInfo(const ZypperCommand & command, const vector<string> & arguments) {
  Resolvable::Kind kind;
  if (command == ZypperCommand::INFO) kind =  ResTraits<Package>::kind;
  else if (command == ZypperCommand::RUG_PATCH_INFO) kind = ResTraits<Patch>::kind;

  ResPool pool = God->pool();

  cout << endl;

  for(vector<string>::const_iterator nameit = arguments.begin();
      nameit != arguments.end(); ++nameit ) {

    // find the resolvable among installed 
    PoolItem installed;
    for(ResPool::byName_iterator it = pool.byNameBegin(*nameit);
        it != pool.byNameEnd(*nameit); ++it) {
      if (it->status().isInstalled()) { installed = *it; break; }
    }

    // find installation candidate
    ProvideProcess installer (God->architecture(), "" /*version*/);
    invokeOnEach(pool.byNameBegin(*nameit), pool.byNameEnd(*nameit),
      resfilter::ByKind(kind),
      zypp::functor::functorRef<bool,const zypp::PoolItem&> (installer)
      );

    if (!installer.item) {
      // TranslatorExplanation E.g. "package zypper not found."
      cout << "\n" << format(_("%s %s not found."))
          % kind_to_string_localized(kind, 1) % *nameit
          << endl;
    }
    else {
      // print info
      // TranslatorExplanation E.g. "Information for package zypper:"
      cout << endl << format(_("Information for %s %s:"))
          % kind_to_string_localized(kind, 1) % *nameit;

      cout << endl << endl;

      if (command == ZypperCommand::INFO)
        printPkgInfo(installer.item,installed);
      else if (command == ZypperCommand::RUG_PATCH_INFO)
        printPatchInfo(installer.item,installed);
    }
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
void printPkgInfo(const PoolItem & pool_item, const PoolItem & ins_pool_item) {

  cout << (gSettings.is_rug_compatible ? _("Catalog: ") : _("Repository: "))
       << pool_item.resolvable()->repository().info().name() << endl;
  cout << _("Name: ") << pool_item.resolvable()->name() << endl;
  cout << _("Version: ") << pool_item.resolvable()->edition().asString() << endl;
  cout << _("Arch: ") << pool_item.resolvable()->arch().asString() << endl;
  cout << _("Installed: ") << (!ins_pool_item ? "No" : "Yes") << endl;

  cout << _("Status: ");
  if (ins_pool_item &&
      ins_pool_item.resolvable()->edition() >= pool_item.resolvable()->edition())
    cout << _("up-to-date") << endl;
  else if (ins_pool_item) {
    cout << _("out-of-date (version ") << ins_pool_item.resolvable()->edition()
      << _(" installed) ") << endl; // TODO use sformat for this for proper translation
  }
  else
    cout << _("not installed") << endl;

  cout << _("Installed Size: ") << pool_item.resolvable()->size().asString() << endl;
  cout << _("Summary: ") << pool_item.resolvable()->summary() << endl;
  cout << _("Description: ") << endl;
  cout << pool_item.resolvable()->description() << endl;
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
void printPatchInfo(const PoolItem & pool_item, const PoolItem & ins_pool_item) {
  cout << _("Name: ") << pool_item.resolvable()->name() << endl;
  cout << _("Version: ") << pool_item.resolvable()->edition().asString() << endl;
  cout << _("Arch: ") << pool_item.resolvable()->arch().asString() << endl;
  
  cout << _("Status: "); // TODO debug
  bool i = ins_pool_item ? true : false;
  if (pool_item.status().isUndetermined ())
    cout << (i ? _("Installed"): _("Uninstalled"));
  else if (pool_item.status().isEstablishedUneeded ())
    cout << (i ? _("No Longer Applicable"): _("Not Applicable"));
  else if (pool_item.status().isEstablishedSatisfied ())
    cout << (i ? _("Applied"): _("Not Needed"));
  else if (pool_item.status().isEstablishedIncomplete ())
    cout << (i ? _("Broken"): _("Needed"));
  cout << endl;

  Patch::constPtr patch = asKind<Patch>(pool_item.resolvable());
  cout << _("Category: ") << patch->category() << endl;
  cout << _("Created On: ") << patch->timestamp().asString() << endl;
  cout << _("Reboot Required: ") << (patch->reboot_needed() ? _("Yes") : _("No")) << endl;

  if (!gSettings.is_rug_compatible)
    cout << _("Package Manager Restart Required") << ": ";
  else
    cout << _("Restart Required: ");
  cout << (patch->affects_pkg_manager() ? _("Yes") : _("No")) << endl;

  cout << _("Interactive: ") << (patch->interactive() ? _("Yes") : _("No")) << endl;
  cout << _("Summary: ") << pool_item.resolvable()->summary() << endl;
  cout << _("Description: ") << pool_item.resolvable()->description() << endl;

  cout << _("Provides:") << endl;
  CapSet capSet = pool_item.resolvable()->dep(zypp::Dep::PROVIDES);
  for (CapSet::const_iterator it = capSet.begin(); it != capSet.end(); ++it) {
    cout << it->refers().asString() << ": " << it->asString() << endl;
  }

  cout << endl << _("Requires:") << endl;
  capSet = pool_item.resolvable()->dep(zypp::Dep::REQUIRES);
  for (CapSet::const_iterator it = capSet.begin(); it != capSet.end(); ++it) {
    cout << it->refers().asString() << ": " << it->asString() << endl;
  }
}
// Local Variables:
// c-basic-offset: 2
// End:
