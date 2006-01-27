#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/target/rpm/RpmDb.h"
#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/SourceManager.h"


using namespace std;
using namespace zypp;
using namespace zypp::target::rpm;

/******************************************************************
**
**
**	FUNCTION NAME : main
**	FUNCTION TYPE : int
**
**	DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  // initialize target
  ZYpp::Ptr zypp = ZYppFactory().letsTest();
  zypp->initTarget("/");
  Target_Ptr target = zypp->target();

  // initialize source
  SourceManager_Ptr mgr = SourceManager::sourceManager();
  unsigned id = mgr->addSource(Url("http://lide.suse.cz/~~jsrain/devel.jsrain"));
  Source & src = mgr->findSource(id);
  ResStore store = src.resolvables();
  RpmDb & rpm = target->rpmDb();
  for (ResStore::const_iterator it = store.begin();
       it != store.end();
       it++)
  {
    if (isKind<Package>(*it))
    {
      Package::Ptr p = dynamic_pointer_cast<Package>(*it);
      try {
        Pathname path = p->getPlainRpm();
        rpm.installPackage(path);
      }
      catch (...)
      {}
    }
  }
 

  INT << "===[END]============================================" << endl;
  return 0;
}
