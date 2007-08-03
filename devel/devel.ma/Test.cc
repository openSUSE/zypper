#include "Tools.h"
#include "ExplicitMap.h"
#include <boost/call_traits.hpp>

#include <iostream>
#include <fstream>
#include <map>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include "zypp/base/Exception.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/DefaultIntegral.h"
#include <zypp/base/Function.h>
#include <zypp/base/Iterator.h>

#include <zypp/Pathname.h>
#include <zypp/Edition.h>
#include <zypp/CheckSum.h>
#include <zypp/Date.h>

#include "zypp/cache/CacheStore.h"
#include <zypp/DiskUsage.h>

using namespace std;
using namespace zypp;

///////////////////////////////////////////////////////////////////


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  DiskUsage a;
  MIL << a << endl;


  INT << "===[END]============================================" << endl << endl;
  return 0;
}

