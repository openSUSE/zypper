#include "zypper-info.h"

#include <iostream>

#include <zypp/ZYpp.h>
#include <zypp/base/Algorithm.h>
#include <zypp/Patch.h>

#include "zmart.h"
#include "zmart-misc.h"

using namespace std;
using namespace zypp;

extern ZYpp::Ptr God;
extern Settings gSettings;

/**
 * 
 */
void printInfo(const string & command, const vector<string> & arguments) {
  Resolvable::Kind kind;
  if (command == "info" || command == "if") kind =  ResTraits<Package>::kind;
  else if (command == "patch-info") kind = ResTraits<Patch>::kind;

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
      cout << "\n" << kind.asString() << " " << *nameit << " not found." << endl;
    }
    else {
      // print info
      cout << "\nInformation for " << kind.asString() << " " << *nameit << ":\n\n";

      if (command == "info" || command == "if")
        printPkgInfo(installer.item,installed);
      else if (command == "patch-info")
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

  cout << "Catalog: " << pool_item.resolvable()->source().alias() << endl;
  cout << "Name: " << pool_item.resolvable()->name() << endl;
  cout << "Version: " << pool_item.resolvable()->edition().asString() << endl;
  cout << "Arch: " << pool_item.resolvable()->arch().asString() << endl;
  cout << "Installed: " << (!ins_pool_item ? "No" : "Yes") << endl;

  cout << "Status: ";
  if (ins_pool_item &&
      ins_pool_item.resolvable()->edition() >= pool_item.resolvable()->edition())
    cout << "up-to-date" << endl;
  else if (ins_pool_item) {
    cout << "out-of-date (version " << ins_pool_item.resolvable()->edition()
      << " installed) " << endl;
  }
  else
    cout << "not installed" << endl;

  cout << "Installed Size: " << pool_item.resolvable()->size().asString() << endl;
  cout << "Summary: " << pool_item.resolvable()->summary() << endl;
  cout << "Description: " << endl;

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
  cout << "Name: " << pool_item.resolvable()->name() << endl;
  cout << "Version: " << pool_item.resolvable()->edition().asString() << endl;
  cout << "Arch: " << pool_item.resolvable()->arch().asString() << endl;
  
  cout << "Status: "; // TODO debug
  bool i = ins_pool_item ? true : false;
  if (pool_item.status().isUndetermined ())
    cout << (i ? "Installed": "Uninstalled");
  else if (pool_item.status().isEstablishedUneeded ())
    cout << (i ? "No Longer Applicable": "Not Applicable");
  else if (pool_item.status().isEstablishedSatisfied ())
    cout << (i ? "Applied": "Not Needed");
  else if (pool_item.status().isEstablishedIncomplete ())
    cout << (i ? "Broken": "Needed");
  cout << endl;

  Patch::constPtr patch = asKind<Patch>(pool_item.resolvable());
  cout << "Category: " << patch->category() << endl;
  cout << "Created On: " << patch->timestamp().asString() << endl;
  cout << "Reboot Required: " << (patch->reboot_needed() ? "Yes" : "No") << endl;
  
  if (!gSettings.is_rug_compatible)
    cout << "Package Manager ";
  cout << "Restart Required: ";
  cout << (patch->affects_pkg_manager() ? "Yes" : "No") << endl;
  
  cout << "Interactive: " << (patch->interactive() ? "Yes" : "No") << endl;
  cout << "Summary: " << pool_item.resolvable()->summary() << endl;
  cout << "Description: " << pool_item.resolvable()->description() << endl;

  cout << "Provides:" << endl;
  CapSet capSet = pool_item.resolvable()->dep(zypp::Dep::PROVIDES);
  for (CapSet::const_iterator it = capSet.begin(); it != capSet.end(); ++it) {
    cout << it->refers().asString() << ": " << it->asString() << endl;
  }

  cout << endl << "Requires:" << endl;
  capSet = pool_item.resolvable()->dep(zypp::Dep::REQUIRES);
  for (CapSet::const_iterator it = capSet.begin(); it != capSet.end(); ++it) {
    cout << it->refers().asString() << ": " << it->asString() << endl;
  }
}
// Local Variables:
// c-basic-offset: 2
// End:
