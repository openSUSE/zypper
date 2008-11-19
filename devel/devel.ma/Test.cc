#include "Tools.h"
#include <zypp/ResObjects.h>

#include "zypp/IdString.h"
#include "zypp/Glob.h"

using filesystem::Glob;

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

  //test.loadRepos();
  test.loadTestcaseRepos( "/suse/ma/BUGS/153548/YaST2/solverTestcase" );

  sat::Pool satpool( test.satpool() );
  for_( it, satpool.reposBegin(), satpool.reposEnd() )
  {
    MIL << *it << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;

  ResPool pool( test.pool() );
  for_( it, pool.byKindBegin<Product>(), pool.byKindEnd<Product>() )
  {
    MIL << *it << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

