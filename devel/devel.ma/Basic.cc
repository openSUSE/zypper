#include "Tools.h"
#include "Tools.h"

#include <iostream>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/String.h>
#include <zypp/base/SerialNumber.h>
#include <zypp/PathInfo.h>
#include <zypp/TmpPath.h>
#include "zypp/ResPoolProxy.h"

using namespace std;
using namespace zypp;
using namespace zypp::ui;

bool chst( Selectable::Ptr & sel, Status status )
{
  DBG << "+++ " << sel << endl;
  Status ostatus( sel->status() );
  bool res = sel->set_status( status );
  (res?MIL:WAR) << ostatus << " -> " << status << " ==>(" << res << ") " << sel->status() << endl;
  DBG << "--- " << sel << endl;
  return res;
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  const char *const lines[] = {
    "@ package",
    "@ installed",
    "- foo 1 1 i686",
    "@ available",
    "- foo 2 1 i686",
    "@ fin"
  };

  debug::addPool( lines, lines+(sizeof(lines)/sizeof(const char *const)) );

  ResPool      pool( getZYpp()->pool() );
  ResPoolProxy uipool( getZYpp()->poolProxy() );

  USR << pool << endl;
  USR << uipool << endl;

  //for_each( pool.begin(), pool.end(), Print() );

  Selectable::Ptr sel( *uipool.byKindBegin<Package>() );

/*    enum Status
    {
      S_Protected,           // Keep this unmodified ( have installedObj && S_Protected )
      S_Taboo,               // Keep this unmodified ( have no installedObj && S_Taboo)
      // requested by user:
      S_Del,                 // delete  installedObj ( clears S_Protected if set )
      S_Update,              // install candidateObj ( have installedObj, clears S_Protected if set )
      S_Install,             // install candidateObj ( have no installedObj, clears S_Taboo if set )
      // not requested by user:
      S_AutoDel,             // delete  installedObj
      S_AutoUpdate,          // install candidateObj ( have installedObj )
      S_AutoInstall,         // install candidateObj ( have no installedObj )
      // no modification:
      S_KeepInstalled,       // no modification      ( have installedObj && !S_Protected, clears S_Protected if set )
      S_NoInst,              // no modification      ( have no installedObj && !S_Taboo, clears S_Taboo if set )
    };
*/
  MIL << sel << endl;
  chst( sel, ui::S_Update );
  chst( sel, ui::S_Install );
  chst( sel, ui::S_Protected );
  chst( sel, ui::S_KeepInstalled );

  INT << "===[END]============================================" << endl
      << endl;
  return 0;
}
