#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Fd.h"
#include "zypp/Pathname.h"

using namespace std;
using namespace zypp;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/******************************************************************
**
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
**
**      DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  base::Fd( "packages", O_RDONLY );

  INT << "===[END]============================================" << endl;
  return 0;
}
