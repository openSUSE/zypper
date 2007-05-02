#if 0
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
#endif

      #include <iostream>
      #include "zypp/base/Logger.h"
      #include "zypp/base/IOStream.h"
      #include "zypp/base/InputStream.h"
      #include "zypp/ProgressData.h"

      using namespace zypp;

      // Defined in ProgressData.h:
      // Initialize ProgressData from an InputStream.
      //
      // ProgressData makeProgressData( const InputStream & input_r )
      // {
      //  ProgressData ret;
      //  ret.name( input_r.name() );
      //  if ( input_r.size() > 0 )
      //    ret.range( input_r.size() );
      //  return ret;
      // }

      void simpleParser( const InputStream & input_r,
			 const ProgressData::ReceiverFnc & fnc_r = ProgressData::ReceiverFnc() )
      {
	ProgressData ticks( makeProgressData( input_r ) );
	ticks.sendTo( fnc_r );
	ticks.toMin(); // start sending min (0)

	iostr::EachLine line( input_r );
	for( ; line; line.next() )
	{
	  /* process the line */

	  if ( ! ticks.set( input_r.stream().tellg() ) )
	    return; // user requested abort
	}

	ticks.toMax(); // take care 100% are reported on success
      }

      int main( int argc, char * argv[] )
      {
        simpleParser( "packages" );
        simpleParser( "packages.gz" );
        return ( 0 );
      }
