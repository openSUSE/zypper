#include "Tools.h"

#include <zypp/ResPool.h>
#include <zypp/ResObjects.h>
#include <zypp/PoolQuery.h>

#include <zypp/misc/CheckAccessDeleted.h>

///////////////////////////////////////////////////////////////////

Pathname sysRoot( getenv("SYSROOT") ? getenv("SYSROOT") : "/Local/ROOT" );

///////////////////////////////////////////////////////////////////

int main( int argc, char * argv[] )
try {
  --argc;
  ++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  if ( argc )
  {
    unsetenv("SYSROOT");
    sysRoot = Pathname(*argv);
    setenv( "ZYPP_CONF", (sysRoot/"etc/zypp.conf").c_str(), true );
  }
  ZConfig::instance();
  //TestSetup::LoadSystemAt( sysRoot );
  ///////////////////////////////////////////////////////////////////
  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );
  ///////////////////////////////////////////////////////////////////
  dumpRange( USR, satpool.reposBegin(), satpool.reposEnd() ) << endl;
  USR << "pool: " << pool << endl;
  ///////////////////////////////////////////////////////////////////

  {
    Measure x("x");
    CheckAccessDeleted checker;
    USR << checker << endl;
  }
  SEC << CheckAccessDeleted::findService( "syslog" ) << endl;
  SEC << CheckAccessDeleted::findService( "syslogd" ) << endl;
  SEC << CheckAccessDeleted::findService( "ssh" ) << endl;
  SEC << CheckAccessDeleted::findService( "sshd" ) << endl;
  SEC << CheckAccessDeleted::findService( 3844 ) << endl;
  ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
catch ( const Exception & exp )
{
  INT << exp << endl << exp.historyAsString();
}
catch (...)
{}

