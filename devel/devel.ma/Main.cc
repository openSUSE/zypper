#include <iostream>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
//#include <zypp/base/Algorithm.h>
#include <zypp/SourceFactory.h>
#include <zypp/Source.h>
#include <zypp/ResStore.h>
#include <zypp/ResObject.h>
#include <zypp/pool/PoolStats.h>
#include <zypp/KeyRing.h>

using namespace std;
using namespace zypp;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( "/Local/ROOT" );

///////////////////////////////////////////////////////////////////

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "" );
  INT << "===[START]==========================================" << endl;
  --argc;
  ++argv;

  cout << "x" << endl;


  INT << "===[END]============================================" << endl << endl;
  return 0;
}

