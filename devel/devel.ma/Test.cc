#include "Tools.h"
#include <zypp/ResObjects.h>

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  // https://bugzilla.novell.com/show_bug.cgi?id=442200
  Pathname mroot( "/tmp/Bb" );
  TestSetup test( mroot, Arch_x86_64 );
  test.loadRepos();

  ResPool pool( test.pool() );
  for_( it, pool.byKindBegin<Patch>(), pool.byKindEnd<Patch>() )
  {
    Patch::constPtr p( (*it)->asKind<Patch>() );
    MIL << p << endl;
    Patch::Contents contents( p->contents() );
    DBG << contents << endl;
  }


  INT << "===[END]============================================" << endl << endl;
  return 0;
}

