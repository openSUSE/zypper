#include "Tools.h"

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;
  std::string appname( Pathname::basename( argv[0] ) );
  --argc;
  ++argv;

  if ( ! argc )
  {
    cerr << "Usage: " << appname << " URL..." << endl;
    cerr << "  Load repos from URL to test system below /tmp/" << appname << endl;
    return 0;
  }

  Pathname mroot( "/tmp/"+appname );
  filesystem::recursive_rmdir( mroot );
  filesystem::assert_dir( mroot );
  TestSetup test( mroot, Arch_x86_64 );

  while ( argc )
  {
    test.loadRepo( Url( argv[0] ) );
    --argc;
    ++argv;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

