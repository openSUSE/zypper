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
    cerr << "Usage: " << appname << "[OPTIONS] URL..." << endl;
    cerr << "  Load repos from URL to test system below /tmp/" << appname << "." << endl;
    cerr << "  --nc Do not clear an existing test system but reuse it." << endl;
    return 0;
  }

  Pathname mroot( "/tmp/"+appname );
  if ( argc && argv[0] == std::string("--nc") )
  {
    --argc;
    ++argv;
  }
  else
  {
    filesystem::recursive_rmdir( mroot );
  }
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

