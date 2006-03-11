#include <iostream>
#include <list>
#include <set>

#include "Tools.h"

#include <zypp/base/LogControl.h>
#include <zypp/ResStatus.h>

using namespace std;
using namespace zypp;


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  ResStatus stat;
  MIL << stat << endl;

  DBG << stat.setSoftTransact( true, ResStatus::SOLVER, ResStatus::APPL_LOW )
      << ' ' << stat << endl;

  DBG << stat.setTransact( false, ResStatus::APPL_LOW )
      << ' ' << stat << endl;

  DBG << stat.setSoftTransact( true, ResStatus::SOLVER, ResStatus::APPL_LOW )
      << ' ' << stat << endl;

  DBG << stat.setTransact( false, ResStatus::APPL_HIGH )
      << ' ' << stat << endl;

  DBG << stat.setSoftTransact( true, ResStatus::SOLVER, ResStatus::APPL_LOW )
      << ' ' << stat << endl;


  INT << "===[END]============================================" << endl << endl;
  return 0;
}

