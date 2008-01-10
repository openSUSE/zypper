#include <iostream>
#include <zypp/base/LogTools.h>
#include <zypp/base/Easy.h>
#include <zypp/sat/Pool.h>

using std::endl;
using std::cout;
using namespace zypp;

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, const char * argv[] )
{
  --argc; ++argv; // skip arg 0
  sat::Pool satpool( sat::Pool::instance() );

  for ( ; argc;  --argc, ++argv )
  {
    cout << "Read: " << argv[0] << endl;
    satpool.addRepoSolv( argv[0] );
  }

  cout << "Done: " << satpool << endl;

  if ( getenv("VERBOSE") )
  {
    for_( it, satpool.solvablesBegin(), satpool.solvablesEnd() )
    {
      cout << dump(*it) << endl;
    }
  }

  return 0;
}

