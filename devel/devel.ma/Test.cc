#include "Tools.h"

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  // https://bugzilla.novell.com/show_bug.cgi?id=442200

  Pathname mroot( "/tmp/ToolScanRepos" );
  TestSetup test( mroot, Arch_x86_64 );
  test.loadRepos();


  INT << "===[END]============================================" << endl << endl;
  return 0;
}

