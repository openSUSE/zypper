#include "Tools.h"

#include "zypp/base/Sysconfig.h"

using std::endl;
using namespace zypp;
namespace sysconfig = base::sysconfig;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  DBG << "===[START]==========================================" << endl;

  Capability f( CapFactory().parse( Resolvable::Kind( "package" ), "filesystem(foo)" ) );
  Capability e( CapFactory().filesystemEvalCap() );

  MIL << f << endl;
  MIL << e << endl;
  MIL << f.matches( e ) << endl;
  MIL << e.matches( f ) << endl;

  DBG << "===[END]============================================" << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}

