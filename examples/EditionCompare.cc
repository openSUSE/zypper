#include <iostream>
#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>

#include <zypp/Edition.h>

using namespace std;
using namespace zypp;

inline std::string asOp( int res )
{ return res ? ( res < 0 ? "  <   " : "  >   " ) : "  ==  "; }

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  --argc;
  ++argv;

  Edition lhs;
  Edition rhs;

  if ( argc >= 1 )
    lhs = Edition( argv[0] );
  if ( argc >= 2 )
    rhs = Edition( argv[1] );

  cerr << "compare: " << lhs << asOp( lhs.compare( rhs ) ) << rhs << endl;
  cerr << "match:   " << lhs << asOp( lhs.match( rhs ) )   << rhs << endl;

  return 0;
}
