#include "Tools.h"
#include <zypp/ResObjects.h>

#include <zypp/sat/LookupAttr.h>

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  Pathname mroot( "/tmp/Bb" );
  TestSetup test( mroot, Arch_x86_64 );
  test.loadRepo( "/Local/ROOT/cache/raw/11.1-update" );
  test.loadRepo( "/Local/ROOT/cache/raw/11.0-update" );

  sat::LookupRepoAttr q( sat::SolvAttr::repositoryAddedFileProvides );
  USR << q << endl;
  USR << dump(q) << endl;
  for_( it, q.begin(), q.end() )
  {
    MIL << it << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

