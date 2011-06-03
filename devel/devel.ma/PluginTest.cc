#include "Tools.h"

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <zypp/base/PtrTypes.h>
#include <zypp/base/Exception.h>
#include <zypp/base/Gettext.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/Debug.h>
#include <zypp/base/Functional.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/InputStream.h>
#include <zypp/base/ProvideNumericId.h>
#include <zypp/base/Flags.h>
#include <zypp/AutoDispose.h>

#include <zypp/PluginScript.h>
#include <zypp/PathInfo.h>


using namespace std;
using namespace zypp;
using namespace zypp::functor;
using namespace zypp::ui;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( getenv("SYSROOT") ? getenv("SYSROOT") : "/Local/ROOT" );

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

static PluginScript scr( "/bin/cat" );

void repeat( const PluginFrame & f )
{
  if ( ! scr.isOpen() )
    scr.open();
  MIL << "--> " << f << endl;
  scr.send( f );
  PluginFrame r( scr.receive() );
  MIL << "<-- " << r << endl;
  if ( r != f )
    ERR << "send/receive does not match." << endl;
}

void send( const PluginFrame & f )
{
  if ( ! scr.isOpen() )
    scr.open();
  MIL << "--> " << f << endl;
  scr.send( f );
}


PluginFrame receive()
{
  if ( ! scr.isOpen() )
    scr.open();
  PluginFrame r( scr.receive() );
  MIL << "<-- " << r << endl;
  return r;
}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#define DOLOG(C) USR << #C << ": " << endl; C;

namespace zypp {
  namespace target {
    void testCommitPlugins( const Pathname & path_r );
  }
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
try {
  --argc,++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  //////////////////////////////////////////////////////////////////

  zypp::target::testCommitPlugins( "/tmp/pltest" );

  if ( 0 )
  {
  Pathname script( "PluginTest.py" );
  PluginScript plugin( script );
  USR << plugin << endl;

  DOLOG( plugin.open() );

  DOLOG( plugin.send( PluginFrame( "PLUGINBEGIN" ) ) );

  PluginFrame ret;
  DOLOG( ret = plugin.receive() );
  MIL << ret << endl;

  DOLOG( plugin.send( PluginFrame( "PLUGINEND" ) ) );
  DOLOG( ret = plugin.receive() );
  MIL << ret << endl;

  DOLOG( plugin.close() );
  }

  if ( 0 ) {
    Pathname script( ZConfig::instance().pluginsPath()/"system/spacewalkx" );
    if ( PathInfo( script ).isX() )
      try {
	PluginScript spacewalk( script );
	spacewalk.open();

	PluginFrame notify( "PACKAGESETCHANGED" );
	spacewalk.send( notify );

	PluginFrame ret( spacewalk.receive() );
	MIL << ret << endl;
	if ( ret.command() == "ERROR" )
	  ret.writeTo( WAR ) << endl;
      }
      catch ( const Exception & excpt )
      {
	WAR << excpt.asUserHistory() << endl;
      }
  }

  if ( 0 ) {
    Measure x( "" );
    PluginFrame f( "a" );
    f.setBody( std::string( 1020, '0' ) );
    if ( ! scr.isOpen() )
      scr.open();
    for ( unsigned i = 1; true; ++i )
    {
      try {
	MIL << "Receiving " << i << endl;
	PluginFrame ret( receive() );
      }
      catch ( const PluginScriptTimeout & excpt )
      {
	ERR << excpt << endl;
	scr.send( f );
      }
      catch ( const PluginScriptDiedUnexpectedly & excpt )
      {
	ERR << excpt << endl;
	ERR << scr << endl;
	scr.close();
	break;
      }
    }
  }

  if ( 0 ) {
    Measure x( "" );
    PluginFrame f( "a" );
    f.setBody( std::string( 10200, '0' ) );
    for ( unsigned i = 1; true; ++i )
    {
      try {
	MIL << "Sending " << i << endl;
	send( f );
      }
      catch ( const PluginScriptTimeout & excpt )
      {
	ERR << excpt << endl;
	::kill( scr.getPid(), SIGKILL);
      }
      catch ( const PluginScriptDiedUnexpectedly & excpt )
      {
	ERR << excpt << endl;
	ERR << scr << endl;
	scr.close();
	break;
      }
    }
  }

  //////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
catch ( const Exception & exp )
{
  INT << exp << endl << exp.historyAsString();
  throw;
}
catch (...)
{
  throw;
}


