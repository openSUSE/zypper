#define INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "../tests/lib/TestSetup.h"
#undef  INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "argparse.h"

#include <iostream>
#include <zypp/DiskUsageCounter.h>

using std::cout;
using std::cerr;
using std::endl;

static std::string appname { "NO_NAME" };

int errexit( const std::string & msg_r = std::string(), int exit_r = 100 )
{
  if ( ! msg_r.empty() )
    cerr << endl << appname << ": ERR: " << msg_r << endl << endl;
  return exit_r;
}

int usage( const argparse::Options & options_r, int return_r = 0 )
{
  cerr << "USAGE: " << appname << " [OPTION]... [ARGS]..." << endl;
  cerr << "    Print default mountpoint set for disk usage computation." << endl;
  cerr << options_r << endl;
  return return_r;
}

int main( int argc, char * argv[] )
{
  appname = Pathname::basename( argv[0] );

  std::string sysRoot { "/" };

  argparse::Options options;
  options.add()
    ( "help,h",	"Print help and exit." )
    ( "root",	"Use the system located below ROOTDIR.", argparse::Option::Arg::required )
    ;
  auto result = options.parse( argc, argv );

  if ( result.count( "help" ) )
    return usage( options );

  if ( result.count( "root" ) )
    sysRoot = result["root"].arg();

  // go...
  cout << "DiskUsageCounter: relevant mount points detected below '" << sysRoot << "':" << endl;
  cout << DiskUsageCounter::detectMountPoints( sysRoot ) << endl;

  return 0;
}
