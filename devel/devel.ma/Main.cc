#include <iostream>
#include <ctime>

#include <fstream>
#include <list>
#include <string>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/PtrTypes.h>

#include <zypp/CapFactory.h>
#include <zypp/CapSet.h>

using namespace std;
using namespace zypp;

namespace zypp
{
  struct foo : public base::ReferenceCounted
  {
  };
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  foo f;
  SEC << f << endl;

  INT << "===[END]============================================" << endl;
  return 0;
}
