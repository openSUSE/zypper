#define INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "zypp/../tests/lib/TestSetup.h"
#undef  INCLUDE_TESTSETUP_WITHOUT_BOOST

static std::string appname( "ToolScanRepos" );

void message( const std::string & msg_r )
{
  cerr << "*** " << msg_r << endl;
}

int usage( const std::string & msg_r = std::string(), int exit_r = 100 )
{
  if ( ! msg_r.empty() )
  {
    cerr << endl;
    message( msg_r );
    cerr << endl;
  }
  cerr << "Usage: " << appname << "[OPTIONS] URL..." << endl;
  cerr << "  Load repos from URL to test system below /tmp/" << appname << "." << endl;
  cerr << "  -r ROOT  Use /tmp/ROOT as location of test system (default: " << appname << ")." << endl;
  cerr << "  -a ARCH  Use ARCH as test system architecture (default: x86_64)." << endl;
  cerr << "  -c       Clear an existing test system (default)." << endl;
  cerr << "  -n       Do not clear an existing test system but reuse it." << endl;
  return exit_r;
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;
  appname = Pathname::basename( argv[0] );
  --argc;
  ++argv;

  if ( ! argc )
  {
    return usage();
  }

  ///////////////////////////////////////////////////////////////////
  Pathname mtmp( "/tmp" );
  Pathname mroot( mtmp/appname );
  Arch     march( Arch_x86_64 );
  bool     oClearRoot = true;

  std::vector<std::string> urls;

  while ( argc )
  {
    if ( argv[0] == std::string("-c") )
    {
      oClearRoot = true;
    }
    else if ( argv[0] == std::string("-n") )
    {
      oClearRoot = false;
    }
    else if ( argv[0] == std::string("-r") || argv[0] == std::string("--root"))
    {
      --argc;
      ++argv;
      if ( ! argc )
        return usage( "Missing arg to -r ROOT", 101 );

      if ( *(argv[0]) ) // empty
        mroot = mtmp/argv[0];
      else
        mroot = mtmp/appname;
    }
     else if ( argv[0] == std::string("-a") )
    {
      --argc;
      ++argv;
      if ( ! argc )
        return usage( "Missing arg to -a ARCH", 101 );

      if ( *(argv[0]) ) // empty
        march = Arch( argv[0] );
      else
        march = Arch_x86_64;
    }
   else
    {
      urls.push_back( argv[0] );
    }
    --argc;
    ++argv;
  }

  if ( urls.empty() )
  {
    return usage( "Missing URLs", 102 );
  }

  ///////////////////////////////////////////////////////////////////

  if ( oClearRoot )
  {
    message( "Clear test system at " + mroot.asString() );
    filesystem::recursive_rmdir( mroot );
  }
  else
  {
    message( "Use test system at " + mroot.asString() );
  }
  filesystem::assert_dir( mroot );

  message( "Use archiecture " + march.asString() );
  TestSetup test( mroot, march );

  int ret = 0;
  for_( it, urls.begin(), urls.end() )
  {
    message( "Setup " + *it );
    try
    {
      test.loadRepo( *it );
    }
    catch ( const Exception & exp )
    {
      message( exp.asString() + "\n" + exp.historyAsString() );
      ++ret;
    }
  }

  INT << "===[END]============================================" << endl << endl;
  return ret;
}