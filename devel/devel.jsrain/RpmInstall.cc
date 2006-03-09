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
  ZYpp::Ptr zypp = ZYppFactory::instance().getZYpp();
  zypp->initTarget("/");
  Target_Ptr target = zypp->target();
  target->setInstallationLogfile("/tmp/instlog");
  RpmDb & rpm = target->rpmDb();
rpm.installPackage("/tmp/xxx.rpm");


  INT << "===[END]============================================" << endl;
  return 0;
}
