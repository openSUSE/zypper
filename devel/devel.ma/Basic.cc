#include "Tools.h"
#include "Tools.h"

#include <iostream>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/String.h>
#include <zypp/PathInfo.h>
#include <zypp/TmpPath.h>

using namespace std;
using namespace zypp;

void chk( ResObject::constPtr p )
{
  MIL << p << endl;
  DBG << p->deps() << endl;
}


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  ResPool pool( getZYpp()->pool() );

  const char *const lines[] = {
    "@ package",
    "@ installed",
    "- foo 1 1 i686",
    "@ provides",
    "modalias(kernel-bigsmp:pci:*provided*)",
    "@ suplements",
    "modalias(kernel-bigsmp:pci:*suplements*)",
    "@ fin"
  };

  //debug::addPool( lines, lines+(sizeof(lines)/sizeof(const char *const)) );
  debug::addPool( "/tmp/a" );
  USR << pool << endl;

  for_each( pool.begin(), pool.end(), chk );

  INT << "===[END]============================================" << endl
      << endl;
  return 0;
}
